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

// skybox
void skyBox_c::setBaseName(const char *newBaseName) {
	this->baseName = newBaseName;
}
class textureAPI_i *skyBox_c::loadSubTexture(const char *sufix) {
	str fullPath = baseName;
	fullPath.append(sufix);
	return MAT_RegisterTexture(fullPath);
}
void skyBox_c::uploadTextures() {
	if(baseName.length() == 0 || baseName[0] == '-')
		return;
	//freeTextures();
	up = loadSubTexture("_up");
	down = loadSubTexture("_dn");
	right = loadSubTexture("_rt");
	left = loadSubTexture("_lf");
	front = loadSubTexture("_ft");
	back = loadSubTexture("_bk");
}
//
//void skyBox_c::freeTextures() {
//	if(up) {
//		MAT_FreeTexture(&up);
//		up = 0;
//	}
//	if(down) {
//		MAT_FreeTexture(&down);
//		down = 0;
//	}
//	if(right) {
//		MAT_FreeTexture(&right);
//		right = 0;
//	}
//	if(left) {
//		MAT_FreeTexture(&left);
//		left = 0;
//	}
//	if(front) {
//		MAT_FreeTexture(&front);
//		front = 0;
//	}
//	if(back) {
//		MAT_FreeTexture(&back);
//		back = 0;
//	}
//}
skyBox_c::skyBox_c() {
	up = 0;
	down = 0;
	right = 0;
	left = 0;
	front = 0;
	back = 0;
}
//skyBox_c::~skyBox_c() {
//	clear();
//}
//void skyBox_c::clear() {
//	freeTextures();
//}

// skyparms
skyParms_c::skyParms_c(const char *farBoxName, float newCloudHeight, const char *nearBoxName) {
	nearBox.setBaseName(farBoxName);
	farBox.setBaseName(nearBoxName);
	this->cloudHeight = newCloudHeight;
}

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
	skyParms = 0;
	polygonOffset = 0;
}
mtrIMPL_c::~mtrIMPL_c() {
	for(u32 i = 0; i < stages.size(); i++) {
		delete stages[i];
	}
	if(skyParms) {
		delete skyParms;
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
void mtrIMPL_c::setSkyParms(const char *farBox, const char *cloudHeightStr, const char *nearBox) {
	float cloudHeight = atof(cloudHeightStr);
	skyParms = new skyParms_c(farBox,cloudHeight,nearBox);
	skyParms->uploadTextures();
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
				} else if(p.atWord("skyparms")) {
					// skyParms <farbox> <cloudheight> <nearbox>
					str farBox, cloudHeight, nearBox;
					farBox = p.getToken();
					if(p.isAtEOL() == false) {
						cloudHeight = p.getToken();
						if(p.isAtEOL() == false) {
							nearBox = p.getToken();
						}
					}
					printf("skyparms: %s %s %s\n",farBox.c_str(),cloudHeight.c_str(),nearBox.c_str());
					if((farBox.length() == 0 || farBox[0] == '-') && (nearBox.length() == 0 || nearBox[0] == '-')) {
						// ignore "empty" skyparms (without sky cube names)
						// NOTE: there is an "empty" skypams in 20kdm2.pk3 shaders
					} else {
						setSkyParms(farBox,cloudHeight,nearBox);
					}
				} else if(p.atWord("diffusemap")) {
					// "diffusemap" keyword is a shortcut for a material stage with single image
					// it was introduced in Doom3
					mtrStage_c *newDiffuseMapStage = new mtrStage_c;
					newDiffuseMapStage->setTexture(p.getToken());
					stages.push_back(newDiffuseMapStage);
				} else if(p.atWord("bumpmap")) {
					p.skipLine();	
				} else if(p.atWord("specularmap")) {
					p.skipLine();	
				} else if(p.atWord("lightFalloffImage")) {
					p.skipLine();			
				} else if(p.atWord("deform")) {
					p.skipLine();			
				} else if(p.atWord("unsmoothedtangents")) {

				} else if(p.atWord("polygonOffset")) {
					this->polygonOffset = p.getFloat();
				} else if(p.atWord("twosided")) {

				} else if(p.atWord("nonsolid")) {

				} else if(p.atWord("noimpact")) {
					
				} else if(p.atWord("noselfshadow")) {

				} else if(p.atWord("noshadows")) {

				} else if(p.atWord("translucent")) {
				} else if(p.atWord("spectrum")) {
					p.skipLine();
				} else if(p.atWord("renderbump")) {
					p.skipLine();
				} else if(p.atWord("materialType")) {
					p.skipLine(); // "glass", etc
				} else {
					p.getToken();
				}
			} else if(level == 2) {
				// parse stage
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

				// Id Tech 4 keywords
				} else if(p.atWord("blend")) {
					if(p.atWord("add")) {
						// needed by Xreal machinegun model
						stage->setBlendDef(BM_ONE,BM_ONE);
					} else if(p.atWord("blend")) {

					} else if(p.atWord("bumpmap")) {

					} else if(p.atWord("specularMap")) {

					} else if(p.atWord("diffusemap")) {

					} else {
						str src, dst;
						src = p.getToken();
						if(p.atWord(",")) {

						}
						dst = p.getToken();
						src.removeCharacter(',');
						dst.removeCharacter(',');
					}
				} else if(p.atWord("alphatest")) {
				} else if(p.atWord("red")) {
					p.skipLine();
				} else if(p.atWord("green")) {
					p.skipLine();
				} else if(p.atWord("blue")) {
					p.skipLine();
				} else if(p.atWord("alpha")) {
					p.skipLine();
				} else if(p.atWord("zeroClamp")) {
					// no arguments
				} else if(p.atWord("colored")) {
					// no arguments
				} else if(p.atWord("rgb")) {
					p.skipLine();
				} else if(p.atWord("nomips")) {

				} else if(p.atWord("highquality")) {
					
				} else if(p.atWord("cubemap")) {
					p.skipLine();
				} else if(p.atWord("texgen")) {
					p.skipLine();
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