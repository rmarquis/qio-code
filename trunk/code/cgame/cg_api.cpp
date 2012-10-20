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
// cg_api.cpp - cgame DLL entry point

#include "cg_local.h"
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/clientAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/cgameAPI.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>
#include <api/cmAPI.h>

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
clAPI_s *g_client = 0;
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
rAPI_i *rf = 0;
rbAPI_i *rb = 0;
cmAPI_i *cm = 0;
// exports
static cgameAPI_s g_staticCGameAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_staticCGameAPI.Init = CG_Init;
	g_staticCGameAPI.Shutdown = CG_Shutdown;
	g_staticCGameAPI.DrawActiveFrame = CG_DrawActiveFrame;
	g_iFaceMan->registerInterface(&g_staticCGameAPI,CGAME_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_client,CLIENT_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rf,RENDERER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rb,RB_SDLOPENGL_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&cm,CM_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_CGAME;
}

