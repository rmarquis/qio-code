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
// wolfAnimCfg.cpp - RTCW character wolfAnim.cfg parser
#include "wolfAnimCfg.h"
#include <shared/parser.h>

bool wolfAnimCfg_c::findWolfAnimData(const char *animName, int *firstFrame, int *lastFrame, float *fps) const {
	for(u32 i = 0; i < anims.size(); i++) {
		if(!stricmp(anims[i].name,animName)) {
			*firstFrame = anims[i].firstFrame;
			*lastFrame = anims[i].firstFrame + anims[i].numFrames;
			*fps = anims[i].FPS;
			return false;
		}
	}
	return true;
}
bool wolfAnimCfg_c::parse(const char *fileName) {
	parser_c p;
	if(p.openFile(fileName)) {
		return true;
	}
	while(p.atEOF() == false) {
		if(p.atWord("VERSION")) {
			p.skipLine();
		} else if(p.atWord("SKELETAL")) {
			p.skipLine();
		} else if(p.atWord("STARTANIMS")) {
			p.skipLine();
		} else if(p.atWord("ENDANIMS")) {
			p.skipLine();
		} else {
			u32 animNum = anims.size();
			wolfAnimDef_s &nextAnim = anims.pushBack();
			nextAnim.name = p.getToken();
			nextAnim.firstFrame = p.getInteger();
			nextAnim.numFrames = p.getInteger();
			nextAnim.loopingFrames = p.getInteger();
			nextAnim.FPS = p.getFloat();
			nextAnim.moveSpeed = p.getFloat();
			// there is also optional "transition" token
			if(p.isAtEOL() == false) {
				// 0?
				nextAnim.transition = p.getInteger();
			} else {
				nextAnim.transition = -1; // default value?
			}
			nextAnim.calcFrameTimeFromFPS();
			nextAnim.calcTotalTimeFromFrameTime();	
			g_core->Print("wolfAnimCfg_c::parse: anim %i is %s\n",anims.size(),nextAnim.name.c_str());
		}
	}
	g_core->Print("wolfAnimCfg_c::parse: %i animations loaded\n",anims.size());
	return false; // no error
}