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
// rd_api.cpp
#include <api/iFaceMgrAPI.h>

//
#include <api/vfsAPI.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>
#include <api/rdAPI.h>
//#include <api/editorAPI.h>
//#include <api/declManagerAPI.h>
//#include <shared/autoCvar.h>
//#include <api/materialSystemAPI.h>
//#include <shared/autoCmd.h>
//#include <api/mtrAPI.h>
//#include <api/mtrStageAPI.h>
//#include <api/materialSystemAPI.h>
class rdIMPL_c : public rdAPI_i {
public:
	virtual void init() {

	}
	virtual void shutdown() {

	}
};


// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
////cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
rAPI_i *rf = 0;
rbAPI_i *rb = 0;
//materialSystemAPI_i *g_ms = 0;
//declManagerAPI_i *g_declMgr = 0;
// exports
static rdIMPL_c g_staticRDAPI;
rdAPI_i *rd = &g_staticRDAPI;


void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;
//
//	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)rd,RD_API_IDENTSTR);
//
//	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
////	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rf,RENDERER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rb,RENDERER_BACKEND_API_IDENTSTR);
//	g_iFaceMan->registerIFaceUser(&g_ms,MATERIALSYSTEM_API_IDENTSTR);
//	g_iFaceMan->registerIFaceUser(&g_declMgr,DECL_MANAGER_API_IDENTSTR);
//
//	g_declMgr->loadAllEntityDecls();
//	g_core->Print("Editor detected %i ent defs\n",g_declMgr->getNumLoadedEntityDecls());
}

qioModule_e IFM_GetCurModule() {
	return QMD_RECASTNAVIGATION;
}

