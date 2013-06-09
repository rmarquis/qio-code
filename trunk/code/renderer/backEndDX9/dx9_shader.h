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
// dx9_shader.cpp
#ifndef __DX9_SHADER_H__
#define __DX9_SHADER_H__

#include "dx9_local.h"

// HLSL shaders can be compiled with various 
// options and defines
struct hlslPermutationFlags_s {
	// draw colormap with lightmap
	bool hasLightmap; // #define HAS_LIGHTMAP 
	bool onlyLightmap; // draw ONLY lightmap
	bool hasVertexColors; // #define HAS_VERTEXCOLORS
	bool hasTexGenEnvironment; // #define HAS_TEXGEN_ENVIROMENT
	bool hasMaterialColor; // #define HAS_MATERIAL_COLOR

	hlslPermutationFlags_s() {
		memset(this,0,sizeof(*this));
	}
};


class hlslShader_c {
friend class rbDX9_c;
	str name;
	ID3DXEffect *effect;

	hlslPermutationFlags_s permutations;
public:
	hlslShader_c() {
		effect = 0;
	}
	~hlslShader_c() {
		if(effect) {
			effect->Release();
			effect = 0;
		}
	}
	const char *getName() const {
		return name;
	}
	const hlslPermutationFlags_s &getPermutations() const {
		return permutations;
	}
	bool isValid() const {
		if(effect)
			return true;
		return false;
	}

friend hlslShader_c *DX9_RegisterShader(const char *baseName, const hlslPermutationFlags_s *p = 0);
friend void DX9_ShutdownHLSLShaders();
};




#endif // __DX9_SHADER_H__
