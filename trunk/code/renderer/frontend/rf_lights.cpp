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
// rf_lights.cpp
#include "rf_local.h"
#include "rf_lights.h"
#include "rf_surface.h"
#include "rf_entities.h"
#include "rf_world.h"
#include "rf_shadowVolume.h"
#include <shared/autoCvar.h>

static aCvar_c rf_skipLightInteractionsDrawCalls("rf_skipLightInteractionsDrawCalls","0");

rLightImpl_c::rLightImpl_c() {
	radius = 512.f;	
	numCurrentStaticInteractions = 0;
	numCurrentEntityInteractions = 0;
}
rLightImpl_c::~rLightImpl_c() {
	clearInteractions();
	for(u32 i = 0; i < entityInteractions.size(); i++) {
		entityInteraction_s &in = this->entityInteractions[i];
		if(in.shadowVolume) {
			delete in.shadowVolume;
			in.shadowVolume = 0;
		}
	}
}
void rLightImpl_c::setOrigin(const class vec3_c &newXYZ) {
	if(pos.compare(newXYZ)) {
		return; // no change
	}
	pos = newXYZ;
	absBounds.fromPointAndRadius(pos,radius);
	recalcLightInteractions();
}
void rLightImpl_c::setRadius(float newRadius) {
	if(radius == newRadius) {
		return; // no change
	}
	radius = newRadius;
	absBounds.fromPointAndRadius(pos,radius);
	recalcLightInteractions();
}
void rLightImpl_c::clearInteractionsWithDynamicEntities() {
	for(u32 i = 0; i < numCurrentEntityInteractions; i++) {
		entityInteraction_s &in = this->entityInteractions[i];
		//in.ent->removeInteraction(this);
	}
	numCurrentEntityInteractions = 0;
}
void rLightImpl_c::clearInteractions() {
	clearInteractionsWithDynamicEntities();
	numCurrentStaticInteractions = 0;
}
void rLightImpl_c::calcPosInEntitySpace(const rEntityAPI_i *ent, vec3_c &out) const {
	const matrix_c &entityMatrix = ent->getMatrix();
	matrix_c entityMatrixInv = entityMatrix.getInversed();
	entityMatrixInv.transformPoint(this->pos,out);
}
void rLightImpl_c::recalcLightInteractionsWithStaticWorld() {
	RF_CacheLightWorldInteractions(this);
}
void rLightImpl_c::recalcLightInteractionsWithDynamicEntities() {
	clearInteractionsWithDynamicEntities();
	arraySTD_c<rEntityImpl_c*> ents;
	RFE_BoxEntities(this->absBounds,ents);
	if(entityInteractions.size() < ents.size()) {
		entityInteractions.resize(ents.size());
	}
	numCurrentEntityInteractions = ents.size();
	for(u32 i = 0; i < ents.size(); i++) {
		rEntityImpl_c *ent = ents[i];
		entityInteraction_s &in = this->entityInteractions[i];
		in.ent = ent;
		//ent->addInteraction(this);
		if(RF_IsUsingShadowVolumes()) {
			if(in.shadowVolume == 0) {
				in.shadowVolume = new rIndexedShadowVolume_c;
			}
			vec3_c lightInEntitySpace;
			this->calcPosInEntitySpace(ent,lightInEntitySpace);
			in.shadowVolume->createShadowVolumeForEntity(ent,lightInEntitySpace);
		}
	}
}
void rLightImpl_c::recalcLightInteractions() {
	recalcLightInteractionsWithDynamicEntities();
	recalcLightInteractionsWithStaticWorld();
}
void rLightImpl_c::addLightInteractionDrawCalls() {
	for(u32 i = 0; i < numCurrentStaticInteractions; i++) {
		staticSurfInteraction_s &in = this->staticInteractions[i];
		if(in.sf) {
			in.sf->addDrawCall();
		}
	}
	for(u32 i = 0; i < numCurrentEntityInteractions; i++) {
		entityInteraction_s &in = this->entityInteractions[i];
#if 0
		in.ent->addDrawCalls();
#else
		// check if the entity is needed for a current scene
		// (first person models are not drawn in 3rd person mode, etc)
		// then do a frustum check, and if the entity is visible,
		// add its draw calls
		RFE_AddEntity(in.ent);
#endif
	}
}
void rLightImpl_c::addLightShadowVolumesDrawCalls() {
	for(u32 i = 0; i < numCurrentEntityInteractions; i++) {
		entityInteraction_s &in = this->entityInteractions[i];
		if(in.shadowVolume == 0)
			continue;
		rf_currentEntity = in.ent;
		in.shadowVolume->addDrawCall();
	}
	rf_currentEntity = 0;
}

static arraySTD_c<rLightImpl_c*> rf_lights;

class rLightAPI_i *RFL_AllocLight() {
	rLightImpl_c *light = new rLightImpl_c;
	rf_lights.push_back(light);
	return light;
}
void RFL_RemoveLight(class rLightAPI_i *light) {
	rLightImpl_c *rlight = (rLightImpl_c*)light;
	rf_lights.remove(rlight);
	delete rlight;
}

static aCvar_c rf_redrawEntireSceneForEachLight("rf_redrawEntireSceneForEachLight","0");

rLightAPI_i *rf_curLightAPI = 0;
void RFL_AddLightInteractionsDrawCalls() {
	if(rf_skipLightInteractionsDrawCalls.getInt())
		return;
	for(u32 i = 0; i < rf_lights.size(); i++) {
		rLightImpl_c *light = rf_lights[i];
		rf_curLightAPI = light;

		// TODO: dont do this every frame
		light->recalcLightInteractionsWithDynamicEntities();

		if(RF_IsUsingShadowVolumes()) {
			light->addLightShadowVolumesDrawCalls();
		}
		if(rf_redrawEntireSceneForEachLight.getInt()) {
			RF_AddGenericDrawCalls();
		} else {
			// blend light interactions (surfaces intersecting light bounds) with drawn world
			light->addLightInteractionDrawCalls();
		}
	}
	rf_curLightAPI = 0;
}