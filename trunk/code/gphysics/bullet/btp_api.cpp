/*
============================================================================
Copyright (C) 2013 V.

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
// btp_api.cpp - Bullet Physics interface
#include <shared/array.h>
#include <shared/str.h>
#include <shared/autoCvar.h>
#include <api/iFaceMgrAPI.h>
#include <api/physAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/cmAPI.h>
#include <api/declManagerAPI.h>
#include <api/loadingScreenMgrAPI.h>
#include <api/rAPI.h>

#include "btp_world.h"


class physDLLBullet_c : public physDLLAPI_i {
	arraySTD_c<bulletPhysicsWorld_c*> worlds;
public:
	virtual void initPhysicsSystem() {
		AUTOCVAR_RegisterAutoCvars();
	}
	virtual void shutdownPhysicsSystem() {
		AUTOCVAR_UnregisterAutoCvars();
	}
	virtual physWorldAPI_i *allocWorld(const char *debugName) {
		bulletPhysicsWorld_c *ret = new bulletPhysicsWorld_c(debugName);
		worlds.push_back(ret);
		return ret;
	}
	virtual void freeWorld(physWorldAPI_i *w) {
		bulletPhysicsWorld_c *btw = dynamic_cast<bulletPhysicsWorld_c*>(w);
		worlds.remove(btw);
		delete btw;
	}
	virtual void doDebugDrawing(class rDebugDrawer_i *dd) { 
		for(u32 i = 0; i < worlds.size(); i++) {
			worlds[i]->doDebugDrawing(dd);
		}
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
cmAPI_i *cm = 0;
loadingScreenMgrAPI_i *g_loadingScreen = 0;
rAPI_i *rf = 0;
declManagerAPI_i *g_declMgr = 0;
// exports
static physDLLBullet_c g_staticPhysDLLImpl;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	g_iFaceMan->registerInterface((iFaceBase_i*)(void*)&g_staticPhysDLLImpl,GPHYSICS_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&cm,CM_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_loadingScreen,LOADINGSCREENMGR_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rf,RENDERER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_declMgr,DECL_MANAGER_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_GPHYSICS;
}

