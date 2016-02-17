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
uniform sampler2D blendChannelRed;
uniform sampler2D blendChannelGreen;
uniform sampler2D blendChannelBlue;

void main() {
	vec2 texCoord = gl_TexCoord[0].st;
	vec4 r = texture2D (blendChannelRed, texCoord);
	vec4 g = texture2D (blendChannelGreen, texCoord);
	vec4 b = texture2D (blendChannelBlue, texCoord);
	vec4 blend = texture2D (colorMap, texCoord*0.05);
	vec4 f = (blend.x * r + blend.y * g + blend.z * b);
	normalize(f);
	gl_FragColor = f;
}