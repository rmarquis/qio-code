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
// rf_main.cpp
#include "rf_local.h"
#include "rf_drawCall.h"
#include "rf_world.h"
#include <shared/autocvar.h>

static aCvar_c rf_enableMultipassRendering("rf_enableMultipassRendering","0");
static aCvar_c rf_shadows("rf_shadows","0");

bool RF_IsUsingShadowVolumes() {
	// TODO: see if the stencil buffer is supported
	if(rf_shadows.getInt() == 1) {
		return true;
	}
	return false;
}
void RF_AddGenericDrawCalls() {
	RF_AddWorldDrawCalls();
	RFE_AddEntityDrawCalls();
	RF_AddWorldDecalDrawCalls();
}
void RF_GenerateDepthBufferOnlySceneDrawCalls() {
	rf_bDrawOnlyOnDepthBuffer = true;
	RF_AddWorldDrawCalls();
	RFE_AddEntityDrawCalls();
	RF_AddWorldDecalDrawCalls();
	rf_bDrawOnlyOnDepthBuffer = false;
}
void RF_Draw3DView() {
	if(rf_enableMultipassRendering.getInt() == 0) { 
		RF_AddGenericDrawCalls();
	} else {
		if(0) {
			// generate prelit world drawcalls
			RF_AddGenericDrawCalls();
		} else {
			// draw on depth buffer
			RF_GenerateDepthBufferOnlySceneDrawCalls();
		}
		// add drawcalls of light interactions
		RFL_AddLightInteractionsDrawCalls();
	}
	// first draw sky (without writing to the depth buffer)
	if(RF_HasSky()) {
		RF_DrawSky();
	}
	// sort and issue drawcalls (transparency rendering)
	RF_SortAndIssueDrawCalls();
	// do a debug drawing on top of everything
	RF_DoDebugDrawing();
}