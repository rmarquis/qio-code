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
// glsl/reflectionCubeMap.frag - reflection cubeMap shader.
// Used for Doom3 glass effects.
// Handles "texgen reflect" material command.

// shader input
uniform samplerCube cubeMap;

varying vec3 v_vertXYZ;
varying vec3 v_vertNormal; 

uniform vec3 u_viewOrigin;
uniform mat4 u_entityMatrix;
uniform mat4 u_entityRotationMatrix;

void main() {
	vec4 absVert = (u_entityMatrix) * vec4(v_vertXYZ,1);
	vec4 absViewer = (u_entityMatrix) * vec4(u_viewOrigin,1);
	vec3 absNormal = (u_entityRotationMatrix) * vec4(v_vertNormal,1);
    vec3 normal = normalize(absNormal);
    vec3 eyeRay = normalize(absVert.xyz-absViewer.xyz);
    
    // reflect the eye ray by pixel normal
    vec3 reflectedRay = reflect(eyeRay, normal);

	// and sample the cubemap
    gl_FragColor = textureCube(cubeMap, reflectedRay).rgba;
}

