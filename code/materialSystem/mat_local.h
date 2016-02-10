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
// mat_local.h - materialSystem local functions

#ifndef __MAT_LOCAL_H__
#define __MAT_LOCAL_H__

#include <shared/typedefs.h>

struct matTextDef_s {
	const char *sourceFile; // name of the source .shader / .mtr file
	const char *p; // pointer to the first '{' (the one after material name)
	const char *textBase; // pointer to the start of cached material file text

	void clear() {
		sourceFile = 0;
		p = 0;
		textBase = 0;
	}
	matTextDef_s() {
		clear();
	}
};

// mat_main.cpp
void MAT_ScanForMaterialFiles();
class mtrAPI_i *MAT_RegisterMaterialAPI(const char *matName);
class mtrAPI_i *MAT_FindLoadedMaterialAPI(const char *matName);
bool MAT_IsMaterialOrImagePresent(const char *matName);
// returns true if material text is found
bool MAT_FindMaterialText(const char *matName, matTextDef_s &out);
void MAT_ReloadSingleMaterial(const char *matName);
void MAT_ReloadMaterialFileSource(const char *mtrSourceFileName);
void MAT_IterateAllAvailableMaterialNames(void (*callback)(const char *s));
void MAT_IterateAllAvailableMaterialFileNames(void (*callback)(const char *s));
class mtrAPI_i *MAT_CreateHLBSPTexture(const char *newMatName, const byte *pixels, u32 width, u32 height, const byte *palette);
// Doom3 material tables interface
const class tableListAPI_i *MAT_GetTablesAPI();
u32 MAT_GetNumAllocatedMaterials();
mtrAPI_i *MAT_GetAllocatedMaterial(u32 i);
u32 MAT_GetNumCachedMaterialFiles();
const char *MAT_GetMaterialFileName(u32 i);
void MAT_CacheAllMaterialsFromMatFile(const char *fname);
void MAT_ClearMaterialInUseFlags();
void MAT_FreeUnusedMaterials();

// mat_textures.cpp
class textureAPI_i *MAT_GetDefaultTexture();
class textureAPI_i *MAT_CreateLightmap(int index, const byte *data, u32 w, u32 h, bool rgba); // for lightmaps
class textureAPI_i *MAT_RegisterTexture(const char *texString, enum textureWrapMode_e wrapMode);
class textureAPI_i *MAT_CreateTexture(const char *texName, const byte *picData, u32 w, u32 h);
void MAT_FreeTexture(class textureAPI_i **p);
void MAT_FreeAllTextures();
void MAT_FreeAllMaterials();
void MAT_FreeCachedMaterialsTest();

// mat_texturesScript.cpp
class textureAPI_i *MAT_ParseImageScript(class parser_c &p);

// mat_cubeMap.cpp
class cubeMapAPI_i *MAT_RegisterCubeMap(const char *texName, bool forceReload = false);
void MAT_FreeAllCubeMaps();

#endif // __MAT_LOCAL_H__
