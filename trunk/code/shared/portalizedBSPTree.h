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
// portalizedBSPTree.h - BSP tree with space divided into areas connected via portals
// This class stores:
// -BSP tree planes
// -BSP tree nodes
// -BSP areas (with portal indexes list)
// -BSP portals (with portal windings and area numbers)
// There is no GEOMETRY data stored in this class.

#ifndef __PORTALIZEDBSPTREE_H__
#define __PORTALIZEDBSPTREE_H__

#include <shared/str.h>
#include <shared/cmWinding.h>
#include <math/plane.h>
#include <math/aabb.h>
#include <api/portalizedWorldAPI.h>

class pbspPortal_c : public portalAPI_i {
friend class portalizedBSPTree_c;
	int areas[2];
	cmWinding_c points;

	virtual int getFrontArea() const {
		return areas[0];
	}
	virtual int getBackArea() const {
		return areas[1];
	}
	virtual int getOtherAreaNum(int oneOfAreas) const {
		if(oneOfAreas == areas[0])
			return areas[1];
		return areas[0];
	}
	virtual const class cmWinding_c *getWindingP() const {
		return &points;
	}
	virtual u32 getBlockingBits() const {
		return 0; // TODO?
	}
};
class pbspArea_c {
friend class portalizedBSPTree_c;
	arraySTD_c<u32> portalIndexes;
	mutable int checkCount;
};
class pbspNode_c {
friend class portalizedBSPTree_c;
	plane_c plane;
	// positive child numbers are nodes
	// a child number of 0 is an opaque, solid area
	// negative child numbers are areas: (-1-child)
	// negative children are AREAS, not leaves
	int children[2];
};

class portalizedBSPTree_c : public portalizedWorldAPI_i {
	str name;
	mutable int checkCount;

	arraySTD_c<plane_c> planes;
	arraySTD_c<pbspNode_c> nodes;
	arraySTD_c<pbspArea_c> areas;
	arraySTD_c<pbspPortal_c> portals;

	void addPortalToArea(pbspPortal_c *portal, int areaNum);
	void addPortalToAreas(pbspPortal_c *portal);

	bool parseNodes(class parser_c &p, const char *fname);
	bool parseAreaPortals(class parser_c &p, const char *fname);

	virtual u32 getNumPortalsInArea(u32 areaNum) const {
		return areas[areaNum].portalIndexes.size();
	}

	mutable struct {
		aabb bb;
		int *areaNums;
		int curAreaNums;
		int maxAreaNums;
	} boxArea;
	void boxAreaNums_r(int nodeNum) const;

	virtual int pointAreaNum(const vec3_c &p) const;
	virtual u32 boxAreaNums(const aabb &bb, int *areaNums, int maxAreaNums) const;

	virtual const class portalAPI_i *getPortal(u32 areaNum, u32 localPortalNum) const {
		u32 absPortalNum = areas[areaNum].portalIndexes[localPortalNum];
		return &portals[absPortalNum];
	}
public:
	bool load(const char *fname);
	bool loadProcFile(const char *fname);

	virtual u32 getNumAreas() const {
		return areas.size();
	}
	virtual u32 getNumPortals() const {
		return portals.size();
	}
};

#endif // __PORTALIZEDBSPTREE_H__
