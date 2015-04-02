/*
 * Copyright 2012, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 *
 * libPDF is the legal property of its developers, whose names are too numerous
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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ConfigView.h"
#include "PDFTranslator.h"
#include "PDFLoader.h"

const char *kDocumentCount = "/documentCount";
const char *kDocumentIndex = "/documentIndex";

#define kPDFMimeType "application/pdf"
#define kPDFName "PDF Document"

static const translation_format sInputFormats[] = {
	{
		PDF_IMAGE_FORMAT,
		B_TRANSLATOR_BITMAP,
		PDF_IN_QUALITY,
		PDF_IN_CAPABILITY,
		kPDFMimeType,
		kPDFName
	},
};

static const translation_format sOutputFormats[] = {
	{
		B_TRANSLATOR_BITMAP,
		B_TRANSLATOR_BITMAP,
		BITS_OUT_QUALITY,
		BITS_OUT_CAPABILITY,
		"image/x-be-bitmap",
		"Be Bitmap Format (PDFTranslator)"
	},
};


static const TranSetting sDefaultSettings[] = {
	{B_TRANSLATOR_EXT_HEADER_ONLY, TRAN_SETTING_BOOL, false},
	{B_TRANSLATOR_EXT_DATA_ONLY, TRAN_SETTING_BOOL, false}
};

const uint32 kNumInputFormats = sizeof(sInputFormats)
	/ sizeof(translation_format);
const uint32 kNumOutputFormats = sizeof(sOutputFormats)
	/ sizeof(translation_format);
const uint32 kNumDefaultSettings = sizeof(sDefaultSettings)
	/ sizeof(TranSetting);


PDFTranslator::PDFTranslator()
	: BaseTranslator("PDF Document",
		"PDF Document translator",
		PDF_TRANSLATOR_VERSION,
		sInputFormats, kNumInputFormats,
		sOutputFormats, kNumOutputFormats,
		"PDFTranslator",
		sDefaultSettings, kNumDefaultSettings,
		B_TRANSLATOR_BITMAP, PDF_IMAGE_FORMAT)
{
}


PDFTranslator::~PDFTranslator()
{
}


status_t
PDFTranslator::DerivedIdentify(BPositionIO *stream,
	const translation_format *format, BMessage *ioExtension,
	translator_info *info, uint32 outType)
{	
	if (outType != B_TRANSLATOR_BITMAP)
		return B_NO_TRANSLATOR;
		
	uint32 signatureData;
	ssize_t signatureSize = 4;
	if (stream->Read(&signatureData, signatureSize) != signatureSize)
		return B_IO_ERROR;
			
	const uint32 kPDFMagic = B_HOST_TO_BENDIAN_INT32('%PDF');
	
	if (signatureData != kPDFMagic)
		return B_ILLEGAL_DATA;
	
		
	PDFLoader PDFFile(stream);
		
	if(!PDFFile.IsLoaded() || PDFFile.PageCount() <= 0 || PDFFile.DocumentType() < 0)
		return B_ILLEGAL_DATA;

	int32 documentCount = PDFFile.PageCount();
	int32 documentIndex = 1;
	
	if (ioExtension) {
		if (ioExtension->FindInt32(DOCUMENT_INDEX, &documentIndex) != B_OK)
			documentIndex = 1;
		if (documentIndex < 1 || documentIndex > documentCount)
			return B_NO_TRANSLATOR;
	}	
	
	if (ioExtension) {
		ioExtension->RemoveName(DOCUMENT_COUNT);
		ioExtension->AddInt32(DOCUMENT_COUNT, documentCount);
	}	
	
	if (PDFFile.DocumentType() == PDF_IMAGE_FORMAT) {
		info->type = PDF_IMAGE_FORMAT;
		info->group = B_TRANSLATOR_BITMAP;
		info->quality = PDF_IN_QUALITY;
		info->capability = PDF_IN_CAPABILITY;
		BString docName("PDF Page");
		docName << " " << documentIndex << "/" << documentCount;
		
		snprintf(info->name, sizeof(info->name), docName.String());
		strcpy(info->MIME, kPDFMimeType);
	}
	
	return B_OK;
}


status_t
PDFTranslator::DerivedTranslate(BPositionIO *source,
	const translator_info *info, BMessage *ioExtension,
	uint32 outType, BPositionIO *target, int32 baseType)
{
	if (outType != B_TRANSLATOR_BITMAP)
		return B_NO_TRANSLATOR;

	switch (baseType) {
		case 0:
		{								
			if (outType != B_TRANSLATOR_BITMAP)
				return B_NO_TRANSLATOR;

			PDFLoader PDFFile(source);
			if (!PDFFile.IsLoaded() || PDFFile.PageCount() <= 0)
				return B_NO_TRANSLATOR;

			int32 documentIndex = 1;

			if (ioExtension) {
				if (ioExtension->FindInt32(DOCUMENT_INDEX,
					&documentIndex) != B_OK)
					documentIndex = 1;
			}
			
			return PDFFile.GetImage(target, documentIndex);			
		}

		default:
			return B_NO_TRANSLATOR;
	}
}


status_t
PDFTranslator::DerivedCanHandleImageSize(float width, float height) const
{
	return B_OK;
}


BView *
PDFTranslator::NewConfigView(TranslatorSettings *settings)
{
	return new ConfigView(settings);
}


BTranslator *
make_nth_translator(int32 n, image_id you, uint32 flags, ...)
{
	if (n != 0)
		return NULL;

	return new PDFTranslator();
}

