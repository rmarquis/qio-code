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
// cmTriSoupOctTree.cpp
#include "cmTriSoupOctTree.h"
#include "cmSurface.h"
#include "trace.h"
#include "autoCvar.h"
#include <api/boxTrianglesCallback.h>

// check all of the triangles in octTree instead of using node/leaves data (default false)
static aCvar_c cms_tsOctTree_checkAllTris("cms_tsOctTree_checkAllTris","0");
// use checkCounter to avoid testing the same triangle more than once (default true)
static aCvar_c cms_tsOctTree_useCheckCounts("cms_tsOctTree_useCheckCounts","1");

// ======================================
//
//	tsOctTreeHeader_s collision detection
//
// ======================================
bool tsOctTreeHeader_s::traceTriangleRay(u32 triangleNum, class trace_c &tr) { 
	// see if the triangle was already checked
	if(cms_tsOctTree_useCheckCounts.getInt() && this->triCheckCounts[triangleNum] == this->checkCount){
		return false;
	}
	// mark as already checked
	this->triCheckCounts[triangleNum] = this->checkCount;

	const u32 *indices = this->getTriIndexes();
	u32 i0 = indices[triangleNum*3+0];
	u32 i1 = indices[triangleNum*3+1];
	u32 i2 = indices[triangleNum*3+2];
	const vec3_c &p0 = this->getTriPoints()[i0];
	const vec3_c &p1 = this->getTriPoints()[i1];
	const vec3_c &p2 = this->getTriPoints()[i2];
	if(tr.clipByTriangle(p0,p1,p2,true)) {
		tr.setHitTriangleIndex(triangleNum);
		return true;
	}
	return false;
}
bool tsOctTreeHeader_s::traceNodeRay_r(u32 nodeNum, class trace_c &tr) {
	tsOctTreeNode_s &node = this->getNodes()[nodeNum];
	if(tr.getTraceBounds().intersect(node.bounds) == false)
		return false;
	if(node.plane.axis == -1) {
		bool hit = false;
		const u32 *leafTriIndexes = this->getLeafTriIndexes() + node.firstTri;
		for(u32 i = 0; i < node.numTris; i++) {
			u32 triNum = leafTriIndexes[i];
			if(traceTriangleRay(triNum,tr)) {
				hit = true;
			}
		}
		return hit; // nothing else to do for leaves
	}
	float d0 = node.plane.distance(tr.getStartPos());
	float d1 = node.plane.distance(tr.getHitPos());
	const float eps = 0.01f;
	if(d0 > eps && d1 > eps)
		return traceNodeRay_r(node.children[0],tr);
	if(d0 < -eps && d1 < -eps)
		return traceNodeRay_r(node.children[1],tr);

	bool hit = false;
	if(traceNodeRay_r(node.children[0],tr)) {
		hit = true;
	}
	if(traceNodeRay_r(node.children[1],tr)) {
		hit = true;
	}
	return hit;
}
bool tsOctTreeHeader_s::traceRay(class trace_c &tr) {
	this->checkCount++;
	if(cms_tsOctTree_checkAllTris.getInt()) {
		bool hit = false;
		u32 numTris = this->numIndexes/3;
		for(u32 i = 0; i < numTris; i++) {
			if(traceTriangleRay(i,tr)) {
				hit = true;
			}
		}
		return hit;
	} 
	return traceNodeRay_r(0,tr);
}
bool tsOctTreeHeader_s::logBoxTri(const class aabb &bounds, class boxTrianglesCallback_i *callback, u32 triangleNum) { 
	// see if the triangle was already checked
	if(cms_tsOctTree_useCheckCounts.getInt() && this->triCheckCounts[triangleNum] == this->checkCount){
		return false;
	}
	// mark as already checked
	this->triCheckCounts[triangleNum] = this->checkCount;

	const u32 *indices = this->getTriIndexes();
	u32 i0 = indices[triangleNum*3+0];
	u32 i1 = indices[triangleNum*3+1];
	u32 i2 = indices[triangleNum*3+2];
	const vec3_c &p0 = this->getTriPoints()[i0];
	const vec3_c &p1 = this->getTriPoints()[i1];
	const vec3_c &p2 = this->getTriPoints()[i2];
	aabb tmpBB;
	tmpBB.fromThreePoints(p0,p1,p2);
	if(tmpBB.intersect(bounds) == false)
		return false;

	callback->onBoxTriangle(p0,p1,p2);
	return true;
}
void tsOctTreeHeader_s::boxTriangles_r(const class aabb &bounds, class boxTrianglesCallback_i *callback, int nodeNum) {
	while(1) {
		tsOctTreeNode_s &node = this->getNodes()[nodeNum];
		if(bounds.intersect(node.bounds) == false)
			return;
		if(node.plane.axis == -1) {
			const u32 *leafTriIndexes = this->getLeafTriIndexes() + node.firstTri;
			for(u32 i = 0; i < node.numTris; i++) {
				u32 triNum = leafTriIndexes[i];
				logBoxTri(bounds,callback,triNum);
			}
			return; // nothing else to do for leaves
		}
		planeSide_e side = node.plane.onSide(bounds);
		if(side == SIDE_FRONT) {
			nodeNum = node.children[0];
		} else if(side == SIDE_BACK) {
			nodeNum = node.children[1];
		} else {
			nodeNum = node.children[0];
			boxTriangles_r(bounds,callback,node.children[1]);
		}
	}
}
void tsOctTreeHeader_s::boxTriangles(const class aabb &bounds, class boxTrianglesCallback_i *callback) {
	this->checkCount++;
	//if(cms_tsOctTree_checkAllTris.getInt()) {
	//
	//} 
	boxTriangles_r(bounds,callback,0);
}
// ======================================
//
//	tsOctTreeHeader_s "compiler"
//
// ======================================
static arraySTD_c<tsOctTreeNode_s> ts_nodes;
static const cmSurface_c *ts_sourceSurf;
static arraySTD_c<u32> ts_leafTris;
static u32 ts_maxDepth = 10;
static u32 ts_minTrisForNode = 32;

u16 CMU_BuildNode(const arraySTD_c<u32> &triNums, const aabb &prevBB, u32 depth) {
	u16 nodeNum = ts_nodes.size();
	ts_nodes.pushBack();
	aabb bb = prevBB;
	// try to tighten the bounds
#if 1
	aabb exactBounds;
	ts_sourceSurf->calcTriListBounds(triNums,exactBounds);
	for(u32 i = 0; i < 3; i++) {
		if(exactBounds.mins[i] > bb.mins[i]) {
			bb.mins[i] = exactBounds.mins[i];
		}
		if(exactBounds.maxs[i] < bb.maxs[i]) {
			bb.maxs[i] = exactBounds.maxs[i];
		}
	}
#endif
	ts_nodes[nodeNum].bounds = bb;
	if(depth > ts_maxDepth) {
		ts_nodes[nodeNum].plane.axis = -1;
		ts_nodes[nodeNum].firstTri = ts_leafTris.size();
		ts_nodes[nodeNum].numTris = triNums.size();
		ts_leafTris.addArray(triNums);
		return nodeNum;
	}
	if(triNums.size() < ts_minTrisForNode) {
		ts_nodes[nodeNum].plane.axis = -1;
		ts_nodes[nodeNum].firstTri = ts_leafTris.size();
		ts_nodes[nodeNum].numTris = triNums.size();
		ts_leafTris.addArray(triNums);
		return nodeNum;
	}
	aaPlane_s pl;
	pl.axis = bb.getLargestAxis();
	float axLen = bb.getLargestAxisLen();
	if(axLen < 8.f) {
		ts_nodes[nodeNum].plane.axis = -1;
		ts_nodes[nodeNum].firstTri = ts_leafTris.size();
		ts_nodes[nodeNum].numTris = triNums.size();
		ts_leafTris.addArray(triNums);
		return nodeNum;
	}
	pl.dist = -(bb.mins[pl.axis] + axLen*0.5f);
	arraySTD_c<u32> front, back;
	u32 c_front = 0;
	u32 c_back = 0;
	u32 c_split = 0;
	for(u32 i = 0; i < triNums.size(); i++) {
		u32 triangleIndex = triNums[i];
		u32 i0 = ts_sourceSurf->getIndex(triangleIndex*3+0);
		u32 i1 = ts_sourceSurf->getIndex(triangleIndex*3+1);
		u32 i2 = ts_sourceSurf->getIndex(triangleIndex*3+2);
		const vec3_c &p0 = ts_sourceSurf->getVert(i0);
		const vec3_c &p1 = ts_sourceSurf->getVert(i1);
		const vec3_c &p2 = ts_sourceSurf->getVert(i2);
		float d0 = pl.distance(p0);
		float d1 = pl.distance(p1);
		float d2 = pl.distance(p2);
		const float eps = 0.01f;
		if(d0 > eps && d1 > eps && d2 > eps) {
			front.push_back(triangleIndex);
			c_front++;
		} else if(d0 < -eps && d1 < -eps && d2 < -eps) {
			back.push_back(triangleIndex);
			c_back++;
		} else {
			front.push_back(triangleIndex);
			back.push_back(triangleIndex);
			c_split++;
		}
	}
	if(c_front == 0 && c_back == 0) {
		ts_nodes[nodeNum].plane.axis = -1;
		ts_nodes[nodeNum].firstTri = ts_leafTris.size();
		ts_nodes[nodeNum].numTris = triNums.size();
		ts_leafTris.addArray(triNums);
		return nodeNum;
	}
	ts_nodes[nodeNum].plane = pl;

	aabb frontBB = bb;
	aabb backBB = bb;
	frontBB.mins[pl.axis] = backBB.maxs[pl.axis] = -pl.dist;

	u16 children[2];
	children[0] = CMU_BuildNode(front,frontBB,depth+1);
	children[1] = CMU_BuildNode(back,backBB,depth+1);
	ts_nodes[nodeNum].children[0] = children[0];
	ts_nodes[nodeNum].children[1] = children[1];
	return nodeNum;
}
tsOctTreeHeader_s *CMU_BuildTriSoupOctTree(const class cmSurface_c &in) {
	ts_sourceSurf = &in;

	aabb baseBounds = in.getAABB();
	arraySTD_c<u32> triNums;
	for(u32 i = 0; i < in.getNumTris(); i++) {
		triNums.push_back(i);
	}
	CMU_BuildNode(triNums,baseBounds,0);

	u32 neededMemory = sizeof(tsOctTreeHeader_s) +
		ts_nodes.getSizeInBytes() +
		ts_leafTris.getSizeInBytes() +
		in.getNumVerts() * sizeof(vec3_c) +
		in.getNumIndices() * sizeof(u32);
	// tsOctTree structure uses a single block of memory,
	// so it can be immediately loaded/saved to file
	tsOctTreeHeader_s *ret = (tsOctTreeHeader_s*)malloc(neededMemory);
	ret->numIndexes = in.getNumIndices();
	ret->numLeafTriIndexes = ts_leafTris.size();
	ret->numVerts = in.getNumVerts();
	ret->numNodes = ts_nodes.size();
	ret->checkCount = 0;
	ret->triCheckCounts = (u32*)malloc((ret->numIndexes/3)*sizeof(u32));
	memcpy(ret->getNodes(),ts_nodes.getArray(),ts_nodes.getSizeInBytes());
	memcpy(ret->getLeafTriIndexes(),ts_leafTris.getArray(),ts_leafTris.getSizeInBytes());
	memcpy(ret->getTriIndexes(),in.getIndices(),in.getNumIndices()*sizeof(u32));
	memcpy(ret->getTriPoints(),in.getVerts(),in.getNumVerts()*sizeof(vec3_c));
	assert(memcmp(ret->getNodes(),ts_nodes.getArray(),ts_nodes.getSizeInBytes())==0);
	assert(memcmp(ret->getLeafTriIndexes(),ts_leafTris.getArray(),ts_leafTris.getSizeInBytes())==0);
	assert(memcmp(ret->getTriIndexes(),in.getIndices(),in.getNumIndices()*sizeof(u32))==0);
	assert(memcmp(ret->getTriPoints(),in.getVerts(),in.getNumVerts()*sizeof(vec3_c))==0);

	ts_nodes.clear();
	ts_leafTris.clear();
	ts_sourceSurf = 0;
	return ret;
}

void CMU_FreeTriSoupOctTree(struct tsOctTreeHeader_s *h) {
	free(h->triCheckCounts);
	free(h);
}

