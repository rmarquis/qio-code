/*
============================================================================
Copyright (C) 2016 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// charFile.cpp - Enemy Territory .char file class
#include "charFile.h"
#include <shared/parser.h>

bool charFile_c::parseCharFile(const char *fname) {
	parser_c p;
	if(p.openFile(fname)) {
		return true;
	}
	if(p.atWord("characterDef") == false) {
		g_core->RedWarning("charFile_c::parseCharFile: expected 'characterDef' at the beginning of %s, found %s\n",fname,p.getToken());
		return true;
	}
	if(p.atWord("{") == false) {
		g_core->RedWarning("charFile_c::parseCharFile: expected '{' at the beginning of %s, found %s\n",fname,p.getToken());
		return true;
	}
	while(p.atWord("}") == false) {
		if(p.atWord("skin")) {
			skin = p.getToken();
		} else if(p.atWord("mesh")) {
			mesh = p.getToken();
		} else if(p.atWord("animationGroup")) {
			animationGroup = p.getToken();
		} else if(p.atWord("animationScript")) {
			animationScript = p.getToken();
		} else if(p.atWord("undressedCorpseModel")) {
			undressedCorpseModel = p.getToken();
		} else if(p.atWord("undressedCorpseSkin")) {
			undressedCorpseSkin = p.getToken();
		} else if(p.atWord("hudHead")) {
			hudHead = p.getToken();
		} else if(p.atWord("hudHeadAnims")) {
			hudHeadAnims = p.getToken();
		} else if(p.atWord("hudHeadSkin")) {
			hudHeadSkin = p.getToken();
		} else {
			g_core->RedWarning("Uknown .char token %s in %s\n",p.getToken(),fname);
		}
	}
	return false;
}


