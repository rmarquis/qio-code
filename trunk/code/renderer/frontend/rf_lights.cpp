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
#include "rf_drawCall.h"
#include <shared/autoCvar.h>
#include "rf_bsp.h"
#include <api/coreAPI.h>

static aCvar_c rf_skipLightInteractionsDrawCalls("rf_skipLightInteractionsDrawCalls","0");
static aCvar_c rf_cullShadowVolumes("rf_cullShadowVolumes","1");
static aCvar_c rf_cullLights("rf_cullLights","1");
static aCvar_c rf_lightRadiusMult("rf_lightRadiusMult","1.0");
static aCvar_c light_shadowMapScale("light_shadowMapScale","1.0");
static aCvar_c rf_printEntityShadowVolumesPrimCounts("rf_printEntityShadowVolumesPrimCounts","0");
static aCvar_c rf_verboseDeltaLightInteractionsUpdate("rf_verboseDeltaLightInteractionsUpdate","0");

rLightImpl_c::rLightImpl_c() {
	radius = 512.f;	
	numCurrentStaticInteractions = 0;
	numCurrentEntityInteractions = 0;
	staticShadowVolume = 0;
	oq = 0;
	shadowMapH = 2048;
	shadowMapW = 2048;
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
	if(staticShadowVolume) {
		delete staticShadowVolume;
	}
	if(oq) {
		delete oq;
	}
}
float rLightImpl_c::getShadowMapW() const {
	return this->shadowMapW * light_shadowMapScale.getFloat();
}
float rLightImpl_c::getShadowMapH() const {
	return this->shadowMapH * light_shadowMapScale.getFloat();
}
void rLightImpl_c::setOrigin(const class vec3_c &newXYZ) {
	if(pos.compare(newXYZ)) {
		return; // no change
	}
	pos = newXYZ;
	absBounds.fromPointAndRadius(pos,radius);
	recalcShadowMappingMatrices();
	recalcLightInteractions();
}
void rLightImpl_c::setRadius(float newRadius) {
	newRadius *= rf_lightRadiusMult.getFloat();
	if(radius == newRadius) {
		return; // no change
	}
	radius = newRadius;
	absBounds.fromPointAndRadius(pos,radius);
	recalcShadowMappingMatrices();
	recalcLightInteractions();
}
occlusionQueryAPI_i *rLightImpl_c::ensureOcclusionQueryAllocated() {
	if(oq) {
		return oq;
	}
	oq = rb->allocOcclusionQuery();
	return oq;
}
bool rLightImpl_c::setBCameraInside() {
	const vec3_c &cameraWorld = rf_camera.getOrigin();
	float distSQ = cameraWorld.distSQ(this->pos);
	if(distSQ > Square((this->radius+2))) {
		bCameraInside = false;
	} else {
		bCameraInside = true;
	}
	return bCameraInside;
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
	numCurrentStaticInteractions = 0;
	RF_CacheLightWorldInteractions(this);
	if(RF_IsUsingShadowVolumes()) {
		recalcShadowVolumeOfStaticInteractions();
	}
}
void rLightImpl_c::recalcShadowVolumeOfStaticInteractions() {
	if(staticShadowVolume == 0) {
		staticShadowVolume = new rIndexedShadowVolume_c;
	} else {
		staticShadowVolume->clear();
	}
	for(u32 i = 0; i < numCurrentStaticInteractions; i++) {
		staticSurfInteraction_s &in = this->staticInteractions[i];
		if(in.isNeededForShadows() == false) {
			// if interaction is not needed for shadowing, skip it
			continue;
		}
		if(in.type == SIT_BSP) {
			RF_AddBSPSurfaceToShadowVolume(in.bspSurfaceNumber,pos,staticShadowVolume,this->radius);
		} else if(in.type == SIT_PROC) {
			staticShadowVolume->addRSurface(in.sf,pos,0,this->radius);
		} else if(in.type == SIT_STATIC) {
			staticShadowVolume->addRSurface(in.sf,pos,0,this->radius);
		}
	}
}		

void rLightImpl_c::refreshIntersection(entityInteraction_s &in) {
	if(RF_IsUsingShadowVolumes()) {
		if(in.ent->isSprite()) {
			printf("skipping sprite..\n");
			if(in.shadowVolume) {
				in.shadowVolume = 0;
				delete in.shadowVolume;
			}
		} else {
			vec3_c lightInEntitySpace;
			this->calcPosInEntitySpace(in.ent,lightInEntitySpace);
			if(in.shadowVolume == 0) {
				in.shadowVolume = new rIndexedShadowVolume_c;
			} else {
				//if(lightInEntitySpace.compare(in.shadowVolume->getLightPos())) {
				//	return; // dont have to recalculate shadow volume
				//}
			}
			in.shadowVolume->createShadowVolumeForEntity(in.ent,lightInEntitySpace,this->radius);

			if(rf_printEntityShadowVolumesPrimCounts.getInt()) {
				g_core->Print("Model %s shadow volume - %i tris, %i verts\n",
					in.ent->getModelName(),in.shadowVolume->getNumTris(),in.shadowVolume->getNumVerts());
			}
		}
	}
	in.lastSilChangeTime = in.ent->getSilChangeCount();
}
void rLightImpl_c::removeEntityFromInteractionsList(class rEntityImpl_c *ent) {
	u32 from = 0;
	u32 to = 0;
	while(from < numCurrentEntityInteractions) {
		entityInteraction_s &in = this->entityInteractions[from];
		if(in.ent == ent) {
			in.clear();
		} else {
			if(from != to) {
				this->entityInteractions[to] = this->entityInteractions[from];
			}
			to++;
		}
		from++;
	}
	numCurrentEntityInteractions = to;
}
void rLightImpl_c::recalcLightInteractionsWithDynamicEntities() {
#if 1
	arraySTD_c<rEntityImpl_c*> ents;
	RFE_BoxEntities(this->absBounds,ents);
	if(entityInteractions.size() < ents.size()) {
		entityInteractions.resize(ents.size());
	}
	u32 from = 0;
	u32 to = 0;
	while(from < numCurrentEntityInteractions) {
		entityInteraction_s &in = this->entityInteractions[from];
		if(ents.isOnList(in.ent)==false) {
			// remove this interaction, it's no longer intersecting light
			if(rf_verboseDeltaLightInteractionsUpdate.getInt()) {
				printf("Removing entity %s from light interactions...\n",in.ent->getModelName());
			}
			in.clear();
		} else {
			ents.remove(in.ent);
			if(rf_verboseDeltaLightInteractionsUpdate.getInt()) {
				printf("Updating light interaction entity %s... (last %i, now %i)",in.ent->getModelName(),in.lastSilChangeTime,in.ent->getSilChangeCount());
			}
			// update this interaction (if needed)
			if(in.lastSilChangeTime != in.ent->getSilChangeCount()) {
				refreshIntersection(in);
				if(rf_verboseDeltaLightInteractionsUpdate.getInt()) {
					printf("(changed)\n");
				}
			} else {	
				if(rf_verboseDeltaLightInteractionsUpdate.getInt()) {
					printf("(no change)\n");
				}
			}
			if(from != to) {
				this->entityInteractions[to] = this->entityInteractions[from];
				this->entityInteractions[from].zero();
			}
			to++;
		}
		from++;
	}
	// add new interactions
	for(u32 i = 0; i < ents.size(); i++) {
		entityInteraction_s &newIn = this->entityInteractions[to];
		newIn.clear();
		newIn.ent = ents[i];
		refreshIntersection(newIn);
		if(rf_verboseDeltaLightInteractionsUpdate.getInt()) {
			printf("Adding new light interaction entity...");
		}
		to++;
	}
	numCurrentEntityInteractions = to;
#else
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
			if(ent->isSprite()) {
				printf("skipping sprite..\n");
				if(in.shadowVolume) {
					in.shadowVolume = 0;
					delete in.shadowVolume;
				}
			} else {
				vec3_c lightInEntitySpace;
				this->calcPosInEntitySpace(ent,lightInEntitySpace);
				if(in.shadowVolume == 0) {
					in.shadowVolume = new rIndexedShadowVolume_c;
				} else {
					if(lightInEntitySpace.compare(in.shadowVolume->getLightPos())) {
						continue; // dont have to recalculate shadow volume
					}
				}
				in.shadowVolume->createShadowVolumeForEntity(ent,lightInEntitySpace,this->radius);

				if(rf_printEntityShadowVolumesPrimCounts.getInt()) {
					g_core->Print("Model %s shadow volume - %i tris, %i verts\n",
						ent->getModelName(),in.shadowVolume->getNumTris(),in.shadowVolume->getNumVerts());
				}
			}
		}
	}
#endif
}
void rLightImpl_c::recalcLightInteractions() {
	if(RF_IsUsingDynamicLights() == false) {
		return; // we dont need light interactions
	}
	// clear interactions with dynamic entities
	this->clearInteractionsWithDynamicEntities();
	// recalculate all of them
	recalcLightInteractionsWithDynamicEntities();
	recalcLightInteractionsWithStaticWorld();
}
static aCvar_c rf_proc_printLitSurfsCull("rf_proc_printLitSurfsCull","0");
void rLightImpl_c::addStaticSurfInteractionDrawCall(staticSurfInteraction_s &in) {
	if(in.type == SIT_BSP) {
		RF_DrawSingleBSPSurface(in.bspSurfaceNumber);
	} else if(in.type == SIT_STATIC) {
		in.sf->addDrawCall();
	} else if(in.type == SIT_PROC) {
		// this check causes some troubles now
		//if(RF_IsWorldAreaVisible(in.areaNum))
		{
			if(rf_proc_printLitSurfsCull.getInt()) {
				g_core->Print("rLightImpl_c::addStaticSurfInteractionDrawCall: adding surf %i because area %i was visible\n",in.sf,in.areaNum);
			}
			in.sf->addDrawCall();
		} 
	///else {
	///		if(rf_proc_printLitSurfsCull.getInt()) {
	///			g_core->Print("rLightImpl_c::addStaticSurfInteractionDrawCall: skipping surf %i because area %i was NOT visible\n",in.sf,in.areaNum);
	///		}
	///	}
	}
}
void rLightImpl_c::addLightInteractionDrawCalls() {
	for(u32 i = 0; i < numCurrentStaticInteractions; i++) {
		staticSurfInteraction_s &in = this->staticInteractions[i];
		if(in.isNeededForLighting() == false) {
			// if interaction is not needed for lighting, skip it
			continue;
		}
		addStaticSurfInteractionDrawCall(in);
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
static aCvar_c rf_printShadowVolumesCulledByViewFrustum("rf_printShadowVolumesCulledByViewFrustum","0");
void rLightImpl_c::addLightShadowVolumesDrawCalls() {
	if(staticShadowVolume) {
		staticShadowVolume->addDrawCall();
	}
	for(u32 i = 0; i < numCurrentEntityInteractions; i++) {
		entityInteraction_s &in = this->entityInteractions[i];
		if(in.shadowVolume == 0)
			continue;
		rf_currentEntity = in.ent;
		// try to cull this shadow volume by view frustum
		// (NOTE: even if the shadow caster entity is outside view frustum,
		// it's shadow volume STILL might be visible !!!)
		if(rf_cullShadowVolumes.getInt() && RF_CullEntitySpaceBounds(in.shadowVolume->getAABB()) == CULL_OUT) {
			if(rf_printShadowVolumesCulledByViewFrustum.getInt()) {
				g_core->Print("Shadow volume of model \"%s\" culled by view frustum for light %i\n",rf_currentEntity->getModelName(),this);
			}
			continue;
		}
		in.shadowVolume->addDrawCall();
	}
	rf_currentEntity = 0;
}

enum {
	CUBE_SIDE_COUNT = 6,
};
void rLightImpl_c::recalcShadowMappingMatrices() {
	// projection matrix is the same for all of cubemap sides
	lightProj.setupProjectionExt(90.f, getShadowMapW(), getShadowMapH(), 1.f, this->radius);
	vec3_c cubeNormals[] = { vec3_c(1,0,0), vec3_c(-1,0,0),
						vec3_c(0,1,0), vec3_c(0,-1,0),
						vec3_c(0,0,1), vec3_c(0,0,-1)};
	for(u32 side = 0; side < CUBE_SIDE_COUNT; side++) {
		// view matrix is different for each cubemap side
		vec3_c upVector;
		// upvector cant be the same as normal (cube direction)
		// otherwise shadow will not show up
		if(side < 2) {
			upVector.set(0,1,0);
		} else if(side < 4) {
			upVector.set(0,0,1);
		} else {
			upVector.set(1,0,0);
		}
		sideViews[side].setupLookAtRH(this->pos,cubeNormals[side],upVector);

		axis_c ax;
		ax.mat[0] = cubeNormals[side];
		ax.mat[1] = upVector;
		ax.mat[2] = ax.mat[1].crossProduct(ax.mat[0]);
		sideFrustums[side].setupExt(90.f, getShadowMapW(), getShadowMapH(), radius, ax, this->pos);
	}
}
void rLightImpl_c::addShadowMapRenderingDrawCalls() {
	rf_currentShadowMapW = getShadowMapW();
	rf_currentShadowMapH = getShadowMapH();
	rf_bDrawOnlyOnDepthBuffer = true;
	for(u32 side = 0; side < CUBE_SIDE_COUNT; side++) {	
		rf_currentShadowMapCubeSide = side;
		const frustum_c &sideFrustum = sideFrustums[side];
		for(u32 j = 0; j < numCurrentStaticInteractions; j++) {
			staticSurfInteraction_s &sIn = this->staticInteractions[j];
			addStaticSurfInteractionDrawCall(sIn);
		}
		for(u32 j = 0; j < numCurrentEntityInteractions; j++) {
			entityInteraction_s &eIn = this->entityInteractions[j];
			RFE_AddEntity(eIn.ent,&sideFrustum);
		}
	}
	rf_bDrawOnlyOnDepthBuffer = false;
	rf_currentShadowMapCubeSide = -1;
	rf_currentShadowMapW = -1;
	rf_currentShadowMapH = -1;
}
bool rLightImpl_c::isCulledByAreas() const {
	for(u32 i = 0; i < numCurrentStaticInteractions; i++) {
		const staticSurfInteraction_s &in = this->staticInteractions[i];
		if(in.isNeededForLighting() == false)
			continue;
		if(in.type == SIT_PROC) {
			if(RF_IsWorldAreaVisible(in.areaNum)) {
				// light intersects at least one visible area, so we can't cull it
				return false;
			}
		}
	}
	// none of lit areas are visible for player, so light is culled
	return true;
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
void RFL_FreeAllLights() {
	for(u32 i = 0; i < rf_lights.size(); i++) {
		delete rf_lights[i];
	}
	rf_lights.clear();
}
void RFL_RemoveAllReferencesToEntity(class rEntityImpl_c *ent) {
	for(u32 i = 0; i < rf_lights.size(); i++) {
		rf_lights[i]->removeEntityFromInteractionsList(ent);
	}
}

static aCvar_c rf_redrawEntireSceneForEachLight("rf_redrawEntireSceneForEachLight","0");
static aCvar_c light_printCullStats("light_printCullStats","0");
static aCvar_c light_useGPUOcclusionQueries("light_useGPUOcclusionQueries","0");

bool RFL_GPUOcclusionQueriesForLightsEnabled() {
	return light_useGPUOcclusionQueries.getInt();
}

rLightAPI_i *rf_curLightAPI = 0;
void RFL_AddLightInteractionsDrawCalls() {
	if(rf_skipLightInteractionsDrawCalls.getInt())
		return;
	u32 c_lightsCulled = 0;
	u32 c_lightsCulledByAABBTest = 0;
	u32 c_lightsCulledBySphereTest = 0;
	u32 c_lightsCulledByAreas = 0;
	
	for(u32 i = 0; i < rf_lights.size(); i++) {
		rLightImpl_c *light = rf_lights[i];
		rf_curLightAPI = light;

		const aabb &bb = light->getABSBounds();
		// try to cull the entire light (culling light
		// will cull out all of its interactions
		// and shadows as well)
		if(rf_cullLights.getInt()) {
			if(rf_camera.getFrustum().cull(bb) == CULL_OUT) {
				c_lightsCulled++;
				c_lightsCulledByAABBTest++;
				light->setCulled(true);
				continue;
			}
			if(rf_camera.getFrustum().cullSphere(light->getOrigin(),light->getRadius()) == CULL_OUT) {
				c_lightsCulled++;
				c_lightsCulledBySphereTest++;
				light->setCulled(true);
				continue;
			}
			// extra .proc tree lights culling
			if(RF_IsWorldTypeProc()) {
				// light is culled if all areas "visible" by light are not visible by player
				if(light->isCulledByAreas()) {
					c_lightsCulled++;
					c_lightsCulledByAreas++;
					light->setCulled(true);
					// NOTE: not much light will be culled this way, because most of the lights
					// are already culled by PVS on the serverside (before sending them to clients)
					continue;
				}	
			}
		}
		light->setCulled(false);

		// TODO: dont do this every frame
		light->recalcLightInteractionsWithDynamicEntities();

		if(RF_IsUsingShadowVolumes()) {
			light->addLightShadowVolumesDrawCalls();
		} else if(RF_IsUsingShadowMapping()) {
			light->addShadowMapRenderingDrawCalls();
		}

		if(rf_redrawEntireSceneForEachLight.getInt()) {
			RF_AddGenericDrawCalls();
		} else {
			// blend light interactions (surfaces intersecting light bounds) with drawn world
			light->addLightInteractionDrawCalls();
		}
	}
	rf_curLightAPI = 0;
	if(light_printCullStats.getInt()) {
		g_core->Print("RFL_AddLightInteractionsDrawCalls: %i lights, %i culled (%i by aabb, %i by sphere, %i by areas)\n",
			rf_lights.size(),c_lightsCulled,c_lightsCulledByAABBTest,c_lightsCulledBySphereTest,c_lightsCulledByAreas);
	}
}
#include <api/occlusionQueryAPI.h>
void RFL_AssignLightOcclusionQueries() {
	if(light_useGPUOcclusionQueries.getInt() == 0)
		return;
	// assign occlusion queries for potentially visible lights
	// (lights intersecting view frustum)
	for(u32 i = 0; i < rf_lights.size(); i++) {
		rLightImpl_c *light = rf_lights[i];
		if(light->getBCulled()) {
			continue;
		}
		if(light->setBCameraInside()) {
			// if camera is inside this light, occlusion checks are meaningless
			continue;
		}
		occlusionQueryAPI_i *oq = light->ensureOcclusionQueryAllocated();
		if(oq == 0) {
			continue; // occlusion queries not supported by renderer backend
		}
		float lightRadius = light->getRadius();
		const vec3_c &lightPos = light->getOrigin();
		oq->assignSphereQuery(lightPos,lightRadius);
	}
}
void RFL_RecalculateLightsInteractions() {
	for(u32 i = 0; i < rf_lights.size(); i++) {
		rLightImpl_c *light = rf_lights[i];
		light->recalcLightInteractions();
	}
}