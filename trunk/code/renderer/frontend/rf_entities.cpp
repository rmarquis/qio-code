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
// rf_entities.cpp
#include "rf_entities.h"
#include "rf_model.h"
#include "rf_local.h"
#include "rf_decals.h"
#include <api/coreAPI.h>
#include <shared/autoCvar.h>

aCvar_c rf_skipEntities("rf_skipEntities","0");
rEntityImpl_c::rEntityImpl_c() {
	model = 0;
	staticDecals = 0;
}
rEntityImpl_c::~rEntityImpl_c() {
	if(staticDecals) {
		delete staticDecals;
		staticDecals = 0;
	}
}
void rEntityImpl_c::recalcABSBounds() {
	if(model == 0) {
		absBB.clear();
		return;
	}
	const aabb &bb = model->getBounds();
	matrix.transformAABB(bb,this->absBB);
}
void rEntityImpl_c::recalcMatrix() {
	// TODO: use axis instead of angles
	matrix.fromAnglesAndOrigin(angles,origin);
	recalcABSBounds();
}
void rEntityImpl_c::setOrigin(const vec3_c &newXYZ) {
	origin = newXYZ;
	recalcMatrix();
}
void rEntityImpl_c::setAngles(const vec3_c &newAngles) { 
	angles = newAngles;
	axis.fromAngles(angles);
	recalcMatrix();
}
void rEntityImpl_c::setModel(class rModelAPI_i *newModel) {
	model = newModel;
	recalcABSBounds();
}
bool rEntityImpl_c::rayTrace(class trace_c &tr) const {
	return model->rayTrace(tr);
}
int rEntityImpl_c::addDecalWorldSpace(const class vec3_c &pos, 
	const class vec3_c &normal, float radius, class mtrAPI_i *material) {
	if(model == 0)
		return -1;
	matrix_c matrixInverse = matrix.getInversed();
	vec3_c posLocal;
	matrixInverse.transformPoint(pos,posLocal);
	vec3_c normLocal;
	matrixInverse.transformNormal(normal,normLocal);
	if(model->isStatic()) {
		if(this->staticDecals == 0) {
			this->staticDecals = new simpleDecalBatcher_c;
		}
		model->createStaticModelDecal(this->staticDecals,posLocal,normLocal,radius,material);
	}
	return -1;
}

static arraySTD_c<rEntityImpl_c*> rf_entities;
rEntityAPI_i *rf_currentEntity = 0;

class rEntityAPI_i *RFE_AllocEntity() {
	rEntityImpl_c *ent = new rEntityImpl_c;
	rf_entities.push_back(ent);
	return ent;
}
void RFE_RemoveEntity(class rEntityAPI_i *ent) {
	rEntityImpl_c *rent = (rEntityImpl_c*)ent;
	rf_entities.remove(rent);
	delete rent;
}

void RFE_AddEntityDrawCalls() {
	if(rf_skipEntities.getInt())
		return;

	u32 c_entitiesCulledByABSBounds = 0;
	for(u32 i = 0; i < rf_entities.size(); i++) {
		rEntityImpl_c *ent = rf_entities[i];
		rf_currentEntity = ent;
		model_c *model = (model_c*)ent->getModel();
		if(model == 0) {
			continue;
		}
		if(rf_camera.getFrustum().cull(ent->getBoundsABS()) == CULL_OUT) {
			c_entitiesCulledByABSBounds++;
			continue;
		}
		model->addModelDrawCalls();
		class simpleDecalBatcher_c *staticDecals = ent->getStaticDecalsBatch();
		if(staticDecals) {
			staticDecals->addDrawCalls();
		}
	}
	if(0) {
		g_core->Print("RFE_AddEntityDrawCalls: %i of %i entities culled\n",c_entitiesCulledByABSBounds,rf_entities.size());
	}
	rf_currentEntity = 0;
}

void RFE_ClearEntities() {
	for(u32 i = 0; i < rf_entities.size(); i++) {
		delete rf_entities[i];
		rf_entities[i] = 0;
	}
	rf_entities.clear();
}
