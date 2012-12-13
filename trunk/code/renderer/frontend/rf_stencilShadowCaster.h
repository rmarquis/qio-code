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
// rf_stencilShadowCaster.h
#ifndef __RF_STENCILSHADOWCASTER_H__
#define __RF_STENCILSHADOWCASTER_H__

#include <shared/vec3hash.h>
#include <math/plane.h>

struct rTri32_s {
	u32 indices[3];
	plane_c plane;
};
struct rEdge_s {
	int tris[2];
	u32 verts[2];

	inline bool isMatched() const {
		return (tris[0] != tris[1]);
	}
	inline u32 getTriangleIndex(u32 idx) const {
		int ret = tris[idx];
		if(ret < 0)
			return -(ret + 1);
		return ret;
	}
};
class r_stencilShadowCaster_c {
	vec3Hash_c points;
	arraySTD_c<rTri32_s> tris;
	arraySTD_c<rEdge_s> edges;
	u32 c_matchedEdges;
	u32 c_unmatchedEdges;
public:
	void addTriangle(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2);
	void addRSurface(const class r_surface_c *sf);
	void addRModel(const class r_model_c *mod);
	void addEdge(u32 triNum, u32 v0, u32 v1);
	void calcEdges();

	void generateShadowVolume(class rIndexedShadowVolume_c *out, const vec3_c &light) const;
};

#endif // __RF_STENCILSHADOWCASTER_H__
