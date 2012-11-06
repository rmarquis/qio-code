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

	void addModelDrawCalls();

	// for bsp inline model
	void initInlineModel(class rBspTree_c *pMyBSP, u32 myBSPModNum) {
		this->type = MOD_BSP;
		this->myBSP = pMyBSP;
		this->bspModelNum = myBSPModNum;
	}

	
	virtual bool rayTrace(class trace_c &tr) const;

// model creation access for BSP class (bsp inline models)
friend void RF_ClearModels();
friend void RF_ClearModel(model_c *m);
friend model_c *RF_AllocModel(const char *modName);
friend rModelAPI_i *RF_FindModel(const char *modName);
friend rModelAPI_i *RF_RegisterModel(const char *modName);
};

#endif // __RF_MODEL_H__

