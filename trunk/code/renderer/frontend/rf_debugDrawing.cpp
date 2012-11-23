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
// rf_debugDrawing.cpp
#include "rf_local.h"
#include <qcommon/q_shared.h>
#include <shared/array.h>
#include <shared/autoCvar.h>
#include <api/iFaceMgrAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/gameAPI.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>

static aCvar_c rf_showEntityABSBounds("rf_showEntityABSBounds","0");

// draw debug info for game module 
// this works (obviously) only for local client
// on non-dedicated server.
// The clientside prediction/interpolation
// does not affect orientations of displayed objects.
void RF_GameDebugDrawing() {
	if(g_game->DebugDrawFrame == 0)
		return;
	g_game->DebugDrawFrame(rf);
}
void RF_DoDebugDrawing() {
	if(g_game) {
		RF_GameDebugDrawing();
	}
	rb->unbindMaterial();
	RFDL_DrawDebugLines();
	if(rf_showEntityABSBounds.getInt()) {
		float redColor [4] = { 1, 0, 0, 1 };
		rb->setColor4(redColor);
		RFE_DrawEntityAbsBounds();
	}
}

//
// debug lines
//
struct rDebugLine_s {
	vec3_c from;
	vec3_c to;
	vec3_c color;
	int endTime;
};
static arraySTD_c<rDebugLine_s> rf_debugLines;

u32 RFDL_AddDebugLine(const vec3_c &from, const vec3_c &to, const vec3_c &color, float life) {
	u32 ret;
	rDebugLine_s *next = rf_debugLines.getArray();
	for(ret = 0; ret < rf_debugLines.size(); ret++, next++) {
		if(next->endTime < rf_curTimeMsec) {
			break;
		}
	}
	if(ret == rf_debugLines.size()) {
		next = &rf_debugLines.pushBack();
	}
	next->from = from;
	next->to = to;
	next->color = color;
	next->endTime = rf_curTimeMsec + (life*1000.f);
	return ret;
}
void RFDL_DrawDebugLines() {
	const rDebugLine_s *l = rf_debugLines.getArray();
	for(u32 i = 0; i < rf_debugLines.size(); i++, l++) {
		if(l->endTime < rf_curTimeMsec)
			continue;
		rb->drawLineFromTo(l->from,l->to,l->color);
	}
}







