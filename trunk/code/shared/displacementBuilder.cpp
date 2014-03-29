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
// displacementBuilder.cpp - helper class used to generate
// points and indices of Source Engine displacement surfaces
// https://developer.valvesoftware.com/wiki/Displacement
#include "displacementBuilder.h"
#include <fileformats/bspFileFormat_hl2.h>

void displacementBuilder_c::setupBaseQuad(const vec3_c *v, u32 stride) {
	baseVerts[0] = *v;
	v = (const vec3_c*)(((const byte*)v)+stride);
	baseVerts[1] = *v;
	v = (const vec3_c*)(((const byte*)v)+stride);
	baseVerts[2] = *v;
	v = (const vec3_c*)(((const byte*)v)+stride);
	baseVerts[3] = *v;
}
void displacementBuilder_c::buildDisplacementSurface(const srcDisplacement_s *disp, const srcDispVert_s *verts) {
	while (baseVerts[0].compare(disp->startPosition,0.1f)==false) {
		vec3_c temp = baseVerts[0];
		baseVerts[0] = baseVerts[1];
		baseVerts[1] = baseVerts[2];
		baseVerts[2] = baseVerts[3];
		baseVerts[3] = temp;
	}
	vec3_c leftEdge = baseVerts[1] - baseVerts[0];
	vec3_c rightEdge = baseVerts[2] - baseVerts[3];

	u32 numEdgeVerts = (1 << disp->power) + 1;
	float subdivideScale = 1.f / (numEdgeVerts - 1);
	vec3_c leftEdgeStep = leftEdge * subdivideScale;
	vec3_c rightEdgeStep = rightEdge * subdivideScale;

	for(u32 i = 0; i < numEdgeVerts; i++) {
		vec3_c leftEnd = leftEdgeStep * i + baseVerts[0];
		vec3_c rightEnd = rightEdgeStep * i + baseVerts[3];

		vec3_c leftRightSeq = rightEnd - leftEnd;
		vec3_c leftRightStep = leftRightSeq * subdivideScale;

		for(u32 j = 0; j < numEdgeVerts; j++) {
			u32 dVertIndex = disp->dispVertStart + i * numEdgeVerts + j;
			const srcDispVert_s &dVert = verts[dVertIndex];
			vec3_c newXYZ = dVert.vec;
			newXYZ *= dVert.dist;
			vec3_c basePos = leftEnd + (leftRightStep * j);
			newXYZ += basePos;
			vertices.push_back(newXYZ);
		}
	}
	for(u32 i = 0; i < numEdgeVerts-1; i++) {
		for(u32 j = 0; j < numEdgeVerts-1; j++) {
			u32 idx = i * numEdgeVerts + j;
			if((idx % 2) == 1) {
				add3Indices(idx,idx + 1,idx + numEdgeVerts);
				add3Indices(idx + 1,idx + numEdgeVerts + 1,idx + numEdgeVerts);
			} else {
				add3Indices(idx,idx + numEdgeVerts + 1,idx + numEdgeVerts);
				add3Indices(idx,idx + 1,idx + numEdgeVerts + 1);
			}
		}
	}
}
