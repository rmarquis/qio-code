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
// sv_vis.cpp - serverside visibility checks (BSP PVS) 
// used to cull entities before sending them to players
#include "server.h"
#include "sv_vis.h"
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include <shared/shared.h>
#include <math/plane.h>

svBSP_c::svBSP_c() {
	h = 0;
	areaPortalStates = 0;
}
svBSP_c::~svBSP_c() {
	clear();
}
void svBSP_c::clear() {
	if(vis) {
		free(vis);
		vis = 0;
	}
	if(areaPortalStates) {
		free(areaPortalStates);
		areaPortalStates = 0;
	}
}
bool svBSP_c::loadPlanes(u32 lumpPlanes) {
	const lump_s &pll = h->getLumps()[lumpPlanes];
	if(pll.fileLen % sizeof(q3Plane_s)) {
		g_core->Print(S_COLOR_RED "svBSP_c::loadPlanes: invalid planes lump size\n");
		return true; // error
	}
	u32 numPlanes = pll.fileLen / sizeof(q3Plane_s);
	planes.resize(numPlanes);
	memcpy(planes.getArray(),h->getLumpData(lumpPlanes),pll.fileLen);
	return false; // OK
}
bool svBSP_c::loadNodesAndLeaves(u32 lumpNodes, u32 lumpLeaves, u32 sizeOfLeaf) {
	const lump_s &nl = h->getLumps()[lumpNodes];
	if(nl.fileLen % sizeof(q3Node_s)) {
		g_core->Print(S_COLOR_RED "svBSP_c::loadNodesAndLeaves: invalid nodes lump size\n");
		return true; // error
	}
	const lump_s &ll = h->getLumps()[lumpLeaves];
	if(ll.fileLen % sizeOfLeaf) {
		g_core->Print(S_COLOR_RED "svBSP_c::loadNodesAndLeaves: invalid leaves lump size\n");
		return true; // error
	}
	u32 numNodes = nl.fileLen / sizeof(q3Node_s);
	u32 numLeaves = ll.fileLen / sizeOfLeaf;
	nodes.resize(numNodes);
	leaves.resize(numLeaves);
	memcpy_strided(nodes.getArray(),h->getLumpData(lumpNodes),numNodes,sizeof(svBSPNode_s),sizeof(svBSPNode_s),sizeof(q3Node_s));
	memcpy_strided(leaves.getArray(),h->getLumpData(lumpLeaves),numLeaves,sizeof(svBSPLeaf_s),sizeof(svBSPLeaf_s),sizeOfLeaf);
	// find the areas count
	int numAreas = -1;
	const svBSPLeaf_s *l = leaves.getArray();
	for(u32 i = 0; i < leaves.size(); i++, l++) {
		if(l->area >= numAreas) {
			numAreas = l->area+1;
		}
	}
	areas.resize(numAreas);
	if(numAreas) {
		areaPortalStates = (int*)malloc(numAreas*numAreas*sizeof(int));
		memset(areaPortalStates,0,numAreas*numAreas*sizeof(int));
		floodAreaConnections();
	}
	return false; // OK
}
bool svBSP_c::loadVisibility(u32 visLump) {
	const lump_s &vl = h->getLumps()[visLump];
	if(vl.fileLen == 0) {
		g_core->Print(S_COLOR_YELLOW "svBSP_c::loadVis: visibility lump is emtpy\n");
		return false; // dont do the error
	}
	vis = (visHeader_s*)malloc(vl.fileLen);
	memcpy(vis,h->getLumpData(visLump),vl.fileLen);
	return false; // no error
}
bool svBSP_c::load(const char *fname) {
	byte *fileData = 0;
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		g_core->Print(S_COLOR_RED "svBSP_c::load: cannot open %s\n",fname);
		return true;
	}
	h = (const q3Header_s*) fileData;
	if(h->ident == BSP_IDENT_IBSP && (h->version == BSP_VERSION_Q3 || h->version == BSP_VERSION_ET)) {
		if(loadNodesAndLeaves(Q3_NODES,Q3_LEAVES,sizeof(q3Leaf_s))) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadPlanes(Q3_PLANES)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadVisibility(Q3_VISIBILITY)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
	} else if(h->ident == BSP_IDENT_2015|| h->ident == BSP_IDENT_EALA) {
		if(loadNodesAndLeaves(MOH_NODES,MOH_LEAVES,sizeof(q3Leaf_s)+16)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadPlanes(MOH_PLANES)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
		if(loadVisibility(MOH_VISIBILITY)) {
			g_vfs->FS_FreeFile(fileData);
			return true; // error
		}
	} else {
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	g_vfs->FS_FreeFile(fileData);
	h = 0; // pointer to header is no longer valid
	return false; // no error
}

void svBSP_c::filterBB_r(const class aabb &bb, struct bspBoxDesc_s &out, int nodeNum) const {
	while(nodeNum >= 0) {
		const svBSPNode_s &node = nodes[nodeNum];
		const q3Plane_s &pl = planes[node.planeNum];
		int s = pl.onSide(bb);
		if (s == SIDE_FRONT) {
			nodeNum = node.children[0];
		} else if (s == SIDE_BACK) {
			nodeNum = node.children[1];
		} else {
			// check both children
			filterBB_r(bb, out, node.children[0]);
			nodeNum = node.children[1];
		}
	}
	int leafNum = (-nodeNum - 1);
	out.leaves.push_back(leafNum);
	const svBSPLeaf_s &l = leaves[leafNum];
	if(l.cluster != -1) {
		out.clusters.add_unique(l.cluster);
		out.areas.add_unique(l.area);
	}
}
void svBSP_c::filterBB(const class aabb &bb, struct bspBoxDesc_s &out) const {
	out.clear();
	filterBB_r(bb,out,0);
}
void svBSP_c::filterPoint(const class vec3_c &p, struct bspPointDesc_s &out) const {
	// special case for empty bsp trees - is this correct?
	if(nodes.size() == 0) {
		out.area = 0;
		out.cluster = 0;
		out.leaf = 0;
		return;
	}
	int index = 0;
	do {
		const svBSPNode_s &n = nodes[index];
		const q3Plane_s &pl = planes[n.planeNum];
		float d = pl.distance(p);
		if (d >= 0) 
			index = n.children[0]; // front
		else 
			index = n.children[1]; // back
	}
	while (index > 0) ;
	out.leaf = -index - 1;
	const svBSPLeaf_s &l = leaves[out.leaf];
	out.cluster = l.cluster;
	if(l.cluster != -1 && vis && vis->clusterSize) {
		out.clusterPVS = &vis->data[l.cluster * vis->clusterSize];
	} else {
		out.clusterPVS = 0;
	}
	out.area = l.area;
}

bool svBSP_c::checkVisibility(struct bspPointDesc_s &p, const struct bspBoxDesc_s &box) const {
	bool bAreasConnected = false;
	for(u32 i = 0; i < box.areas.size(); i++) {
		int boxArea = box.areas[i];
		if(areasConnected(p.area,boxArea)) {
			bAreasConnected = true;
			break;
		}
	}
	if(bAreasConnected == false) {
		return false; // culled by areas
	}
	if(p.clusterPVS) {
		bool clusterVisible = false;
		for(u32 i = 0; i < box.clusters.size(); i++) {
			int cluster = box.clusters[i];
			if(p.clusterPVS[cluster >> 3] & (1 << (cluster & 7))) {
				clusterVisible = true; // visible
				break;
			}
		}
		if(clusterVisible == false) {
			return false; // culled by clusters
		}
	}
	return true; // visible
}
void svBSP_c::floodArea_r(int areaNum, int floodNum) {
	svBSPArea_s &area = areas[areaNum];
	if(area.floodValid == floodValid) {
		if(area.floodNum == floodNum)
			return;
		g_core->Print(S_COLOR_RED"svBSP_c::floodArea_r: reflooded\n");
		return;
	}

	area.floodNum = floodNum;
	area.floodValid = floodValid;
	int *con = areaPortalStates + areaNum * areas.size();
	for(u32 i = 0; i < areas.size(); i++) {
		if(con[i] > 0) {
			floodArea_r(i, floodNum);
		}
	}
}

void svBSP_c::floodAreaConnections() {
	// all current floods are now invalid
	floodValid++;
	int floodNum = 0;

	for(u32 i = 0; i < areas.size(); i++) {
		svBSPArea_s &area = areas[i];
		if(area.floodValid == floodValid) {
			continue;			// already flooded into
		}
		floodNum++;
		floodArea_r(i, floodNum);
	}
}
bool svBSP_c::areasConnected(int area0, int area1) const {
	if(area0 < 0 || area1 < 0) {
		return false;
	}
	if(areas[area0].floodNum == areas[area1].floodNum)
		return true;
	return false;
}
void svBSP_c::adjustAreaPortalState(int area0, int area1, bool open) {
	if(area0 < 0 || area1 < 0) {
		return;
	}
	if(area0 >= areas.size()) {
		g_core->Print(S_COLOR_RED"svBSP_c::adjustAreaPortalState: area index %i out of range <0,%i)\n",
			area0,areas.size());
		return;
	}
	if(area1 >= areas.size()) {
		g_core->Print(S_COLOR_RED"svBSP_c::adjustAreaPortalState: area index %i out of range <0,%i)\n",
			area1,areas.size());
		return;
	}

	if(open) {
		areaPortalStates[area0 * areas.size() + area1]++;
		areaPortalStates[area1 * areas.size() + area0]++;
	} else {
		areaPortalStates[area0 * areas.size() + area1]--;
		areaPortalStates[area1 * areas.size() + area0]--;
		if(areaPortalStates[area1 * areas.size() + area0] < 0) {
			g_core->Print(S_COLOR_RED"svBSP_c::adjustAreaPortalState: negative portal reference count!\n");
		}
	}
	floodAreaConnections();
}
u32 svBSP_c::getNumAreaBytes() const {
	return (areas.size() + 7) >> 3;
}
#include <shared/bitset.h>
void svBSP_c::appendCurrentAreaBits(int area, class bitSet_c &bs) {
	if(area == -1) {
		bs.setAll(true); // mark all areas as visible
	} else {
		u32 floodNum = areas[area].floodNum;
		for(u32 i = 0; i < areas.size(); i++) {
			if(areas[i].floodNum == floodNum)
				bs.set(i,true);
		}
	}
}
