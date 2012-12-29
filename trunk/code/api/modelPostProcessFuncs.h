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
// modelPostProcessFuncs.h - functions used by model postprocessing code
// (scaling, rotating, etc)
#ifndef __MODELPOSTPROCESSFUNCS_H__
#define __MODELPOSTPROCESSFUNCS_H__

#include <shared/typedefs.h>

class modelPostProcessFuncs_i {
public:
	virtual void scaleXYZ(float scale) = 0;
	virtual void swapYZ() = 0;
	virtual void translateY(float ofs) = 0;
	virtual void multTexCoordsY(float f) = 0;
	virtual void multTexCoordsXY(float f) = 0;
	virtual void translateXYZ(const class vec3_c &ofs) = 0;
	virtual void getCurrentBounds(class aabb &out) = 0;
	virtual void setAllSurfsMaterial(const char *newMatName) = 0;
	virtual u32 getNumSurfs() const = 0;
	virtual void setSurfsMaterial(const u32 *surfIndexes, u32 numSurfIndexes, const char *newMatName) = 0;
	virtual void recalcBoundingBoxes() = 0;

	// optional, per-surface functions
	virtual bool hasPerSurfaceFunctionsImplemented() const {
		return false;
	}
	virtual void setNumSurfs(u32 newSurfsCount) {

	}
	virtual void resizeSurfaceVerts(u32 surfNum, u32 numVerts)  {

	}
	virtual void setSurfaceVert(u32 surfNum, u32 vertIndex, const float *xyz, const float *st)  {

	}
	virtual void setSurfaceIndicesU32(u32 surfNum, u32 numIndices, const u32 *indices)  {

	}
	virtual void setSurfaceMaterial(u32 surfNum, const char *material) {

	}
};

#endif // __MODELPOSTPROCESSFUNCS_H__
