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
// rf_decalProjector.cpp
#include "rf_local.h"
#include "rf_decalProjector.h"
#include "rf_decals.h"
#include <api/coreAPI.h>

decalProjector_c::decalProjector_c() {
	mat = 0;
}
void decalProjector_c::init(const vec3_c &pos, const vec3_c &normal, float radius) {
	inPos = pos;
	inNormal = normal;
	inRadius = radius;

	planes[0].fromPointAndNormal(pos-normal*radius,normal);
	planes[1].fromPointAndNormal(pos+normal*radius,-normal);
	perp = normal.getPerpendicular();
	perp2.crossProduct(normal,perp);
	RFDL_AddDebugLine(pos-normal,pos+perp*10.f-normal,vec3_c(0,0,1),5.f);
	RFDL_AddDebugLine(pos-normal,pos+perp2*10.f-normal,vec3_c(0,1,0),5.f);
	vec3_c points[4];
	points[0] = pos + perp * radius + perp2 * radius - normal * radius;
	points[1] = pos - perp * radius + perp2 * radius - normal * radius;
	points[2] = pos - perp * radius - perp2 * radius - normal * radius;
	points[3] = pos + perp * radius - perp2 * radius - normal * radius;
	bounds.addPoint(points[0]);
	bounds.addPoint(points[1]);
	bounds.addPoint(points[2]);
	bounds.addPoint(points[3]);
	bounds.addPoint(points[0] + normal*radius*2.f);
	bounds.addPoint(points[1] + normal*radius*2.f);
	bounds.addPoint(points[2] + normal*radius*2.f);
	bounds.addPoint(points[3] + normal*radius*2.f);
	planes[2].fromThreePointsINV(points[0],points[1],points[1] + normal*radius*2.f);
	planes[3].fromThreePointsINV(points[1],points[2],points[2] + normal*radius*2.f);
	planes[4].fromThreePointsINV(points[2],points[3],points[3] + normal*radius*2.f);
	planes[5].fromThreePointsINV(points[3],points[0],points[0] + normal*radius*2.f);
}
void decalProjector_c::setMaterial(class mtrAPI_i *newMat) {
	mat = newMat;
}
u32 decalProjector_c::clipTriangle(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2) {
	aabb tmpBB;
	tmpBB.fromThreePoints(p0,p1,p2);
	if(tmpBB.intersect(bounds) == false)
		return 0;
#if 0
	plane_c triPlane;
	triPlane.fromThreePoints(p0,p1,p2);
	triPlane.norm *= 0.001f;
	cmWinding_c w(p0-triPlane.norm,p1-triPlane.norm,p2-triPlane.norm);
#else
	cmWinding_c w(p0,p1,p2);
#endif
	for(u32 i = 0; i < 6; i++) {
		w.clipWindingByPlane(planes[i]);
	}
	if(w.size() == 0) {
		//g_core->Print("decalProjector_c::clipTriangle: winding chopped away\n");
		return 0;
	}
	//g_core->Print("decalProjector_c::clipTriangle: remaining windings points: %i\n",w.size());
	results.push_back(w);
	return w.size();
}
void decalProjector_c::iterateResults(void (*untexturedTriCallback)(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2)) {
	for(u32 i = 0; i < results.size(); i++) {
		results[i].iterateTriangles(untexturedTriCallback);
	}
}
void decalProjector_c::iterateResults(class staticModelCreatorAPI_i *smc) {
	for(u32 i = 0; i < results.size(); i++) {
		results[i].iterateTriangles(smc);
	}
}
void decalProjector_c::addResultsToDecalBatcher(class simpleDecalBatcher_c *batcher) {
	float texCoordScale = 0.5f * 1.0f / inRadius;
	for(u32 i = 0; i < results.size(); i++) {
		const cmWinding_c &w = results[i];
		simplePoly_s p;
		p.material = this->mat;
		p.verts.resize(w.size());
		for(u32 j = 0; j < w.size(); j++) {
			p.verts[j].xyz = w[j];		
			vec3_c delta = w[j] - inPos;
			p.verts[j].tc.x = 0.5 + delta.dotProduct(perp)*texCoordScale;
			p.verts[j].tc.y = 0.5 + delta.dotProduct(perp2)*texCoordScale;
		}
		batcher->addDecalToBatch(p);
	}		
	batcher->rebuildBatches();
}
const aabb &decalProjector_c::getBounds() const {
	return bounds;
}
