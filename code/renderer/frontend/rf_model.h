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
// rf_model.h
#ifndef __RF_MODEL_H__
#define __RF_MODEL_H__

#include "../rModelAPI.h"
#include <shared/str.h>
#include <math/aabb.h>

enum modelType_e {
	MOD_BAD,
	MOD_BSP, // inline bsp model (model name starts with '*')
	MOD_STATIC, // static (non-animated) triangle model
	MOD_SKELETAL, // animated skeletal model
	MOD_PROC, // inline proc model
	MOD_DECL, // model declared in Doom3 .def files
	MOD_KEYFRAMED, // per-vertex animation (single .md3/.md2 model)
	// Quake3 three-parts player model (separate .md3 for legs, for torso, and for head)
	MOD_Q3PLAYERMODEL, 
	// single sprite, defined by material name and radius
	MOD_SPRITE,
	// FAKK/MoHAA .tik model
	MOD_TIKI,
	MOD_NUM_MODEL_TYPES,
};

class model_c : public rModelAPI_i {
	str name;
	model_c *hashNext;

	modelType_e type;
	union {
		struct {
			class rBspTree_c *myBSP;
			u32 bspModelNum;
		}; // only if this->type == MOD_BSP
		class r_model_c *staticModel; // only if this->type == MOD_STATIC
		class skelModelAPI_i *skelModel; // only if this->type == MOD_SKELETAL
		struct {
			class r_model_c *procModel;
			class procTree_c *myProcTree;
		}; // only if this->type == MOD_PROC
		class modelDeclAPI_i *declModel; // only if this->type == MOD_DECL
		class kfModelAPI_i *kfModel; // only if this->type == MOD_KEYFRAMED
		class q3PlayerModelAPI_i *q3PlayerModel;  // only if this->type == MOD_Q3PLAYERMODEL
		struct {
			class mtrAPI_i *spriteMaterial;
			float spriteRadius;
		}; // only if this->type == MOD_SPRITE
		class tiki_i *tiki;  // only if this->type == MOD_TIKI
	};
	aabb bb;
	// RTCW wolfAnim.cfg extra animation data.
	// wolfAnim.cfg file is used to map animation names 
	// to mds model frames.
	class rWolfAnimCfg_c *wolfAnim;
	// ET character extra data
	class etChar_c *etChar;
public:
	model_c() {
		hashNext = 0;
		myBSP = 0;
		type = MOD_BAD;
		wolfAnim = 0;
		etChar = 0;
	}
	virtual const char *getName() const {
		return name;
	}
	virtual const class aabb &getBounds() const {
		return bb;
	}
	virtual bool isStatic() {
		if(type == MOD_STATIC)
			return true;
		if(type == MOD_BSP)
			return true; // bsp models are never animated (obviously)
		if(type == MOD_PROC)
			return true; // the same goes for proc models
		return false;
	}
	virtual bool isValid() const {
		if(type == MOD_BAD)
			return false;
		return true;
	}
	virtual bool isInlineBSPModel() const {
		if(type == MOD_BSP)
			return true;
		return false;
	}
	virtual bool isInlinePROCModel() const {
		if(type == MOD_PROC)
			return true;
		return false;
	}
	virtual bool isDeclModel() const {
		if(type == MOD_DECL)
			return true;
		return false;
	}
	virtual bool isTIKI() const {
		if(type == MOD_TIKI)
			return true;
		return false;
	}
	virtual bool isSkeletal() const;
	virtual bool isKeyframed() const;
	virtual bool isQ3PlayerModel() const { 
		if(type == MOD_Q3PLAYERMODEL)
			return true;
		return false;
	}
	virtual bool isSprite() const {
		if(type == MOD_SPRITE)
			return true;
		return false;
	}
	virtual float getSpriteRadius() const {
		if(type != MOD_SPRITE) {
			return 0;
		}
		return spriteRadius;
	}
	virtual mtrAPI_i *getSpriteMaterial() const {
		if(type != MOD_SPRITE) {
			return 0;
		}
		return spriteMaterial;
	}
	virtual bool hasWolfAnimConfig() const {
		if(wolfAnim)
			return true;
		return false;
	}
	virtual bool hasCharacterFile() const {
		if(etChar)
			return true;
		return false;
	}
	virtual bool hasDefaultSkinName() const;
	virtual const char *getDefaultSkinName() const;
	virtual bool findWolfAnimData(const char *animName, int *firstFrame, int *lastFrame, float *fps) const;
	virtual const skelAnimAPI_i *findSkelAnim(const char *animName) const;
	virtual u32 getNumSurfaces() const;
	virtual u32 getNumAnims() const;
	virtual bool hasAnim(const char *animName) const;

	inline void setHashNext(model_c *hn) {
		this->hashNext = hn;
	}
	inline model_c *getHashNext() const {
		return hashNext;
	}
	void setBounds(const aabb &newBB) {
		bb = newBB;
	}
	void setBounds(const vec3_c &newMins, const vec3_c &newMaxs) {
		bb.mins = newMins;
		bb.maxs = newMaxs;
	}

	void addModelDrawCalls(const class rfSurfsFlagsArray_t *extraSfFlags, const vec3_c *extraRGB);

	// for bsp inline models
	void initInlineModel(class rBspTree_c *pMyBSP, u32 myBSPModNum);
	// for proc inline models
	void initProcModel(class procTree_c *pMyPROC, class r_model_c *modPtr);
	// for static models (r_model_c class)
	// NOTE: "myNewModelPtr" will be stored in this model class and fried on renderer shutdown!
	void initStaticModel(class r_model_c *myNewModelPtr);
	// for sprites
	void initSprite(const char *matName, float newSpriteRadius);

	// call markAsUsed on every referenced material
	void markModelMaterials();

	const r_model_c *getRModel() const {
		if(type == MOD_STATIC) {
			return staticModel;
		}
		return 0;
	}
	
	virtual bool rayTrace(class trace_c &tr) const;
	virtual bool createStaticModelDecal(class simpleDecalBatcher_c *out, const class vec3_c &pos,
		const class vec3_c &normal,	float radius, class mtrAPI_i *material);

	virtual class skelModelAPI_i *getSkelModelAPI() const;
	virtual class modelDeclAPI_i *getDeclModelAPI() const;
	virtual class tiki_i *getTIKI() const;
	virtual const class skelAnimAPI_i *getDeclModelAFPoseAnim() const;
	virtual class kfModelAPI_i *getKFModelAPI() const;
	virtual const q3PlayerModelAPI_i *getQ3PlayerModelAPI() const;

	virtual bool getTagOrientation(int tagNum, const struct singleAnimLerp_s &legs, const struct singleAnimLerp_s &torso, class matrix_c &out) const;
	virtual bool getTagOrientation(int tagNum, class matrix_c &out) const;
	virtual bool getModelData(class staticModelCreatorAPI_i *out) const;
	virtual void printBoneNames() const;
	virtual u32 getTotalTriangleCount() const;
	virtual u32 getTotalVertexCount() const;
	virtual bool hasStageWithoutBlendFunc() const;
	// will return -1 if bone not found
	virtual int findBone(const char *boneName) const;

	void clear();

// model creation access for BSP class (bsp inline models)
friend void RF_ClearModels();
friend model_c *RF_AllocModel(const char *modName);
friend rModelAPI_i *RF_FindModel(const char *modName);
friend rModelAPI_i *RF_RegisterModel(const char *modName);
friend void RF_MarkModelMaterials();
friend u32 RF_GetNumAllocatedModels();
friend const char *RF_GetAllocatedModelName(u32 i);
};

#endif // __RF_MODEL_H__

