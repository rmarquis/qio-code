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
// mat_impl.h - material class implementation

#ifndef __MAT_IMPL_H__
#define __MAT_IMPL_H__

#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <shared/str.h>
#include <shared/array.h>
#include <renderer/drawCallSort.h>
#include "mat_public.h"
#include "mat_stageTexture.h"

class skyBox_c : public skyBoxAPI_i {
	str baseName;
	class textureAPI_i *up;
	class textureAPI_i *down;
	class textureAPI_i *right;
	class textureAPI_i *left;
	class textureAPI_i *front;
	class textureAPI_i *back;
	class textureAPI_i *loadSubTexture(const char *sufix);
public:
	void setBaseName(const char *newBaseName);
	void uploadTextures();
	//void freeTextures();
	skyBox_c();
	///~skyBox_c();
	//void clear();
	virtual textureAPI_i *getUp() const {
		return up;
	}
	virtual textureAPI_i *getDown() const {
		return down;
	}
	virtual textureAPI_i *getRight() const {
		return right;
	}
	virtual textureAPI_i *getLeft() const {
		return left;
	}
	virtual textureAPI_i *getFront() const {
		return front;
	}
	virtual textureAPI_i *getBack() const {
		return back;
	}
	virtual bool isValid() const {
		if(baseName.length() == 0)
			return false;
		if(baseName[0] == '-')
			return false;
		return true;
	}
};
class skyParms_c : public skyParmsAPI_i {
	skyBox_c farBox;
	float cloudHeight;
	skyBox_c nearBox;
public:
	skyParms_c(const char *farBoxName, float newCloudHeight, const char *nearBoxName);
	void uploadTextures() {
		farBox.uploadTextures();
		nearBox.uploadTextures();
	}
	virtual float getCloudHeight() const {
		return cloudHeight;
	}
	virtual const skyBoxAPI_i *getFarBox() const {
		return &farBox;
	}
	virtual const skyBoxAPI_i *getNearBox() const {
		return &nearBox;
	}
};

// parsed from xmap_sun.
// Xreal is parsing this keyword in tr_shader.c and automatically applies it to tr.globals.
// xmap_sun syntax: colorR colorG colorB anglesPitch anglesYaw anglesRoll
// NOTE: Xreal ignores the anglesPitch component.
// NOTE: there is also "xmap_skylight" keyword which seem to be ignored in Xreal.
class sunParms_c : public sunParmsAPI_i {
	vec3_c sunColor; // usually 1 1 1
	vec3_c sunAngles;
	vec3_c sunDirection; // calculated from angles
public:
	void setFromColorAndAngles(const vec3_c &nColor, const vec3_c &nAngles) {
		sunColor = nColor;
		sunAngles = nAngles;
#if 0
		sunDirection = nAngles.getForward();
#else
		// let's stick to Xreal "xmap_sun" angles format.
		// It's different than the one used in vec3_c::getForward()
		float radiansA = DEG2RAD(sunAngles.y);
		float radiansB = DEG2RAD(sunAngles.z);
		sunDirection[0] = cos(radiansA) * cos(radiansB);
		sunDirection[1] = sin(radiansA) * cos(radiansB);
		sunDirection[2] = sin(radiansB);
#endif
	}
	virtual const class vec3_c &getSunDir() const {
		return sunDirection;
	}
	virtual const class vec3_c &getSunColor() const {
		return sunColor;
	}
};

class deform_c {
	deformType_e type;
public:
	deform_c(deformType_e newType) {
		type = newType;
	}
	deformType_e getType() const {
		return type;
	}
};

class deformArray_c : public deformArrayAPI_i {
	arraySTD_c<deform_c*> deforms;

	// deformArrayAPI_i impl
	virtual u32 getNumDeforms() const {
		return deforms.size();
	}
	virtual deformType_e getDeformType(u32 idx) const {
		return deforms[idx]->getType();
	}
public:
	void addDeformSprite() {
		deforms.push_back(new deform_c(DEFORM_AUTOSPRITE));
	}
	bool hasDeformOfType(enum deformType_e type) const {
		for(u32 i = 0; i < deforms.size(); i++) {
			if(deforms[i]->getType() == type)
				return true;
		}
		return false;
	}
	~deformArray_c() {
		for(u32 i = 0; i < deforms.size(); i++) {
			delete deforms[i];
		}
		deforms.clear();
	}
};

class mtrStage_c : public mtrStageAPI_i {
	stageTexture_c stageTexture;
	alphaFunc_e alphaFunc;
	blendDef_s blend;
	enum texCoordGen_e tcGen;
	class texModArray_c *texMods;
	enum stageType_e type;
	class rgbGen_c *rgbGen;
	bool depthWrite; // glDepthMask(stage->depthWrite); (true by default)
	// custom per-stage RGBA masking for Doom3 materials
	maskState_s colorMask;
	bool bMarkedForDelete;
	// only if this->type == ST_COLORMAP
	mtrStage_c *subStageBumpMap;
	mtrStage_c *subStageHeightMap;
	mtrStage_c *nextBundle;
	// "if" condition for Doom3 materials
	class astAPI_i *condition;
	// Doom3 replacements for alphaFunc_e 
	class astAPI_i *alphaTestAST;
	// custom gpu shader name (not used yet)
	str programName;
	// cubemap
	class cubeMapAPI_i *cubeMap;
public:
	mtrStage_c();
	~mtrStage_c();

	virtual textureAPI_i *getTexture(float curTimeSec) const {
		return stageTexture.getTexture(curTimeSec);
	}	
	virtual class textureAPI_i *getTextureForFrameNum(u32 frameNum) const {
		return stageTexture.getTextureForFrameNum(frameNum);
	}
	virtual alphaFunc_e getAlphaFunc() const {
		return alphaFunc;
	}
	virtual const struct blendDef_s &getBlendDef() const {
		return blend;
	}
	virtual cubeMapAPI_i *getCubeMap() const {
		return cubeMap;
	}
	virtual mtrStageAPI_i *getBumpMap() const {
		return subStageBumpMap;
	}
	virtual mtrStageAPI_i *getHeightMap() const {
		return subStageHeightMap;
	}
	virtual mtrStageAPI_i *getSpecularMap() const {
		return 0;
	}
	virtual bool hasTexMods() const {
		if(texMods)
			return true;
		return false;
	}
	virtual void applyTexMods(class matrix_c &out, float curTimeSec, const class astInputAPI_i *in) const;
	virtual stageType_e getStageType() const {
		return type;
	}
	virtual bool hasRGBGen() const;
	virtual enum rgbGen_e getRGBGenType() const;
	virtual bool getRGBGenConstantColor3f(float *out3Floats) const;
	virtual float getRGBGenWaveValue(float curTimeSec) const;
	virtual void evaluateRGBGen(const class astInputAPI_i *in, float *out3Floats) const;
	virtual bool getDepthWrite() const {
		return depthWrite;
	}
	virtual bool isUsingCustomProgram() const {
		if(programName.length())
			return true;
		return false;
	}
	bool isLightmapStage() const {
		return (type == ST_LIGHTMAP);
	}
	void setStageType(stageType_e newType) {
		this->type = newType;
	}
	void setTexture(class textureAPI_i *nt) {
		stageTexture.fromTexturePointer(nt);
	}
	void setCubeMap(class cubeMapAPI_i *n) {
		cubeMap = n;
	}
	void setSubStageBumpMap(class mtrStage_c *s) {
		this->subStageBumpMap = s;
	}
	void setSubStageHeightMap(class mtrStage_c *s) {
		this->subStageHeightMap = s;
	}
	void setNextBundle(class mtrStage_c *s) {
		this->nextBundle = s;
	}
	void setAlphaFunc(alphaFunc_e newAF) {
		alphaFunc = newAF;
	}
	void setBlendDef(u16 srcEnum, u16 dstEnum) {
		blend.src = srcEnum;
		blend.dst = dstEnum;
	}
	void disableBlending() {
		blend.src = 0;
		blend.dst = 0;
	}
	u16 getBlendSrc() const {
		return blend.src;
	}
	u16 getBlendDst() const {
		return blend.dst;
	}
	bool hasBlendFunc() const {
		if(blend.src || blend.dst)
			return true;
		return false;
	}
	void setTCGen(texCoordGen_e nTCGen) {
		tcGen = nTCGen;
	}
	bool hasTexGen() const {
		return (tcGen != TCG_NONE);
	}
	bool hasAlphaTest() const {
		if(alphaFunc == AF_NONE)
			return false;
		return true;
	}
	enum texCoordGen_e getTexGen() const {
		return tcGen;
	}
	void setDepthWrite(bool newDepthWrite) {
		this->depthWrite = newDepthWrite;
	}
	void setMarkedForDelete(bool newbMarkedForDelete) {
		this->bMarkedForDelete = newbMarkedForDelete;
	}
	bool isMarkedForDelete() const {
		return bMarkedForDelete;
	}
	void setTexture(const char *newMapName);
	int getImageWidth() const;
	int getImageHeight() const;
	u32 getNumImageFrames() const;
	void addTexMod(const class texMod_c &newTM);
	void addD3TexModRotate(class astAPI_i *value);
	void addD3TexModScale(class astAPI_i *val0, class astAPI_i *val1);
	void addD3TexModShear(class astAPI_i *val0, class astAPI_i *val1);
	void addD3TexModScroll(class astAPI_i *val0, class astAPI_i *val1);
	void addD3TexModCenterScale(class astAPI_i *val0, class astAPI_i *val1);
	void setRGBGenAST(class astAPI_i *ast);
	class rgbGen_c *allocRGBGen();
	class stageTexture_c &getStageTexture() {
		return stageTexture;
	}
	void setStageCondition(class astAPI_i *newCondition) {
		condition = newCondition;
	}
	// return true if stage is conditional (has Doom3 'if' condition)
	virtual bool hasIFCondition() const;
	// return true if drawing condition is met for given input variables
	virtual bool conditionMet(const class astInputAPI_i *in) const;
	// Doom3 glColorMask(...) settings
	void setMaskRed(bool bMask);
	void setMaskGreen(bool bMask);
	void setMaskBlue(bool bMask);
	void setMaskAlpha(bool bMask);
	void setMaskColor(bool bMask);
	virtual bool getColorMaskRed() const;
	virtual bool getColorMaskGreen() const;
	virtual bool getColorMaskBlue() const;
	virtual bool getColorMaskAlpha() const;
	void setAlphaTestAST(class astAPI_i *ast);
	virtual float evaluateAlphaTestValue(const class astInputAPI_i *in) const;
	void setRedAST(class astAPI_i *ast);
	void setGreenAST(class astAPI_i *ast);
	void setBlueAST(class astAPI_i *ast);
	// custom GPU shader for stage
	void setProgramName(const char *newProgram) {
		programName = newProgram;
	}
	// 'vertexColor' keyword
	void setRGBAGenVertex();
};

class mtrIMPL_c : public mtrAPI_i { 
	str name; // name of the material (without extension)
	str sourceFileName; // name of material source file (.shader, .mtr or .tga/.jpg/.png - if loaded directly)
	mtrIMPL_c *hashNext;
	arraySTD_c<mtrStage_c*> stages;
	skyParms_c *skyParms;
	sunParms_c *sunParms;
	float polygonOffset;
	enum cullType_e cullType;
	bool bPortalMaterial; // set to true by "portal" global material keyword
	bool bMirrorMaterial; // set to true by "mirror" global material keyword
	str editorImage; // set by "qer_editorimage" keyword (Q3/D3)
	// vertex deforms array (they are per-material, not per-stage)
	deformArray_c *deforms;
	// for .vmt support, set by $compilesky key
	bool bGenericSky;;

	void addDeformSprite();

	void removeAllStagesOfType(enum stageType_e type);
	class mtrStage_c *getFirstStageOfType(enum stageType_e type);
	const class mtrStage_c *getFirstStageOfType(enum stageType_e type) const;
	void replaceStageType(enum stageType_e stageTypeToFind, enum stageType_e replaceWith);
public:
	mtrIMPL_c();
	~mtrIMPL_c();

	void clear();

	virtual const char *getName() const {
		return name;
	}
	virtual const char *getSourceFileName() const {
		return sourceFileName;
	}
	void setName(const char *newName) {
		name = newName;
	}
	virtual u32 getNumStages() const {
		return stages.size();
	}
	virtual const mtrStageAPI_i *getStage(u32 stageNum) const {
		return stages[stageNum];
	}
	virtual enum cullType_e getCullType() const {
		return cullType;
	}
	virtual bool hasTexGen() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->hasTexGen())
				return true;
		}
		return false;
	}
	virtual bool hasRGBGen() const  {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->hasRGBGen())
				return true;
		}
		return false;
	}
	virtual bool hasBlendFunc() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->hasBlendFunc())
				return true;
		}
		return false;
	}
	virtual bool hasStageWithoutBlendFunc() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->hasBlendFunc() == false)
				return true;
		}
		return false;
	}
	virtual bool hasAlphaTest() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->hasAlphaTest())
				return true;
		}
		return false;
	}
	virtual const class mtrStageAPI_i *getFirstStageWithAlphaFunc() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->hasAlphaTest())
				return stages[i];
		}
		return 0;
	}
	virtual bool isPortalMaterial() const {
		return this->bPortalMaterial;
	}
	virtual bool isMirrorMaterial() const {
		return this->bMirrorMaterial;
	}
	virtual bool hasDeforms() const {
		if(deforms)
			return true;
		return false;
	}
	virtual class deformArrayAPI_i *getDeformsArray() const {
		return deforms;
	}
	virtual bool hasDeformOfType(enum deformType_e type) const {
		if(deforms == 0)
			return false;
		return deforms->hasDeformOfType(type);
	}
	virtual bool hasStageWithCubeMap() const {
		for(u32 i = 0; i < stages.size(); i++) {
			const mtrStage_c *s = stages[i];
			if(s->getCubeMap())
				return true;
		}
		return false;
	}	
	virtual bool isNeededForLightPass() const {
		if(hasStageWithoutBlendFunc() == false)
			return false;
		if(hasStageWithCubeMap())
			return false;
		return true;
	}
	virtual bool isGenericSky() const {
		return bGenericSky;
	}
	virtual bool hasStageOfType(stageType_e type) const {
		for(u32 i = 0; i < stages.size(); i++) {
			const mtrStage_c *s = stages[i];
			if(s->getStageType() == type)
				return true;
			if(type == ST_BUMPMAP) {
				if(s->getBumpMap())
					return true;
			}
			if(type == ST_HEIGHTMAP) { 
				if(s->getHeightMap())
					return true;
			}
			if(type == ST_SPECULARMAP) {
				if(s->getSpecularMap())
					return true;
			}
		}
		return false;
	}
	virtual bool hasOnlyStagesOfType(enum stageType_e type) const {
		for(u32 i = 0; i < stages.size(); i++) {
			const mtrStage_c *s = stages[i];
			if(s->getStageType() != type)
				return false;
		}
		return true;
	}
	bool hasPolygonOffset() const {
		if(polygonOffset == 0.f)
			return false;
		return true;
	}
	inline mtrIMPL_c *getHashNext() const {
		return hashNext;
	}
	inline void setHashNext(mtrIMPL_c *p) {
		hashNext = p;
	}
	bool hasStageWithBlendFunc() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->hasBlendFunc()) {
				return true;
			}
		}
		return false;
	}
	bool hasStageWithDepthWriteEnabled() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->getDepthWrite()) {
				return true;
			}
		}
		return false;
	}
	virtual const class skyParmsAPI_i *getSkyParms() const {
		return skyParms;
	}
	virtual const class sunParmsAPI_i *getSunParms() const {
		return sunParms;
	}
	// TODO: precalculate stage->sort once and just return the stored value here?
	virtual enum drawCallSort_e getSort() const { 
		if(bPortalMaterial) {
			return DCS_PORTAL;
		}
		if(bMirrorMaterial) {
			return DCS_PORTAL;
		}
		if(hasStageWithBlendFunc()) {
			// we must distinct between two more shader types.
			// otherwise bsp surfaces (like walls) are sometimes drawn
			// ON TOP of beams/glass panels. (q3dm11, etc)
			if(hasStageWithoutBlendFunc()) {
				// this material has fixed blendfunc stages with non-blendfunc stages
				// Such materials are used on q3 map walls, etc...
				return DCS_BLEND; // 2 # priority
			} else {
				// this material has ONLY blendfunc stages
				// Such materials are used on q3 glass panels and beams.
				// They must be drawn last.
				// (eg. see textures/sfx/beam from q3 shaders/sfx.shader)

				// plasma projectiles must be drawn after q3dm0 world glass panels and decals
				if(this->getFirstStageOfType(ST_LIGHTMAP) == 0) {
					// plasma projectiles must be drawn after plasma decals...
					if(this->hasPolygonOffset()) {
						return DCS_BLEND3; // 3b # priority 
					} else {
						return DCS_BLEND4; // 3c # priority 
					}
				} else {
					return DCS_BLEND2; // 3a # priority 
				}
			}
		}
		return DCS_OPAQUE; // 1 # priority
	}
	virtual int getImageWidth() const {
		if(stages.size() == 0)
			return 32;
		return stages[0]->getImageWidth();
	}
	virtual int getImageHeight() const {
		if(stages.size() == 0)
			return 32;
		return stages[0]->getImageHeight();
	}
	virtual u32 getColorMapImageFrameCount() const {
		if(stages.size() == 0)
			return 0;
		return stages[0]->getNumImageFrames();
	}
	virtual bool hasStageWithoutCustomProgram() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->isUsingCustomProgram()==false)
				return true; 
		}
		// all stages are using custom programs
		return false;
	}
	virtual float getPolygonOffset() const {
		return polygonOffset;
	}

	bool isVMTMaterial() const {
		return name.hasExt("vmt");
	}

	void createFromImage();
	// Source Engine .vmt support (Valve MaTerials)
	bool loadFromVMTFile();
	void createFromTexturePointer(class textureAPI_i *tex);
	u16 readBlendEnum(class parser_c &p);
	void setSkyParms(const char *farBox, const char *cloudHeight, const char *nearBox);
	bool loadFromText(const struct matTextDef_s &txt);
};

#endif // __MAT_IMPL_H__
