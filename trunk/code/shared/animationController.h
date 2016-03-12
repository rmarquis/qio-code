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
// animationController.h - animation blending code
#ifndef __ANIMATION_CONTROLLER_H__
#define __ANIMATION_CONTROLLER_H__

#include <shared/skelUtils.h>

// unfinished animation blender
class skelAnimController_c {
	float time; // in seconds
	const class skelAnimAPI_i *anim;
	int flags;
	int nextFlags;
	int lastUpdateTime;
	// extra variables for blending between new and old animation
	//singleAnimLerp_s oldState;
	boneOrArray_c oldBonesState;
	const class skelAnimAPI_i *nextAnim;
	float blendTime;

	// bones that were used to create model instance in last update
	boneOrArray_c currentBonesArray;

	static void getSingleLoopAnimLerpValuesForTime(struct singleAnimLerp_s &out, const class skelAnimAPI_i *anim, float time);
public:
	skelAnimController_c();
	void resetToAnim(const class skelAnimAPI_i *newAnim, int curGlobalTimeMSec, int newFlags);
	void setNextAnim(const class skelAnimAPI_i *newAnim, const class skelModelAPI_i *skelModel, int curGlobalTimeMSec, int newFlags);
	void runAnimController(int curGlobalTimeMSec);
	void updateModelAnimationLocal(const class skelModelAPI_i *skelModel, boneOrArray_c &outLocalBones);
	void updateModelAnimation(const class skelModelAPI_i *skelModel, bool transformToABS);
	const boneOrArray_c &getCurBones() const;
	const skelAnimAPI_i *getAnim() const {
		return anim;
	}
};

#endif // __ANIMATION_CONTROLLER_H__
