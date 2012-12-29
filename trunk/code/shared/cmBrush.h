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
// cmBrush.h
#ifndef __CMBRUSH_H__
#define __CMBRUSH_H__

#include <shared/array.h>
#include <shared/str.h>
#include <math/vec3.h>
#include <math/aabb.h>
#include <math/plane.h>
#include <shared/perPlaneCallback.h>

class cmBrushSide_c {
friend class cmBrush_c;
	plane_c pl;
	str matName;
public:
	cmBrushSide_c() {

	}
	cmBrushSide_c(const plane_c &otherPlane) {
		this->pl = otherPlane;
	}
};
//struct cmEdge_s {
//	u16 verts[2];
//};
//class cmBrushMeshSide_c {
//public:
//	u16 firstEdgeIndex;
//	u16 numEdgeIndexes;
//};
//class cmBrushMesh_c {
//	arraySTD_c<vec3_c> points;
//	arraySTD_c<cmEdge_s> edges;
//	arraySTD_c<short> edges;
//public:
//	void fromBrush(const class cmBrush_c &in);
//};

class cmBrush_c : public perPlaneCallbackListener_i {
	arraySTD_c<cmBrushSide_c> sides;
	aabb bounds;
public:
	/// perPlaneCallbackListener_i IMPL
	// add a new plane to current brush
	virtual void perPlaneCallback(const float plEq[4]);

	void fromBounds(const class aabb &bb);
	void fromPoints(const vec3_c *points, u32 numPoints);
	void fromPoints(const arraySTD_c<vec3_c> &points) {
		fromPoints(points.getArray(),points.size());
	}
	bool hasPlane(const plane_c &pl, const float normalEps = PL_NORM_EPS, const float distEps = PL_DIST_EPS) {
		for(u32 i = 0; i < sides.size(); i++) {
			if(sides[i].pl.compare(pl,normalEps,distEps)) {
				return true;
			}
		}
		return false;
	}
	u32 getNumSides() const {
		return sides.size();
	}
	const class plane_c &getSidePlane(u32 sideNum) const {
		return sides[sideNum].pl;
	}
	const aabb &getBounds() const {
		return bounds;
	}
	void iterateSidePlanes(void (*callback)(const float planeEq[4])) const {
		for(u32 i = 0; i < sides.size(); i++) {
			callback(&sides[i].pl.norm.x);
		}
	}
	void negatePlaneDistances() {
		for(u32 i = 0; i < sides.size(); i++) {
			sides[i].pl.dist *= -1;
		}
	}
	bool hasSideWithMaterial(const char *matName) const;
	void translateXYZ(const class vec3_c &ofs);

	// returns true if brush is invalid
	bool calcBounds();

	bool traceRay(class trace_c &tr);
	
	bool parseBrushQ3(class parser_c &p);
	bool parseBrushD3(class parser_c &p);
	void writeSingleBrushToMapFileVersion2(class writeStreamAPI_i *out);
	void writeSingleBrushToMapFileVersion2(const char *outFName);
};

#endif // __CMBRUSH_H__
