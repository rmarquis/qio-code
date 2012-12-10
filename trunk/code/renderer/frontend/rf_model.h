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
	};
	aabb bb;
public:
	model_c() {
		hashNext = 0;
		myBSP = 0;
		type = MOD_BAD;
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
	virtual bool isDeclModel() const {
		if(type == MOD_DECL)
			return true;
		return false;
	}
	virtual u32 getNumSurfaces() const;
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

	void addModelDrawCalls(const class rfSurfsFlagsArray_t *extraSfFlags);

	// for bsp inline models
	void initInlineModel(class rBspTree_c *pMyBSP, u32 myBSPModNum);
	// for proc inline models
	void initProcModel(class procTree_c *pMyPROC, class r_model_c *modPtr);
	// for static models (r_model_c class)
	// NOTE: "myNewModelPtr" will be stored in this model class and fried on renderer shutdown!
	void initStaticModel(class r_model_c *myNewModelPtr);

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
	virtual const class skelAnimAPI_i *getDeclModelAFPoseAnim() const;

	void clear();

// model creation access for BSP class (bsp inline models)
friend void RF_ClearModels();
friend model_c *RF_AllocModel(const char *modName);
friend rModelAPI_i *RF_FindModel(const char *modName);
friend rModelAPI_i *RF_RegisterModel(const char *modName);
};

#endif // __RF_MODEL_H__

