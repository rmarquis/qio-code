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
// displacementBuilder.h - helper class used to generate
// points and indices of Source Engine displacement surfaces
// https://developer.valvesoftware.com/wiki/Displacement
#ifndef __SHARED_DISPLACEMENT_BUILDER_H__
#define __SHARED_DISPLACEMENT_BUILDER_H__

#include "math/vec3.h"
#include "array.h"

class displacementBuilder_c {
	vec3_c baseVerts[4];
	arraySTD_c<vec3_c> vertices;
	arraySTD_c<u16> indices;

	void add3Indices(u16 i0, u16 i1, u16 i2) {
		indices.push_back(i2);
		indices.push_back(i1);
		indices.push_back(i0);
	}
public:
	void setupBaseQuad(const vec3_c *v, u32 stride = sizeof(vec3_c));
	void buildDisplacementSurface(const struct srcDisplacement_s *disp, const struct srcDispVert_s *verts);

	void setPoint(u32 i, const vec3_c &p) {
		baseVerts[i] = p;
	}
	u32 getNumVerts() const {
		return vertices.size();
	}
	u32 getNumIndices() const {
		return indices.size();
	}
	u16 getIndex(u32 which) const {
		return indices[which];
	}
	const arraySTD_c<vec3_c> &getVerts() const {
		return vertices;
	}
	const arraySTD_c<u16> &getIndices() const {
		return indices;
	}
};

#endif // __SHARED_DISPLACEMENT_BUILDER_H__
