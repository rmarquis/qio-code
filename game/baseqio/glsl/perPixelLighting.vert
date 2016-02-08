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
// glsl/perPixelLighting.vert - per pixel lighting shader for OpenGL backend

// shader varying variables
varying vec3 v_vertXYZ;
varying vec3 v_vertNormal; 
#ifdef SHADOW_MAPPING_SPOTLIGHT
varying vec4 spotShadowCoord;
#elif defined(SHADOW_MAPPING_POINT_LIGHT)
// used for shadow lookup
varying vec4 shadowCoord0;
varying vec4 shadowCoord1;
varying vec4 shadowCoord2;
varying vec4 shadowCoord3;
varying vec4 shadowCoord4;
varying vec4 shadowCoord5;
#endif // SHADOW_MAPPING_POINT_LIGHT
#if defined(HAS_BUMP_MAP) || defined(HAS_HEIGHT_MAP)
attribute vec3 atrTangents;
attribute vec3 atrBinormals;
varying mat3 tbnMat;
#endif
#ifdef HAS_BUMP_MAP
uniform sampler2D bumpMap;
#endif
#ifdef HAS_HEIGHT_MAP
uniform sampler2D heightMap;
#endif
#if (defined(HAS_HEIGHT_MAP) || defined(HAS_BUMP_HEIGHTMAP_MAP))
varying vec3 v_tbnEyeDir;
uniform vec3 u_viewOrigin;
#endif

void main() {	
#if (defined(HAS_HEIGHT_MAP) || defined(HAS_BUMP_HEIGHTMAP_MAP))
    // calculate the direction of the viewOrigin from the vertex;
    vec3 dirEye = u_viewOrigin  - gl_Vertex;
        
    // transform eyeDirection into tangent space
    v_tbnEyeDir.x = dot(atrTangents , dirEye);
    v_tbnEyeDir.y = dot(atrBinormals , dirEye);
    v_tbnEyeDir.z = dot(gl_Normal.xyz , dirEye);
#endif
    
#ifdef SHADOW_MAPPING_POINT_LIGHT
	// this is the only shadow part in the Vertex Shader
	shadowCoord0 = gl_TextureMatrix[1] * gl_Vertex;
	shadowCoord1 = gl_TextureMatrix[2] * gl_Vertex;
	shadowCoord2 = gl_TextureMatrix[3] * gl_Vertex;
	shadowCoord3 = gl_TextureMatrix[4] * gl_Vertex;
	shadowCoord4 = gl_TextureMatrix[5] * gl_Vertex;
	shadowCoord5 = gl_TextureMatrix[6] * gl_Vertex;
#endif
#ifdef SHADOW_MAPPING_SPOTLIGHT
	spotShadowCoord = gl_TextureMatrix[1] * gl_Vertex;
#endif
	gl_Position = ftransform();
	// this is needed here for GL_CLIP_PLANE0 to work.
	// clipping planes are used by mirrors
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
#if defined(HAS_BUMP_MAP) || defined(HAS_HEIGHT_MAP)
	tbnMat = mat3(atrTangents,atrBinormals,gl_Normal);
#endif
	
	v_vertXYZ = gl_Vertex.xyz;
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	v_vertNormal = normalize(gl_Normal);
}