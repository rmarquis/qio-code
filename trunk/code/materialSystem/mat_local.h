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
// returns true if material text is found
bool MAT_FindMaterialText(const char *matName, matTextDef_s &out);
// mat_textures.cpp
class textureAPI_i *MAT_GetDefaultTexture();
class textureAPI_i *MAT_CreateLightmap(const byte *data, u32 w, u32 h); // for lightmaps
class textureAPI_i *MAT_RegisterTexture(const char *texString);
void MAT_FreeTexture(class textureAPI_i **p);
void MAT_FreeAllTextures();
void MAT_FreeAllMaterials();

#endif // __MAT_LOCAL_H__
