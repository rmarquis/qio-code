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
// rf_map.cpp - loads world map directly from .map file
#include "rf_local.h"
#include "rf_surface.h"
#include "rf_model.h"
#include "rf_bezier.h"
#include <api/coreAPI.h>
#include <api/modelLoaderDLLAPI.h>

class rWorldMapLoader_c : public staticModelCreatorAPI_i {
	u32 currentEntityNum;
	arraySTD_c<r_model_c*> entModels;
	// model post process funcs api impl
	virtual void scaleXYZ(float scale) {
		for(u32 i = 0; i < entModels.size(); i++) {
			if(entModels[i]) {
				entModels[i]->scaleXYZ(scale);
			}
		}
	}
	virtual void swapYZ() {
		for(u32 i = 0; i < entModels.size(); i++) {
			if(entModels[i]) {
				entModels[i]->swapYZ();
			}
		}
	}
	virtual void swapIndexes() {
		for(u32 i = 0; i < entModels.size(); i++) {
			if(entModels[i]) {
				entModels[i]->swapIndexes();
			}
		}
	}
	virtual void translateY(float ofs) {
		for(u32 i = 0; i < entModels.size(); i++) {
			if(entModels[i]) {
				entModels[i]->translateY(ofs);
			}
		}
	}
	virtual void multTexCoordsY(float f) {
		for(u32 i = 0; i < entModels.size(); i++) {
			if(entModels[i]) {
				entModels[i]->multTexCoordsY(f);
			}
		}
	}
	virtual void multTexCoordsXY(float f) {
		for(u32 i = 0; i < entModels.size(); i++) {
			if(entModels[i]) {
				entModels[i]->multTexCoordsXY(f);
			}
		}
	}
	virtual void translateXYZ(const class vec3_c &ofs) {
		for(u32 i = 0; i < entModels.size(); i++) {
			if(entModels[i]) {
				entModels[i]->translateXYZ(ofs);
			}
		}
	}
	// well I dont think those functions are needed for .map files
	virtual void getCurrentBounds(class aabb &out) {
	}
	virtual void setAllSurfsMaterial(const char *newMatName) {
	}
	virtual u32 getNumSurfs() const {
		return 0;
	}
	virtual void setSurfsMaterial(const u32 *surfIndexes, u32 numSurfIndexes, const char *newMatName) {
	}
	virtual void clear() {

	}
public:	
	virtual void addTriangle(const char *matName, const struct simpleVert_s &v0,
		const struct simpleVert_s &v1, const struct simpleVert_s &v2) {
		// ensure that model is allocated
		if(entModels[currentEntityNum] == 0) {
			entModels[currentEntityNum] = new r_model_c;
		}
		entModels[currentEntityNum]->addTriangle(matName,v0,v1,v2);
	}
	virtual void addTriangleToSF(u32 surfNum, const struct simpleVert_s &v0,
		const struct simpleVert_s &v1, const struct simpleVert_s &v2) {
		// NOT NEEDED FOR .MAP FILES
	}
	virtual void resizeVerts(u32 newNumVerts) { }
	virtual void setVert(u32 vertexIndex, const struct simpleVert_s &v) { }
	virtual void setVertexPos(u32 vertexIndex, const vec3_c &newPos) { }
	virtual void resizeIndices(u32 newNumIndices) { }
	virtual void setIndex(u32 indexNum, u32 value) { }
	virtual void recalcBoundingBoxes() {
		if(entModels[currentEntityNum]) {
			entModels[currentEntityNum]->recalcBoundingBoxes();
		}
	}

	// only for .map -> trimesh converter
	virtual void onNewMapEntity(u32 entityNum) {
		currentEntityNum = entityNum;
		entModels.resize(currentEntityNum+1);
		entModels[currentEntityNum] = 0;//new r_model_c;
	}
	virtual bool onBezierPatch(const char *patchDefStart, const char *patchDefEnd) {
		// ensure that model is allocated
		if(entModels[currentEntityNum] == 0) {
			entModels[currentEntityNum] = new r_model_c;
		}
		r_model_c *mod = entModels[currentEntityNum];
		r_bezierPatch_c *bezierPatch = new r_bezierPatch_c;
		if(bezierPatch->fromString(patchDefStart,patchDefEnd)) {
			g_core->RedWarning("rWorldMapLoader_c::onBezierPatch: failed to parse bezier patch text def\n");
			delete bezierPatch;
		} else {
			mod->addPatch(bezierPatch);
		}
		return true;
	}
	r_model_c *getWorldModel() const {
		if(entModels.size() == 0) {
			g_core->RedWarning("rWorldMapLoader_c::getWorldModel: world map loader has 0 entModels\n");
			return 0;
		}
		return entModels[0];
	}
	void registerSubModels() {
		if(entModels.size() == 0) {
			return; // nothing to do, no models at all
		}

		// upload world data to GPU
		if(entModels[0]) {
			entModels[0]->createVBOsAndIBOs();
		}

		u32 modelNum = 1;
		for(u32 i = 1; i < entModels.size(); i++) {
			r_model_c *m = entModels[i];
			if(m == 0) {
				continue;
			}
			if(m->getNumSurfs() == 0) {
				delete m;
				continue;
			}
#if 0
			// center of mass of model must be at 0 0 0 for Bullet...
			// let's do this the simplest way
			///THIS IS NO LONGER NEEDED
			vec3_c center = m->getBounds().getCenter();
			m->translateXYZ(-center);
#endif
			str modName = va("*%i",modelNum);
			model_c *mod = RF_AllocModel(modName);
			// upload model data to GPU
			m->createVBOsAndIBOs();
			// precalculate data for stencil shadow volumes generation
			m->precalculateStencilShadowCaster();
			mod->initStaticModel(m);
			entModels[i] = 0;
			modelNum++;
		}
	}
	void calcNormals() {
		for(u32 i = 0; i < entModels.size(); i++) {
			if(entModels[i]) {
				entModels[i]->recalcModelNormals();
			}
		}
	}
	void calcTBNs() {
		for(u32 i = 0; i < entModels.size(); i++) {
			if(entModels[i]) {
				entModels[i]->recalcModelTBNs();
			}
		}
	}
};

// used to load entire world scene directly from .map file
// (without using .bsp / .proc data)
class r_model_c *RF_LoadMAPFile(const char *fname) {
	rWorldMapLoader_c loader;
	if(g_modelLoader->loadStaticModelFile(fname,&loader)) {
		return 0;
	}
	loader.calcTBNs();
	loader.registerSubModels();
	return loader.getWorldModel();
}


