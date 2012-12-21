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
// gl_shader.h - GLSL shaders for openGL backend
#ifndef __GL_SHADER_H__
#define __GL_SHADER_H__

#include "gl_local.h"
#include <shared/str.h>

// GLSL shaders can be compiled with various 
// options and defines
struct permutationFlags_s {
	bool hasLightmap; // #define HAS_LIGHTMAP
	bool hasVertexColors; // #define HAS_VERTEXCOLORS
	bool hasTexGenEnvironment; // #define HAS_TEXGEN_ENVIROMENT

	permutationFlags_s() {
		memset(this,0,sizeof(*this));
	}
};

class glShader_c {
friend class rbSDLOpenGL_c;
	str name; // shader name
	GLuint handle; // openGL handle
	
	// uniform locations
	int uLightOrigin;
	int uLightRadius;
	int uViewOrigin;

	// sampler2D locations
	int sColorMap; // main diffuse texture
	int sLightMap; // Quake3 bsp lightmap

	permutationFlags_s permutations;

public:
	glShader_c() {
		handle = 0;
	}
	~glShader_c() {
		if(handle) {
			glDeleteProgram(handle);
		}
	}
	const char *getName() const {
		return name;
	}
	GLuint getGLHandle() const {
		return handle;
	}
	const permutationFlags_s &getPermutations() const {
		return permutations;
	}
	bool isValid() const {
		if(handle)
			return true;
		return false;
	}

friend glShader_c *GL_RegisterShader(const char *baseName, const permutationFlags_s *permutations = 0);
friend void GL_ShutdownGLSLShaders();
};

#endif // __GL_SHADER_H__
