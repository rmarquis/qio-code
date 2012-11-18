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
// rf_decals.cpp
#include "rf_local.h"
#include "rf_entities.h"
#include "rf_surface.h"
#include "rf_decals.h"
#include <shared/autoCvar.h>
#include <shared/array.h>
#include <shared/simpleTexturedPoly.h>

static aCvar_c rf_maxDecals("rf_maxDecals","512");
// 512 * 2~~ triangles = 1024 tris

struct simpleDecal_s {
	simplePoly_s poly;
	int addTime;

	simpleDecal_s(const simplePoly_s &newPoly) {
		poly = newPoly;
		addTime = rf_curTimeMsec;
	}
};

void simpleDecalBatcher_c::addDecalsWithMatToBatch(class mtrAPI_i *mat, class r_surface_c *batch) {
	for(u32 i = 0; i < decals.size(); i++) {
		const simpleDecal_s *sp = decals[i];
		if(sp->poly.material == mat) {
			batch->addPoly(sp->poly);
		}
	}
}
bool simpleDecalBatcher_c::hasDecalWithMat(mtrAPI_i *m) {
	for(u32 i = 0; i < decals.size(); i++) {
		if(decals[i]->poly.material == m)
			return true;
	}
	return false;
}
void simpleDecalBatcher_c::rebuildBatchWithMat(mtrAPI_i *m) {
	if(hasDecalWithMat(m) == false) {
		// there are no decals with this material, 
		// so remove empty batches
		for(u32 i = 0; i < batches.size(); i++) {
			if(batches[i]->getMat() == m) {
				delete batches[i];
				batches.erase(i);
				return;
			}
		}
		return;
	}
	// see if there is already a batch with this material
	for(u32 i = 0; i < batches.size(); i++) {
		if(batches[i]->getMat() == m) {
			r_surface_c *b = batches[i];
			b->clear();
			b->setMaterial(m);
			addDecalsWithMatToBatch(m,b);
			b->createVBOandIBO();
			return;
		}
	}
	// if there was no batch with given material,
	// create a new one
	r_surface_c *nb = new r_surface_c;
	nb->setMaterial(m);
	addDecalsWithMatToBatch(m,nb);
	nb->createVBOandIBO();
	batches.push_back(nb);
}
void simpleDecalBatcher_c::addDecalToBatch(const simplePoly_s &decalPoly) {
	while(decals.size() >= rf_maxDecals.getInt()) {
		// TODO: do it other way, erase is slow
		touchedMaterials.add_unique(decals[0]->poly.material);
		delete decals[0];
		decals[0] = 0;
		decals.erase(0);
	}
	touchedMaterials.add_unique(decalPoly.material);
	decals.push_back(new simpleDecal_s(decalPoly));
}
void simpleDecalBatcher_c::rebuildBatches() {
	// rebuild batches and reupload IBO/VBO
	for(u32 i = 0; i < touchedMaterials.size(); i++) {
		rebuildBatchWithMat(touchedMaterials[i]);
	}
	touchedMaterials.clear();
}
void simpleDecalBatcher_c::addDrawCalls() {
	for(u32 i = 0; i < batches.size(); i++) {
		r_surface_c *b = batches[i];
		
		b->addDrawCall();
	}
}
simpleDecalBatcher_c::simpleDecalBatcher_c() {

}
simpleDecalBatcher_c::~simpleDecalBatcher_c() {
	for(u32 i = 0; i < batches.size(); i++) {
		r_surface_c *b = batches[i];
		delete b;
	}			
	for(u32 i = 0; i < decals.size(); i++) {
		simpleDecal_s *d = decals[i];
		delete d;
	}
}

static simpleDecalBatcher_c *rf_worldDecalsBatcher = 0;

void RF_InitDecals() {
	rf_worldDecalsBatcher = new simpleDecalBatcher_c;
}
void RF_ShutdownDecals() {
	delete rf_worldDecalsBatcher;
	rf_worldDecalsBatcher = 0;
}
void RF_AddWorldDecalDrawCalls() {
	if(rf_worldDecalsBatcher == 0)
		return;
	rf_currentEntity = 0;
	rf_worldDecalsBatcher->addDrawCalls();
}
simpleDecalBatcher_c *RF_GetWorldDecalBatcher() {
	return rf_worldDecalsBatcher;
}