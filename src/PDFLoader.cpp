/*
 * Copyright 2012, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 *
 * libpsd is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "PDFLoader.h"
#include "BaseTranslator.h"

struct istream_filter {
    BPositionIO *stream;
    unsigned char buf[4096];
};

static int next_istream(fz_stream *stm, int max)
{
	printf("next_istream\n");
    istream_filter *state = (istream_filter *)stm->state;
    int cbRead = sizeof(state->buf);        
    
    state->stream->Seek(stm->pos, SEEK_SET);
    cbRead = state->stream->Read(state->buf, sizeof(state->buf));
    
    stm->rp = state->buf;
    stm->wp = stm->rp + cbRead;
    stm->pos += cbRead;
    
    printf("next_istream: cbRead=%d stm->pos=%d\n", cbRead, stm->pos);

    return cbRead > 0 ? *stm->rp++ : EOF;
}

static void seek_istream(fz_stream *stm, int offset, int whence)
{
    istream_filter *state = (istream_filter *)stm->state;
    printf("seek_istream offset=%d\n",offset);
    state->stream->Seek(offset, whence);
  	  	
    stm->pos = state->stream->Position();
    stm->rp = stm->wp = state->buf;
}

static void close_istream(fz_context *ctx, void *state_)
{
    istream_filter *state = (istream_filter *)state_;
    state->stream->Seek(0, SEEK_END);
    fz_free(ctx, state);
}
/*
static fz_stream *
stream_rebind(fz_stream *s)
{
	return s;
}
*/
static int
stream_meta(fz_stream *stream, int key, int size, void *ptr)
{
    BPositionIO *source = (BPositionIO*)(stream->state);

    switch(key)
    {
	    case FZ_STREAM_META_LENGTH:
	    {
			size_t pos = source->Position();
			source->Seek(0, SEEK_END);
			size_t iosize = source->Position();
			source->Seek(pos, SEEK_SET);
			printf("FZ_STREAM_META_LENGTH: %ld\n", iosize);
			return iosize;
	    }
	    case FZ_STREAM_META_PROGRESSIVE:
	        return 1;
	    }
    return -1;
}


PDFLoader::PDFLoader(BPositionIO *source)
{
	fLoaded = false;
	fDocumentType = 0;	
	fPageCount = 0;
	
	doc = NULL;
	ctx = NULL;
	stream = NULL;

	source->Seek(0, SEEK_SET);
	
	uint32 signatureData;
	ssize_t signatureSize = 4;
	if (source->Read(&signatureData, signatureSize) != signatureSize)
		return;
			
	source->Seek(0, SEEK_SET);
	
	const uint32 kPDFMagic = B_HOST_TO_BENDIAN_INT32('%PDF');
	
	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	
	istream_filter *state = fz_malloc_struct(ctx, istream_filter);
    state->stream = source;
        
    stream = fz_new_stream(ctx, state, next_istream, close_istream, NULL);
    
    fz_register_document_handlers(ctx);
    
    stream->seek = seek_istream;
    //stream->meta = stream_meta;
        
	if (signatureData == kPDFMagic) {
		fDocumentType = PDF_IMAGE_FORMAT;
    	doc = fz_open_document_with_stream(ctx, "magic.pdf", stream);
	} else {
    	return;    
	}

	fPageCount = fz_count_pages(doc);
	
	fLoaded = true;	
}


PDFLoader::~PDFLoader()
{
	if(doc!=NULL)
		fz_close_document(doc);
	if(stream!=NULL)
		fz_close(stream);	
	if(ctx!=NULL)
		fz_free_context(ctx);
}


bool
PDFLoader::IsLoaded(void)
{
	return fLoaded;
}

uint32
PDFLoader::DocumentType(void)
{
	return fDocumentType;
}	
	
int
PDFLoader::PageCount(void)
{
	return fPageCount;
}


int
PDFLoader::GetImage(BPositionIO *target, int index)
{
	if (index < 1 || index > fPageCount + 1 || !fLoaded)
		return B_NO_TRANSLATOR;

	int zoom = 100;
	int rotation = 0;

	fz_page *page = fz_load_page(doc, index - 1);
	
	fz_matrix transform;
	fz_rotate(&transform, rotation);
	fz_pre_scale(&transform, zoom / 100.0f, zoom / 100.0f);

	fz_rect bounds;
	fz_bound_page(doc, page, &bounds);
	fz_transform_rect(&bounds, &transform);

	fz_irect bbox;
	fz_round_rect(&bbox, &bounds);
	fz_pixmap *pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), &bbox);
	fz_clear_pixmap_with_value(ctx, pix, 0xff);

	fz_device *dev = fz_new_draw_device(ctx, pix);
	fz_run_page(doc, page, dev, &transform, NULL);
	fz_free_device(dev);	

	TranslatorBitmap bitsHeader;
	bitsHeader.magic = B_TRANSLATOR_BITMAP;
	bitsHeader.bounds.left = 0;
	bitsHeader.bounds.top = 0;
	bitsHeader.bounds.right = pix->w - 1;
	bitsHeader.bounds.bottom = pix->h - 1;
	bitsHeader.rowBytes = sizeof(uint32) * pix->w;
	bitsHeader.colors = B_RGB32;
	bitsHeader.dataSize = bitsHeader.rowBytes * pix->h;
	if (swap_data(B_UINT32_TYPE, &bitsHeader,
		sizeof(TranslatorBitmap), B_SWAP_HOST_TO_BENDIAN) != B_OK) {		
		return B_NO_TRANSLATOR;
	}
	target->Write(&bitsHeader, sizeof(TranslatorBitmap));

	int pixels = pix->w * pix->h;

	uint8 *ptr = pix->samples;
	for (int i=0; i<pixels; i++, ptr+=4) {
		uint8 rgba[4];
		rgba[2] = ptr[0];
		rgba[1] = ptr[1];
		rgba[0] = ptr[2];
		target->Write(rgba, 4);
	}

	fz_drop_pixmap(ctx, pix);
	fz_free_page(doc, page);	

	return B_OK;
}
