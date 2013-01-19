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
// rf_proc.h - header for rProcTree_c class
#ifndef __RF_PROC_H__
#define __RF_PROC_H__

#include <shared/array.h>
#include <shared/str.h>
#include <math/plane.h>

class procNode_c {
friend class procTree_c;
	plane_c plane;
	// positive child numbers are nodes
	// a child number of 0 is an opaque, solid area
	// negative child numbers are areas: (-1-child)
	int children[2];
};

class procArea_c {
friend class procTree_c;
	procArea_c() {
		areaModel = 0;
		visCount = -1;
		litCount = -1;
		checkCount = -1;
	}
	int visCount;
	int litCount;
	int checkCount;
	arraySTD_c<class procPortal_c*> portals;
	class r_model_c *areaModel; // this is a pointer to model from procTree_c::models array
};

class procTree_c {
	str procFileName;

	arraySTD_c<procNode_c> nodes;
	arraySTD_c<r_model_c*> models;
	arraySTD_c<procArea_c*> areas;
	arraySTD_c<procPortal_c*> portals;

	int visCount;
	int litCount;
	int checkCount;
	int camArea;

	procArea_c *getArea(u32 areaNum) {
		u32 needAreaCount = areaNum+1;
		if(needAreaCount > areas.size()) {
			u32 oldAreaCount = areas.size();
			areas.resize(needAreaCount);
			u32 addAreaCount = needAreaCount -  oldAreaCount;
			memset(&areas[oldAreaCount],0, addAreaCount * sizeof(procArea_c*));
		}
		if(areas[areaNum] == 0) {
			areas[areaNum] = new procArea_c;
		}
		procArea_c *ret = areas[areaNum];
		return ret;
	}
	void setAreaModel(u32 areaNum, r_model_c *newAreaModel);
	void addPortalToArea(u32 areaNum, procPortal_c *portal) {
		procArea_c *area = getArea(areaNum);
		area->portals.add_unique(portal);
	}
	void addPortalToAreas(procPortal_c *portal);
	void clear();
	bool parseNodes(class parser_c &p, const char *fname);
	bool parseAreaPortals(class parser_c &p, const char *fname);
	void drawArea(procArea_c *area);

	void addAreaDrawCalls_r(int areaNum, const class frustumExt_c &fr, procPortal_c *prevPortal);
	void traceNodeRay_r(int nodeNum, class trace_c &out);
	void boxAreas_r(const aabb &bb, arraySTD_c<u32> &out, int nodeNum);
	u32 createAreaDecals(u32 areaNum, class decalProjector_c &proj) const;
public:
	procTree_c() {
	}
	~procTree_c() {
		clear();
	}
	bool loadProcFile(const char *fname);
	bool loadProcFileBinary(const char *fname);

	u32 getAreaModels(arraySTD_c<const r_model_c *> &out) const {
		for(u32 i = 0; i < areas.size(); i++) {
			if(areas[i]->areaModel == 0)
				continue;
			out.push_back(areas[i]->areaModel);
		}
		return out.size();
	}

	bool isAreaVisibleByPlayer(int areaNum) const {
		if(areas[areaNum]->visCount == this->visCount)
			return true;
		return false;
	}

	int pointArea(const vec3_c &xyz);
	u32 boxAreas(const aabb &bb, arraySTD_c<u32> &out);
	u32 boxAreaSurfaces(const aabb &bb, arraySTD_c<const class r_surface_c*> &out);

	void addDrawCalls();
	bool traceRay(class trace_c &tr);
	int addWorldMapDecal(const vec3_c &pos, const vec3_c &normal, float radius, class mtrAPI_i *material);

	bool cullBoundsByPortals(const aabb &absBB);

	void addSingleAreaSurfacesInteractions(int areaNum, class rLightImpl_c *l);
	void cacheLightWorldInteractions_r(class rLightImpl_c *l, int areaNum, const frustumExt_c &fr, procPortal_c *prevPortal);
	void cacheLightWorldInteractions(class rLightImpl_c *l);
};

procTree_c *RF_LoadPROC(const char *fname);
procTree_c *RF_LoadPROCB(const char *fname);

#endif // __RF_PROC_H__

