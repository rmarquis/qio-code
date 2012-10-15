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
#include "mat_public.h"

class mtrStage_c : public mtrStageAPI_i {
	class textureAPI_i *texture; 
	alphaFunc_e alphaFunc;
public:
	mtrStage_c();

	void setTexture(class textureAPI_i *nt) {
		texture = nt;
	}
	virtual textureAPI_i *getTexture() const {
		return texture;
	}	
	void setAlphaFunc(alphaFunc_e newAF) {
		alphaFunc = newAF;
	}
	alphaFunc_e getAlphaFunc() const {
		return alphaFunc;
	}
	void setTexture(const char *newMapName);
};

class mtrIMPL_c : public mtrAPI_i { 
	str name;
	mtrIMPL_c *hashNext;
	arraySTD_c<mtrStage_c*> stages;
public:
	mtrIMPL_c();
	~mtrIMPL_c();

	virtual const char *getName() const {
		return name;
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
	inline mtrIMPL_c *getHashNext() const {
		return hashNext;
	}
	inline void setHashNext(mtrIMPL_c *p) {
		hashNext = p;
	}

	void createFromImage();
	bool loadFromText(const struct matTextDef_s &txt);
};

#endif // __MAT_IMPL_H__
