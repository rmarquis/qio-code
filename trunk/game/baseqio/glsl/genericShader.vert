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

#ifdef HAS_HEIGHT_MAP
attribute vec3 atrTangents;
attribute vec3 atrBinormals;
varying vec3 v_tbnEyeDir;
#endif

void main() {
	gl_Position = ftransform();
	
#ifdef HAS_HEIGHT_MAP
    // calculate the direction of the viewOrigin from the vertex;
    vec3 dirEye = u_viewOrigin  - gl_Vertex;
        
    // transform eyeDirection into tangent space
    v_tbnEyeDir.x = dot(atrTangents , dirEye);
    v_tbnEyeDir.y = dot(atrBinormals , dirEye);
    v_tbnEyeDir.z = dot(gl_Normal.xyz , dirEye);
#endif
    
    
#ifdef HAS_TEXGEN_ENVIROMENT
	vec3 dir = u_viewOrigin - gl_Vertex.xyz;
	dir = normalize(dir);
	float dotValue = dot(gl_Normal,dir);
	float twoDot = 2.f * dotValue;

	vec3 reflected;
	reflected.x = gl_Normal.x * twoDot - dir.x;
	reflected.y = gl_Normal.y * twoDot - dir.y;
	reflected.z = gl_Normal.z * twoDot - dir.z;

	vec4 calcTexCoord;

	calcTexCoord.x = 0.5f + reflected.y * 0.5f;
	calcTexCoord.y = 0.5f - reflected.z * 0.5f;
	
	gl_TexCoord[0] = gl_TextureMatrix[0] * calcTexCoord;
#else
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
#endif
#ifdef HAS_LIGHTMAP
	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;
#endif // HAS_LIGHTMAP
#ifdef HAS_VERTEXCOLORS
	v_color4 = gl_Color;
#endif // HAS_VERTEXCOLORS
}

