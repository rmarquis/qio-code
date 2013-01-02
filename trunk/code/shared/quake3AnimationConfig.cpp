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
// quake3AnimationConfig.cpp - quake3 character animation.cfg parser
#include "quake3AnimationConfig.h"
#include <shared/parser.h>
#include <shared/quake3Anims.h>

bool q3AnimCfg_c::parse(const char *fileName) {
	parser_c p;
	u32 skip;
	if(p.openFile(fileName)) {
		return true;
	}
	while(p.atEOF() == false) {
		if(p.atWord("headoffset")) {
			p.skipLine();
		} else if(p.atWord("sex")) {
			p.skipLine();
		} else {
			u32 animNum = anims.size();
			q3AnimDef_s &nextAnim = anims.pushBack();
			nextAnim.firstFrame = p.getInteger();
			nextAnim.numFrames = p.getInteger();
			nextAnim.loopingFrames = p.getInteger();
			nextAnim.FPS = p.getFloat();
			nextAnim.calcFrameTimeFromFPS();
			nextAnim.calcTotalTimeFromFrameTime();	
			
			// leg only frames are adjusted to not count the upper body only frames
			if ( animNum == LEGS_WALKCR ) {
				skip = anims[LEGS_WALKCR].firstFrame - anims[TORSO_GESTURE].firstFrame;
			}
			if ( animNum >= LEGS_WALKCR && animNum < TORSO_GETFLAG) {
				nextAnim.firstFrame -= skip;
			}
		}
	}
	return false; // no error
}