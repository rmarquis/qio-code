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

class model_c : public rModelAPI_i {
	str name;
	model_c *hashNext;

	class rBspTree_c *myBSP;
	u32 bspModelNum;
public:
	model_c() {
		hashNext = 0;
		myBSP = 0;
	}
	virtual const char *getName() const {
		return name;
	}
	inline void setHashNext(model_c *hn) {
		this->hashNext = hn;
	}
	inline model_c *getHashNext() const {
		return hashNext;
	}

	void addModelDrawCalls();

	// for bsp inline model
	void initInlineModel(class rBspTree_c *pMyBSP, u32 myBSPModNum) {
		this->myBSP = pMyBSP;
		this->bspModelNum = myBSPModNum;
	}

// model creation access for BSP class (bsp inline models)
friend void RF_ClearModels();
friend void RF_ClearModel(model_c *m);
friend model_c *RF_AllocModel(const char *modName);
friend rModelAPI_i *RF_FindModel(const char *modName);
friend rModelAPI_i *RF_RegisterModel(const char *modName);
};

#endif // __RF_MODEL_H__

