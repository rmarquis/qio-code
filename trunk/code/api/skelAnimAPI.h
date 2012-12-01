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
// skelAnimAPI.h
#ifndef __SKELANIMAPI_H__
#define __SKELANIMAPI_H__

#include "skelAnimPostProcessFuncs.h"

enum {
	AF_LOOP_LAST_FRAME = 1, // stop the animation at the last frame instead of looping it
};

// used for single looping animations
struct singleAnimLerp_s {
	u16 from; // previous frame num
	u16 to; // next frame num
	float frac; // 0.0 - 1.0 lerp value
};

class skelAnimAPI_i : public skelAnimPostProcessFuncs_i {
public:
	virtual ~skelAnimAPI_i() { };
	virtual const char *getName() const = 0;
	virtual u32 getNumFrames() const = 0;
	virtual u32 getNumBones() const = 0;
	// boneDefs array might be not present for some other animation types than md5 (??)
	virtual const class boneDefArray_c *getBoneDefs() const = 0;
	virtual float getFrameTime() const = 0;
	virtual float getTotalTimeSec() const = 0;
	// returns true if animation should stop after reaching the last frame (instead of starting again)
	virtual bool getBLoopLastFrame() const = 0;
	virtual int getLocalBoneIndexForBoneName(const char *boneName) const = 0;
	virtual void addChildrenOf(arraySTD_c<u32> &list, const char *baseBoneName) const = 0;
	virtual void removeChildrenOf(arraySTD_c<u32> &list, const char *baseBoneName) const = 0;
	virtual void buildFrameBonesLocal(u32 frameNum, class boneOrArray_c &out) const = 0;
	virtual void buildFrameBonesABS(u32 frameNum, class boneOrArray_c &out) const = 0;
	// for looping animations
	virtual void buildLoopAnimLerpFrameBonesLocal(const struct singleAnimLerp_s &lerp, class boneOrArray_c &out) const = 0;
};

#endif // __SKELANIMAPI_H__