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
	subStageSpecularMap = 0;
	subStageHeightMap = 0;
	subStageBumpHeightMap = 0;
	blendChannelBlue = 0;
	blendChannelRed = 0;
	blendChannelGreen = 0;
	nextBundle = 0;
	condition = 0;
	alphaTestAST = 0;
	cubeMap = 0;
	alphaGen = ALPHAGEN_NOT_SET;
}
mtrStage_c::mtrStage_c(class textureAPI_i *tex) {
	alphaFunc = AF_NONE;
	texMods = 0;
	tcGen = TCG_NONE;
	rgbGen = 0;
	type = ST_NOT_SET;
	depthWrite = true;
	bMarkedForDelete = false;
	subStageBumpMap = 0;
	subStageSpecularMap = 0;
	subStageHeightMap = 0;
	subStageBumpHeightMap = 0;
	nextBundle = 0;
	condition = 0;
	alphaTestAST = 0;
	cubeMap = 0;
	alphaGen = ALPHAGEN_NOT_SET;
	blendChannelBlue = 0;
	blendChannelRed = 0;
	blendChannelGreen = 0;

	this->stageTexture.fromTexturePointer(tex);
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
	if(subStageSpecularMap) {
		delete subStageSpecularMap;
	}
	if(subStageHeightMap) {
		delete subStageHeightMap;
	}
	if(subStageBumpHeightMap) {
		delete subStageBumpHeightMap;
	}
	if(blendChannelBlue) {
		delete blendChannelBlue;
	}
	if(blendChannelRed) {
		delete blendChannelRed;
	}
	if(blendChannelGreen) {
		delete blendChannelGreen;
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
enum alphaGen_e mtrStage_c::getAlphaGenType() const {
	return alphaGen;
}
enum rgbGen_e mtrStage_c::getRGBGenType() const {
	if(rgbGen == 0)
		return RGBGEN_NONE;
	return rgbGen->getType();
}
void mtrStage_c::setRGBGenVerex() {
	allocRGBGen()->setVertex();
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
void mtrStage_c::setAlphaAST(class astAPI_i *ast) {
#if 0
	if(alphaGen == 0)
		alphaGen = new alphaGen_c;
	alphaGen->setAlphaAST(ast);
#else
	delete ast;
#endif
}
void mtrStage_c::setAlphaGenVertex() {
	alphaGen = ALPHAGEN_VERTEX;
}
void mtrStage_c::setRGBAGenVertex() {
	allocRGBGen();
	rgbGen->setVertex();
}
// material class
mtrIMPL_c::mtrIMPL_c() {
	skyParms = 0;
	sunParms = 0;
	polygonOffset = 0;
	hashNext = 0;
	cullType = CT_FRONT_SIDED;
	bPortalMaterial = false;
	bMirrorMaterial = false;
	deforms = 0;
	bGenericSky = false;
	bHasEditorTransparency = false;
	editorTransparency = 1.f;;
	bMarkedAsUsed = true;
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
	if(sunParms) {
		delete sunParms;
		sunParms = 0;
	}
	if(deforms) {
		delete deforms;
		deforms = 0;
	}
	// reset variables to their default values
	polygonOffset = 0;
	cullType = CT_FRONT_SIDED;
	bPortalMaterial = false;
	bGenericSky = false;
	bHasEditorTransparency = false;
	editorTransparency = 1.f;;
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
	// automatically try to find bump/height maps
	{
		str bumpTextureName = this->name;
		bumpTextureName.stripExtension();
		bumpTextureName.append("_n");
		textureAPI_i *bump = MAT_RegisterTexture(bumpTextureName,TWM_REPEAT);
		if(bump && bump != MAT_GetDefaultTexture()) {
			this->stages[0]->setSubStageBumpMap(new mtrStage_c(bump));
		}
	}
	{
		str heightTextureName = this->name;
		heightTextureName.stripExtension();
		heightTextureName.append("_h");
		textureAPI_i *height = MAT_RegisterTexture(heightTextureName,TWM_REPEAT);
		if(height && height != MAT_GetDefaultTexture()) {
			this->stages[0]->setSubStageHeightMap(new mtrStage_c(height));
		}
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
		// NOTE: those are the cubemap references (it's the cubemap origin)
		// They are used for reflection effects. "env_cubemap"
		
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
		} else if(p.atWord("$translucent")) {
			iAlphaTest = p.getInteger();
			bAlphaTestSetInVMT = true;
		} else if(p.atWord("%compilesky")) {
			// this is used in tools/toolsskybox.vmt from hl2 gamedirectory
			int iCompileSky = p.getInteger();
			if(iCompileSky) {
				this->bGenericSky = true;
			} else {
				this->bGenericSky = false;
			}
		} else {
			const char *unkToken = p.getToken();
		}
	}
	if(bAlphaTestSetInVMT && iAlphaTest) {
		for(u32 i = 0; i < stages.size(); i++) {
			stages[i]->setAlphaFunc(AF_GE128);
		}
	}
	if(stages.size() == 0) {
		createFromImage();
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
#define ADDOPTION(label, value) if(!_stricmp(token,label)) return value;
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
	g_core->RedWarning("Unknown blendFunc src/dst %s in file %s at line %i, setting to BM_ONE \n",token.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
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
	// get the expression - stop at line break, ',' or at '}'
	// (quote from Prey's dreamworld.mtr: "		scale	1 + parm5, 1 + parm5	}")
	str s = p.getLine(",}");
	// convert expression text to AST
	astAPI_i *ret = AST_ParseExpression(s);
	if(p.atWord_dontNeedWS(",")) {
		// skip it
	}
	return ret;
}
void mtrIMPL_c::addDeformSprite() {
	if(deforms == 0) {
		deforms = new deformArray_c;
	}
	deforms->addDeformSprite();
}
bool mtrIMPL_c::loadFromText(const matTextDef_s &txt) {
	parser_c p;
	p.setup(txt.textBase,txt.p);
	p.setDebugFileName(txt.sourceFile);
	str qerAlphaFuncType;
	float qerAlphaFuncRefValue;
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
					//if(stage->getStageType() == ST_NOT_SET) {
					//	stage->setStageType(ST_COLORMAP);
					//}
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
				} else if(p.atWord("diffusemap") || p.atWord("colormap") || p.atWord("diffuseMapLightmapped")) {
					mtrStage_c *newDiffuseMapStage = new mtrStage_c;
					if(p.atWord("-")) { // ET's way
						newDiffuseMapStage->setTexture(this->getName());
					} else {
						newDiffuseMapStage->setTexture(MAT_ParseImageScript(p));
					}
					//newDiffuseMapStage->setStageType(ST_COLORMAP);
					newDiffuseMapStage->setStageType(ST_COLORMAP_LIGHTMAPPED);
					stages.push_back(newDiffuseMapStage);
				} else if(p.atWord("implicitMap")) {
					// "implicitMap" is used only in Wolfeinstein: Enemy Territory
					// ET's way
					mtrStage_c *newDiffuseMapStage = new mtrStage_c;
					if(p.atWord("-")) {
						newDiffuseMapStage->setTexture(this->getName());
					} else {
						newDiffuseMapStage->setTexture(MAT_ParseImageScript(p));
					}
					// V: it seems that implicitMap materials should be drawn with vertex colors.
					// See bezier patches on oasis with stucco07a material (from egypt_wals_sd)
					/*
					textures/egypt_walls_sd/stucco07a
					{
						q3map_nonplanar
						q3map_shadeangle 90
						implicitMap -
					}
					*/
					//newDiffuseMapStage->setStageType(ST_COLORMAP);
					newDiffuseMapStage->setStageType(ST_COLORMAP_LIGHTMAPPED);
					stages.push_back(newDiffuseMapStage);
				} else if(p.atWord("bumpmap") || p.atWord("normalmap")) {
					mtrStage_c *newBumpMapStage = new mtrStage_c;
					newBumpMapStage->setStageType(ST_BUMPMAP);
					newBumpMapStage->setTexture(MAT_ParseImageScript(p));
					stages.push_back(newBumpMapStage);
				} else if(p.atWord("bumpHeightMap")) {
					mtrStage_c *newBumpMapStage = new mtrStage_c;
					newBumpMapStage->setStageType(ST_BUMPHEIGHTMAP);
					newBumpMapStage->setTexture(MAT_ParseImageScript(p));
					stages.push_back(newBumpMapStage);
				} else if(p.atWord("specularmap")) {
					//p.skipLine();	
					mtrStage_c *newSpecularMapStage = new mtrStage_c;
					newSpecularMapStage->setStageType(ST_SPECULARMAP);
					newSpecularMapStage->setTexture(MAT_ParseImageScript(p));
					stages.push_back(newSpecularMapStage);
				} else if(p.atWord("heightmap")) {
					mtrStage_c *newHeightMapStage = new mtrStage_c;
					newHeightMapStage->setStageType(ST_HEIGHTMAP);
					newHeightMapStage->setTexture(MAT_ParseImageScript(p));
					stages.push_back(newHeightMapStage);

				} else if(p.atWord("implicitMask")) {
					// Enemy Territory - implicitMask for fast alpha-tested materials
					// for models/mapobjects/tree_desert_sd/palm_leaf1
					// from ET models_mapobject.shader
					mtrStage_c *newDiffuseMapStage = new mtrStage_c;
					if(p.atWord("-")) {
						newDiffuseMapStage->setTexture(this->getName());
					} else {
						newDiffuseMapStage->setTexture(MAT_ParseImageScript(p));
					}
					//newDiffuseMapStage->setStageType(ST_COLORMAP);
					newDiffuseMapStage->setStageType(ST_COLORMAP_LIGHTMAPPED);
					newDiffuseMapStage->setAlphaFunc(AF_GE128); // implicitMask for Enemy Territory
					stages.push_back(newDiffuseMapStage);
				} else if(p.atWord("lightFalloffImage")) {
					p.skipLine();	
				} else if(p.atWord("deformVertexes")) {
					// Quake3 vertex deform
					p.skipLine();
				} else if(p.atWord("deform")) {
					// Doom3/Quake4 vertex deform
					if(p.atWord("sprite")) {
						// deform sprite is used for Plasma/BFG projectiles in Doom3.
						// See material "models/weapons/plasmagun/plasmashot2" from senetmp.mtr
						this->addDeformSprite();
					} else {
						p.skipLine();
					}
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
					// draw this material with an alpha blend
					// used eg. for decals

					// TODO
				} else if(p.atWord("spectrum")) {
					// usage: spectrum <integer>
					// used by Doom3 to match materials with lights
					// light<->surface interaction is allowed only if light->getSpectrum()==surf->getSpectrum()
					p.getInteger();
					// TODO
				} else if(p.atWord("renderbump")) {
					p.skipLine();
				} else if(p.atWord("materialType")) {
					p.skipLine(); // "glass", etc
				} else if(p.atWord("portal")) {
					// QuakeIII portal keyword.
					// See q3 textures/sfx/portal_sfx from scripts/common.shader.
					// It's the shader of q3dm0 portal (teleporter)
					// It's also used for mirrors ("textures/common/mirror1","textures/common/mirror2")
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
					// NOTE: this keyword is *not* used in Quake3 mirrors.
					// They have only "portal" keyword used.
					this->bMirrorMaterial = true;
				} else if(p.atWord("qer_editorimage")) {
					this->editorImage = p.getToken();
				} else if(p.atWord("qer_keyword")) {
					// Material global keywords
					// it is used in some script what are present
					// inside testing maps
					// EXAMPLE : baseqio/materials/knightBK_glass.mtr
					//           line 19, 36 and 59
					// 
					this->keyword = p.getToken();
				} else if(p.atWord("qer_trans")) {
					// This is used by our built-in editor
					// 0.5 means 50% transparency
					editorTransparency = p.getFloat();
					bHasEditorTransparency = true;
				} else if(p.atWord("qer_nocarve")) {

				} else if(p.atWord("nopicmip")) {

				} else if(p.atWord("nomipmaps")) {

				} else if(p.atWord("q3map_surfacelight")) {
					p.getFloat();
				} else if(p.atWord("sort")) {
					p.getToken();
				} else if(p.atWord("guisurf")) {
					p.getToken();
				} else if(p.atWord("clamp")) {
					// NOTE: clamp is also used as per-stage keyword
				} else if(p.atWord("zeroclamp")) {

				} else if(p.atWord("alphazeroclamp")) {

				} else if(p.atWord("discrete")) {

				} else if(p.atWord("decalInfo")) {
					p.skipLine();
				} else if(p.atWord("description")) {
					p.getToken();
				} else if(p.atWord("playerclip")) {

				} else if(p.atWord("glass")) {

				} else if(p.atWord("forceOverlays")) {

				} else if(p.atWord("blood")) {

				} else if(p.atWord("monsterclip")) {

				} else if(p.atWord("noFragment")) {

				} else if(p.atWord("forceOpaque")) {

				} else if(p.atWord("stone")) {

				} else if(p.atWord("ricochet")) {

				} else if(p.atWord("metal")) {

				} else if(p.atWord("flesh")) {

				} else if(p.atWord("forceshadows")) {

				} else if(p.atWord("cardboard")) {

				} else if(p.atWord("plastic")) {

				} else if(p.atWord("nooverlays")) {

				} else if(p.atWord("wood")) {

				} else if(p.atWord("xmap_sun") || p.atWord("q3map_sun")) {
					// Xreal sun params.
					// In Xreal they are automatically applied to tr. globals when they are parsed.
					// We're using a separate class to store them.
					if(sunParms) {
						u32 line = p.getCurrentLineNumber();
						g_core->RedWarning("mtrIMPL_c::loadFromText: sunParms already defined - check line %i of %s in material %s\n",line,txt.sourceFile,this->getName());
					} else {
						sunParms = new sunParms_c;
					}
					vec3_c color;
					if(p.getFloatMat(color,3)) {
						u32 line = p.getCurrentLineNumber();
						g_core->RedWarning("mtrIMPL_c::loadFromText: failed to read color vec3 of sunParms - check line %i of %s in material %s\n",line,txt.sourceFile,this->getName());			
					}
					vec3_c angles;
					if(p.getFloatMat(angles,3)) {
						u32 line = p.getCurrentLineNumber();
						g_core->RedWarning("mtrIMPL_c::loadFromText: failed to read color vec3 of sunParms - check line %i of %s in material %s\n",line,txt.sourceFile,this->getName());			
					}
					sunParms->setFromColorAndAngles(color,angles);
				} else if(p.atWord("collision")) {
					// Id Tech 4 token
				} else if(p.atWord("matter_metal")) {
					// Prey keyword?
				} else if(p.atWord("matter_flesh")) {
					// Prey keyword?
				} else if(p.atWord("matter_glass")) {
					// Prey keyword?
				} else if(p.atWord("matter_pipe")) {
					// Prey keyword?
				} else if(p.atWord("matter_stone")) {
					// Prey keyword?
				} else if(p.atWord("matter_wood")) {
					// Prey keyword?
				} else if(p.atWord("matter_altmetal")) {
					// Prey keyword?
				} else if(p.atWord("matter_cardboard")) {
					// Prey keyword?
				} else if(p.atWord("wallwalk")) {
					// Prey keyword?
				} else if(p.atWord("nosteps")) {
				} else if(p.atWord("skipClip")) {
				} else if(p.atWord("lightWholeMesh")) {
					// Prey keyword?
				} else if(p.atWord("decal_alphatest_macro")) {
					// Prey keyword?
				} else if(p.atWord("forcefield")) {
					// Prey keyword?
				} else if(p.atWord("skybox_macro")) {
					// Prey keyword?
				} else if(p.atWord("skyboxportal")) {
					// Prey keyword?
				} else if(p.atWord("glass_macro")) {
					// Prey keyword?
				} else if(p.atWord("qer_alphaFunc")) {
					// V: found in cqbtest materials
					// qer_alphaFunc greater 0.5
					//p.skipLine();
					qerAlphaFuncType = p.getToken();
					qerAlphaFuncRefValue = p.getFloat();
				} else if(p.atWord("q3map_bounceScale")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_shadeAngle")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("tessSize")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_lightmapMergable")) {
					// V: found in cqbtest materials
				} else if(p.atWord("q3map_lightmapSize")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_tcGen")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_shadeAngle")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_nonplanar")) {
					// V: found in cqbtest materials
				} else if(p.atWord("q3map_clipModel")) {
					// V: found in cqbtest materials
				} else if(p.atWord("implicitMask")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_sunExt")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_baseShader")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_foliage")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_noFog")) {
					// V: found in cqbtest materials
				} else if(p.atWord("q3map_skyLight")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("distanceCull")) {
					// V: found in cqbtest materials
					p.skipLine();
				} else if(p.atWord("q3map_forceMeta")) {
					// V: found in cqbtest materials
				} else if(p.atWord("q3map_globalTexture")) {
					// V: found in cqbtest materials
				} else if(p.atWord("nocompress")) {
					// V: found in cqbtest materials
				} else {
					u32 line = p.getCurrentLineNumber();
					str token  = p.getToken();
					g_core->RedWarning("mtrIMPL_c::loadFromText: unknown material token '%s' at line %i of %s in material %s\n",
						token.c_str(),line,txt.sourceFile,this->getName());
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
#if 0
						stage->getStageTexture().parseMap(p);
						stage->getStageTexture().uploadTexture();
#else
						class textureAPI_i *tex = MAT_ParseImageScript(p);
						stage->setTexture(tex);
#endif
					}	
				} else if(p.atWord("lightmap")) {
					// V: this is used in ET, eg. on fueldump
					// Allow any arbitraty image as lighmap?
					// TODO
					stage->getStageTexture().parseMap(p);
					stage->getStageTexture().uploadTexture();
					stage->setStageType(ST_LIGHTMAP);
				} else if(p.atWord("clampmap")) {
					stage->getStageTexture().setBClamp(true);
					stage->getStageTexture().parseMap(p);
					stage->getStageTexture().uploadTexture();
				} else if(p.atWord("animmapdir")) {
					// V: added for Qio
					stage->getStageTexture().parseAnimMapDir(p);
					stage->getStageTexture().uploadTexture();
				} else if(p.atWord("animmap")) {
					stage->getStageTexture().parseAnimMap(p);
					stage->getStageTexture().uploadTexture();
				} else if(p.atWord("nextbundle")) {
					mtrStage_c *nextBundle = new mtrStage_c;
					stage->setNextBundle(nextBundle);
					stage = nextBundle;
					bundleCount++;
				
				} else if(p.atWord("blendChannelRed")) {
					// for Qio
					mtrStage_c *subStage = new mtrStage_c();
					subStage->getStageTexture().parseMap(p);
					subStage->getStageTexture().uploadTexture();
					stage->setBlendChannelRed(subStage);
				} else if(p.atWord("blendChannelBlue")) {
					// for Qio
					mtrStage_c *subStage = new mtrStage_c();
					subStage->getStageTexture().parseMap(p);
					subStage->getStageTexture().uploadTexture();
					stage->setBlendChannelBlue(subStage);
				} else if(p.atWord("blendChannelGreen")) {
					// for Qio
					mtrStage_c *subStage = new mtrStage_c();
					subStage->getStageTexture().parseMap(p);
					subStage->getStageTexture().uploadTexture();
					stage->setBlendChannelGreen(subStage);
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
					// blendfunc GL_ONE GL_ZERO disables blending (see Q3 source)
					// without this check Quake3 ammo boxes (plasma, rockets, etc) will be drawn transparent
					if(stage->getBlendSrc() == BM_ONE && stage->getBlendDst() == BM_ZERO) {
						stage->disableBlending();
					} else {
						// disable writing to depth buffer for translucent surfaces
						// (unless otherwise specified)
						if(depthWriteSetInMaterial == false) {
							stage->setDepthWrite(false);
						}
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
				} else if(p.atWord("stage")) {
					if(p.atWord("diffuseMap") || p.atWord("colorMap")) {
						stage->setStageType(ST_COLORMAP);
					} else if(p.atWord("normalMap")) {
						// It's used in Xreal railgun
						stage->setStageType(ST_BUMPMAP);
					} else if(p.atWord("specularMap")) {
						stage->setStageType(ST_SPECULARMAP);
					} else if(p.atWord("skyboxMap")) {
						// it's used in material textures/mawi/mawi_sky from test_tree.pk3
						// It's a modern replacement for old Q3 "skyparms" keyword
						stage->setStageType(ST_CUBEMAP_SKYBOX);
					} else if(p.atWord("reflectionMap")) {
						// usefull for Doom3-style glass reflections
						stage->setStageType(ST_CUBEMAP_REFLECTION);
					} else if(p.atWord("blendMap")) {
						// Qio...
						stage->setStageType(ST_BLENDMAP);
					} else {
						u32 line = p.getCurrentLineNumber();
						str token = p.getToken();
						g_core->RedWarning("Unknown stage type %s in material %s in file %s at line %i\n",
							token.c_str(),this->name.c_str(),p.getDebugFileName(),line);
					}
				// Id Tech 4 keywords
				} else if(p.atWord("blend")) {
					bool bDontDisableDepthWrite = false;
					if(p.atWord("add")) {
						// needed by Xreal machinegun model
						stage->setBlendDef(BM_ONE,BM_ONE);
					} else if(p.atWord("blend")) {
						stage->setBlendDef(BM_SRC_ALPHA,BM_ONE_MINUS_SRC_ALPHA);
					} else if(p.atWord("bumpmap")) {
						stage->setStageType(ST_BUMPMAP);
						bDontDisableDepthWrite = true;
					} else if(p.atWord("specularMap")) {
						stage->setStageType(ST_SPECULARMAP);
						bDontDisableDepthWrite = true;
					} else if(p.atWord("diffusemap")) {
						// ST_COLORMAP is a default stage type, but set it anyway
						stage->setStageType(ST_COLORMAP);
						bDontDisableDepthWrite = true;
					} else if(p.atWord("filter")) {
						stage->setBlendDef(BM_DST_COLOR,BM_ZERO);
					} else if(p.atWord("shader")) {
						// seen in Prey textures/dreamworld/la_floor, etc (dreamworld.mtr)
						// for stages with custom shader? (.vfp files)
						bDontDisableDepthWrite = true; // ??

					} else if(p.atWord("none")) {

						bDontDisableDepthWrite = true; // ??
					} else if(p.atWord("map")) {

						bDontDisableDepthWrite = true; // ??
					} else {
						// NOTE: for some reasons blend types are separated
						// with ',' in Doom3
						u16 src = readBlendEnum(p);
						u16 dst = readBlendEnum(p);
						stage->setBlendDef(src,dst);
					}			
					// disable writing to depth buffer for translucent surfaces
					// (unless otherwise specified)
					if(depthWriteSetInMaterial == false && bDontDisableDepthWrite == false) {
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
					astAPI_i *ast = MAT_ParseExpression(p);
					if(ast) {
						stage->setAlphaAST(ast);
					} else {
						g_core->RedWarning("Failed to parse 'alpha' AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
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
				} else if(p.atWord("nopicmip")) {

				} else if(p.atWord("highquality")) {

				} else if(p.atWord("highres")) {
					
				} else if(p.atWord("cubemap") || p.atWord("cameraCubeMap")) {
					// FIXME: what's the difference between these two?
					//p.skipLine();
					const char *cubeMapName = p.getToken();
					if(!_stricmp(cubeMapName,"nearest_env_cubemap")) {
						// support 'env_cubemap' system similiar to one in Source Engine.
						stage->setStageType(ST_ENV_CUBEMAP);
					} else {
						cubeMapAPI_i *cubeMap = MAT_RegisterCubeMap(cubeMapName);
						if(cubeMap) {
							stage->setCubeMap(cubeMap);
						} else {

						}
					}
				} else if(p.atWord("texgen") || p.atWord("tcgen")) {
					if(p.atWord("environment")) {
						stage->setTCGen(TCG_ENVIRONMENT);
					} else if(p.atWord("skybox")) {
						// used eg. in Prey's (Id Tech 4) dave.mtr -> textures/skybox/dchtest
						//stage->setTCGen(TCG_SKYBOX);
						// This is used along with cubemap for Doom3 skyboxes.
						stage->setStageType(ST_CUBEMAP_SKYBOX);
					} else if(p.atWord("reflect")) {
						// This is used along with cubemap for Doom3 glass materials.
						//stage->setTCGen(TCG_REFLECT);
						stage->setStageType(ST_CUBEMAP_REFLECTION);
					} else if(p.atWord("screen")) {
						// Prey's keyword?
						//stage->setTCGen(TCG_SCREEN);
					} else if(p.atWord("wobbleSky")) {
						class astAPI_i *exp0 = MAT_ParseExpression(p);
						if(exp0 == 0) {
							g_core->RedWarning("mtrIMPL_c::loadFromText: failed to parse exp0 of 'wobbleSke' of material %s in file %s at line %i\n",
								this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
							continue;
						}
						class astAPI_i *exp1 = MAT_ParseExpression(p);
						if(exp1 == 0) {
							delete exp0;
							g_core->RedWarning("mtrIMPL_c::loadFromText: failed to parse exp1 of 'wobbleSke' of material %s in file %s at line %i\n",
								this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
							continue;
						}
						class astAPI_i *exp2 = MAT_ParseExpression(p);
						if(exp2 == 0) {
							delete exp0;
							delete exp1;
							g_core->RedWarning("mtrIMPL_c::loadFromText: failed to parse exp2 of 'wobbleSke' of material %s in file %s at line %i\n",
								this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
							continue;
						}

#if 1
						delete exp0;
						delete exp1;
						delete exp2;
#else

#endif
					} else {
						str tok = p.getToken();
						g_core->RedWarning("mtrIMPL_c::loadFromText: unknown texgen type %s in definition of material %s in file %s at line %i\n",
							tok.c_str(),this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
				} else if(p.atWord_dontNeedWS("if")) {
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
					stage->setRGBAGenVertex();
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
					} else {
						if(scrollVal0)
							delete scrollVal0;
						if(scrollVal1)
							delete scrollVal1;
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
					} else {
						if(scaleVal0)
							delete scaleVal0;
						if(scaleVal1)
							delete scaleVal1;
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
					} else {
						if(shearVal0)
							delete shearVal0;
						if(shearVal1)
							delete shearVal1;
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
					} else {
						if(centerScaleVal0)
							delete centerScaleVal0;
						if(centerScaleVal0)
							delete centerScaleVal1;
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
				} else if(p.atWord("program")) {
					// custom gpu program for this stage. Used in Doom3. 
					stage->setProgramName(p.getToken());
				} else if(p.atWord("fragmentProgram")) {
					// FIXME
					stage->setProgramName(p.getToken());
				} else if(p.atWord("vertexProgram")) {
					// FIXME
					stage->setProgramName(p.getToken());
				} else if(p.atWord("clamp")) {

				} else if(p.atWord("forceHighQuality")) {

				} else if(p.atWord("vertexParm")) {
					p.skipLine();
				} else if(p.atWord("fragmentParm")) {
					p.skipLine();
				} else if(p.atWord("fragmentMap")) {
					p.skipLine();
				} else if(p.atWord("ignoreAlphaTest")) {
				} else if(p.atWord("privatePolygonOffset")) {
					if(p.isAtEOL()) {

					} else {
						float val = p.getFloat();
					}
				} else if(p.atWord("color")) {
					// usage: "color <expR expG expB expA>"
					astAPI_i *expRed = MAT_ParseExpression(p);
					if(expRed == 0) {
						g_core->RedWarning("Failed to parse 'color' red value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					astAPI_i *expGreen= MAT_ParseExpression(p);
					if(expGreen == 0) {
						g_core->RedWarning("Failed to parse 'color' green value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					astAPI_i *expBlue = MAT_ParseExpression(p);
					if(expGreen == 0) {
						g_core->RedWarning("Failed to parse 'color' blue value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					astAPI_i *expAlpha = MAT_ParseExpression(p);
					if(expGreen == 0) {
						g_core->RedWarning("Failed to parse 'color' alpha value AST in material %s in file %s at line %i\n",
							this->name.c_str(),p.getDebugFileName(),p.getCurrentLineNumber());
					}
					stage->setRedAST(expRed);
					stage->setGreenAST(expGreen);
					stage->setBlueAST(expBlue);
					stage->setAlphaAST(expAlpha);
				} else if(p.atWord("linear")) {
				} else if(p.atWord("spiritWalk")) {
					// Prey keyword, used to affect gameplay?
				} else if(p.atWord("shaderFallback2")) {
					// related to D3 shaders handling?
				} else if(p.atWord("shaderLevel2")) {
					// related to D3 shaders handling?
				} else if(p.atWord("scopeView")) {
					// for sniper rifle?
				} else if(p.atWord("specularExp")) {
					p.skipLine();
				} else if(p.atWord("detail")) {
					// In ET it's used in mp_siwa.shader
				} else if(p.atWord("alphaGen")) {
					// V: it is used in TCQ3b (ET) terrain blending
					// for example, dim_terrblendtest.bsp
					if(p.atWord("vertex")) {
						stage->setAlphaGenVertex();
					} else {
						p.skipLine();
					}
				} else if(p.atWord("fog")) {
					// Enemy Territory keyword? "fog off"
					// It's used in skies_sd.shader
					p.skipLine();
				//Dushan 
				} else if(p.atWord("q3map_sunExt")) {
					//extended sun directive
					p.skipLine();
				} else if(p.atWord("q3map_skyLight")) {
					//q3map_skyLight <value> <iterations>
					p.skipLine();
				} else if(p.atWord("nolightmap")) {
					// surface don't generate a lightmap
					p.skipLine();
				} else if(p.atWord("sky")) {
					// sky surfaceparm
					p.skipLine();
				} else if(p.atWord("alphaShadow")) {
					// test light on a per-pixel basis
					p.skipLine();
				} else {
					u32 line = p.getCurrentLineNumber();
					str token  = p.getToken();
					g_core->RedWarning("mtrIMPL_c::loadFromText: unknown stage token '%s' at line %i of %s in material %s\n",
						token.c_str(),line,txt.sourceFile,this->getName());
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
#if 1
		// V: can't do that, it breaks the mirrors.
		// It would render editor image on mirror, check test_mirror map
		if(editorImage.length()) {
			mtrStage_c *s = new mtrStage_c;
			s->setStageType(ST_EDITORIMAGE);
			s->setTexture(editorImage);
			stages.push_back(s);
		} else {
			// V: load image automatically
			// TODO: check if this is okay with other engines
			mtrStage_c *s = new mtrStage_c;
			s->setStageType(ST_EDITORIMAGE);
			s->setTexture(getName());
			stages.push_back(s);
		}
#endif
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
			if(colorMapStage == 0) {
				colorMapStage = this->getFirstStageOfType(ST_NOT_SET);
				if(colorMapStage) {
					colorMapStage->setStageType(ST_COLORMAP_LIGHTMAPPED);
				}
			}
		}
		if(colorMapStage) {
			mtrStage_c *bumpMapStage = this->getFirstStageOfType(ST_BUMPMAP);
			if(bumpMapStage) {
				colorMapStage->setSubStageBumpMap(bumpMapStage);
				this->stages.removeObject(bumpMapStage);
			}
			mtrStage_c *speclarMapStage = this->getFirstStageOfType(ST_SPECULARMAP);
			if(speclarMapStage) {
				colorMapStage->setSubStageSpecularMap(speclarMapStage);
				this->stages.removeObject(speclarMapStage);
			}
			mtrStage_c *heightMapStage = this->getFirstStageOfType(ST_HEIGHTMAP);
			if(heightMapStage) {
				colorMapStage->setSubStageHeightMap(heightMapStage);
				this->stages.removeObject(heightMapStage);
			}
			mtrStage_c *bumpHeightMapStage = this->getFirstStageOfType(ST_BUMPHEIGHTMAP);
			if(bumpHeightMapStage) {
				colorMapStage->setSubStageBumpHeightMap(bumpHeightMapStage);
				this->stages.removeObject(bumpHeightMapStage);
			}
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
					if(s->isLightmapStage() == false && s->hasBlendFunc() == false
						&& s->getAlphaGenType() == ALPHAGEN_NOT_SET) {
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