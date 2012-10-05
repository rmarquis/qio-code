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

#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/gameAPI.h>
#include <api/rAPI.h>

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
}








