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
#include <api/mtrAPI.h>

static aCvar_c rf_skipLightInteractionsDrawCalls("rf_skipLightInteractionsDrawCalls","0");
static aCvar_c rf_cullShadowVolumes("rf_cullShadowVolumes","1");
static aCvar_c rf_cullLights("rf_cullLights","1");
static aCvar_c rf_lightRadiusMult("rf_lightRadiusMult","1.0");
static aCvar_c light_shadowMapScale("light_shadowMapScale","1.0");
static aCvar_c light_printLightTypes("light_printLightTypes","0");
static aCvar_c rf_printEntityShadowVolumesPrimCounts("rf_printEntityShadowVolumesPrimCounts","0");
static aCvar_c rf_verboseDeltaLightInteractionsUpdate("rf_verboseDeltaLightInteractionsUpdate","0");
static aCvar_c rf_verboseAddLightInteractionsDrawCalls("rf_verboseAddLightInteractionsDrawCalls","0");
static aCvar_c light_batchBSPInteractions("light_batchBSPInteractions","1");
static aCvar_c light_printBSPBatchingStats("light_printBSPBatchingStats","0");
static aCvar_c light_spotLights_printCulledEntities("light_spotLights_printCulledEntities","0");
static aCvar_c light_printCulledShadowMappingPointLightSides("light_printCulledShadowMappingPointLightSides","0");
static aCvar_c light_printCulledShadowMappingPointLightSideInteractions("light_printCulledShadowMappingPointLightSideInteractions","0");
static aCvar_c light_printShadowMappingPointLightSideCullingStats("light_printShadowMappingPointLightSideCullingStats","0");
static aCvar_c light_minShadowCastingLightRadius("light_minShadowCastingLightRadius","30");

void staticInteractionBatch_c::recalcBounds() {
	bounds.clear();
	for(u32 i = 0; i < indices.getNumIndices(); i++) {
		bounds.addPoint(vertices->getXYZ(indices.getIndex(i)));
	}
}
void entityInteraction_s::clear() {
	ent = 0;
	if(shadowVolume) {
		delete shadowVolume;
		shadowVolume = 0;
	}
	lastSilChangeTime = 0;
}

rLightImpl_c::rLightImpl_c() {
	lightType = LT_POINT;
	radius = 512.f;	
	numCurrentStaticInteractions = 0;
	numCurrentEntityInteractions = 0;
	staticShadowVolume = 0;
	oq = 0;
	shadowMapH = 2048;
	shadowMapW = 2048;
	bColoured = false;
	color.set(1.f,1.f,1.f);
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
	///printf("Changing light origin from %f %f %f to %f %f %f\n",pos.x,pos.y,pos.z,newXYZ.x,newXYZ.y,newXYZ.z);
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
void rLightImpl_c::setBNoShadows(bool newBNoShadows) {
	if(this->bNoShadows == newBNoShadows) {
		return; // no change
	}
	this->bNoShadows = newBNoShadows;
	recalcShadowMappingMatrices();
	recalcLightInteractions();
}
void rLightImpl_c::setLightType(rLightType_e newLightType) {
	lightType = newLightType;
}
void rLightImpl_c::setSpotLightTarget(const class vec3_c &newTargetPos) {
	spotLightTarget = newTargetPos;
	this->recalcSpotLightCos();
}
void rLightImpl_c::setSpotRadius(float newSpotRadius) {
	spotRadius = newSpotRadius;
	this->recalcSpotLightCos();
}
void rLightImpl_c::setBColoured(bool newBColoured) {
	bColoured = newBColoured;
}
void rLightImpl_c::setColor(const class vec3_c &newRGB) {
	color = newRGB;
}
void rLightImpl_c::recalcSpotLightCos() {
	spotLightDir = this->spotLightTarget - this->pos;
	float d = spotLightDir.normalize2();
	this->spotLightCos = d / sqrt(Square(d)+Square(this->spotRadius));
	// calculate spot light angle
	float spotLightFOV = 2.0f * RAD2DEG(acos(this->spotLightCos));
	axis_c ax;
	ax[0] = spotLightDir;
	spotLightDir.makeNormalVectors(ax[1],ax[2]);
	spotLightFrustum.setup(spotLightFOV,spotLightFOV,radius,ax,this->getOrigin());
	// light view matrix
	spotLightView.setupLookAtRH(this->pos,spotLightDir,ax.getUp());
	// light projection matrix
	lightProj.setupProjectionExt(90.f, getShadowMapW(), getShadowMapH(), 1.f, this->radius);
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
	if(light_batchBSPInteractions.getInt()) {
		recalcStaticInteractionBatches();
	}
}
// create batches for interactions with bsp polygons.
void rLightImpl_c::recalcStaticInteractionBatches() {
	staticBatcher.clear();
	u32 c_addedSurfaces = 0;
	for(u32 i = 0; i < numCurrentStaticInteractions; i++) {
		staticSurfInteraction_s &in = this->staticInteractions[i];
		int batchIndex;
		if(in.type == SIT_BSP) {
			const rIndexBuffer_c *indices = RF_GetSingleBSPSurfaceABSIndices(in.bspSurfaceNumber);
			const rVertexBuffer_c *vertices = RF_GetBSPVertices();
			if(indices == 0) {
				batchIndex = -1;
			} else {
				mtrAPI_i *mat = RF_GetSingleBSPSurfaceMaterial(in.bspSurfaceNumber);
				batchIndex = staticBatcher.addSurface(mat,indices,vertices);
				c_addedSurfaces++;
			}
		} else {
			batchIndex = -1;
		}
		in.batchIndex = batchIndex;
	}
	staticBatcher.uploadIBOs();
	staticBatcher.recalcBounds();
	// for test_tree.pk3 light there are 1563 surfaces and (!) 3 batches.
	if(light_printBSPBatchingStats.getInt()) {
		g_core->Print("%i surfaces, %i batches\n",c_addedSurfaces,staticBatcher.getNumBatches());
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
			g_core->Print("skipping sprite..\n");
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
	if(this == 0) {
		// sanity check, this should never happen
		g_core->RedWarning("rLightImpl_c::removeEntityFromInteractionsList: 'this' is NULL\n");
		return;
	}
	u32 from = 0;
	u32 to = 0;
	while(from < numCurrentEntityInteractions) {
		entityInteraction_s &in = this->entityInteractions[from];
		if(in.ent == ent) {
			in.clear();
		} else {
			if(from != to) {
				this->entityInteractions[to] = this->entityInteractions[from];
				this->entityInteractions[from].shadowVolume = 0;
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
	if(lightType == LT_SPOTLIGHT) {
		u32 c_lightEntityInteractionsCulledBySpotLightFrustum = 0;
		// for spot lights use box and then spot frustum
		// First get box entities
		arraySTD_c<rEntityImpl_c*> boxEnts;
		RFE_BoxEntities(this->absBounds,boxEnts);
		// then check them against frustum
		for(u32 i = 0; i < boxEnts.size(); i++) {
			rEntityImpl_c *e = boxEnts[i];
			if(spotLightFrustum.cull(e->getBoundsABS()) == CULL_OUT) {
				c_lightEntityInteractionsCulledBySpotLightFrustum++;
				continue;
			}
			ents.push_back(e);
		}
		if(light_spotLights_printCulledEntities.getInt()) {
			g_core->Print("rLightImpl_c::recalcLightInteractionsWithDynamicEntities: %i entities culled by spot light frustum\n",
				c_lightEntityInteractionsCulledBySpotLightFrustum);
		}
	} else {
		// for point lights just get all entities in box
		RFE_BoxEntities(this->absBounds,ents);
	}
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
				printf("(light %i): Removing entity %s from light interactions...\n",this,in.ent->getModelName());
			}
			in.clear();
		} else {
			ents.remove(in.ent);
			if(rf_verboseDeltaLightInteractionsUpdate.getInt()) {
				printf("(light %i): Updating light interaction entity %s... (last %i, now %i)",this,in.ent->getModelName(),in.lastSilChangeTime,in.ent->getSilChangeCount());
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
			printf("(light %i): Adding new light interaction entity... (%s)\n",this,newIn.ent->getModelName());
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
#if 0
	this->clearInteractionsWithDynamicEntities();
#else
	for(u32 i = 0; i < numCurrentEntityInteractions; i++) {
		entityInteractions[i].lastSilChangeTime = 0;
	}
#endif
	// recalculate all of them
	recalcLightInteractionsWithDynamicEntities();
	recalcLightInteractionsWithStaticWorld();
}
static aCvar_c rf_proc_printLitSurfsCull("rf_proc_printLitSurfsCull","0");
void rLightImpl_c::addStaticSurfInteractionDrawCall(staticSurfInteraction_s &in, bool addingForShadowMapping) {
	if(in.type == SIT_BSP) {
		RF_DrawSingleBSPSurface(in.bspSurfaceNumber);
	} else if(in.type == SIT_STATIC) {
		in.sf->addDrawCall();
	} else if(in.type == SIT_PROC) {
		// don't add lighting interactions if interaction area was not visible by player.
		// NOTE: if we're adding interaction for shadow mapping, we still need
		// to add it even if interaction area was NOT visible by player.
		if(addingForShadowMapping == true || RF_IsWorldAreaVisible(in.areaNum)) {
			if(rf_proc_printLitSurfsCull.getInt()==2) {
				g_core->Print("rLightImpl_c::addStaticSurfInteractionDrawCall: adding surf %i because area %i was visible\n",in.sf,in.areaNum);
			}
			in.sf->addDrawCall();
		} else {
			if(rf_proc_printLitSurfsCull.getInt()) {
				g_core->Print("rLightImpl_c::addStaticSurfInteractionDrawCall: skipping surf %i because area %i was NOT visible\n",in.sf,in.areaNum);
			}
		}
	}
}
void rLightImpl_c::addBatchedStaticInteractionDrawCalls(const class aabb *sideBB, const class frustum_c *sideFrustum) {
	// add static batches
	const rVertexBuffer_c *verts = RF_GetBSPVertices();
	for(u32 i = 0; i < staticBatcher.getNumBatches(); i++) {
		const staticInteractionBatch_c *b = staticBatcher.getBatch(i);
		if(sideBB) {
			if(sideBB->intersect(b->getBounds())==false) {
				if(0) {
					g_core->Print("rLightImpl_c::addBatchedStaticInteractionDrawCalls: batch with %i tris culled by light cube side %i bounds\n",
						b->getNumTris(),rf_currentShadowMapCubeSide);
				}
				continue;
			}
		}
		if(sideFrustum) {
			if(sideFrustum->cull(b->getBounds())==CULL_OUT) {
				if(0) {
					g_core->Print("rLightImpl_c::addBatchedStaticInteractionDrawCalls: batch with %i tris culled by light cube side %i frustum\n",
						b->getNumTris(),rf_currentShadowMapCubeSide);
				}
				continue;
			}
		}
		RF_AddDrawCall(verts,&b->getIndices(),b->getMaterial(),0,b->getMaterial()->getSort(),false);
	}
}
void rLightImpl_c::addLightInteractionDrawCalls() {
	bool bUseBatches = light_batchBSPInteractions.getInt();
	for(u32 i = 0; i < numCurrentStaticInteractions; i++) {
		staticSurfInteraction_s &in = this->staticInteractions[i];
		if(in.isNeededForLighting() == false) {
			// if interaction is not needed for lighting, skip it
			continue;
		}
		// check if interaction is batched
		if(bUseBatches && in.batchIndex != -1)
			continue;
		// add static surf interaction for lighting
		addStaticSurfInteractionDrawCall(in,false);
	}
	if(bUseBatches) {
		// add static batches
		addBatchedStaticInteractionDrawCalls();
	}

	for(u32 i = 0; i < numCurrentEntityInteractions; i++) {
		entityInteraction_s &in = this->entityInteractions[i];
		if(rf_verboseAddLightInteractionsDrawCalls.getInt()) {
			g_core->Print("rLightImpl_c::addLightInteractionDrawCalls(): (light %i): adding %s\n",this,in.ent->getModelName());
		}
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

	// NOTE: the layout of those vectors has been changed by me for opengl depth cubemap format
	vec3_c cubeNormals[] = { vec3_c(1,0,0), vec3_c(-1,0,0),
						vec3_c(0,-1,0), vec3_c(0,1,0),
						vec3_c(0,0,-1), vec3_c(0,0,1)};
	for(u32 side = 0; side < CUBE_SIDE_COUNT; side++) {
		// view matrix is different for each cubemap side
		vec3_c upVector;
		// upvector must be perpendicular to forward vector
		if(side < 2) {
			upVector.set(0,1,0);
		} else if(side < 4) {
			if(side == 2)
				upVector.set(0,0,-1);
			else
				upVector.set(0,0,1);
		} else {
			upVector.set(0,1,0);
		}
		sideViews[side].setupLookAtRH(this->pos,cubeNormals[side],upVector);

		axis_c ax;
		ax.mat[0] = cubeNormals[side];
		ax.mat[1] = upVector;
		ax.mat[2] = ax.mat[1].crossProduct(ax.mat[0]);
		sideFrustums[side].setupExt(90.f, getShadowMapW(), getShadowMapH(), radius, ax, this->pos);
		sideFrustums[side].getBounds(sideBounds[side]);
	}
//#elif 1			
//	vec3_c vecs[] = {
//		vec3_c(-1, +0, 0), vec3_c(0, -1, 0),
//		vec3_c(+1, +0, 0), vec3_c(0, -1, 0),
//		vec3_c(0, -1, 0), vec3_c(0, 0, -1),
//		vec3_c(0, +1, 0), vec3_c(0, 0, -1),
//		vec3_c(0, 0, -1), vec3_c(0, 1, 0),
//		vec3_c(0, 0, +1), vec3_c(0, 1, 0),
//	};
//	for(u32 side = 0; side < CUBE_SIDE_COUNT; side++) {
//		// view matrix is different for each cubemap side
//		vec3_c forward = vecs[side*2];
//		vec3_c upVector = vecs[side*2+1];
//
//		axis_c ax;
//		ax.mat[0] = forward;
//		ax.mat[1] = upVector;
//		ax.mat[2] = upVector.crossProduct(forward);
//
//		sideViews[side].setupLookAtRH(this->pos,forward,upVector);
//		//sideViews[side].invFromAxisAndVector(ax,pos);
//		sideFrustums[side].setupExt(90.f, getShadowMapW(), getShadowMapH(), radius, ax, this->pos);
//	}
//#else
//	vec3_c lookAngles[] = { 
//		vec3_c(0,0,0), vec3_c(0,90,0),
//		vec3_c(0,180,0), vec3_c(0,270,0),
//		vec3_c(-90,0,0), vec3_c(90,0,0)};
//	for(u32 side = 0; side < CUBE_SIDE_COUNT; side++) {
//		axis_c ax;
//		ax.fromAngles(lookAngles[side]);
//		sideViews[side].setupLookAtRH(this->pos,ax.getForward(),ax.getUp());
//		//sideViews[side].fromAxisAndOrigin(ax,this->pos);
//		sideFrustums[side].setupExt(90.f, getShadowMapW(), getShadowMapH(), radius, ax, this->pos);
//	}
//#endif
}
void rLightImpl_c::addShadowMapRenderingDrawCalls() {
	rf_currentShadowMapW = getShadowMapW();
	rf_currentShadowMapH = getShadowMapH();
	rf_bDrawOnlyOnDepthBuffer = true;
	bool bUseBatches = light_batchBSPInteractions.getInt();
	if(isSpotLight()) {
		// spotlights are not using shadow cube maps,
		// they are using only single shadow map
		rf_currentShadowMapCubeSide = 0;
		for(u32 j = 0; j < numCurrentStaticInteractions; j++) {
			staticSurfInteraction_s &sIn = this->staticInteractions[j];
			if(bUseBatches && sIn.batchIndex != -1)
				continue;
			// add static surf interaction for shadow mapping
			addStaticSurfInteractionDrawCall(sIn,true);
		}		
		// add static batches
		if(bUseBatches) {
			addBatchedStaticInteractionDrawCalls();
		}
		for(u32 j = 0; j < numCurrentEntityInteractions; j++) {
			entityInteraction_s &eIn = this->entityInteractions[j];
			RFE_AddEntity(eIn.ent,0,true);
		}
	} else {
		u32 c_culledCubeSides = 0;
		u32 c_staticInteractionsCulledBySideBounds = 0;
		u32 c_staticInteractionsCulledBySideFrustums = 0;
		for(u32 side = 0; side < CUBE_SIDE_COUNT; side++) {	
			const aabb &sideBB = sideBounds[side];
			// light side bounds VS view camera frustum
			if(rf_camera.getFrustum().cull(sideBB) == CULL_OUT) {
				if(light_printCulledShadowMappingPointLightSides.getInt()) {
					g_core->Print("Side %i of light %i culled by camera frustum VS light side bounds\n",side,this);
				}
				c_culledCubeSides++;
				continue;
			}

			rf_currentShadowMapCubeSide = side;
			const frustum_c &sideFrustum = sideFrustums[side];
			for(u32 j = 0; j < numCurrentStaticInteractions; j++) {
				staticSurfInteraction_s &sIn = this->staticInteractions[j];
				if(bUseBatches && sIn.batchIndex != -1)
					continue;
				if(sIn.type == SIT_BSP) {
					// check light side bounds against single interaction bounds
					const aabb &interactionBB = RF_GetSingleBSPSurfaceBounds(sIn.bspSurfaceNumber);
					if(sideBB.intersect(interactionBB)==false) {
						if(light_printCulledShadowMappingPointLightSideInteractions.getInt()) {
							g_core->Print("rLightImpl_c::addShadowMapRenderingDrawCalls: interaction %i with %i tris culled by light cube side %i bounds\n",
								j,RF_GetSingleBSPSurfaceTrianglesCount(sIn.bspSurfaceNumber),rf_currentShadowMapCubeSide);
						}
						c_staticInteractionsCulledBySideBounds++;
						continue;
					}
					// check light frustum (more expensive) against single interaction bounds
					if(sideFrustum.cull(interactionBB)==CULL_OUT) {
						if(light_printCulledShadowMappingPointLightSideInteractions.getInt()) {
							g_core->Print("rLightImpl_c::addShadowMapRenderingDrawCalls: interaction %i with %i tris culled by light cube side %i frustum\n",
								j,RF_GetSingleBSPSurfaceTrianglesCount(sIn.bspSurfaceNumber),rf_currentShadowMapCubeSide);
						}
						c_staticInteractionsCulledBySideFrustums++;
						continue;
					}
				}
				// add static surf interaction for shadow mapping
				addStaticSurfInteractionDrawCall(sIn,true);
			}		
			// add static batches
			if(bUseBatches) {
				addBatchedStaticInteractionDrawCalls(&sideBB,&sideFrustum);
			}
			for(u32 j = 0; j < numCurrentEntityInteractions; j++) {
				entityInteraction_s &eIn = this->entityInteractions[j];
				RFE_AddEntity(eIn.ent,&sideFrustum,true);
			}
		}
		if(light_printShadowMappingPointLightSideCullingStats.getInt()) {
			g_core->Print("rLightImpl_c::addShadowMapRenderingDrawCalls: [POINT LIGHT]: %i has %i sides culled, %i Sinteractions culled by sideBB, %i SInteractions culled by sideFrustum.\n",
			this,c_culledCubeSides,c_staticInteractionsCulledBySideBounds,c_staticInteractionsCulledBySideFrustums);
		}
	}
	rf_bDrawOnlyOnDepthBuffer = false;
	rf_currentShadowMapCubeSide = -1;
	rf_currentShadowMapW = -1;
	rf_currentShadowMapH = -1;
}
extern aCvar_c rf_proc_useProcDataToOptimizeLighting;

bool rLightImpl_c::isCulledByAreas() const {
	if(rf_proc_useProcDataToOptimizeLighting.getInt() == 0)
		return false;
	if(RF_GetNumAreas() == 0)
		return false;
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
// used for debuging scene with single light
static aCvar_c light_useOnlyNearestLight("light_useOnlyNearestLight","0");

bool RFL_GPUOcclusionQueriesForLightsEnabled() {
	return light_useGPUOcclusionQueries.getInt();
}
rLightImpl_c *RFL_GetNearestLightToPoint(const vec3_c &p) {
	if(rf_lights.size() == 0)
		return 0;
	float bestDist = rf_lights[0]->getOrigin().dist(p);
	rLightImpl_c *best = rf_lights[0];
	for(u32 i = 1; i < rf_lights.size(); i++) {
		float d = rf_lights[i]->getOrigin().dist(p);
		if(d < bestDist) {
			bestDist = d;
			best = rf_lights[i];
		}
	}
	return best;
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

		if(light_printLightTypes.getInt()) {
			g_core->Print("Light %i of %i: type %i\n",i,rf_lights.size(),light->getLightType());
		}
		if(light_useOnlyNearestLight.getInt()) {
			rLightImpl_c *nearest = RFL_GetNearestLightToPoint(rf_camera.getOrigin());
			if(nearest != light) {
				light->setCulled(true);
				continue;
			}
		}

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
			// extra .bsp tree lights culling
			if(RF_IsWorldTypeBSP()) {
				//arraySTD_c<u32> tmp;
				//RF_BoxAreas(light->getABSBounds(),tmp);
				if(RF_CullBoundsByPortals(light->getABSBounds())) {
					c_lightsCulled++;
					c_lightsCulledByAreas++;
					g_core->Print("Culled light %i\n",i);
					light->setCulled(true);
					continue;
				}
			}
		}
		light->setCulled(false);

		// TODO: dont do this every frame
		light->recalcLightInteractionsWithDynamicEntities();

		// some lights might have shadow casting disabled - "noShadows" "1" key value
		if(light->getBNoShadows() == false)  {
			// skip shadows of very small lights as they wouldn't be noticeable in game
			if(light_minShadowCastingLightRadius.getFloat() < light->getRadius()) {
				// add shadow drawcalls (but only if shadow casting is enabled for this light)
				if(RF_IsUsingShadowVolumes()) {
					light->addLightShadowVolumesDrawCalls();
				} else if(RF_IsUsingShadowMapping()) {
					light->addShadowMapRenderingDrawCalls();
				}
			}
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