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
// rf_proc.cpp - Doom3/Quake4 .proc rendering code
#include "rf_proc.h"
#include "rf_surface.h"
#include "rf_local.h"
#include "rf_model.h"
#include <api/coreAPI.h>
#include <shared/parser.h>
#include <shared/cmWinding.h>
#include <math/frustumExt.h>

class procPortal_c {
friend class procTree_c;
	int areas[2];
	cmWinding_c points;
	plane_c plane;
	aabb bounds;

	void calcPlane() {
		points.getPlane(plane);
	}
	void calcBounds() {
		points.getBounds(bounds);
	}
};

void procTree_c::setAreaModel(u32 areaNum, r_model_c *newAreaModel) {
	procArea_c *area = getArea(areaNum);
	if(area->areaModel) {
		g_core->RedWarning("procTree_c::setAreaModel: WARNING: area %i already has its model set (modName: %s)\n",areaNum,area->areaModel->getName());
		return;
	}
	area->areaModel = newAreaModel;
}
void procTree_c::addPortalToAreas(procPortal_c *portal) {
	addPortalToArea(portal->areas[0],portal);
	addPortalToArea(portal->areas[1],portal);
}
void procTree_c::clear() {
	nodes.clear();
	for(u32 i = 0; i < models.size(); i++) {
		if(models[i]) {
			delete models[i];
		}
	}
	models.clear();
	for(u32 i = 0; i < areas.size(); i++) {
		if(areas[i]) {
			delete areas[i];
		}
	}
	areas.clear();
	for(u32 i = 0; i < portals.size(); i++) {
		if(portals[i]) {
			delete portals[i];
		}
	}
	portals.clear();
}
bool procTree_c::parseNodes(class parser_c &p, const char *fname) {
	if(p.atWord("{")==false) {
		g_core->RedWarning("procTree_c::parseNodes: expected '{' to follow \"nodes\" in file %s at line %i, found %s\n",
			fname,p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	u32 numNodes = p.getInteger();
	this->nodes.resize(numNodes);
	procNode_c *n = this->nodes.getArray();
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
	}
	if(p.atWord("}")==false) {
		g_core->RedWarning("procTree_c::parseNodes: expected closing '}' for \"nodes\" block in file %s at line %i, found %s\n",
			fname,p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	return false; // OK
}
bool procTree_c::parseAreaPortals(class parser_c &p, const char *fname) {
	if(p.atWord("{")==false) {
		g_core->RedWarning("procTree_c::parseAreaPortals: expected '{' to follow \"interAreaPortals\" in file %s at line %i, found %s\n",
			fname,p.getCurrentLineNumber(),p.getToken());
		return true; // error
	}
	u32 numAreas = p.getInteger();
	u32 numAreaPortals = p.getInteger();
	this->portals.resize(numAreaPortals);
	for(u32 i = 0; i < numAreaPortals; i++) {
		procPortal_c *portal = this->portals[i] = new procPortal_c;
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
		portal->calcPlane();
		portal->calcBounds();
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
bool procTree_c::loadProcFile(const char *fname) {
	parser_c p;
	if(p.openFile(fname)) {
		g_core->RedWarning("procTree_c::loadProcFile: cannot open %s\n",fname);
		return true; // error
	}

	// check for Doom3 ident first
	if(p.atWord("mapProcFile003") == false) {
		if(p.atWord("PROC")) {
			// Quake4 ident
			str version = p.getToken();
		} else {
			g_core->RedWarning("procTree_c::loadProcFile: %s has bad ident %s, should be %s or %s\n",fname,p.getToken(),"mapProcFile003","PROC");
			return true; // error
		}
	}

	while(p.atEOF() == false) {
		if(p.atWord("model")) {
			r_model_c *newModel = new r_model_c;
			this->models.push_back(newModel);
			if(newModel->parseProcModel(p)) {
				return true; // error occured during model parsing
			}
			model_c *m = RF_AllocModel(newModel->getName());
			m->initProcModel(this,newModel);	

			if(newModel->isAreaModel()) {
				u32 areaNum = newModel->getAreaNumber();
				this->setAreaModel(areaNum,newModel);
			}
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
			g_core->RedWarning("procTree_c::loadProcFile: skipping unknown token %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
		}
	}

	this->procFileName = fname;

	// create model VBOs and IBOs
	for(u32 i = 0; i < models.size(); i++) {
		r_model_c *m = models[i];
		m->createVBOsAndIBOs();
	}

	g_core->RedWarning("procTree_c::loadProcFile: %s has %i models, %i areas, %i portals, and %i nodes\n",
		fname,models.size(),areas.size(),portals.size(),nodes.size());

	return false; // OK
}
int procTree_c::pointArea(const vec3_c &xyz) {
	if(nodes.size() == 0)
		return 0;

	int nodeNum = 0;
	do {
		const procNode_c &node = this->nodes[nodeNum];
		float d = node.plane.distance(xyz);
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
void procTree_c::addAreaDrawCalls_r(int areaNum, const frustumExt_c &fr, procPortal_c *prevPortal) {
	if(areaNum >= areas.size() || areaNum < 0)
		return;
	procArea_c *ar = areas[areaNum];
	if(ar == 0)
		return;
	if(ar->visCount != this->visCount) {
		//drawAreaDrawCalls(ar);
		ar->areaModel->addDrawCalls();
		ar->visCount = this->visCount;
	}
	for(u32 i = 0; i < ar->portals.size(); i++) {
		procPortal_c *p = ar->portals[i];
		if(p == prevPortal) {
			continue;
		}
		// first check if the portal is in the frustum
		if(fr.cull(p->bounds) == CULL_OUT)
			continue;
		// then check if the portal side facing camera
		// belong to current area. If not, portal is occluded
		// by the wall and is not really visible
		float d = p->plane.distance(rf_camera.getOrigin());
		if(p->areas[0] == areaNum) {
			if(d > 0)
				continue;
		} else {
			if(d < 0)
				continue;
		}
		// adjust the frustum, so addAreaDrawCalls_r will never loop and cause a stack overflow...
		frustumExt_c adjusted;
		adjusted.adjustFrustum(fr,rf_camera.getOrigin(),p->points,p->plane);
		if(p->areas[0] == areaNum) {
			addAreaDrawCalls_r(p->areas[1],adjusted,p);
		} else {
			addAreaDrawCalls_r(p->areas[0],adjusted,p);
		}
	}
}
void procTree_c::addDrawCalls() {
	visCount++;
	int camArea = pointArea(rf_camera.getOrigin());
	printf("camera is in area %i of %i\n",camArea,areas.size());
	frustumExt_c baseFrustum(rf_camera.getFrustum());
	addAreaDrawCalls_r(camArea,baseFrustum,0);
}
#include <shared/trace.h>
void procTree_c::traceNodeRay_r(int nodeNum, class trace_c &out) {
	if(nodeNum < 0) {
		int areaNum = (-nodeNum-1);
		procArea_c *ar = areas[areaNum];
		if(ar->visCount != this->visCount) {
			ar->areaModel->traceRay(out);
			ar->visCount = this->visCount;
		}
		return; // done.
	}
	const procNode_c &n = nodes[nodeNum];
	// classify ray against split plane
	float d0 = n.plane.distance(out.getStartPos());
	// hitPos is the actual endpos of the trace
	float d1 = n.plane.distance(out.getHitPos());

	if (d0 >= 0 && d1 >= 0) {
		// trace is on the front side of the plane
		if(n.children[0]) {
		  traceNodeRay_r(n.children[0],out);
		}
	} else if (d0 < 0 && d1 < 0) {
		// trace is on the back side of the plane
		if(n.children[1]) {
			traceNodeRay_r(n.children[1],out);
		}
	} else {
		// trace crosses the plane - both childs must be checked.
		// TODO: clip the trace start/end points?
		if(n.children[0]) {
		   traceNodeRay_r(n.children[0],out);
		}
		if(n.children[1]) {
			traceNodeRay_r(n.children[1],out);
		}
	}
}	
bool procTree_c::traceRay(class trace_c &out) {
	visCount++;
	float prevFrac = out.getFraction();
	traceNodeRay_r(0,out);
	if(out.getFraction() < prevFrac)
		return true;
	return false;
}

procTree_c *RF_LoadPROC(const char *fname) {
	procTree_c *ret = new procTree_c;
	if(ret->loadProcFile(fname)) {
		delete ret;
		return 0;
	}
	return ret;
}
