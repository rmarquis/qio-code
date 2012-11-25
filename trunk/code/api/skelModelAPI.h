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
// skelModelAPI.h - interface of skeletal model class (used eg. for md5mesh models)
#ifndef __SKELMODELAPI_H__
#define __SKELMODELAPI_H__

#include <math/vec2.h>
#include <math/vec3.h>
#include <math/matrix.h>
#include <shared/array.h>
#include <shared/skelUtils.h>

// shared raw data structs
struct skelVert_s {
	vec2_c tc;
	u16 firstWeight;
	u16 numWeights;
};
struct skelWeight_s {
	vec3_c ofs;
	float weight;
	u16 boneName;
	short boneIndex; // "-1" means not attached
};

// skeletal model interface
class skelSurfaceAPI_i {
public:
	virtual const char *getMatName() const = 0;
	//virtual const char *getSurfName() const = 0;

	virtual u32 getNumVerts() const = 0;
	virtual u32 getNumWeights() const = 0;
	virtual u32 getNumIndices() const = 0;
	// raw data access
	virtual const skelVert_s *getVerts() const = 0;
	virtual const skelWeight_s *getWeights() const = 0;
	virtual const u16 *getIndices() const = 0;
};

class skelModelAPI_i {
public:
	virtual ~skelModelAPI_i() { };
	virtual u32 getNumSurfs() const = 0;
	virtual u32 getNumBones() const = 0;
	virtual const skelSurfaceAPI_i *getSurface(u32 surfNum) const = 0;
	virtual const boneOrArray_c &getBaseFrameABS() const = 0;
	virtual bool hasCustomScaling() const = 0;
	virtual const vec3_c& getScaleXYZ() const = 0;
	virtual int getLocalBoneIndexForBoneName(const char *nameStr) const = 0;
	//virtual const aabb &getEstimatedBounds() const = 0;
};

#endif // __SKELMODELAPI_H__
