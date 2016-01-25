/*
============================================================================
Copyright (C) 2016 V.

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
// wolfAnimCfg.h - RTCW wolfAnim.cfg files loader
#ifndef __SHARED_WOLFANIMATIONCONFIG_H__
#define __SHARED_WOLFANIMATIONCONFIG_H__

#include <shared/typedefs.h>
#include <shared/array.h>
#include <shared/str.h>

// first frame, num frames, looping frames, frames per second
struct wolfAnimDef_s {
	str name; // added for RTCW
	u32 firstFrame;
	u32 numFrames;
	u32 loopingFrames;
	float FPS;
	float frameTime;
	float totalTime;
	// ADDED for RTCW
	float moveSpeed;

	inline void calcFrameTimeFromFPS() {
		frameTime = 1.f / FPS;
	}
	inline void calcTotalTimeFromFrameTime() {
		totalTime = float(numFrames) * frameTime;
	}
};

class wolfAnimCfg_c {
protected:
	arraySTD_c<wolfAnimDef_s> anims;
public:
	bool parse(const char *fileName);
	bool findWolfAnimData(const char *animName, int *firstFrame, int *lastFrame, float *fps) const;

	const struct wolfAnimDef_s *getAnimCFGForIndex(u32 localAnimIndex) const {
		if(anims.size() <= localAnimIndex)
			return 0;
		return &anims[localAnimIndex];
	}
};

#endif // __SHARED_WOLFANIMATIONCONFIG_H__

