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
// rf_stencilShadowCaster.cpp
#include "rf_stencilShadowCaster.h"
#include "rf_surface.h"
void r_stencilShadowCaster_c::addTriangle(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2) {
	u32 i0 = points.registerVec3(p0);
	u32 i1 = points.registerVec3(p1);
	u32 i2 = points.registerVec3(p2);
	const vec3_c &v0 = points[i0].v;
	const vec3_c &v1 = points[i1].v;
	const vec3_c &v2 = points[i2].v;
	rTri32_s &nt = tris.pushBack();
	nt.indices[0] = i0;
	nt.indices[1] = i1;
	nt.indices[2] = i2;
	nt.plane.fromThreePoints(v0,v1,v2);
}
void r_stencilShadowCaster_c::addEdge(u32 triNum, u32 v0, u32 v1) {
	rEdge_s *e = edges.getArray();
	for(u32 i = 0; i < edges.size(); i++, e++) {
		if(e->tris[0] != e->tris[1])
			continue; // already matched
		if(e->verts[0] == v0 && e->verts[1] == v1) {
			e->tris[1] = triNum;
			c_matchedEdges++;
			return;
		}
		if(e->verts[0] == v1 && e->verts[1] == v0) {
			e->tris[1] = -triNum - 1;
			c_matchedEdges++;
			return;
		}
	}
	rEdge_s &newEdge = edges.pushBack();
	newEdge.verts[0] = v0;
	newEdge.verts[1] = v1;
	newEdge.tris[0] = newEdge.tris[1] = triNum;
}
void r_stencilShadowCaster_c::calcEdges() {
	c_matchedEdges = 0;
	const rTri32_s *t = tris.getArray();
	for(u32 i = 0; i < tris.size(); i++, t++) {
		addEdge(i,t->indices[0],t->indices[1]);
		addEdge(i,t->indices[1],t->indices[2]);
		addEdge(i,t->indices[2],t->indices[0]);
	}
	c_unmatchedEdges = edges.size() - c_matchedEdges;
}
void r_stencilShadowCaster_c::addRSurface(const class r_surface_c *sf) {
	for(u32 i = 0; i < sf->getNumTris(); i++) {
		vec3_c points[3];
		sf->getTriangle(i,points[0],points[1],points[2]);
		this->addTriangle(points[0],points[1],points[2]);
	}
}
void r_stencilShadowCaster_c::addRModel(const class r_model_c *mod) {
	for(u32 i = 0; i < mod->getNumSurfs(); i++) {
		addRSurface(mod->getSurf(i));
	}
}
#include "rf_shadowVolume.h"
void r_stencilShadowCaster_c::generateShadowVolume(class rIndexedShadowVolume_c *out, const vec3_c &light) const {
	arraySTD_c<byte> bFrontFacing;
	bFrontFacing.resize(tris.size());
	bFrontFacing.nullMemory();
	// mark triangles frontfacing the light and
	// add front cap and back cap made  
	// from triangles frontfacing the light
	const rTri32_s *t = tris.getArray();
	for(u32 i = 0; i < tris.size(); i++, t++) {
		float d = t->plane.distance(light);
		if(d > 0) {
			bFrontFacing[i] = 1;
			const vec3_c &v0 = points.getVec3(t->indices[0]);
			const vec3_c &v1 = points.getVec3(t->indices[1]);
			const vec3_c &v2 = points.getVec3(t->indices[2]);
			out->addFrontCapAndBackCapForTriangle(v0,v1,v2,light);
		}
	}
	const rEdge_s *e = edges.getArray();
	for(u32 i = 0; i < edges.size(); i++, e++) {
		if(e->isMatched() == false)
			continue;
		int t0 = e->getTriangleIndex(0);
		int t1 = e->getTriangleIndex(1);
		if(bFrontFacing[t0] == bFrontFacing[t1])
			continue;
		const vec3_c &v0 = points.getVec3(e->verts[0]);
		const vec3_c &v1 = points.getVec3(e->verts[1]);
		if(bFrontFacing[t0]) {
			out->addEdge(v0,v1,light);
		} else {
			out->addEdge(v1,v0,light);
		}
	}
}
