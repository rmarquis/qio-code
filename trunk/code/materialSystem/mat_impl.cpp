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
#include "mat_texMods.h"
#include "mat_rgbGen.h"
#include <shared/parser.h>
#include <shared/cullType.h>
#include <api/coreAPI.h>
#include <api/textureAPI.h>
#include <math/matrix.h>
#include <shared/autoCvar.h>
#include <shared/textureWrapMode.h>
#include <shared/ast.h>

static aCvar_c mat_collapseMaterialStages("mat_collapseMaterialStages","1");

// skybox
void skyBox_c::setBaseName(const char *newBaseName) {
	this->baseName = newBaseName;
}
class textureAPI_i *skyBox_c::loadSubTexture(const char *sufix) {
	str fullPath = baseName;
	fullPath.append(sufix);
	return MAT_RegisterTexture(fullPath,TWM_CLAMP_TO_EDGE);
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
	alphaFunc = AF_NONE;
	texMods = 0;
	tcGen = TCG_NONE;
	rgbGen = 0;
	type = ST_NOT_SET;
	depthWrite = true;
	bMarkedForDelete = false;
	subStageBumpMap = 0;
	subStageHeightMap = 0;
	nextBundle = 0;
	condition = 0;
	alphaTestAST = 0;
}
mtrStage_c::~mtrStage_c() {
	if(texMods) {
		delete texMods;
	}
	if(rgbGen) {
		delete rgbGen;
	}
	if(subStageBumpMap) {
		delete subStageBumpMap;
	}
	if(subStageHeightMap) {
		delete subStageHeightMap;
	}
	if(nextBundle) {
		delete nextBundle;
	}
	if(condition) {
		condition->destroyAST();
		condition = 0;
	}
	if(alphaTestAST) {
		alphaTestAST->destroyAST();
		alphaTestAST = 0;
	}
}
void mtrStage_c::setTexture(const char *newMapName) {
	textureAPI_i *tex = MAT_RegisterTexture(newMapName,TWM_REPEAT);
	this->setTexture(tex);
}
int mtrStage_c::getImageWidth() const {
	return stageTexture.getAnyTexture()->getWidth();
}
int mtrStage_c::getImageHeight() const {
	return stageTexture.getAnyTexture()->getHeight();
}
u32 mtrStage_c::getNumImageFrames() const {
	return stageTexture.getNumFrames();
}
void mtrStage_c::addTexMod(const class texMod_c &newTM) {
	if(this->texMods == 0) {
		this->texMods = new texModArray_c;
	}
	this->texMods->push_back(newTM);
}
void mtrStage_c::addD3TexModRotate(class astAPI_i *value) {
	if(this->texMods == 0) {
		this->texMods = new texModArray_c;
	}
	this->texMods->addD3TexModRotate(value);
}
void mtrStage_c::addD3TexModScale(class astAPI_i *val0, class astAPI_i *val1) {
	if(this->texMods == 0) {
		this->texMods = new texModArray_c;
	}
	this->texMods->addD3TexModScale(val0,val1);
}
void mtrStage_c::addD3TexModShear(class astAPI_i *val0, class astAPI_i *val1) {
	if(this->texMods == 0) {
		this->texMods = new texModArray_c;
	}
	this->texMods->addD3TexModShear(val0,val1);
}
void mtrStage_c::addD3TexModScroll(class astAPI_i *val0, class astAPI_i *val1) {
	if(this->texMods == 0) {
		this->texMods = new texModArray_c;
	}
	this->texMods->addD3TexModScroll(val0,val1);
}
void mtrStage_c::addD3TexModCenterScale(class astAPI_i *val0, class astAPI_i *val1) {
	if(this->texMods == 0) {
		this->texMods = new texModArray_c;
	}
	this->texMods->addD3TexModCenterScale(val0,val1);
}
void mtrStage_c::setRGBGenAST(class astAPI_i *ast) {
	if(rgbGen == 0)
		rgbGen = new rgbGen_c;
	rgbGen->setRGBGenAST(ast);
}
void mtrStage_c::applyTexMods(class matrix_c &out, float curTimeSec, const class astInputAPI_i *in) const {
	if(texMods == 0) {
		out.identity();
		return;
	}
	texMods->calcTexMatrix(out,curTimeSec,in);
}
bool mtrStage_c::hasRGBGen() const {
	if(rgbGen == 0)
		return false;
	if(rgbGen->isNone()) {
		return false; // ignore invalid rgbGens
	}
	return true;
}
enum rgbGen_e mtrStage_c::getRGBGenType() const {
	if(rgbGen == 0)
		return RGBGEN_NONE;
	return rgbGen->getType();
}
class rgbGen_c *mtrStage_c::allocRGBGen() {
	if(rgbGen)
		return rgbGen;
	rgbGen = new rgbGen_c;
	return rgbGen;
}
bool mtrStage_c::getRGBGenConstantColor3f(float *out3Floats) const {
	if(rgbGen == 0)
		return true; // error
	if(rgbGen->isConst() == false)
		return true; // error
	const float *in = rgbGen->getConstValues();
	out3Floats[0] = in[0];
	out3Floats[1] = in[1];
	out3Floats[2] = in[2];
	return false;
}
float mtrStage_c::getRGBGenWaveValue(float curTimeSec) const {
	if(rgbGen == 0)
		return 0; // error
	float ret = rgbGen->getWaveForm().evaluate(curTimeSec);
	return ret;
}
void mtrStage_c::evaluateRGBGen(const class astInputAPI_i *in, float *out3Floats) const {
	if(rgbGen == 0)
		return; // errorr
	rgbGen->evaluateRGBGen(in,out3Floats);
}
// return true if stage is conditional (has Doom3 'if' condition)
bool mtrStage_c::hasIFCondition() const {
	if(condition)
		return true;
	return false;
}
// return true if drawing condition is met for given input variables
bool mtrStage_c::conditionMet(const class astInputAPI_i *in) const {
	float result = condition->execute(in);
	if(result == 0.f)
		return false;
	return true;
}
bool mtrStage_c::getColorMaskRed() const {
	return colorMask.getMaskRed();
}
bool mtrStage_c::getColorMaskGreen() const {
	return colorMask.getMaskGreen();
}
bool mtrStage_c::getColorMaskBlue() const {
	return colorMask.getMaskBlue();
}
bool mtrStage_c::getColorMaskAlpha() const {
	return colorMask.getMaskAlpha();
}
// Doom3 glColorMask(...) settings
void mtrStage_c::setMaskRed(bool bMask) {
	colorMask.setMaskRed(bMask);
}
void mtrStage_c::setMaskGreen(bool bMask) {
	colorMask.setMaskGreen(bMask);
}
void mtrStage_c::setMaskBlue(bool bMask) {
	colorMask.setMaskBlue(bMask);
}
void mtrStage_c::setMaskAlpha(bool bMask) {
	colorMask.setMaskAlpha(bMask);
}
void mtrStage_c::setMaskColor(bool bMask) {
	colorMask.setMaskColor(bMask);
}
void mtrStage_c::setAlphaTestAST(class astAPI_i *ast) {
	if(alphaTestAST) {
		alphaTestAST->destroyAST();
	}
	alphaFunc = AF_D3_ALPHATEST;
	alphaTestAST = ast;
}
float mtrStage_c::evaluateAlphaTestValue(const class astInputAPI_i *in) const {
	if(alphaFunc != AF_D3_ALPHATEST) {
		g_core->RedWarning("mtrStage_c::evaluateAlphaTestValue: called on non-alphatest stage\n");
		return 0.f;
	}
	if(alphaTestAST == 0) {
		g_core->RedWarning("mtrStage_c::evaluateAlphaTestValue: alphatest stage had NULL alphaTestAST\n");
		return 0.f;
	}
	return alphaTestAST->execute(in);
}
void mtrStage_c::setRedAST(class astAPI_i *ast) {
	if(rgbGen == 0)
		rgbGen = new rgbGen_c;
	rgbGen->setRedAST(ast);
}
void mtrStage_c::setGreenAST(class astAPI_i *ast) {
	if(rgbGen == 0)
		rgbGen = new rgbGen_c;
	rgbGen->setGreenAST(ast);
}
void mtrStage_c::setBlueAST(class astAPI_i *ast) {
	if(rgbGen == 0)
		rgbGen = new rgbGen_c;
	rgbGen->setBlueAST(ast);
}
// material class
mtrIMPL_c::mtrIMPL_c() {
	skyParms = 0;
	polygonOffset = 0;
	hashNext = 0;
	cullType = CT_FRONT_SIDED;
	bPortalMaterial = false;
	bMirrorMaterial = false;
}
mtrIMPL_c::~mtrIMPL_c() {
	clear();
}
void mtrIMPL_c::clear() {
	// free allocated sub-structures
	for(u32 i = 0; i < stages.size(); i++) {
		delete stages[i];
	}
	stages.clear();
	if(skyParms) {
		delete skyParms;
		skyParms = 0;
	}
	// reset variables to their default values
	polygonOffset = 0;
	cullType = CT_FRONT_SIDED;
	bPortalMaterial = false;
	// but don't clear material name and this->hashNext pointer...
}

void mtrIMPL_c::createFromImage() {
	textureAPI_i *tex = MAT_RegisterTexture(this->name,TWM_REPEAT);
	// save the source file name
	this->sourceFileName = tex->getName();
	// create single material stage
	this->createFromTexturePointer(tex);

	// automatically set alphatest for Call Of Duty images
	// (they are missing a definition is .shader file)
	if(
		// foliage_masked is used in truckride.bsp
		this->name.findToken("foliage_masked@",0,false)
		||
		// metal_fence_nonsolid is used in training.bsp
		this->name.findToken("metal_fence_nonsolid@",0,false)
		||
		// metal_masked is used in training.bsp
		this->name.findToken("metal_masked@",0,false)
		||
		// metal_fence is used in pathfinder.bsp
		this->name.findToken("metal_fence@",0,false)
		) {
		this->stages[0]->setAlphaFunc(AF_GE128);
	}	
	if(	// decal is used in training.bsp
		this->name.findToken("decal@",0,false)) {
		this->polygonOffset = 0.1f;
		this->stages[0]->setBlendDef(BM_SRC_ALPHA,BM_ONE_MINUS_SRC_ALPHA);
	}
}
// Source Engine .vmt support (Valve MaTerials)
bool mtrIMPL_c::loadFromVMTFile() {
	str vmtName = this->name;
	vmtName.defaultExtension("vmt");

	// fix the strange .vmt paths
	if(!Q_stricmpn(vmtName,"maps/",5)) {
		// fix the strange map-dependant file names
		// change this:
		// vmtName = {data=0x05517738 "materials/maps/d1_town_01/tile/tileroof004b_503_632_-2999.vmt" buffer=0x0018e91c "面面面面面面面面=" len=61 ...}
		// into this:
		// "materials/tile/tileroof004b.vmt"
		
		str fixed = "";
		const char *p = vmtName.c_str() + strlen("maps/");
		p = strchr(p,'/');
		p++;
		fixed.append(p);
		fixed.stripExtension();
		const char *e = fixed.c_str() + fixed.length() - 1;
		for(u32 i = 0; i < 3; i++) {
			while(isdigit(*e) || *e == '-') {
				e--;
			}
			if(*e != '_')
				break;
			e--;
		}
		e++;
		vmtName.setFromTo(fixed.c_str(),e);
		vmtName.setExtension("vmt");
	}

	// parse the .vmt file
	parser_c p;
	if(p.openFile(vmtName) == true) {
		str fixed = "materials/";
		fixed.append(vmtName);
		fixed.defaultExtension("vmt");
		if(p.openFile(fixed) == true) {	
			return true; // error.
		}
		vmtName = fixed;
	}
	this->sourceFileName = vmtName;

	int iAlphaTest = -1;
	bool bAlphaTestSetInVMT = false;

	while(p.atEOF() == false) {
		if(p.atWord("$basetexture")) {
			str baseTexture = "materials/";
			baseTexture.append(p.getToken());
			// without it TEX_Load might load .jpg thumbnail file first instead of desired vtf
			baseTexture.setExtension("vtf");
			
			mtrStage_c *newStage = new mtrStage_c;
			newStage->setStageType(ST_COLORMAP_LIGHTMAPPED);
			newStage->setTexture(baseTexture);
			stages.push_back(newStage);
		} else if(p.atWord("$alphatest")) {
			iAlphaTest = p.getInteger();
			bAlphaTestSetInVMT = true;
		} else {
			const char *unkToken = p.getToken();
		}
	}
	if(bAlphaTestSetInVMT && iAlphaTest) {
		for(u32 i = 0; i < stages.size(); i++) {
			stages[i]->setAlphaFunc(AF_GE128);
		}
	}
	return false; // ok
}
void mtrIMPL_c::createFromTexturePointer(class textureAPI_i *tex) {
	mtrStage_c *ns = new mtrStage_c;
	ns->setTexture(tex);
	ns->setStageType(ST_COLORMAP_LIGHTMAPPED);
	stages.push_back(ns);
}
u16 mtrIMPL_c::readBlendEnum(class parser_c &p) {
	str token = p.getD3Token();
#define ADDOPTION(label, value) if(!stricmp(token,label)) return value;
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
	g_core->Print(S_COLOR_RED"Unknown blendFunc src/dst %s in file %s at line %i, setting to BM_ONE \n",token.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
	return BM_ZERO;
}
void mtrIMPL_c::setSkyParms(const char *farBox, const char *cloudHeightStr, const char *nearBox) {
	float cloudHeight = atof(cloudHeightStr);
	skyParms = new skyParms_c(farBox,cloudHeight,nearBox);
	skyParms->uploadTextures();
}
mtrStage_c *mtrIMPL_c::getFirstStageOfType(enum stageType_e type) {
	for(u32 i = 0; i < stages.size(); i++) {
		if(stages[i]->getStageType() == type) {
			return stages[i];
		}
	}
	return 0;
}
const mtrStage_c *mtrIMPL_c::getFirstStageOfType(enum stageType_e type) const {
	for(u32 i = 0; i < stages.size(); i++) {
		if(stages[i]->getStageType() == type) {
			return stages[i];
		}
	}
	return 0;
}
void mtrIMPL_c::replaceStageType(enum stageType_e stageTypeToFind, enum stageType_e replaceWith) {
	for(u32 i = 0; i < stages.size(); i++) {
		if(stages[i]->getStageType() == stageTypeToFind) {
			stages[i]->setStageType(replaceWith);
		}
	}
}
void mtrIMPL_c::removeAllStagesOfType(enum stageType_e type) {
	for(int i = 0; i < stages.size(); i++) {
		if(stages[i]->getStageType() == type) {
			delete stages[i];
			stages.erase(i);
			i--;
		}
	}
}

class astAPI_i *MAT_ParseExpression(parser_c &p) {
	str s = p.getLine(",");
	astAPI_i *ret = AST_ParseExpression(s);
	if(p.atWord_dontNeedWS(",")) {
		// skip it
	}
	return ret;
}
bool mtrIMPL_c::loadFromText(const matTextDef_s &txt) {
	parser_c p;
	p.setup(txt.textBase,txt.p);
	p.setDebugFileName(txt.sourceFile);
	
	if(p.atChar('{') == false) {
		int line = p.getCurrentLineNumber();
		str tok = p.getToken();
		g_core->RedWarning("mtrIMPL_c::loadFromText: expected '{' to follow material %s at line %i of %s, found %s\n",
			getName(),line,txt.sourceFile,tok.c_str());
		return true; // error
	}
	// save the source file name
	this->sourceFileName = txt.sourceFile;
	mtrStage_c *stage = 0;
	int level = 1;
	bool depthWriteSetInMaterial = false;
	// incremented every "nextbundle" keyword
	// not present in Q3
	// used in MoHAA and CoD
	u32 bundleCount = 0;
	while(level) {
		if(p.atEOF()) {
			g_core->RedWarning("mtrIMPL_c::loadFromText: unexpected end of file hit while parsing material %s in file %s\n",
				getName(),txt.sourceFile);
			break;
		}
		if(p.atChar('{')) {
			level++;
			if(level == 2) {
				stage = new mtrStage_c;
				stages.push_back(stage);
				depthWriteSetInMaterial = false;
			}
			bundleCount = 0;
		} else if(p.atChar('}')) {
			if(level == 2) {
				if(stage) {
					//if(stage->getTexture(0) == 0) {
					//	delete stage;
					//} else {
					//}
					stage = 0;
				}
			}
			bundleCount = 0;
			level--;
		} else {
			if(level == 1) {
				if(p.atWord("cull")) {
					// "cull disable" is used eg. in Quake3 rocket fire model
					if(p.atWord("none") || p.atWord("disable")) {
						cullType = CT_TWO_SIDED;
					} else if(p.atWord("twosided")) {
						cullType = CT_TWO_SIDED;
					} else {
						u32 lineNum = p.getCurrentLineNumber();
						str tok = p.getToken();
						g_core->Print("Unkownn culltype \"%s\" at line %i of %s\n",tok.c_str(),lineNum,p.getDebugFileName());
					}
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
					//if((farBox.length() == 0 || farBox[0] == '-') && (nearBox.length() == 0 || nearBox[0] == '-')) {
					//	// ignore "empty" skyparms (without sky cube names)
					//	// NOTE: there is an "empty" skypams in 20kdm2.pk3 shaders
					//} else {
						setSkyParms(farBox,cloudHeight,nearBox);
				//	}
				} else if(p.atWord("diffusemap") || p.atWord("colormap")) {
					// "diffusemap" keyword is a shortcut for a material stage with single image
					// it was introduced in Doom3
					mtrStage_c *newDiffuseMapStage = new mtrStage_c;
					newDiffuseMapStage->setTexture(MAT_ParseImageScript(p));
					//newDiffuseMapStage->setStageType(ST_COLORMAP);
					newDiffuseMapStage->setStageType(ST_COLORMAP_LIGHTMAPPED);
					stages.push_back(newDiffuseMapStage);
				} else if(p.atWord("bumpmap") || p.atWord("normalmap") ) {
					mtrStage_c *newBumpMapStage = new mtrStage_c;
					newBumpMapStage->setStageType(ST_BUMPMAP);
					newBumpMapStage->setTexture(MAT_ParseImageScript(p));
					stages.push_back(newBumpMapStage);
				} else if(p.atWord("specularmap")) {
					p.skipLine();	
				} else if(p.atWord("heightmap")) {
					mtrStage_c *newHeightMapStage = new mtrStage_c;
					newHeightMapStage->setStageType(ST_HEIGHTMAP);
					newHeightMapStage->setTexture(MAT_ParseImageScript(p));
					stages.push_back(newHeightMapStage);
				} else if(p.atWord("lightFalloffImage")) {
					p.skipLine();			
				} else if(p.atWord("deform")) {
					p.skipLine();			
				} else if(p.atWord("unsmoothedtangents")) {

				} else if(p.atWord("polygonOffset")) {
					if(p.getNextWordInLine()) {
						this->polygonOffset = atof(p.getLastStoredToken());
					} else {
						// FIXME? textures/decals/xlabs from RTCW decals.shader
						// has polygonoffset without value specified
						this->polygonOffset = 0.1f;
					}
				} else if(p.atWord("twosided")) {
					this->cullType = CT_TWO_SIDED;
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
				} else if(p.atWord("portal")) {
					// QuakeIII portal keyword.
					// See q3 textures/sfx/portal_sfx from scripts/common.shader.
					// It's the shader of q3dm0 portal (teleporter)
					this->bPortalMaterial = true;
				} else if(p.atWord("decal_macro")) {
					// Doom3 decal_macro

					// set polygon offset
					this->polygonOffset = 0.1f;

					// turn of shadow casting - TODO

					// clear CONTENTS_SOLID - TODO

					// set decal sort - TODO

				} else if(p.atWord("mirror")) {
					// Doom3 mirror
					// used eg. in Prey's game/roadhouse.proc
					this->bMirrorMaterial = true;
				} else if(p.atWord("qer_editorimage")) {
					this->editorImage = p.getToken();
				} else {
					p.getToken();
				}
			} else if(level == 2) {
				// parse stage
				if(p.atWord("map")) {
					if(p.atWord("$lightmap")) {
						// quick fix for MoHAA "nextbundle" keyword
						// .. just do not ever overwrite colormaps with $lightmap
						if(stage->getStageTexture().isEmpty()) {
							stage->setStageType(ST_LIGHTMAP);
						} else {
							// just let the stage know it's lightmap-compatible
							stage->setStageType(ST_COLORMAP_LIGHTMAPPED);
						}
					} else {
						if(p.atWord("clamp")) {
							// extra CoD "clamp" keyword
							// used eg. in truckride.shader
							// map clamp textures/austria/background/foliage_clamp@truck_single.tga

						}
						stage->getStageTexture().parseMap(p);
						stage->getStageTexture().uploadTexture();
					}		
				} else if(p.atWord("clampmap")) {
					stage->getStageTexture().setBClamp(true);
					stage->getStageTexture().parseMap(p);
					stage->getStageTexture().uploadTexture();
				} else if(p.atWord("animmap")) {
					stage->getStageTexture().parseAnimMap(p);
					stage->getStageTexture().uploadTexture();
				} else if(p.atWord("nextbundle")) {
					mtrStage_c *nextBundle = new mtrStage_c;
					stage->setNextBundle(nextBundle);
					stage = nextBundle;
					bundleCount++;
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
					// disable writing to depth buffer for translucent surfaces
					// (unless otherwise specified)
					if(depthWriteSetInMaterial == false) {
						stage->setDepthWrite(false);
					}
				} else if(p.atWord("depthWrite")) {
					depthWriteSetInMaterial = true;
					stage->setDepthWrite(true);
				} else if(p.atWord("rgbGen")) {
					if(stage->hasRGBGen()) {
						g_core->RedWarning("mtrIMPL_c::loadFromText: WARNING: rgbGen defined twice at line %i of file %s in material def %s\n",
							p.getCurrentLineNumber(),p.getDebugFileName(),this->getName());
					}
					if(stage->allocRGBGen()->parse(p)) {
						//stage->freeRGBGen();
					}
				} else if(p.atWord("tcmod")) {
					// texture coordinates modifier
					texMod_c tm;
					if(tm.parse(p)) {
						//p.skipLine();
					} else {
						stage->addTexMod(tm);
					}
				// Id Tech 4 keywords
				} else if(p.atWord("blend")) {
					if(p.atWord("add")) {
						// needed by Xreal machinegun model
						stage->setBlendDef(BM_ONE,BM_ONE);
					} else if(p.atWord("blend")) {
						stage->setBlendDef(BM_SRC_ALPHA,BM_ONE_MINUS_SRC_ALPHA);
					} else if(p.atWord("bumpmap")) {
						stage->setStageType(ST_BUMPMAP);
					} else if(p.atWord("specularMap")) {
						stage->setStageType(ST_SPECULARMAP);
					} else if(p.atWord("diffusemap")) {
						// ST_COLORMAP is a default stage type, but set it anyway
						stage->setStageType(ST_COLORMAP);
					} else if(p.atWord("filter")) {
						stage->setBlendDef(BM_DST_COLOR,BM_ZERO);
					} else if(p.atWord("shader")) {
						// seen in Prey textures/dreamworld/la_floor, etc (dreamworld.mtr)
						// for stages with custom shader? (.vfp files)

					} else if(p.atWord("none")) {

					} else if(p.atWord("map")) {

					} else {
						// NOTE: for some reasons blend types are separated
						// with ',' in Doom3
						u16 src = readBlendEnum(p);
						u16 dst = readBlendEnum(p);
						stage->setBlendDef(src,dst);
					}			
					// disable writing to depth buffer for translucent surfaces
					// (unless otherwise specified)
					if(depthWriteSetInMaterial == false) {
						stage->setDepthWrite(false);
					}
				} else if(p.atWord("alphatest")) {
					// example: "alphaTest ( time + parm4 ) * 0.5 - 0.2"
#if 0
					// TODO: handle Doom3 math expressions for alphaTest?
					float val = p.getFloat();
					if(val >= 0.5) {
						stage->setAlphaFunc(AF_GE128);
					}
#else
					astAPI_i *ast = MAT_ParseExpression(p);
					if(ast) {
						stage->setAlphaTestAST(ast);
					} else {
						g_core->RedWarning("Failed to parse 'alphaTest' AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}		
#endif
				} else if(p.atWord("red")) {
					// example: "red 0.68"
					astAPI_i *ast = MAT_ParseExpression(p);
					if(ast) {
						stage->setRedAST(ast);
					} else {
						g_core->RedWarning("Failed to parse 'red' AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
				} else if(p.atWord("green")) {
					astAPI_i *ast = MAT_ParseExpression(p);
					if(ast) {
						stage->setGreenAST(ast);
					} else {
						g_core->RedWarning("Failed to parse 'green' AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
				} else if(p.atWord("blue")) {
					astAPI_i *ast = MAT_ParseExpression(p);
					if(ast) {
						stage->setBlueAST(ast);
					} else {
						g_core->RedWarning("Failed to parse 'blue' AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
				} else if(p.atWord("alpha")) {
					p.skipLine();
				} else if(p.atWord("zeroClamp")) {
					// no arguments
				} else if(p.atWord("colored")) {
					// no arguments
				} else if(p.atWord("rgb")) {
					// usage: "rgb <expression>"
					// example: "rgb		sosTable[ time * 0.25 ]"
					astAPI_i *ast = MAT_ParseExpression(p);
					if(ast) {
						stage->setRGBGenAST(ast);
					} else {
						g_core->RedWarning("Failed to parse 'rgb' AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
				} else if(p.atWord("nomips")) {

				} else if(p.atWord("highquality")) {
					
				} else if(p.atWord("cubemap")) {
					p.skipLine();
				} else if(p.atWord("texgen") || p.atWord("tcgen")) {
					if(p.atWord("environment")) {
						stage->setTCGen(TCG_ENVIRONMENT);
					} else if(p.atWord("skybox")) {
						// used eg. in Prey's (Id Tech 4) dave.mtr -> textures/skybox/dchtest
						//stage->setTCGen(TCG_SKYBOX);
					} else if(p.atWord("reflect")) {
						//stage->setTCGen(TCG_REFLECT);
					} else if(p.atWord("screen")) {
						// Prey's keyword?
						//stage->setTCGen(TCG_SCREEN);
					} else {
						str tok = p.getToken();
						g_core->RedWarning("mtrIMPL_c::loadFromText: unknown texgen type %s in definition of material %s in file %s at line %i\n",
							tok.c_str(),this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
				} else if(p.atWord("if")) {
					// NOTE: this is the condition for entire material stage
					// example: "if ( parm5 == 0 )"
					// example: "if ( ( time * 4 ) % 3 == 0 )"
#if 0
					// TODO: evaluate Doom3 conditional expressions?
					stage->setMarkedForDelete(true);
#else
					astAPI_i *ast = MAT_ParseExpression(p);
					if(ast) {
						stage->setStageCondition(ast);
					} else {
						g_core->RedWarning("Failed to parse stage condition ('if') AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
#endif
				} else if(p.atWord("glowStage")) {

				} else if(p.atWord("vertexcolor")) {

				} else if(p.atWord("inversevertexcolor")) {

				} else if(p.atWord("rotate")) {
					// example: "rotate time / -700"
					// example:	"rotate .65 + bfg_flare1_rotate[time*.5]" 
					astAPI_i *ast = MAT_ParseExpression(p);
					if(ast) {
						stage->addD3TexModRotate(ast);
					} else {
						g_core->RedWarning("Failed to parse 'rotate' AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
				} else if(p.atWord("scroll") || p.atWord("translate")) {
					// NOTE: "scroll" and "translate" are handled in one 'if' block in Doom3 as well
					// example: "scroll time / 1000, time / 1000"
					astAPI_i *scrollVal0 = MAT_ParseExpression(p);
					if(scrollVal0 == 0) {
						g_core->RedWarning("Failed to parse 'scroll' first value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					astAPI_i *scrollVal1 = MAT_ParseExpression(p);
					if(scrollVal1 == 0) {
						g_core->RedWarning("Failed to parse 'scroll' second value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					if(scrollVal0 && scrollVal1) {
						stage->addD3TexModScroll(scrollVal0,scrollVal1);
					}
				} else if(p.atWord("scale")) {
					// example: "scale	0.5 - parm4, 0.5 - parm4"
					astAPI_i *scaleVal0 = MAT_ParseExpression(p);
					if(scaleVal0 == 0) {
						g_core->RedWarning("Failed to parse 'scale' first value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					astAPI_i *scaleVal1 = MAT_ParseExpression(p);
					if(scaleVal1 == 0) {
						g_core->RedWarning("Failed to parse 'scale' second value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					if(scaleVal0 && scaleVal1) {
						stage->addD3TexModScale(scaleVal0,scaleVal1);
					}
				} else if(p.atWord("shear")) {
					// example: "shear	scaleTable[time * 0.1] , 0"
					astAPI_i *shearVal0 = MAT_ParseExpression(p);
					if(shearVal0 == 0) {
						g_core->RedWarning("Failed to parse 'shear' first value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					astAPI_i *shearVal1 = MAT_ParseExpression(p);
					if(shearVal1 == 0) {
						g_core->RedWarning("Failed to parse 'shear' second value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					if(shearVal0 && shearVal1) {
						stage->addD3TexModShear(shearVal0,shearVal1);
					}
				} else if(p.atWord("centerScale")) {
					// example: "centerScale	0.5 - parm4, 0.5 - parm4"
					astAPI_i *centerScaleVal0 = MAT_ParseExpression(p);
					if(centerScaleVal0 == 0) {
						g_core->RedWarning("Failed to parse 'centerScale' first value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					astAPI_i *centerScaleVal1 = MAT_ParseExpression(p);
					if(centerScaleVal1 == 0) {
						g_core->RedWarning("Failed to parse 'centerScale' second value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					if(centerScaleVal0 && centerScaleVal1) {
						stage->addD3TexModCenterScale(centerScaleVal0,centerScaleVal1);
					}
				} else if(p.atWord("maskRed")) {
					stage->setMaskRed(true);
				} else if(p.atWord("maskGreen")) {
					stage->setMaskGreen(true);
				} else if(p.atWord("maskBlue")) {
					stage->setMaskBlue(true);
				} else if(p.atWord("maskAlpha")) {
					stage->setMaskAlpha(true);
				} else if(p.atWord("maskColor")) {
					// maskRGB
					stage->setMaskColor(true);
				} else {
					p.getToken();
				}
			} else {
				p.getToken();
				g_core->RedWarning("mtrIMPL_c::loadFromText: invalid level %i\n",level);
			}
		}
	}


	if(stages.size() == 0) {
		g_core->RedWarning("mtrIMPL_c::loadFromText: %s has 0 stages\n",this->getName());
		//this->createFromImage();
		if(editorImage.length()) {
			mtrStage_c *s = new mtrStage_c;
			s->setStageType(ST_COLORMAP);
			s->setTexture(editorImage);
			stages.push_back(s);
		}
	} else {
		// delete unwanted stages
		for(int i = 0; i < stages.size(); i++) {
			mtrStage_c *s = stages[i];
			if(s->isMarkedForDelete()) {
				delete s;
				stages.erase(i);
				i--;
			}
		}
		// link bumpmaps to their colormaps
		mtrStage_c *colorMapStage = this->getFirstStageOfType(ST_COLORMAP);
		if(colorMapStage==0) {
			colorMapStage = this->getFirstStageOfType(ST_COLORMAP_LIGHTMAPPED);
		}
		if(colorMapStage) {
			mtrStage_c *bumpMapStage = this->getFirstStageOfType(ST_BUMPMAP);
			if(bumpMapStage) {
				colorMapStage->setSubStageBumpMap(bumpMapStage);
				this->stages.removeObject(bumpMapStage);
			}
			mtrStage_c *heightMapStage = this->getFirstStageOfType(ST_HEIGHTMAP);
			if(heightMapStage) {
				colorMapStage->setSubStageHeightMap(heightMapStage);
				this->stages.removeObject(heightMapStage);
			}

		//	mtrStage_c *specularMapStage = this->getFirstStageOfType(ST_SPECULARMAP);
		}
		this->removeAllStagesOfType(ST_BUMPMAP);
		this->removeAllStagesOfType(ST_SPECULARMAP);
		this->removeAllStagesOfType(ST_HEIGHTMAP);

		if(mat_collapseMaterialStages.getInt()) {
			mtrStage_c *lightmapped = this->getFirstStageOfType(ST_LIGHTMAP);
			if(lightmapped) {
				// if we have a non-lightmap stage without blendfunc, we can collapse...
				for(int i = 0; i < stages.size(); i++) {
					mtrStage_c *s = stages[i];
					if(s->isLightmapStage() == false && s->hasBlendFunc() == false) {
						this->removeAllStagesOfType(ST_LIGHTMAP);
						this->replaceStageType(ST_NOT_SET,ST_COLORMAP_LIGHTMAPPED);
						this->replaceStageType(ST_COLORMAP,ST_COLORMAP_LIGHTMAPPED);
						break;
					}
				}
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