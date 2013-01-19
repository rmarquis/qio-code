/*
============================================================================
Copyright (C) 2013 V.

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
// portalizedBSPTree.cpp - BSP tree with space divided into areas connected via portals
// This class stores:
// -BSP tree planes
// -BSP tree nodes
// -BSP areas (with portal indexes list)
// -BSP portals (with portal windings and area numbers)
// There is no GEOMETRY data stored in this class.
#include "portalizedBSPTree.h"
#include <shared/parser.h>

int portalizedBSPTree_c::pointAreaNum(const vec3_c &p) const {
	if(nodes.size() == 0)
		return 0;

	int nodeNum = 0;
	do {
		const pbspNode_c &node = this->nodes[nodeNum];
		float d = node.plane.distance(p);
		if(d > 0) {
			// front
			nodeNum = node.children[0];
		} else {
			// back
			nodeNum = node.children[1];
		}
	} while(nodeNum > 0);
	return -nodeNum - 1;
}
void portalizedBSPTree_c::boxAreaNums_r(int nodeNum) const {
	while(1) {
		if(nodeNum < 0) {
			int areaNum = (-nodeNum-1);
			if(areaNum >= areas.size()) {
				// this should never happen...
				g_core->RedWarning("portalizedBSPTree_c::boxAreaNums_r: node %i has area index %i out of range <0,%i)\n",
					nodeNum,areaNum,areas.size());
				return; 
			}
			const pbspArea_c *ar = &areas[areaNum];
			if(ar->checkCount != this->checkCount) {
				ar->checkCount = this->checkCount;
				if(boxArea.curAreaNums < boxArea.maxAreaNums) {
					boxArea.areaNums[boxArea.curAreaNums] = areaNum;
					boxArea.curAreaNums++;
				}
			}
			return; // done.
		}
		const pbspNode_c &n = nodes[nodeNum];
		planeSide_e ps = n.plane.onSide(boxArea.bb);

		if (ps == SIDE_FRONT) {
			if(n.children[0] == 0) {
				return;
			}
			nodeNum = n.children[0];
		} else if (ps == SIDE_BACK) {
			if(n.children[1] == 0) {
				return;
			}
			nodeNum = n.children[1];
		} else {
			if(n.children[0]) {
				nodeNum = n.children[0];
				if(n.children[1]) {
					boxAreaNums_r(n.children[1]);
				}
			} else {
				nodeNum = n.children[1];
			}
		}
	}	
}
u32 portalizedBSPTree_c::boxAreaNums(const aabb &bb, int *areaNums, int maxAreaNums) const {
	checkCount++;
	boxArea.bb = bb;
	boxArea.areaNums = areaNums;
	boxArea.curAreaNums = 0;
	boxArea.maxAreaNums = maxAreaNums;
	// start with root node
	boxAreaNums_r(0);
	return boxArea.curAreaNums;
}
void portalizedBSPTree_c::addPortalToArea(pbspPortal_c *portal, int areaNum) {
	if(areas.size() <= areaNum) {
		areas.resize(areaNum+1);
	}
	u32 portalIndex = portal - this->portals.getArray();
	areas[areaNum].portalIndexes.push_back(portalIndex);
}
void portalizedBSPTree_c::addPortalToAreas(pbspPortal_c *portal) {
	addPortalToArea(portal,portal->areas[0]);
	addPortalToArea(portal,portal->areas[1]);
}
bool portalizedBSPTree_c::load(const char *fname) {
	const char *ext = G_strgetExt(fname);
	bool result;
	if(!stricmp(ext,"proc")) {
		result = loadProcFile(fname);
	} else {
		result = true;
	}
	return result;
}
bool portalizedBSPTree_c::parseNodes(class parser_c &p, const char *fname) {
	if(p.atWord("{")==false) {
		g_core->RedWarning("procTree_c::parseNodes: expected '{' to follow \"nodes\" in file %s at line %i, found %s\n",
			fname,p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	u32 numNodes = p.getInteger();
	this->nodes.resize(numNodes);
	pbspNode_c *n = this->nodes.getArray();
	for(u32 i = 0; i < numNodes; i++, n++) {
		if(p.atWord("(")==false) {
			g_core->RedWarning("procTree_c::parseNodes: expected '(' to follow node %i in file %s at line %i, found %s\n",
				i,fname,p.getCurrentLineNumber(),p.getToken());
			return true; // error
		}
		p.getFloatMat(n->plane.norm,3);
		n->plane.dist = p.getFloat();
		if(p.atWord(")")==false) {
			g_core->RedWarning("procTree_c::parseNodes: expected '(' after node's %i plane equation in file %s at line %i, found %s\n",
				i,fname,p.getCurrentLineNumber(),p.getToken());
			return true; // error
		}
		n->children[0] = p.getInteger();
		n->children[1] = p.getInteger();

		// ensure that there is enough of areas allocated
		// (the area count isnt stored in .proc file,
		// we need to discover it ourselves)
		for(u32 j = 0; j < 2; j++) {
			if(n->children[j] < 0) {
				u32 areaNum = -n->children[j] - 1;
				if(areas.size() <= areaNum) {
					areas.resize(areaNum+1);
				}
			}
		}
	}
	if(p.atWord("}")==false) {
		g_core->RedWarning("procTree_c::parseNodes: expected closing '}' for \"nodes\" block in file %s at line %i, found %s\n",
			fname,p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	return false; // OK
}
bool portalizedBSPTree_c::parseAreaPortals(class parser_c &p, const char *fname) {
	if(p.atWord("{")==false) {
		g_core->RedWarning("procTree_c::parseAreaPortals: expected '{' to follow \"interAreaPortals\" in file %s at line %i, found %s\n",
			fname,p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	u32 numAreas = p.getInteger();
	u32 numAreaPortals = p.getInteger();
	this->portals.resize(numAreaPortals);
	for(u32 i = 0; i < numAreaPortals; i++) {
		pbspPortal_c *portal = &this->portals[i];
		u32 numPoints = p.getInteger();
		if(numPoints <= 2) {
			g_core->RedWarning("procTree_c::parseAreaPortals: WARNING: portal %i has less than three points! (%s at line %i)\n",i,fname,p.getCurrentLineNumber());
		}
		portal->areas[0] = p.getInteger();
		portal->areas[1] = p.getInteger();
		portal->points.resize(numPoints);
		vec3_c *v = portal->points.getArray();
		for(u32 j = 0; j < numPoints; j++, v++) {
			p.getFloatMat_braced(&v->x,3);
		}
	//	portal->calcPlane();
	//	portal->calcBounds();
		// let the areas know that they are connected by a portal
		this->addPortalToAreas(portal);
	}
	if(p.atWord("}")==false) {
		g_core->RedWarning("procTree_c::parseAreaPortals: expected closing '}' for \"interAreaPortals\" block in file %s at line %i, found %s\n",
			fname,p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	return false; // OK
}
bool portalizedBSPTree_c::loadProcFile(const char *fname) {
	parser_c p;
	if(p.openFile(fname)) {
		g_core->RedWarning("portalizedBSPTree_c::loadProcFile: cannot open %s\n",fname);
		return true; // error
	}

	// check for Doom3 ident first
	if(p.atWord("mapProcFile003") == false) {
		if(p.atWord("PROC")) {
			// Quake4 ident
			str version = p.getToken();
		} else {
			g_core->RedWarning("portalizedBSPTree_c::loadProcFile: %s has bad ident %s, should be %s or %s\n",fname,p.getToken(),"mapProcFile003","PROC");
			return true; // error
		}
	}

	while(p.atEOF() == false) {
		if(p.atWord("model")) {
			// we dont need geometry data
			p.skipCurlyBracedBlock();
		} else if(p.atWord("interAreaPortals")) {
			if(parseAreaPortals(p,fname)) {
				return true;
			}
		} else if(p.atWord("nodes")) {
			if(parseNodes(p,fname)) {
				return true;
			}
		} else if(p.atWord("shadowModel")) {
			p.skipCurlyBracedBlock();
		} else {
			g_core->RedWarning("portalizedBSPTree_c::loadProcFile: skipping unknown token %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
		}
	}
	return false;
}