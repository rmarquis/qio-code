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
// kfAnimCtrl.h - simple animation controler for keyframed (md3, md2) models
#ifndef __SHARED_KFANIMCTRL_H__
#define __SHARED_KFANIMCTRL_H__

#include "typedefs.h"
#include "singleAnimLerp.h"
#include <renderer/rModelAPI.h>

struct kfAnimCtrl_s {
	int prevTimeMsec;
	float curTime; // in seconds
	const q3AnimDef_s *animDef;
	singleAnimLerp_s curLerp;

	kfAnimCtrl_s() {
		curTime = 0;
		animDef = 0;
		prevTimeMsec = 0;
	}
	void setAnim(u32 animIndex, const class rModelAPI_i *model) {
		const q3PlayerModelAPI_i *q3model = model->getQ3PlayerModelAPI();
		const q3AnimDef_s *newAnimDef = q3model->getAnimCFGForIndex(animIndex);
		if(newAnimDef == animDef) {
			return; // no change
		}
		animDef = newAnimDef;
		curTime = 0;
		//prevTimeMsec = curGlobalTimeMSec;
	}
	void runAnimController(int curGlobalTimeMSec) {
		if(prevTimeMsec >= curGlobalTimeMSec)
			return;
		if(animDef == 0) {
			prevTimeMsec = curGlobalTimeMSec;
			return;
		}
		int deltaMsec = curGlobalTimeMSec - prevTimeMsec;
		prevTimeMsec = curGlobalTimeMSec;
		float deltaTimeSec = float(deltaMsec) * 0.001f;
		curTime += deltaTimeSec;
		float t = curTime;
		u32 first = 0;
		while(t > animDef->frameTime) {
			t -= animDef->frameTime;
			first ++;
			if(first == animDef->numFrames) {
				first = 0;
				curTime -= animDef->totalTime;
			}
		}
		u32 next = first + 1;
		if(next >= animDef->numFrames) {
			next = 0;
		}
		curLerp.from = animDef->firstFrame + first;
		curLerp.to = animDef->firstFrame + next;
		curLerp.frac = t / animDef->frameTime;
		//g_core->Print("From %i, to %i, frac %f\n",curLerp.from,curLerp.to,curLerp.frac);
	}
};

#endif // __SHARED_KFANIMCTRL_H__
