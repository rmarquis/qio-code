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
// skelAnimImpl.h - implementation of skelAnimAPI_i
#ifndef __SKELANIMIMPL_H__
#define __SKELANIMIMPL_H__

#include "sk_local.h"
#include <api/skelAnimAPI.h>
#include <math/aabb.h>
#include <shared/array.h>
#include <shared/str.h>
#include "skelAnimMD5.h"

class skelFrame_c {
friend class skelAnimGeneric_c;
	arraySTD_c<md5BoneVal_s> bones;
	aabb bounds;

	void setOrs(const class boneOrArray_c &ors);
};
class skelAnimGeneric_c : public skelAnimAPI_i {
	str animFileName;
	float frameRate; // fps
	float frameTime; // 1 / fps
	float totalTime; // frameTime * frames.size()
	arraySTD_c<skelFrame_c> frames;
	boneDefArray_c bones;
	arraySTD_c<md5BoneVal_s> baseFrame;
	int animFlags;

	virtual const char *getName() const {
		return animFileName;
	}
	virtual u32 getNumFrames() const {
		return frames.size();
	}
	virtual u32 getNumBones() const {
		return bones.size();
	}
	// boneDefs array might be not present for some other animation types than md5 (??)
	virtual const class boneDefArray_c *getBoneDefs() const {
		return &bones;
	}
	virtual float getFrameTime() const {
		return frameTime;
	}
	virtual float getTotalTimeSec() const {
		return totalTime;
	}
	virtual skelAnimAPI_i *createSubAnim(u32 firstFrame, u32 numFrames) const;
	// anim post process funcs impl
	virtual void scaleAnimationSpeed(float scale) {
		frameTime *= scale;
		frameRate = 1.f / frameTime;
		totalTime = frames.size() * frameTime;
	}
	virtual void setLoopLastFrame(bool bLoopLastFrame) {
		if(bLoopLastFrame) {
			animFlags |= AF_LOOP_LAST_FRAME;
		} else {
			animFlags &= ~AF_LOOP_LAST_FRAME;
		}
	}

	virtual bool getBLoopLastFrame() const {
		return animFlags & AF_LOOP_LAST_FRAME;
	}
	virtual int findBoneForBoneNameIndex(u32 boneNameIndex) const {
		return bones.getLocalBoneIndexForBoneName(boneNameIndex);
	}
	virtual int getLocalBoneIndexForBoneName(const char *nameStr) const {
		u16 nameIndex = SK_RegisterString(nameStr);
		return bones.getLocalBoneIndexForBoneName(nameIndex);
	}
	virtual int getBoneParentLocalIndex(int boneNum) const {
		return bones[boneNum].parentIndex;
	}
	virtual void addChildrenOf(arraySTD_c<u32> &list, const char *baseBoneName) const {
		int baseBoneIndex = getLocalBoneIndexForBoneName(baseBoneName);
		if(baseBoneIndex == -1)
			return;
		for(u32 i = 0; i < bones.size(); i++) {
			int p = i;
			while(p != -1) {
				if(p == baseBoneIndex) {
					list.add_unique(i);
					break;
				}
				p = bones[p].parentIndex;
			}
		}
	}
	virtual void removeChildrenOf(arraySTD_c<u32> &list, const char *baseBoneName) const {
		int baseBoneIndex = getLocalBoneIndexForBoneName(baseBoneName);
		if(baseBoneIndex == -1)
			return;
		for(u32 i = 0; i < bones.size(); i++) {
			int p = i;
			while(p != -1) {
				if(p == baseBoneIndex) {
					list.remove(i);
					break;
				}
				p = bones[p].parentIndex;
			}
		}
	}
	int registerBone(const char *boneName);
	void buildSingleBone(int boneNum, const skelFrame_c &f, class vec3_c &pos, class quat_c &quat) const;
	virtual void buildFrameBonesLocal(u32 frameNum, class boneOrArray_c &out, const class skelModelAPI_i *skelModel = 0) const;
	virtual void buildFrameBonesABS(u32 frameNum, class boneOrArray_c &out) const;
	virtual void buildLoopAnimLerpFrameBonesLocal(const struct singleAnimLerp_s &lerp, class boneOrArray_c &out, const class skelModelAPI_i *skelModel = 0) const;
	virtual void scaleAnimation(float s);
	void addFrameRelative(const class boneOrArray_c &ors);
public:
	skelAnimGeneric_c();
	virtual ~skelAnimGeneric_c();
	bool loadMDXAnim(const char *fname);
	bool loadMDSAnim(const char *fname);
	bool loadSMDAnim(const char *fname);
	bool loadSKAAnim(const char *fname);
	bool loadPSKAnim(const char *fname);
	bool loadPSAAnim(const char *fname);
	// not fully supported
	bool loadSKCAnim(const char *fname);

	bool getSKCData(class skc_c *out) const;
};

#endif //__SKELANIMIMPL_H__