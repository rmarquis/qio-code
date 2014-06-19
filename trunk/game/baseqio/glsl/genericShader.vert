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
// glsl/genericShader.vert

#if defined(HAS_TEXGEN_ENVIROMENT) || defined(HAS_HEIGHT_MAP)
uniform vec3 u_viewOrigin;
#endif // HAS_TEXGEN_ENVIROMENT

#ifdef HAS_VERTEXCOLORS
varying vec4 v_color4;
#endif // HAS_VERTEXCOLORS

#if (defined(HAS_BUMP_MAP) && defined(HAS_DELUXEMAP)) || defined(HAS_HEIGHT_MAP)
attribute vec3 atrTangents;
attribute vec3 atrBinormals;
#endif
#ifdef HAS_HEIGHT_MAP
varying vec3 v_tbnEyeDir;
#endif

#if defined(HAS_BUMP_MAP) && defined(HAS_DELUXEMAP)
varying mat3 tbnMat;
#endif    
    
#ifdef HAS_SUNLIGHT
varying vec3 v_vertNormal;
#endif
#ifdef HAS_DIRECTIONAL_SHADOW_MAPPING
// used for shadow lookup
varying vec4 shadowCoord;
#endif
#ifdef HAS_SHADOWMAP_LOD1
varying vec4 shadowCoord_lod1;
#endif
#ifdef HAS_SHADOWMAP_LOD1
varying vec3 v_vertXYZ;
uniform mat4 u_entityMatrix;
#endif


void main() {
	gl_Position = ftransform();
#ifdef HAS_SHADOWMAP_LOD1
	v_vertXYZ = (u_entityMatrix * gl_Vertex).xyz;
#endif

#ifdef HAS_HEIGHT_MAP
    // calculate the direction of the viewOrigin from the vertex;
    vec3 dirEye = u_viewOrigin  - gl_Vertex;
        
    // transform eyeDirection into tangent space
    v_tbnEyeDir.x = dot(atrTangents , dirEye);
    v_tbnEyeDir.y = dot(atrBinormals , dirEye);
    v_tbnEyeDir.z = dot(gl_Normal.xyz , dirEye);
#endif

#if defined(HAS_BUMP_MAP) && defined(HAS_DELUXEMAP)
	tbnMat = mat3(atrTangents,atrBinormals,gl_Normal);
#endif    
        
#ifdef HAS_DIRECTIONAL_SHADOW_MAPPING
	// this is the only shadow part in the Vertex Shader
	shadowCoord = gl_TextureMatrix[1] * gl_Vertex;
#endif

#ifdef HAS_SHADOWMAP_LOD1
	shadowCoord_lod1 = gl_TextureMatrix[2] * gl_Vertex;
#endif

#ifdef HAS_TEXGEN_ENVIROMENT
	vec3 dir = u_viewOrigin - gl_Vertex.xyz;
	dir = normalize(dir);
	float dotValue = dot(gl_Normal,dir);
	float twoDot = 2.0 * dotValue;

	vec3 reflected;
	reflected.x = gl_Normal.x * twoDot - dir.x;
	reflected.y = gl_Normal.y * twoDot - dir.y;
	reflected.z = gl_Normal.z * twoDot - dir.z;

	vec4 calcTexCoord;

	calcTexCoord.x = 0.5 + reflected.y * 0.5;
	calcTexCoord.y = 0.5 - reflected.z * 0.5;
	
	gl_TexCoord[0] = gl_TextureMatrix[0] * calcTexCoord;
#else
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
#endif
#ifdef HAS_LIGHTMAP
	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;
#endif // HAS_LIGHTMAP
	// this is needed here for GL_CLIP_PLANE0 to work.
	// clipping planes are used by mirrors
	gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
#ifdef HAS_VERTEXCOLORS
	v_color4 = gl_Color;
#endif // HAS_VERTEXCOLORS
#ifdef HAS_SUNLIGHT
	v_vertNormal = gl_Normal;
#endif
}

