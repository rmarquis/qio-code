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

enum stageType_e {
	ST_NOT_SET,
	ST_LIGHTMAP,
};

class mtrStage_c : public mtrStageAPI_i {
	stageTexture_c stageTexture;
	alphaFunc_e alphaFunc;
	blendDef_s blend;
	enum texCoordGen_e tcGen;
	class texModArray_c *texMods;
	enum stageType_e type;
	class rgbGen_c *rgbGen;
public:
	mtrStage_c();
	~mtrStage_c();

	virtual textureAPI_i *getTexture(float curTimeSec) const {
		return stageTexture.getTexture(curTimeSec);
	}	
	virtual alphaFunc_e getAlphaFunc() const {
		return alphaFunc;
	}
	virtual const struct blendDef_s &getBlendDef() const {
		return blend;
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

	bool isLightmapStage() const {
		return (type == ST_LIGHTMAP);
	}
	void setStageType(stageType_e newType) {
		this->type = newType;
	}
	void setTexture(class textureAPI_i *nt) {
		stageTexture.fromTexturePointer(nt);
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
	enum texCoordGen_e getTexGen() const {
		return tcGen;
	}
	void setTexture(const char *newMapName);
	int getImageWidth() const;
	int getImageHeight() const;
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
	virtual const class skyParmsAPI_i *getSkyParms() const {
		return skyParms;
	}
	virtual enum drawCallSort_e getSort() const { 
		if(hasStageWithBlendFunc()) {
			return DCS_BLEND;
		}
		return DCS_OPAQUE;
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
	virtual float getPolygonOffset() const {
		return polygonOffset;
	}

	void createFromImage();
	u16 readBlendEnum(class parser_c &p);
	void setSkyParms(const char *farBox, const char *cloudHeight, const char *nearBox);
	bool loadFromText(const struct matTextDef_s &txt);
};

#endif // __MAT_IMPL_H__
