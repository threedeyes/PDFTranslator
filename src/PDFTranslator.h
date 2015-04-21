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


#ifndef PDF_TRANSLATOR_H
#define PDF_TRANSLATOR_H


#include "BaseTranslator.h"

#include <Translator.h>
#include <TranslatorFormats.h>
#include <TranslationDefs.h>
#include <GraphicsDefs.h>
#include <InterfaceDefs.h>
#include <DataIO.h>
#include <File.h>
#include <ByteOrder.h>
#include <String.h>

#define DOCUMENT_COUNT "/documentCount"
#define DOCUMENT_INDEX "/documentIndex"

#define PDF_SETTING_DPI				"pdf /dpi"
#define PDF_SETTING_ANTIALIASING	"pdf /antialiasing"

#define PDF_TRANSLATOR_VERSION B_TRANSLATION_MAKE_VERSION(1, 0, 2)
#define PDF_IMAGE_FORMAT	'PDF '

#define PDF_IN_QUALITY		0.5
#define PDF_IN_CAPABILITY	0.5

#define BITS_OUT_QUALITY	1
#define BITS_OUT_CAPABILITY	0.9

class PDFTranslator : public BaseTranslator {
	public:
		PDFTranslator();

		virtual status_t DerivedIdentify(BPositionIO *inSource,
			const translation_format *inFormat, BMessage *ioExtension,
			translator_info *outInfo, uint32 outType);

		virtual status_t DerivedTranslate(BPositionIO *inSource,
			const translator_info *inInfo, BMessage *ioExtension,
			uint32 outType, BPositionIO *outDestination, int32 baseType);

		virtual status_t DerivedCanHandleImageSize(float width,
			float height) const;

		virtual BView *NewConfigView(TranslatorSettings *settings);	

	protected:
		virtual ~PDFTranslator();
};

extern const char *kDocumentCount;
extern const char *kDocumentIndex;

#endif	/* PDF_TRANSLATOR_H */
