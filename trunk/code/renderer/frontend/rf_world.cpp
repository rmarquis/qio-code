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
// rf_world.h - functions used for all world map types (.bsp, .map, .proc...)
#include "rf_bsp.h"

static class rBspTree_c *r_bspTree = 0;

void RF_ClearWorldMap() {
	if(r_bspTree) {
		delete r_bspTree;
		r_bspTree = 0;
	}
}
bool RF_LoadWorldMap(const char *name) {
	RF_ClearWorldMap();
	r_bspTree = RF_LoadBSP(name);
	if(r_bspTree)
		return false; // ok
	return true; // error
}
void RF_AddWorldDrawCalls() {
	if(r_bspTree) {
		r_bspTree->addDrawCalls();
	}
}
void RF_RayTraceWorld(class trace_c &tr) {
	if(r_bspTree) {
		r_bspTree->traceRay(tr);
	}
}
void RF_SetWorldAreaBits(const byte *bytes, u32 numBytes) {
	if(r_bspTree) {
		r_bspTree->setWorldAreaBits(bytes,numBytes);
	}
}


