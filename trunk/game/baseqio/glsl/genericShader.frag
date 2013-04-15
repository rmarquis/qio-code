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
// glsl/genericShader.frag

// shader input
uniform sampler2D colorMap;
#ifdef HAS_LIGHTMAP
uniform sampler2D lightMap;
#endif // HAS_LIGHTMAP
#ifdef HAS_HEIGHT_MAP
uniform sampler2D heightMap;
varying vec3 v_tbnEyeDir;
#endif

#ifdef HAS_VERTEXCOLORS
varying vec4 v_color4;
#endif // HAS_VERTEXCOLORS

void main() {
#ifdef HAS_HEIGHT_MAP
    vec3 eyeDirNormalized = normalize(v_tbnEyeDir);
    vec4 offset = texture2D(heightMap, gl_TexCoord[0].xy);
	offset = offset * 0.1 - 0.02;
	vec2 texCoord = offset.xy * eyeDirNormalized.xy +  gl_TexCoord[0].xy;   
#else
	vec2 texCoord = gl_TexCoord[0].st;
#endif 
    
  // calculate the final color
#ifdef HAS_LIGHTMAP
#ifdef HAS_VERTEXCOLORS
  gl_FragColor = texture2D (colorMap, texCoord)*texture2D (lightMap, gl_TexCoord[1].st)*v_color4;
#else
  gl_FragColor = texture2D (colorMap, texCoord)*texture2D (lightMap, gl_TexCoord[1].st);
#endif
#else
#ifdef HAS_VERTEXCOLORS
  gl_FragColor = texture2D (colorMap, texCoord)*v_color4;
#else
  gl_FragColor = texture2D (colorMap, texCoord);
#endif
#endif
}