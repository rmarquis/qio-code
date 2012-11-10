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
#include "rf_surface.h"
#include <api/coreAPI.h>
#include <api/modelLoaderDLLAPI.h>

static class rBspTree_c *r_bspTree = 0; // for .bsp files
static class r_model_c *r_worldModel = 0; // for .map files (converted to trimeshes)

void RF_ClearWorldMap() {
	if(r_bspTree) {
		delete r_bspTree;
		r_bspTree = 0;
	}
	if(r_worldModel) {
		delete r_worldModel;
		r_worldModel = 0;
	}
}
bool RF_LoadWorldMap(const char *name) {
	RF_ClearWorldMap();
	const char *ext = G_strgetExt(name);
	if(ext == 0) {
		g_core->RedWarning("RF_LoadWorldMap: %s has no extension\n",name);
		return true;
	}
	if(!stricmp(ext,"bsp")) {
		r_bspTree = RF_LoadBSP(name);
		if(r_bspTree)
			return false; // ok
		return true; // error
	} else if(g_modelLoader->isStaticModelFile(name)) {
		r_model_c *m = new r_model_c;
		if(g_modelLoader->loadStaticModelFile(name,m)) {
			delete m;
			return true; // error
		}
		m->createVBOsAndIBOs();
		r_worldModel = m;
		return false; // ok
	}
	g_core->RedWarning("Cannot load worldmap %s\n",name);
	return true; // error
}
void RF_AddWorldDrawCalls() {
	if(r_bspTree) {
		r_bspTree->addDrawCalls();
	}
	if(r_worldModel) {
		r_worldModel->addDrawCalls();
	}
}
void RF_RayTraceWorld(class trace_c &tr) {
	if(r_bspTree) {
		r_bspTree->traceRay(tr);
	}
	if(r_worldModel) {
		r_worldModel->traceRay(tr);
	}
}
void RF_SetWorldAreaBits(const byte *bytes, u32 numBytes) {
	if(r_bspTree) {
		r_bspTree->setWorldAreaBits(bytes,numBytes);
	}
}


