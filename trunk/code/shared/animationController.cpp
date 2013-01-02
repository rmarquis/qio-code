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
// animationController.cpp - animation blending code
#include "animationController.h"
#include <shared/autoCvar.h>
#include <api/coreAPI.h>
#include <api/skelModelAPI.h>
#include <api/skelAnimAPI.h>

aCvar_c anim_printAnimCtrlTime("anim_printAnimCtrlTime","0");
aCvar_c anim_printAnimChange("anim_printAnimChange","0");

void skelAnimController_c::getSingleLoopAnimLerpValuesForTime(singleAnimLerp_s &out, const class skelAnimAPI_i *anim, float time) {
	if(anim->getNumFrames() == 1) {
		out.from = 0;
		out.to = 0;
		out.frac = 0.f;
		return;
	}
	float step = anim->getFrameTime();
	float cur = 0;
	float next = step;
	for(u32 i = 0; i < anim->getNumFrames(); i++) {
		if(time >= cur && time < next) {
			out.from = i;
			out.to = i + 1;
			if(out.to == anim->getNumFrames()) {
				out.to = 0;
			}
			out.frac = (time - cur) / step;
			return;
		}
		next += step;
		cur += step;
	}
}
void skelAnimController_c::resetToAnim(const class skelAnimAPI_i *newAnim, int curGlobalTimeMSec) {
	time = 0.f;
	anim = newAnim;
	nextAnim = anim;
	lastUpdateTime = curGlobalTimeMSec;
}
void skelAnimController_c::setNextAnim(const class skelAnimAPI_i *newAnim, const class skelModelAPI_i *skelModel, int curGlobalTimeMSec) {
	if(anim == newAnim)
		return;
	if(nextAnim == newAnim)
		return;
	if(anim_printAnimChange.getInt()) {
		g_core->Print("setNextAnim: %i, %s\n",curGlobalTimeMSec,newAnim->getName());
	}
	// build previous bone state which we will use
	// to interpolate between old animation state and newAnim
	oldBonesState.resize(anim->getNumBones());
	if(nextAnim != anim) {
		g_core->RedWarning("skelAnimController_c::setNextAnim: havent finished lerping the previous animation change; jittering might be visible\n");
		updateModelAnimationLocal(skelModel,oldBonesState);
	} else {
		if(anim->getBLoopLastFrame() && (time > anim->getTotalTimeSec())) {
			//oldState.from = oldState.to = anim->getNumFrames()-1;
			//oldState.frac = 0.f;
			anim->buildFrameBonesLocal(anim->getNumFrames()-1,oldBonesState);
		} else {
			singleAnimLerp_s oldStateTMP;
			getSingleLoopAnimLerpValuesForTime(oldStateTMP,anim,time);
			anim->buildLoopAnimLerpFrameBonesLocal(oldStateTMP,oldBonesState);
		}
	}
	time = 0;
	blendTime = 0.1f;
	nextAnim = newAnim;
}
void skelAnimController_c::runAnimController(int curGlobalTimeMSec) {
	int deltaTime = curGlobalTimeMSec - lastUpdateTime;
	if(deltaTime == 0)
		return;
	lastUpdateTime = curGlobalTimeMSec;
	float deltaTimeSec = float(deltaTime) * 0.001f;
	time += deltaTimeSec;
	if(anim == nextAnim) {
		if(anim->getBLoopLastFrame()) {
			return; // dont loop this animation, just stop at the last frame
		} else {
			while(time > anim->getTotalTimeSec()) {
			//	g_core->Print("Clamping time %f by %f\n",time,anim->getTotalTimeSec());
				time -= anim->getTotalTimeSec();
			}
			if(anim_printAnimCtrlTime.getInt()) {
				g_core->Print("Final time: %f\n",time);
			}
		}
	} else {
		while(time > blendTime) {
			time -= blendTime;
			anim = nextAnim;
		}
	}
}
void skelAnimController_c::updateModelAnimationLocal(const class skelModelAPI_i *skelModel, boneOrArray_c &outLocalBones) {
	if(anim == nextAnim) {
		outLocalBones.resize(anim->getNumBones());
		if(anim->getBLoopLastFrame() && (time > ((anim->getNumFrames()-1)*anim->getFrameTime()))) {
			anim->buildFrameBonesLocal(anim->getNumFrames()-1,outLocalBones);			
		} else {
			singleAnimLerp_s lerp;
			getSingleLoopAnimLerpValuesForTime(lerp,anim,time);
			//g_core->Print("From %i to %i - %f\n",lerp.from,lerp.to,lerp.frac);

			//anim->buildFrameBonesLocal(lerp.from,bones);
			anim->buildLoopAnimLerpFrameBonesLocal(lerp,outLocalBones);
		}
	} else {
		// build old skeleton first
		// ( TODO: we might do it once and just store the bones here...)
		//boneOrArray_c previous;
		//previous.resize(anim->getNumBones());
		//anim->buildLoopAnimLerpFrameBonesLocal(oldState,previous);
		
		// build skeleton for the first frame of new animation
		boneOrArray_c newBones;
		newBones.resize(nextAnim->getNumBones());
		nextAnim->buildFrameBonesLocal(0,newBones);
		
		// do the interpolation
		float frac = this->time / this->blendTime;
	//	g_core->Print("Blending between two anims, frac: %f\n",frac);	
		outLocalBones.setBlendResult(oldBonesState, newBones, frac);		
	}
}
void skelAnimController_c::updateModelAnimation(const class skelModelAPI_i *skelModel) {
	// create bone matrices (relative to their parents)
	updateModelAnimationLocal(skelModel,currentBonesArray);
	// convert relative bones matrices to absolute bone matrices
	currentBonesArray.localBonesToAbsBones(anim->getBoneDefs());
	if(skelModel->hasCustomScaling()) {
		currentBonesArray.scaleXYZ(skelModel->getScaleXYZ());
	}
}
const boneOrArray_c &skelAnimController_c::getCurBones() const {
	return currentBonesArray;
}
