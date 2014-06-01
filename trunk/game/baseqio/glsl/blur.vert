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
// glsl/blur.vert

varying vec2 v_texCoord;
varying vec2 v_blurTexCoords[14];

uniform float u_blurScale;

void main() {
	gl_Position = ftransform();  
	 
	v_texCoord = gl_MultiTexCoord0;
	
	// setup texcoords for vertical pass
#ifdef HORIZONTAL_PASS
    v_blurTexCoords[ 0] = v_texCoord + vec2(-0.028, 0.0)*u_blurScale;
    v_blurTexCoords[ 1] = v_texCoord + vec2(-0.024, 0.0)*u_blurScale;
    v_blurTexCoords[ 2] = v_texCoord + vec2(-0.020, 0.0)*u_blurScale;
    v_blurTexCoords[ 3] = v_texCoord + vec2(-0.016, 0.0)*u_blurScale;
    v_blurTexCoords[ 4] = v_texCoord + vec2(-0.012, 0.0)*u_blurScale;
    v_blurTexCoords[ 5] = v_texCoord + vec2(-0.008, 0.0)*u_blurScale;
    v_blurTexCoords[ 6] = v_texCoord + vec2(-0.004, 0.0)*u_blurScale;
    v_blurTexCoords[ 7] = v_texCoord + vec2( 0.004, 0.0)*u_blurScale;
    v_blurTexCoords[ 8] = v_texCoord + vec2( 0.008, 0.0)*u_blurScale;
    v_blurTexCoords[ 9] = v_texCoord + vec2( 0.012, 0.0)*u_blurScale;
    v_blurTexCoords[10] = v_texCoord + vec2( 0.016, 0.0)*u_blurScale;
    v_blurTexCoords[11] = v_texCoord + vec2( 0.020, 0.0)*u_blurScale;
    v_blurTexCoords[12] = v_texCoord + vec2( 0.024, 0.0)*u_blurScale;
    v_blurTexCoords[13] = v_texCoord + vec2( 0.028, 0.0)*u_blurScale;
#else
    v_blurTexCoords[ 0] = v_texCoord + vec2(0.0, -0.028)*u_blurScale;
    v_blurTexCoords[ 1] = v_texCoord + vec2(0.0, -0.024)*u_blurScale;
    v_blurTexCoords[ 2] = v_texCoord + vec2(0.0, -0.020)*u_blurScale;
    v_blurTexCoords[ 3] = v_texCoord + vec2(0.0, -0.016)*u_blurScale;
    v_blurTexCoords[ 4] = v_texCoord + vec2(0.0, -0.012)*u_blurScale;
    v_blurTexCoords[ 5] = v_texCoord + vec2(0.0, -0.008)*u_blurScale;
    v_blurTexCoords[ 6] = v_texCoord + vec2(0.0, -0.004)*u_blurScale;
    v_blurTexCoords[ 7] = v_texCoord + vec2(0.0,  0.004)*u_blurScale;
    v_blurTexCoords[ 8] = v_texCoord + vec2(0.0,  0.008)*u_blurScale;
    v_blurTexCoords[ 9] = v_texCoord + vec2(0.0,  0.012)*u_blurScale;
    v_blurTexCoords[10] = v_texCoord + vec2(0.0,  0.016)*u_blurScale;
    v_blurTexCoords[11] = v_texCoord + vec2(0.0,  0.020)*u_blurScale;
    v_blurTexCoords[12] = v_texCoord + vec2(0.0,  0.024)*u_blurScale;
    v_blurTexCoords[13] = v_texCoord + vec2(0.0,  0.028)*u_blurScale;
#endif
}