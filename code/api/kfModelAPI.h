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
// keyFramedModelAPI.h - per vertex model animation (Quake3 md3 style) interface
#ifndef __KEYFRAMEDMODELAPI_H__
#define __KEYFRAMEDMODELAPI_H__

class kfSurfAPI_i {
public:
	virtual const char *getSurfName() const = 0;
	virtual const char *getMatName() const = 0;
	//virtual class mtrAPI_i *getMaterial() const = 0;
	virtual const class rIndexBuffer_c *getIBO() const = 0;
	virtual u32 getNumVertices() const = 0;
	virtual u32 getNumTriangles() const = 0;
	virtual void copyTexCoords(void *outTC, u32 outStride) const = 0;
	virtual void instanceSingleFrame(void *outXYZ, u32 outStride, u32 frameNum) const = 0;
	virtual void instance(void *outXYZ, u32 outStride, u32 from, u32 to, float lerp) const = 0;
};

class kfModelAPI_i {
public:
	virtual const char *getName() const = 0;
	virtual u32 getNumFrames() const = 0;
	virtual u32 getNumSurfaces() const = 0;
	virtual float getFrameTime() const = 0;
	virtual u32 getNumTags() const = 0;
	virtual u32 getTotalTriangleCount() const = 0;
	virtual const kfSurfAPI_i *getSurfAPI(u32 surfNum) const = 0;
	virtual u32 fixFrameNum(u32 inFrameNum) const = 0;
	virtual void scale(float f) = 0;
	virtual int getTagIndexForName(const char *tagName) const = 0;
	virtual const class tagOr_c *getTagOrientation(int tagNum, u32 frameNum) const = 0;
	virtual const class tagOr_c *getTagOrientation(const char *tagName, u32 frameNum) const = 0;
};

#endif // __KEYFRAMEDMODELAPI_H__