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

#ifndef CONFIG_VIEW_H
#define CONFIG_VIEW_H

#include "TranslatorSettings.h"

#include <View.h>
#include <TextView.h>
#include <String.h>
#include <GroupView.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>
#include <SpaceLayoutItem.h>
#include <ControlLook.h>

#define MSG_DPI_CHANGED 'dpic'
#define MSG_ANTIALIASING_CHANGED 'aach'

class ConfigView : public BGroupView {
	public:
		ConfigView(TranslatorSettings *settings);
		virtual ~ConfigView();
		
		virtual void AllAttached();
		virtual void MessageReceived(BMessage* message);

	private:
		void _AddItemToMenu(BMenu* menu, const char* label,
			uint32 mess, uint32 value, uint32 current_value);
	
		BTextView*			fCopyrightView;
		BMenuField*			fDPIField;
		BMenuField*			fAntialiasingField;
		
		TranslatorSettings *fSettings;
};

#endif	// CONFIG_VIEW_H
