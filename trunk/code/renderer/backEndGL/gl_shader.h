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

class glShader_c {
friend class rbSDLOpenGL_c;
	str name; // shader name
	GLuint handle; // openGL handle
	
	// uniform locations
	int uLightOrigin;
	int uLightRadius;

	// sampler2D locations
	int sColorMap; // main diffuse texture

friend glShader_c *GL_RegisterShader(const char *baseName);
public:
	glShader_c() {
		handle = 0;
	}
	const char *getName() const {
		return name;
	}
	GLuint getGLHandle() const {
		return handle;
	}
	bool isValid() const {
		if(handle)
			return true;
		return false;
	}
};

#endif // __GL_SHADER_H__
