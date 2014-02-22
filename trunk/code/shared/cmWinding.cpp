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
// cmWinding.cpp - a set of points (usually lying on the single plane)
#include "cmWinding.h"
#include <math/plane.h>
#include <math/aabb.h>
#include <api/coreAPI.h>

bool cmWinding_c::createBaseWindingFromPlane(const class plane_c &pl, float maxCoord) {
	// find the major axis
	float max = -maxCoord;
	int x = -1;
	for (u32 i = 0; i < 3; i++) {
		float v = fabs(pl.norm[i]);
		if (v > max) {
			x = i;
			max = v;
		}
	}
	if (x == -1) {
		g_core->RedWarning("cmWinding_c::createBaseWindingFromPlane: plane is degnerate - no axis found\n");
		return true; // error
	}
	
	vec3_c up(0,0,0);
	if(x == 2) {
		up.x = 1;
	} else {
		up.z = 1;
	}

	float v = pl.norm.dotProduct(up);
	up.vectorMA(up, pl.norm, -v);
	up.normalize();

	vec3_c org = pl.norm * -pl.dist;

	vec3_c vright;
	vright.crossProduct(up,pl.norm);

	up *= maxCoord;
	vright *= maxCoord;

	// project a really big	axis aligned box onto the plane
	vec3_c p[4];
	p[0] = org - vright;
	p[0] += up;

	p[1] = org + vright;
	p[1] += up;

	p[2] = org + vright;
	p[2] -= up;

	p[3] = org - vright;
	p[3] -= up;

	points.push_back(p[0]);
	points.push_back(p[1]);
	points.push_back(p[2]);
	points.push_back(p[3]);

	for (u32 i = 0; i < points.size(); i++) {
		float d = pl.distance(points[i]);
		if(d != 0.f) {
			vec3_c delta = -d * pl.norm;
			vec3_c fixed = points[i] + delta;
			float fixedDist = pl.distance(fixed);
			if(abs(fixedDist) < abs(d)) {
				points[i] = fixed;
			}
		}
	}

	//if(areAllPointsOnPlane(pl,1.f) == false) {
	//	__asm int 3
	//}
	return false; // no error
}
void cmWinding_c::addWindingPointsUnique(const vec3_c *addPoints, u32 numPointsToAdd) {
	for(u32 i = 0; i < numPointsToAdd; i++) {
		const vec3_c &newPoint = addPoints[i];
		bool found = false;
		// see if the point is already on list
		for(u32 j = 0; j < points.size(); j++) {
			if(newPoint.compare(points[j])) {
				found = true;
				break;
			}
		}
		if(found)
			continue;
		points.push_back(newPoint);
	}
}
planeSide_e cmWinding_c::clipWindingByPlane(const class plane_c &pl, float epsilon) {
	u32 counts[3];
	counts[SIDE_FRONT] = counts[SIDE_BACK] = counts[SIDE_ON] = 0;
	arraySTD_c<float> dists;
	dists.resize(points.size()+1);
	arraySTD_c<u32> sides;
	sides.resize(points.size()+1);
	// determine sides for each point
	u32 i;
	for (i = 0; i < points.size(); i++) {
		float d = pl.distance(points[i]);
	
		dists[i] = d;

		if (d > epsilon)
			sides[i] = SIDE_FRONT;
		else if (d < -epsilon)
			sides[i] = SIDE_BACK;
		else {
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];
	if (!counts[SIDE_FRONT]) {
		// there are no points on the front side of the plane
		points.clear(); // (winding gets entirely chopped away)
		return SIDE_BACK;
	}
	if (!counts[SIDE_BACK]) {
		// there are no points on the back side of the plane
		return SIDE_FRONT;
	}
	arraySTD_c<vec3_c> f;
	f.reserve(points.size()*2);
	for ( i = 0; i < points.size(); i++) {
		vec3_c p1 = points[i];

		if (sides[i] == SIDE_ON) {
			f.push_back(p1);
			continue;
		}
		if (sides[i] == SIDE_FRONT)
			f.push_back(p1);
		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
		vec3_c p2 = points[(i+1)%points.size()];

		float dot = dists[i] / (dists[i]-dists[i+1]);
		vec3_c mid;
		if (pl.norm.x == 1)
			mid.x = -pl.dist;
		else if (pl.norm.x == -1)
			mid.x = pl.dist;
		else
			mid.x = p1.x + dot*(p2.x-p1.x);

		if (pl.norm.y == 1)
			mid.y = -pl.dist;
		else if (pl.norm.y == -1)
			mid.y = pl.dist;
		else
			mid.y = p1.y + dot*(p2.y-p1.y);

		if (pl.norm.z == 1)
			mid.z = -pl.dist;
		else if (pl.norm.z == -1)
			mid.z = pl.dist;
		else
			mid.z = p1.z + dot*(p2.z-p1.z);

		f.push_back(mid);
	}
	points = f;
	return SIDE_CROSS;
}
void cmWinding_c::getBounds(aabb &out) const {
	out.clear();
	for(u32 i = 0; i < points.size(); i++) {
		out.addPoint(points[i]);
	}
}
void cmWinding_c::getPlane(class plane_c &out) const {
	if(size() < 3) {
		out.clear();
		return;
	}
	out.fromThreePoints(points[0],points[1],points[2]);
}
void cmWinding_c::addPointsToBounds(aabb &out) const {
	for(u32 i = 0; i < points.size(); i++) {
		out.addPoint(points[i]);
	}
}
void cmWinding_c::removeDuplicatedPoints(float epsilon) {
	arraySTD_c<vec3_c> newPoints;
	for(u32 i = 0; i < points.size(); i++) {
		const vec3_c &p = points[i];
		bool found = false;
		for(u32 j = 0; j < newPoints.size(); j++) {
			if(newPoints[j].compare(p,epsilon)) {
				found = true;
				break;
			}
		}
		if(found)
			continue;
		newPoints.push_back(p);
	}
	if(newPoints.size() != points.size()) {
		points = newPoints;
	}
}

void cmWinding_c::addPointsUnique(const vec3_c *first, u32 numPoints, float epsilon) {
	for(u32 i = 0; i < numPoints; i++) {
		const vec3_c &p = first[i];
		bool found = false;
		for(u32 j = 0; j < points.size(); j++) {
			if(points[j].compare(p,epsilon)) {
				found = true;
				break;
			}
		}
		if(found)
			continue;
		points.push_back(p);
	}
}
vec3_c cmWinding_c::getCenter() const {
	double v[3];
	v[0] = v[1] = v[2] = 0;
	for(u32 i = 0; i < points.size(); i++) {
		const vec3_c &p = points[i];
		v[0] += p.x;
		v[1] += p.y;
		v[2] += p.z;
	}
	v[0] /= double(points.size());
	v[1] /= double(points.size());
	v[2] /= double(points.size());
	return vec3_c(v);
}
void cmWinding_c::iterateTriangles(void (*triCallback)(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2)) const {
	for(u32 i = 2; i < points.size(); i++) {
		triCallback(points[0],points[i-1],points[i]);
	}
}
#include <api/staticModelCreatorAPI.h>
#include <api/colMeshBuilderAPI.h>
void cmWinding_c::iterateTriangles(class staticModelCreatorAPI_i *smc) const {
	for(u32 i = 2; i < points.size(); i++) {
		smc->addTriangleOnlyXYZ("nomaterial",points[0],points[i-1],points[i]);
	}
}
void cmWinding_c::iterateTriangles(class colMeshBuilderAPI_i *out) const {
	for(u32 i = 2; i < points.size(); i++) {
		out->addXYZTri(points[0],points[i-1],points[i]);
	}
}

