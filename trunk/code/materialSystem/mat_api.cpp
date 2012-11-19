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
// mat_api.cpp - material system interface

#include "mat_local.h"
#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>
#include <api/imgAPI.h>
#include <api/materialSystemAPI.h>
#include <shared/autoCvar.h>

class msIMPL_c : public materialSystemAPI_i {
public:
	virtual void initMaterialsSystem() {
		MAT_ScanForMaterialFiles();
	}
	virtual void shutdownMaterialsSystem() {
		MAT_FreeAllMaterials();
		MAT_FreeAllTextures();
	}
	virtual mtrAPI_i *registerMaterial(const char *matName) {
		return MAT_RegisterMaterialAPI(matName);
	}
	virtual textureAPI_i *getDefaultTexture() {
		return MAT_GetDefaultTexture();
	}
	virtual textureAPI_i *createLightmap(const byte *data, u32 w, u32 h) {
		return MAT_CreateLightmap(data,w,h);
	}
	virtual mtrAPI_i *getDefaultMaterial() {
		return MAT_RegisterMaterialAPI("default");
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
rAPI_i *rf = 0;
rbAPI_i *rb = 0;
imgAPI_i *g_img = 0;
// exports
static msIMPL_c g_staticMaterialSystemAPI;
materialSystemAPI_i *g_ms = &g_staticMaterialSystemAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)g_ms,MATERIALSYSTEM_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rf,RENDERER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rb,RENDERER_BACKEND_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_img,IMG_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_MATERIALSYSTEM;
}
