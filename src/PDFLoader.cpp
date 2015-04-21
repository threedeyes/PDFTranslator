/*
 * Copyright 2015, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 *
 * This file is part of PDFTranslator
 *
 * PDFTranslator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "PDFLoader.h"
#include "BaseTranslator.h"

struct istream_filter {
    BPositionIO *stream;
    unsigned char buf[4096];
};

static int next_istream(fz_context *ctx, fz_stream *stm, int max)
{
    istream_filter *state = (istream_filter *)stm->state;
    int cbRead = sizeof(state->buf);        
    
    state->stream->Seek(stm->pos, SEEK_SET);
    cbRead = state->stream->Read(state->buf, sizeof(state->buf));
    
    stm->rp = state->buf;
    stm->wp = stm->rp + cbRead;
    stm->pos += cbRead;

    return cbRead > 0 ? *stm->rp++ : EOF;
}

static void seek_istream(fz_context *ctx, fz_stream *stm, int offset, int whence)
{
    istream_filter *state = (istream_filter *)stm->state;
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
	fDPI = 72;
	fAntialiasingBits = 8;

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
        
    stream = fz_new_stream(ctx, state, next_istream, close_istream);
    
    fz_register_document_handlers(ctx);
    
    stream->seek = seek_istream;
    //stream->meta = stream_meta;
        
	if (signatureData == kPDFMagic) {
		fDocumentType = PDF_IMAGE_FORMAT;
    	doc = fz_open_document_with_stream(ctx, "mupdf.pdf", stream);
	} else {
    	return;    
	}

	fPageCount = fz_count_pages(ctx, doc);
	
	fLoaded = true;	
}


PDFLoader::~PDFLoader()
{
	if(doc!=NULL)
		fz_drop_document(ctx, doc);
	if(stream!=NULL)
		fz_drop_stream(ctx, stream);
	if(ctx!=NULL)
		fz_drop_context(ctx);
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


void
PDFLoader::SetDPI(uint32 dpi)
{
	fDPI = dpi;
}


void
PDFLoader::SetAntialiasingBits(uint32 bits)
{
	fAntialiasingBits = bits;
}


int
PDFLoader::GetImage(BPositionIO *target, int index)
{
	if (index < 1 || index > fPageCount + 1 || !fLoaded)
		return B_NO_TRANSLATOR;

	int rotation = 0;

	fz_page *page = fz_load_page(ctx, doc, index - 1);
	
	fz_matrix transform;
	fz_rotate(&transform, rotation);
	fz_pre_scale(&transform, fDPI / 72.0f, fDPI / 72.0f);
	
	fz_set_aa_level(ctx, fAntialiasingBits);

	fz_rect bounds;
	fz_bound_page(ctx, page, &bounds);
	fz_transform_rect(&bounds, &transform);

	fz_irect bbox;
	fz_round_rect(&bbox, &bounds);
	fz_pixmap *pix = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), &bbox);
	fz_clear_pixmap_with_value(ctx, pix, 0xff);

	fz_device *dev = fz_new_draw_device(ctx, pix);
	if (fAntialiasingBits == 0)
		fz_enable_device_hints(ctx, dev, FZ_DONT_INTERPOLATE_IMAGES);
	fz_run_page(ctx, page, dev, &transform, NULL);
	fz_drop_device(ctx, dev);

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
	fz_drop_page(ctx, page);

	return B_OK;
}
