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
// glsl/perPixelLighting.frag - per pixel lighting shader for OpenGL backend

// shader input
uniform sampler2D colorMap;
uniform vec3 u_lightOrigin;
uniform float u_lightRadius;
// shader varying variables
varying vec3 v_vertXYZ;
varying vec3 v_vertNormal; 

void main() {
#if 0
	gl_FragColor.rgb = v_vertNormal;
	return;
#endif
	// calculate light direction and distance to current pixel
	vec3 lightToVert = u_lightOrigin - v_vertXYZ;
	float distance = length(lightToVert);
	if(distance > u_lightRadius) {
		// pixel is too far from the ligh
		return;
	}
    vec3 lightDirection = normalize(lightToVert);
    // calculate the diffuse value based on light angle	
    float angleFactor = dot(v_vertNormal, lightDirection);
    if(angleFactor < 0) {
		// light is behind the surface
		return;
    }
	//  apply distnace scale
  	float distanceFactor = 1 - distance / u_lightRadius;
	// calculate the final color
	gl_FragColor = texture2D (colorMap, gl_TexCoord[0].st) * angleFactor * distanceFactor;
}