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
// tiki_api.cpp - TIKI API implementation
#include "tiki_local.h"
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/cmAPI.h>
#include <api/tikiAPI.h>
#include <api/modelLoaderDLLAPI.h>
#include <api/moduleManagerAPI.h>

class tikiAPIImpl_c : public tikiAPI_i {
public:
	tikiAPIImpl_c() {

	}
	~tikiAPIImpl_c() {
	}

	virtual void init() {
		g_core->Print("====== tikiAPIImpl_c::init() ===== \n");

		g_core->Print("TIKI module ready!\n");
	}
	virtual void shutdown() {
		g_core->Print("====== tikiAPIImpl_c::shutdown() ===== \n");
	}
	virtual class cMod_i *registerModel(const char *modName) {

		return 0;
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
cmAPI_i *cm = 0;
modelLoaderDLLAPI_i *g_modelLoader = 0;
moduleManagerAPI_i *g_moduleMgr = 0;

// exports
static tikiAPIImpl_c g_staticTIKIAPI;
tikiAPI_i *tiki = &g_staticTIKIAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)tiki,TIKI_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_moduleMgr,MODULEMANAGER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_modelLoader,MODELLOADERDLL_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&cm,CM_API_IDENTSTR);

	//TIKI_AddConsoleCommands();
}

qioModule_e IFM_GetCurModule() {
	return QMD_TIKI;
}

