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
#include <api/textureAPI.h>

// material stage class
mtrStage_c::mtrStage_c() {
	texture = 0;
	alphaFunc = AF_NONE;
}
void mtrStage_c::setTexture(const char *newMapName) {
	textureAPI_i *tex = MAT_RegisterTexture(newMapName);
	this->setTexture(tex);
}
int mtrStage_c::getImageWidth() const {
	return texture->getWidth();
}
int mtrStage_c::getImageHeight() const {
	return texture->getHeight();
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
u16 mtrIMPL_c::readBlendEnum(class parser_c &p) {
#define ADDOPTION(label, value) if(p.atWord(label)) return value;
	ADDOPTION("GL_ONE",BM_ONE)
	ADDOPTION("GL_ZERO",BM_ZERO) 
	ADDOPTION("GL_DST_COLOR",BM_DST_COLOR)
	ADDOPTION("GL_SRC_COLOR",BM_SRC_COLOR)
	ADDOPTION("GL_ONE_MINUS_DST_COLOR",BM_ONE_MINUS_DST_COLOR)
	ADDOPTION("GL_ONE_MINUS_SRC_COLOR",BM_ONE_MINUS_SRC_COLOR)
	ADDOPTION("GL_ONE_MINUS_SRC_ALPHA",BM_ONE_MINUS_SRC_ALPHA)
	ADDOPTION("GL_ONE_MINUS_DST_ALPHA",BM_ONE_MINUS_DST_ALPHA)
	ADDOPTION("GL_DST_ALPHA",BM_DST_ALPHA)
	ADDOPTION("GL_SRC_ALPHA",BM_SRC_ALPHA)
	ADDOPTION("GL_SRC_ALPHA_SATURATE",BM_SRC_ALPHA_SATURATE)
#undef ADDOPTION
	g_core->Print(S_COLOR_RED,"Unknown blendFunc src/dst %s in file %s, setting to BM_ONE \n",p.getToken(),p.getDebugFileName(),p.getCurrentLineNumber());
	return BM_ZERO;
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
					if(!stricmp(mapName,"$lightmap")) {

					} else {
						stage->setTexture(mapName);
					}
				} else if(p.atWord("alphaFunc")) {
					if(p.atWord("GT0")) {
						stage->setAlphaFunc(AF_GT0);
					} else if(p.atWord("LT128")) {				
						stage->setAlphaFunc(AF_LT128);				
					} else if(p.atWord("GE128")) {
						stage->setAlphaFunc(AF_GE128);
					} else {
						
					}
				} else if(p.atWord("blendFunc")) {
					if(p.atWord("add")) {
						stage->setBlendDef(BM_ONE,BM_ONE);
					} else if(p.atWord("filter")) {
						stage->setBlendDef(BM_DST_COLOR,BM_ZERO);
					} else if(p.atWord("blend")) {
						stage->setBlendDef(BM_SRC_ALPHA,BM_ONE_MINUS_SRC_ALPHA);
					} else if(p.atWord("alphaadd")) {
						stage->setBlendDef(BM_SRC_ALPHA,BM_ONE);
					} else {
						u16 src = readBlendEnum(p);
						u16 dst = readBlendEnum(p);
						stage->setBlendDef(src,dst);
					}
				} else if(p.atWord("depthWrite")) {

				} else if(p.atWord("rgbGen")) {

				} else {
					p.getToken();
				}
			} else {
				p.getToken();
				g_core->Print("mtrIMPL_c::loadFromText: invalid level %i\n",level);
			}
		}
	}


	if(stages.size() == 0) {
		g_core->RedWarning("mtrIMPL_c::loadFromText: %s has 0 stages\n",this->getName());
		this->createFromImage();
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