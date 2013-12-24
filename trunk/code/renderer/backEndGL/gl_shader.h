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
struct glslPermutationFlags_s {
	// Quake3 lightmaps
	bool hasLightmap; // #define HAS_LIGHTMAP
	// Quake3 vertex colors
	bool hasVertexColors; // #define HAS_VERTEXCOLORS
	// Quake3 texgen (eg. for glass surfaces)
	bool hasTexGenEnvironment; // #define HAS_TEXGEN_ENVIROMENT
	bool pointLightShadowMapping; // #define SHADOW_MAPPING_POINT_LIGHT
	bool hasBumpMap; // #define HAS_BUMP_MAP
	bool hasHeightMap; // #define HAS_HEIGHT_MAP
	int useReliefMapping; // #define USE_RELIEF_MAPPING
	// deluxemap is a "lightmap" with light direction normals encoded as colors
	bool hasDeluxeMap; // #define HAS_DELUXEMAP
	// extra per-surface material colo
	bool hasMaterialColor; // #define HAS_MATERIAL_COLOR
	bool isSpotLight; // #define LIGHT_IS_SPOTLIGHT
	// lighting debug tools
	bool debug_ignoreAngleFactor; // #define DEBUG_IGNOREANGLEFACTOR
	bool debug_ignoreDistanceFactor; // #define DEBUG_IGNOREDISTANCEFACTOR

	glslPermutationFlags_s() {
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
	// per-surface material color
	int u_materialColor;
	// current entity matrix
	// (identity for world)
	int u_entityMatrix;

	// sampler2D locations
	int sColorMap; // main diffuse texture
	int sLightMap; // Quake3 bsp lightmap
	int sBumpMap; // Doom3-style bumpmap (normalmap)
	int sHeightMap; // heightmap (not used directly in Doom3)
	int sDeluxeMap; // deluxemap (lightmap with light directions - not colors)

	// shadow mapping
	int u_shadowMap[6];
	// for spotlights
	int u_lightDir;
	int u_spotLightMaxCos;

	int atrTangents;
	int atrBinormals;

	glslPermutationFlags_s permutations;

public:
	glShader_c() {
		handle = 0;
		atrTangents = 0;
		atrBinormals = 0;
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
	int getAtrTangentsLocation() const {
		return atrTangents;
	}
	int getAtrBinormalsLocation() const {
		return atrBinormals;
	}
	const glslPermutationFlags_s &getPermutations() const {
		return permutations;
	}
	bool isValid() const {
		if(handle)
			return true;
		return false;
	}

friend glShader_c *GL_RegisterShader(const char *baseName, const glslPermutationFlags_s *permutations = 0);
friend void GL_ShutdownGLSLShaders();
};

#endif // __GL_SHADER_H__
