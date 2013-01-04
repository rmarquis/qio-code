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
// keyFramedModelImpl.h - per vertex model animation (Quake3 md3 style)
#ifndef __KEYFRAMEDMODELIMPL_H__
#define __KEYFRAMEDMODELIMPL_H__

#include <shared/array.h>
#include <shared/str.h>
#include <math/aabb.h>
#include <math/vec3.h>
#include <math/vec2.h>
#include <shared/tagOr.h> // for tags
#include <renderer/rIndexBuffer.h>

#include <api/kfModelAPI.h>

class kfVert_c {
public:
	vec3_c xyz;
	vec3_c normal;
};

class kfSurfFrame_c {
public:
	arraySTD_c<kfVert_c> verts;
};
class kfSurf_c : public kfSurfAPI_i {
public:
	arraySTD_c<vec2_c> texCoords;
	arraySTD_c<kfSurfFrame_c> xyzFrames;
	rIndexBuffer_c indices;
	//class mtrAPI_i *mat;
	str matName;
	str name;

	kfSurf_c() {
		//mat = 0;
	}

	virtual const char *getSurfName() const;
	virtual const char *getMatName() const;
	//virtual class mtrAPI_i *getMaterial() const;
	virtual const rIndexBuffer_c *getIBO() const;
	virtual u32 getNumVertices() const;
	virtual void copyTexCoords(void *outTC, u32 outStride) const;
	virtual void instanceSingleFrame(void *outXYZ, u32 outStride, u32 frameNum) const;
	virtual void instance(void *outXYZ, u32 outStride, u32 from, u32 to, float lerp) const;
};
class kfFrame_c {
public:
	str name;
	aabb bounds;
	float radius;
	vec3_c localOrigin;
};
class kfTagFrame_c {
public:
	arraySTD_c<tagOr_c> tags;
};
class kfModelImpl_c : public kfModelAPI_i {
public:
	str fname;
	arraySTD_c<kfSurf_c> surfs;
	arraySTD_c<kfFrame_c> frames;
	arraySTD_c<kfTagFrame_c> tagFrames;
	arraySTD_c<str> tagNames;

	virtual const char *getName() const {
		return fname;
	}
	virtual u32 getNumFrames() const {
		return frames.size();
	}
	virtual u32 getNumSurfaces() const {
		return surfs.size();
	}
	virtual const kfSurfAPI_i *getSurfAPI(u32 surfNum) const {
		return &surfs[surfNum];
	}
	virtual u32 fixFrameNum(u32 inFrameNum) const {
		if(inFrameNum >= frames.size())
			return frames.size()-1;
		return inFrameNum;
	}
	virtual int getTagIndexForName(const char *tagName) const {
		for(u32 i = 0; i < tagNames.size(); i++) {
			if(!stricmp(tagName,tagNames[i])) {
				return i;
			}
		}
		return -1;
	}
	virtual const tagOr_c *getTagOrientation(u32 tagNum, u32 frameNum) const {
		if(frameNum >= frames.size()) {
			frameNum = frames.size()-1;
		}
		return &tagFrames[frameNum].tags[tagNum];
	}
	virtual const class tagOr_c *getTagOrientation(const char *tagName, u32 frameNum) const {
		int index = getTagIndexForName(tagName);
		if(index == -1)
			return 0;
		return getTagOrientation(index,frameNum);
	}
	bool load(const char *fname);

	// famous Quake3 md3
	bool loadMD3(const char *fname);
	bool loadMD3(const byte *buf, const u32 fileLen, const char *fname);
	// Quake2 md2
	bool loadMD2(const char *fname);
	bool loadMD2(const byte *buf, const u32 fileLen, const char *fname);
};

kfModelImpl_c *KF_LoadKeyFramedModel(const char *fname);
bool KF_HasKeyFramedModelExt(const char *fname);

#endif // __KEYFRAMEDMODELIMPL_H__
