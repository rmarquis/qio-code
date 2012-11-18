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

//
// shared raw data structs
//
struct boneOr_s {
	matrix_c mat;
	u32 boneName;
};
// array of bone orientations
class boneOrArray_c :  public arraySTD_c<boneOr_s> {
public:
	// concat bone transforms
	void localBonesToAbsBones(const class boneDefArray_c *boneDefs);
	void setBlendResult(const boneOrArray_c &from, const boneOrArray_c &to, float frac);	

	void scale(float scale);
	void scaleXYZ(const vec3_c &scaleXYZ);
};

struct boneDef_s {
	u16 nameIndex;
	short parentIndex;
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
