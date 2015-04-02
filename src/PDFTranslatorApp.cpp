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


#include <Application.h>

#include "PDFTranslator.h"
#include "TranslatorWindow.h"

int
main(int argc, char *argv[])
{
	BApplication application("application/x-vnd.Haiku-PDFTranslator");

	status_t result;
	result = LaunchTranslatorWindow(new PDFTranslator, "PDFTranslator Settings",
		BRect(0, 0, 320, 200));
	if (result != B_OK)
		return 1;

	application.Run();

	return 0;
}

