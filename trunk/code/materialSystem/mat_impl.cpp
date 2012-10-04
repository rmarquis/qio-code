/*
============================================================================
Copyright (C) 2012 V.

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
// mat_impl.cpp - material class implementation Source
#include "mat_impl.h"
#include "mat_local.h"
#include <shared/parser.h>
#include <api/coreAPI.h>

// material stage class
mtrStage_c::mtrStage_c() {
	texture = 0;
	alphaFunc = AF_NONE;
}
void mtrStage_c::setTexture(const char *newMapName) {
	textureAPI_i *tex = MAT_RegisterTexture(newMapName);
	this->setTexture(tex);
}
// material class
mtrIMPL_c::mtrIMPL_c() {

}
mtrIMPL_c::~mtrIMPL_c() {
	for(u32 i = 0; i < stages.size(); i++) {
		delete stages[i];
	}
}

void mtrIMPL_c::createFromImage() {
	textureAPI_i *tex = MAT_RegisterTexture(this->name);
	mtrStage_c *ns = new mtrStage_c;
	ns->setTexture(tex);
	stages.push_back(ns);
}

bool mtrIMPL_c::loadFromText(const matTextDef_s &txt) {
	parser_c p;
	p.setup(txt.textBase,txt.p);
	p.setDebugFileName(txt.sourceFile);
	
	if(p.atChar('{') == false) {

		return true; // error
	}
	mtrStage_c *stage = 0;
	int level = 1;
	while(level) {
		if(p.atEOF()) {
			g_core->Print(S_COLOR_RED"mtrIMPL_c::loadFromText: unexpected end of file hit while parsing material %s in file %s\n",
				getName(),txt.sourceFile);
			break;
		}
		if(p.atChar('{')) {
			level++;
			if(level == 2) {
				stage = new mtrStage_c;
			}
		} else if(p.atChar('}')) {
			if(level == 2) {
				if(stage) {
					if(stage->getTexture() == 0) {
						delete stage;
					} else {
						stages.push_back(stage);
					}
					stage = 0;
				}
			}
			level--;
		} else {
			if(level == 1) {
				if(p.atWord("cull")) {

				} else if(p.atWord("surfaceparm")) {
					p.getToken();
				} else {
					p.getToken();
				}
			} else if(level == 2) {
				if(p.atWord("map")) {
					const char *mapName = p.getToken();
					stage->setTexture(mapName);
				} else if(p.atWord("alphaFunc")) {
					if(p.atWord("GT0")) {
						stage->setAlphaFunc(AF_GT0);
					} else if(p.atWord("LT128")) {				
						stage->setAlphaFunc(AF_LT128);				
					} else if(p.atWord("GE128")) {
						stage->setAlphaFunc(AF_GE128);
					} else {
						
					}
				} else if(p.atWord("depthWrite")) {

				} else if(p.atWord("rgbGen")) {

				} else {
					p.getToken();
				}
			} else {
				p.getToken();
				g_core->Print("mtrIMPL_c::loadFromText: invalid level %s\n");
			}
		}
	}


	//const char *p = txt.p;
	//if(*p != '{') {
	//	return true;
	//}
	//p++;
	//int level = 1;
	//str token;
	//while(level) {
	//	if(*p == '}') {
	//		level--;
	//		p++;
	//	} else if(*p == '{') {
	//		level++;
	//		p++;
	//	} else {

	//	}
	//	p = G_SkipToNextToken(p);
	//}
	return false; // ok
}