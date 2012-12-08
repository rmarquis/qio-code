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

class hlslShader_c {
friend class rbDX9_c;
	str name;
	ID3DXEffect *effect;

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
	bool isValid() const {
		if(effect)
			return true;
		return false;
	}

friend hlslShader_c *DX9_RegisterShader(const char *baseName);
friend void DX9_ShutdownHLSLShaders();
};




#endif // __DX9_SHADER_H__
