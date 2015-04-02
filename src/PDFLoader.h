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


#ifndef PDF_LOADER_H
#define PDF_LOADER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Translator.h>
#include <TranslatorFormats.h>
#include <TranslationDefs.h>
#include <GraphicsDefs.h>
#include <InterfaceDefs.h>
#include <DataIO.h>
#include <File.h>
#include <ByteOrder.h>
#include <List.h>

#include "PDFTranslator.h"

extern "C" {
#include <mupdf/fitz.h>
};

class PDFLoader {
public:
					PDFLoader(BPositionIO *source);
					~PDFLoader();
		
	int				PageCount(void);
	int 			GetImage(BPositionIO *target, int index);
	bool			IsLoaded(void);
	uint32			DocumentType(void);
private:
	fz_context 		*ctx;
	fz_document 	*doc;
	fz_stream		*stream;
	
	uint32			fDocumentType;
	int				fPageCount;
	bool			fLoaded;	
};


#endif	/* PDF_LOADER_H */
