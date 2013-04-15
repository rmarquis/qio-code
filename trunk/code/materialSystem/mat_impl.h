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

class mtrStage_c : public mtrStageAPI_i {
	stageTexture_c stageTexture;
	alphaFunc_e alphaFunc;
	blendDef_s blend;
	enum texCoordGen_e tcGen;
	class texModArray_c *texMods;
	enum stageType_e type;
	class rgbGen_c *rgbGen;
	bool depthWrite; // glDepthMask(stage->depthWrite); (true by default)
	bool bMarkedForDelete;
	// only if this->type == ST_COLORMAP
	mtrStage_c *subStageBumpMap;
	mtrStage_c *subStageHeightMap;
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
	virtual mtrStageAPI_i *getBumpMap() const {
		return subStageBumpMap;
	}
	virtual mtrStageAPI_i *getHeightMap() const {
		return subStageHeightMap;
	}
	virtual bool hasTexMods() const {
		if(texMods)
			return true;
		return false;
	}
	virtual void applyTexMods(class matrix_c &out, float curTimeSec) const;
	virtual stageType_e getStageType() const {
		return type;
	}
	virtual bool hasRGBGen() const;
	virtual enum rgbGen_e getRGBGenType() const;
	virtual bool getRGBGenConstantColor3f(float *out3Floats) const;
	virtual float getRGBGenWaveValue(float curTimeSec) const;
	virtual bool getDepthWrite() const {
		return depthWrite;
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
	void setSubStageBumpMap(class mtrStage_c *s) {
		this->subStageBumpMap = s;
	}
	void setSubStageHeightMap(class mtrStage_c *s) {
		this->subStageHeightMap = s;
	}
	void setAlphaFunc(alphaFunc_e newAF) {
		alphaFunc = newAF;
	}
	void setBlendDef(u16 srcEnum, u16 dstEnum) {
		blend.src = srcEnum;
		blend.dst = dstEnum;
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
	class rgbGen_c *allocRGBGen();
	class stageTexture_c &getStageTexture() {
		return stageTexture;
	}
};

class mtrIMPL_c : public mtrAPI_i { 
	str name; // name of the material (without extension)
	str sourceFileName; // name of material source file (.shader, .mtr or .tga/.jpg/.png - if loaded directly)
	mtrIMPL_c *hashNext;
	arraySTD_c<mtrStage_c*> stages;
	skyParms_c *skyParms;
	float polygonOffset;
	enum cullType_e cullType;
	bool bPortalMaterial; // set to true by "portal" global material keyword
	bool bMirrorMaterial; // set to true by "mirror" global material keyword

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
	virtual bool hasAlphaTest() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->hasAlphaTest())
				return true;
		}
		return false;
	}
	virtual bool isPortalMaterial() const {
		return this->bPortalMaterial;
	}
	virtual bool isMirrorMaterial() const {
		return this->bMirrorMaterial;
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
	bool hasStageWithoutBlendFunc() const {
		for(u32 i = 0; i < stages.size(); i++) {
			if(stages[i]->hasBlendFunc() == false) {
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
