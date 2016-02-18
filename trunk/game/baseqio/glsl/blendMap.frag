/*
============================================================================
Copyright (C) 2016 V.

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
// glsl/blendMap.frag

// shader input
uniform sampler2D colorMap;
uniform sampler2D blendChannelRed;
uniform sampler2D blendChannelGreen;
uniform sampler2D blendChannelBlue;
#ifdef HAS_BLEND_BOX
uniform vec3 u_blendBoxMins;
uniform vec3 u_blendBoxMaxs;
#endif

varying vec3 v_vertXYZ;

void main() {
	vec2 texCoord = gl_TexCoord[0].st;
	vec4 r = texture2D (blendChannelRed, texCoord);
	vec4 g = texture2D (blendChannelGreen, texCoord);
	vec4 b = texture2D (blendChannelBlue, texCoord);
#ifdef HAS_BLEND_BOX
	vec3 size = u_blendBoxMaxs - u_blendBoxMins;
	vec3 local = v_vertXYZ - u_blendBoxMins;
	float s = local.x / size.x;
	float t = local.y / size.y;
	if(s < 0.0)
		discard; // bug test
	if(t < 0.0)
		discard; // bug test
	if(s > 1.0)
		discard; // bug test
	if(t > 1.0)
		discard; // bug test
	vec4 blend = texture2D (colorMap, vec2(s,t));
#else
	vec4 blend = texture2D (colorMap, texCoord);
#endif
	vec4 f = (blend.x * r + blend.y * g + blend.z * b);
	normalize(f);
	gl_FragColor = f;
}