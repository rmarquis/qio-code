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

#ifdef SHADOW_MAPPING_POINT_LIGHT
varying vec4 shadowCoord0;
varying vec4 shadowCoord1;
varying vec4 shadowCoord2;
varying vec4 shadowCoord3;
varying vec4 shadowCoord4;
varying vec4 shadowCoord5;
uniform sampler2DShadow shadowMap0;
uniform sampler2DShadow shadowMap1;
uniform sampler2DShadow shadowMap2;
uniform sampler2DShadow shadowMap3;
uniform sampler2DShadow shadowMap4;
uniform sampler2DShadow shadowMap5;

float calcShadow0() {
   return shadow2DProj(shadowMap0, shadowCoord0).s;
}
float calcShadow1() {
   return shadow2DProj(shadowMap1, shadowCoord1).s;
}
float calcShadow2() {
   return shadow2DProj(shadowMap2, shadowCoord2).s;
}
float calcShadow3() {
   return shadow2DProj(shadowMap3, shadowCoord3).s;
}
float calcShadow4() {
   return shadow2DProj(shadowMap4, shadowCoord4).s;
}
float calcShadow5() {
   return shadow2DProj(shadowMap5, shadowCoord5).s;
}
int cubeSide(vec3 v) {
	vec3 normals[] = { vec3(1,0,0), vec3(-1,0,0),
						vec3(0,1,0), vec3(0,-1,-0),
						vec3(0,0,1), vec3(0,0,-1)};	
	float max = 0;
	int ret;
	for(int i = 0; i < 6; i++) {
		float d = dot(normals[i],v);
		if(d < max) {
			max = d;
			ret = i;
		}
	}
	return ret;
}
float computeShadow(vec3 lightToVertDirection) {
	float shadow = 0.0;
  	int side = cubeSide(lightToVertDirection);
	if (side == 0) {
		shadow += calcShadow0();
	} else if(side == 1) {
		shadow += calcShadow1();
	} else if(side == 2) {
		shadow += calcShadow2();
	} else if(side == 3) {
		shadow += calcShadow3();
	} else if(side == 4) {
		shadow += calcShadow4();
	} else if(side == 5) {
		shadow += calcShadow5();
	} else {
		// never gets here
	}
	return shadow;
}
#endif // SHADOW_MAPPING_POINT_LIGHT

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
#ifdef SHADOW_MAPPING_POINT_LIGHT
	float shadow = computeShadow(lightToVert);
#else
	float shadow = 1.f;
#endif
	// calculate the final color
	gl_FragColor = texture2D (colorMap, gl_TexCoord[0].st) * angleFactor * distanceFactor * shadow;
}