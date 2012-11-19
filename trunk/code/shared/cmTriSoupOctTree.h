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
// cmTriSoupOctTree.h - very simple, but still powerfull 
// octTree structure used to speed up ray-trisoup intersection.
#ifndef __CMTRISOUPOCTREE_H__
#define __CMTRISOUPOCTREE_H__

#include <math/vec3.h>
#include <math/aabb.h>
#include <math/plane.h>

struct aaPlane_s {
	int axis;
	float dist;

	float distance(const float *p) const {
		float ret = p[axis] + dist;
		return ret;
	}	
	planeSide_e onSide(const aabb &bb) const {
		if(bb.mins[axis] > -this->dist)
			return SIDE_FRONT;
		if(bb.maxs[axis] < -this->dist)
			return SIDE_BACK;
		return SIDE_CROSS;
	}
};
struct tsOctTreeNode_s {
	aabb bounds;
	aaPlane_s plane;
	union {
		u32 children[2]; // for nodes
		struct {
			u32 firstTri, numTris; // for leaves (it must be u32; u16 is not enough on eg. zero2_ctf)
		};
	};
};

struct tsOctTreeHeader_s {
	int ident;
	int version;
	u32 numNodes;
	u32 numIndexes;
	u32 numLeafTriIndexes;
	u32 numVerts;
	// extra data needed to avoid repeated testing
	u32 checkCount;
	u32 *triCheckCounts;

	tsOctTreeHeader_s(); // intentionally undefined - dont use these!
	~tsOctTreeHeader_s();
private:
	// internal functions
	bool traceTriangleRay(u32 triangleNum, class trace_c &tr);
	bool traceNodeRay_r(u32 nodeNum, class trace_c &tr);
	void boxTriangles_r(const class aabb &bounds, class boxTrianglesCallback_i *callback, int nodeNum);
	bool logBoxTri(const class aabb &bounds, class boxTrianglesCallback_i *callback, u32 triangleNum);
public:
	// raycasting entry point
	bool traceRay(class trace_c &tr);
	// iterate all the triangles intersecting given bounds
	void boxTriangles(const class aabb &bounds, class boxTrianglesCallback_i *callback);

	// direct data access
	tsOctTreeNode_s *getNodes() {
		return (tsOctTreeNode_s*)(((byte*)this)+sizeof(*this));
	}
	u32 *getLeafTriIndexes() {
		return (u32*)(((byte*)this)+sizeof(*this)
			+ sizeof(tsOctTreeNode_s) * this->numNodes);
	}
	vec3_c *getTriPoints() {
		return (vec3_c*)(((byte*)this)+sizeof(*this)
			+ sizeof(tsOctTreeNode_s) * this->numNodes
			+ sizeof(u32) * numLeafTriIndexes);
	}
	u32 *getTriIndexes() {
		return (u32*)(((byte*)this)+sizeof(*this)
			+ sizeof(tsOctTreeNode_s) * this->numNodes
			+ sizeof(u32) * numLeafTriIndexes
			+ sizeof(vec3_c) * numVerts);
	}
	u32 getMemSize() const {
		return (sizeof(*this)
			+ sizeof(tsOctTreeNode_s) * this->numNodes
			+ sizeof(u32) * numLeafTriIndexes
			+ sizeof(vec3_c) * numVerts
			+ sizeof(u32) * numIndexes);
	}
};

tsOctTreeHeader_s *CMU_BuildTriSoupOctTree(const class cmSurface_c &in);
void CMU_FreeTriSoupOctTree(struct tsOctTreeHeader_s *h);

#endif // __CMTRISOUPOCTREE_H__