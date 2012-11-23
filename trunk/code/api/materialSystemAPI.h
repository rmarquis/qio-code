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
	virtual class textureAPI_i *createLightmap(const byte *data, u32 w, u32 h) = 0;
	virtual class textureAPI_i *getDefaultTexture() = 0;
	virtual bool isMaterialOrImagePresent(const char *matName) = 0;
};

extern materialSystemAPI_i *g_ms;

#endif // __MATERIALSYSTEM_API_H__
