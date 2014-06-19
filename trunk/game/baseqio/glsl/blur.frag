/*
============================================================================
Copyright (C) 2014 V.

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
// glsl/blur.frag

uniform sampler2D colorMap;
 
varying vec2 v_texCoord;
varying vec2 v_blurTexCoords[14];
 
void main()
{
    gl_FragColor = vec4(0.0);
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 0])*0.0044299121055113265;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 1])*0.00895781211794;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 2])*0.0215963866053;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 3])*0.0443683338718;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 4])*0.0776744219933;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 5])*0.115876621105;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 6])*0.147308056121;
    gl_FragColor += texture2D(colorMap, v_texCoord         )*0.159576912161;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 7])*0.147308056121;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 8])*0.115876621105;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[ 9])*0.0776744219933;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[10])*0.0443683338718;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[11])*0.0215963866053;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[12])*0.00895781211794;
    gl_FragColor += texture2D(colorMap, v_blurTexCoords[13])*0.0044299121055113265;	
}

