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
#include <shared/autoCmd.h>
#include <shared/waveForm.h>
#include <shared/textureWrapMode.h>

class msIMPL_c : public materialSystemAPI_i {
public:
	virtual void initMaterialsSystem() {
		AUTOCVAR_RegisterAutoCvars();
		AUTOCMD_RegisterAutoConsoleCommands();
		MAT_ScanForMaterialFiles();
	}
	virtual void shutdownMaterialsSystem() {
		AUTOCVAR_UnregisterAutoCvars();
		AUTOCMD_UnregisterAutoConsoleCommands();
		MAT_FreeAllMaterials();
		MAT_FreeAllTextures();
		MAT_FreeAllCubeMaps();
		MAT_FreeCachedMaterialsTest();
	}
	virtual mtrAPI_i *registerMaterial(const char *matName) {
		return MAT_RegisterMaterialAPI(matName);
	}
	virtual textureAPI_i *getDefaultTexture() {
		return MAT_GetDefaultTexture();
	}
	virtual bool isMaterialOrImagePresent(const char *matName) {
		return MAT_IsMaterialOrImagePresent(matName);
	}
	virtual class textureAPI_i *loadTexture(const char *fname) {
		return MAT_RegisterTexture(fname,TWM_REPEAT);
	}
	virtual textureAPI_i *createLightmap(int index, const byte *data, u32 w, u32 h, bool rgba) {
		return MAT_CreateLightmap(index,data,w,h,rgba);
	}
	virtual mtrAPI_i *getDefaultMaterial() {
		return MAT_RegisterMaterialAPI("default");
	}
	virtual void reloadSingleMaterial(const char *matName) {
		MAT_ReloadSingleMaterial(matName);
	}
	virtual void reloadMaterialFileSource(const char *mtrSourceFileName) {
		MAT_ReloadMaterialFileSource(mtrSourceFileName);
	}
	virtual void iterateAllAvailableMaterialNames(void (*callback)(const char *s)) const {
		MAT_IterateAllAvailableMaterialNames(callback);
	}
	virtual void iterateAllAvailableMaterialFileNames(void (*callback)(const char *s)) const {
		MAT_IterateAllAvailableMaterialFileNames(callback);
	}
	// for Quake1 / HalfLife .wad / .bsp textures
	virtual class mtrAPI_i *createHLBSPTexture(const char *newMatName, const byte *pixels, u32 width, u32 height, const byte *palette) {
		return MAT_CreateHLBSPTexture(newMatName,pixels,width,height,palette);
	}
	// Doom3 material tables interface
	virtual const class tableListAPI_i *getTablesAPI() const {
		return MAT_GetTablesAPI();
	}
	// cubemap loader access (for "env_cubemap" handling in renderer)
	virtual class cubeMapAPI_i *registerCubeMap(const char *cubeMapName, bool forceReload) {
		return MAT_RegisterCubeMap(cubeMapName,forceReload);
	}
	virtual u32 getNumAllocatedMaterials() const {
		return MAT_GetNumAllocatedMaterials();
	}
	virtual mtrAPI_i *getAllocatedMaterial(u32 i) const {
		return MAT_GetAllocatedMaterial(i);
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

	waveForm_c::initTables();
}

qioModule_e IFM_GetCurModule() {
	return QMD_MATERIALSYSTEM;
}

