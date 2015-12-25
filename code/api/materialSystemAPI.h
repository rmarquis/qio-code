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
// materialSystemAPI.h - .shader / .mtr script parsing and evaluation
// Note that entire materialSystem is CLIENT-ONLY.

#ifndef __MATERIALSYSTEM_API_H__
#define __MATERIALSYSTEM_API_H__

#include "iFaceBase.h"

#define MATERIALSYSTEM_API_IDENTSTR "MaterialSystemAPI0001"

class materialSystemAPI_i : public iFaceBase_i {
public:
	virtual void initMaterialsSystem() = 0;
	virtual void shutdownMaterialsSystem() = 0;
	virtual class mtrAPI_i *registerMaterial(const char *matName) = 0;
	virtual class mtrAPI_i *getDefaultMaterial() = 0;
	virtual class textureAPI_i *loadTexture(const char *fname) = 0;
	virtual class textureAPI_i *createLightmap(int index, const byte *data, u32 w, u32 h, bool rgba = false) = 0;
	virtual class textureAPI_i *getDefaultTexture() = 0;
	virtual bool isMaterialOrImagePresent(const char *matName) = 0;
	virtual void reloadSingleMaterial(const char *matName) = 0;
	virtual void reloadMaterialFileSource(const char *mtrSourceFileName) = 0;
	// for console command autocompletion
	virtual void iterateAllAvailableMaterialNames(void (*callback)(const char *s)) const = 0;
	virtual void iterateAllAvailableMaterialFileNames(void (*callback)(const char *s)) const = 0;
	// creates new material for HL/Q1-bsp texture data
	virtual class mtrAPI_i *createHLBSPTexture(const char *newMatName, const byte *pixels, u32 width, u32 height, const byte *palette) = 0;
	// Doom3 material tables interface
	virtual const class tableListAPI_i *getTablesAPI() const = 0;
	// cubemap loader access (for "env_cubemap" handling in renderer)
	virtual class cubeMapAPI_i *registerCubeMap(const char *cubeMapName, bool forceReload = false) = 0;
	// for Editor
	virtual u32 getNumAllocatedMaterials() const = 0;
	virtual mtrAPI_i *getAllocatedMaterial(u32 i) const = 0;
};

extern materialSystemAPI_i *g_ms;

#endif // __MATERIALSYSTEM_API_H__
