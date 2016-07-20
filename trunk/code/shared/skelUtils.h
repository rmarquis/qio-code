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
// skelUtils.h - helper functions for skeletal animation
#ifndef __SKELUTILS_H__
#define __SKELUTILS_H__

#include <math/matrix.h>
#include <shared/array.h>

///
/// shared raw data structs
///

// bone orientation defined by 4x4 matrix and bone name index
struct boneOr_s {
	matrix_c mat;
	u32 boneName;

	void setBlendResult(const boneOr_s &from, const boneOr_s &to, float frac);
	const vec3_c &getOrigin() const {
		return mat.getOrigin();
	}
}; // 68 bytes

// array of bone orientations
class boneOrArray_c :  public arraySTD_c<boneOr_s> {
public:
	// concat bone transforms
	void localBonesToAbsBones(const class boneDefArray_c *boneDefs);
	void absBonesToLocalBones(const class boneDefArray_c *boneDefs, const boneOrArray_c *absBones);
	void setBlendResult(const boneOrArray_c &from, const boneOrArray_c &to, float frac);	
	void setBlendResult(const boneOrArray_c &from, const boneOrArray_c &to, float frac, const arraySTD_c<u32> &bonesToBlend);	
	void setBones(const boneOrArray_c &from, const arraySTD_c<u32> &bonesToSet);	
	u32 findNearestBone(const vec3_c &pos, float *outDist) const;
	void inverse();
	void identity();

	void transform(const matrix_c &ofs);
	void scale(float scale);
	void scaleXYZ(const vec3_c &scaleXYZ);
	boneOrArray_c getInversed() const;

	const vec3_c &getBonePos(u32 idx) const {
		return (*this)[idx].getOrigin();
	}
	const matrix_c &getBoneMat(u32 idx) const {
		return (*this)[idx].mat;
	}
};

struct boneDef_s {
	u16 nameIndex;
	short parentIndex;
	int flags;

	boneDef_s() {
		flags = 0;
	}
};
// array of bone definitions (name index + parent index)
class boneDefArray_c :  public arraySTD_c<boneDef_s> {
public:
	int getLocalBoneIndexForBoneName(u16 boneName) const {
		const boneDef_s *d = getArray();
		for(u32 i = 0; i < size(); i++, d++) {
			if(d->nameIndex == boneName)
				return i;
		}
		return -1;
	}
};

#endif // __SKELUTILS_H__
