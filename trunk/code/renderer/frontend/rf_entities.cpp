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
#include "rf_surface.h"
#include "rf_anims.h"
#include "rf_world.h"
#include <api/coreAPI.h>
#include <api/skelModelAPI.h>
#include <api/skelAnimAPI.h>
#include <api/afDeclAPI.h>
#include <api/modelDeclAPI.h>
#include <shared/autoCvar.h>
#include <shared/boneOrQP.h>
#include <shared/afRagdollHelper.h>
#include <shared/trace.h>
#include <shared/animationController.h>
#include <renderer/rfSurfFlags.h>

static aCvar_c rf_skipEntities("rf_skipEntities","0");

rEntityImpl_c::rEntityImpl_c() {
	model = 0;
	staticDecals = 0;
	instance = 0;
	animCtrl = 0;
	axis.identity();
	matrix.identity();
	bFirstPersonOnly = false;
	bThirdPersonOnly = false;
	bHidden = false;
	bIsPlayerModel = false;
	myRagdollDef = 0;
	ragOrs = 0;
}
rEntityImpl_c::~rEntityImpl_c() {
	if(staticDecals) {
		delete staticDecals;
		staticDecals = 0;
	}
	if(instance) {
		delete instance;
		instance = 0;
	}
	if(animCtrl) {
		delete animCtrl;
		animCtrl = 0;
	}
	if(ragOrs) {
		delete ragOrs;
		ragOrs = 0;
	}
}

void rEntityImpl_c::recalcABSBounds() {
	if(model == 0) {
		absBB.clear();
		return;
	}
	if(myRagdollDef) {
		this->absBB.fromHalfSize(999999.f); // TODO
	} else if(instance) {
		const aabb &bb = instance->getBounds();
		matrix.transformAABB(bb,this->absBB);
	} else {
		const aabb &bb = model->getBounds();
		matrix.transformAABB(bb,this->absBB);
	}
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
	if(model == newModel) {
		return;
	}
	if(staticDecals) {
		delete staticDecals;
		staticDecals = 0;
	}
	if(instance) {
		delete instance;
		instance = 0;
	}
	surfaceFlags.resize(newModel->getNumSurfaces());
	surfaceFlags.nullMemory();
	if(newModel->isStatic() == false && newModel->isValid()) {
		skelModelAPI_i *skelModel = newModel->getSkelModelAPI();
		if(skelModel) {
			instance = new r_model_c;
			instance->initSkelModelInstance(skelModel);
#if 1
			instance->updateSkelModelInstance(skelModel,skelModel->getBaseFrameABS());
#else
			rfAnimation_c *anim = RF_RegisterAnimation("models/player/shina/run.md5anim");
			boneOrArray_c bones;
			bones.resize(anim->getAPI()->getNumBones());
			anim->getAPI()->buildFrameBonesLocal(0,bones);
			bones.localBonesToAbsBones(anim->getAPI()->getBoneDefs());
			if(skelModel->hasCustomScaling()) {
				bones.scaleXYZ(skelModel->getScaleXYZ());
			}
			instance->updateSkelModelInstance(skelModel,bones);	
#endif
		}
	}
	model = newModel;
	recalcABSBounds();
}
void rEntityImpl_c::setAnim(const class skelAnimAPI_i *anim) {
	if(anim == 0 && animCtrl == 0)
		return; // ignore
	if(this->model == 0)
		return; // ignore
	const class skelModelAPI_i *skelModel = this->model->getSkelModelAPI();
	if(animCtrl == 0) {
		animCtrl = new animController_c;
		animCtrl->resetToAnim(anim,rf_curTimeMsec);
	} else {
		// TODO
		animCtrl->setNextAnim(anim,skelModel,rf_curTimeMsec);
	}
}
void rEntityImpl_c::setDeclModelAnimLocalIndex(int localAnimIndex) {
	if(model->isDeclModel() == false) {
		g_core->Print("rEntityImpl_c::setDeclModelAnimLocalIndex: called on non-decl model %s\n",model->getName());
		return;
	}
	const class skelAnimAPI_i *a = model->getDeclModelAPI()->getSkelAnimAPIForLocalIndex(localAnimIndex);
	this->setAnim(a);
}
void rEntityImpl_c::hideModel() {
	bHidden = true;
}
void rEntityImpl_c::showModel() {
	bHidden = false;
}	
bool rEntityImpl_c::hasDeclModel() const {
	if(model == 0)
		return false;
	if(model->isDeclModel())
		return true;
	return false;
}
rModelAPI_i *rEntityImpl_c::getModel() const {
	return model;
}
const char *rEntityImpl_c::getModelName() const {
	if(model == 0)
		return "noModel";
	return model->getName();
}
void rEntityImpl_c::hideSurface(u32 surfNum) {
	if(surfaceFlags.size() <= surfNum) {
		g_core->RedWarning("rEntityImpl_c::hideSurface: surface index %i out of range <0,%i)\n",surfNum,surfaceFlags.size());
		return;
	}
	surfaceFlags[surfNum].setFlag(RSF_HIDDEN_3RDPERSON);
	surfaceFlags[surfNum].setFlag(RSF_HIDDEN_1STPERSON);
}
void rEntityImpl_c::setRagdoll(const class afDeclAPI_i *af) {
	if(myRagdollDef == af)
		return;
	myRagdollDef = af;
	if(myRagdollDef == 0) {
		if(ragOrs != 0) {
			delete ragOrs;
			ragOrs = 0;
		}
		return;
	}
	if(ragOrs == 0) {
		ragOrs = new boneOrQPArray_t;
	}
	ragOrs->resize(af->getNumBodies());
	afRagdollHelper_c rh;
	rh.calcBoneParentBody2BoneOfsets(af->getName(),boneParentBody2Bone);
}
void rEntityImpl_c::setRagdollBodyOr(u32 partIndex, const class boneOrQP_c &or) {
	if(ragOrs == 0)
		return;
	(*ragOrs)[partIndex] = or;
}
void rEntityImpl_c::addDrawCalls() {
	if(model->isStatic()) {
		((model_c*)model)->addModelDrawCalls(&surfaceFlags);
		if(staticDecals) {
			staticDecals->addDrawCalls();
		}
	} else if(instance) {
		const skelModelAPI_i *skelModel = model->getSkelModelAPI();
		const skelAnimAPI_i *anim = model->getDeclModelAFPoseAnim();
		// ragdoll controlers ovverides all the animations
		if(myRagdollDef) {
			rf_currentEntity = 0; // HACK, USE WORLD TRANSFORMS
			const afPublicData_s *af = this->myRagdollDef->getData();
			arraySTD_c<u32> refCounts;
			boneOrArray_c bones;
			refCounts.resize(skelModel->getNumBones());
			refCounts.nullMemory();
			bones.resize(skelModel->getNumBones());
			for(u32 i = 0; i < af->bodies.size(); i++) {
				const afBody_s &b = af->bodies[i];
				const boneOrQP_c &partOr = (*ragOrs)[i];
				matrix_c bodyMat;
				bodyMat.fromQuatAndOrigin(partOr.getQuat(),partOr.getPos());
				arraySTD_c<u32> boneNumbers;
				afRagdollHelper_c::containedJointNamesArrayToJointIndexes(b.containedJoints,boneNumbers,anim,af->name);
				for(u32 j = 0; j < boneNumbers.size(); j++) {
					int boneNum = boneNumbers[j];
					if(refCounts[boneNum]) {
						// it should NEVER happen
						g_core->RedWarning("Bone %i is referenced more than once in AF\n",boneNum);
						continue;
					}
					refCounts[boneNum] ++;
					const matrix_c &bpb2b = boneParentBody2Bone[boneNum];
					bones[boneNum].mat = bodyMat * bpb2b;
				}
			}
			instance->updateSkelModelInstance(skelModel,bones);	
			instance->recalcModelNormals(); // this is slow
		} else if(animCtrl) {
			animCtrl->runAnimController(rf_curTimeMsec);
			animCtrl->updateModelAnimation(skelModel);
			instance->updateSkelModelInstance(skelModel,animCtrl->getCurBones());	
			instance->recalcModelNormals(); // this is slow
		}
		instance->addDrawCalls(&surfaceFlags);
	}
}
bool rEntityImpl_c::getBoneWorldOrientation(int localBoneIndex, class matrix_c &out) {
	if(model == 0)
		return true; // error
	skelModelAPI_i *skel = model->getSkelModelAPI();
	if(skel == 0)
		return true; // error 
	if(animCtrl == 0)
		return true; // error
	const boneOrArray_c &curBones = animCtrl->getCurBones();
	if(localBoneIndex < 0 || localBoneIndex >= curBones.size()) {
		g_core->RedWarning("rEntityImpl_c::getBoneWorldOrientation: bone index %i out of range <0,%i)\n",localBoneIndex,curBones.size());
		return true;
	}
	const matrix_c &localMat = curBones[localBoneIndex].mat;
	out = this->matrix * localMat;
	return false;
}
bool rEntityImpl_c::getBoneWorldOrientation(const char *boneName, class matrix_c &out) {
	if(model == 0)
		return true; // error
	skelModelAPI_i *skel = model->getSkelModelAPI();
	if(skel == 0)
		return true;
	int boneIndex = skel->getLocalBoneIndexForBoneName(boneName);
	if(boneIndex == -1)
		return true; // error, bone not found
	return getBoneWorldOrientation(boneIndex,out);
}
bool rEntityImpl_c::rayTraceLocal(class trace_c &tr) const {
	if(instance) {
		// TODO: ensure that skeletal model instance is up to date?
		if(instance->traceRay(tr,false)) {
			tr.setHitREntity((class rEntityImpl_c*)this);
			return true; // hit skeletal model instance
		}
		return false; // no hit
	} else if(model) {
		// raycast static model
		if(model->rayTrace(tr)) {
			tr.setHitREntity((class rEntityImpl_c*)this);
			return true; // hit
		}
		return false; // no hit
	}
	return false; // no hit
}
bool rEntityImpl_c::rayTraceWorld(class trace_c &tr) const {
	if(this->myRagdollDef) {
		// ragdolls are in world space
		return this->rayTraceLocal(tr);
	}
	trace_c transformedTrace;
	tr.getTransformed(transformedTrace,this->getMatrix());
	if(rayTraceLocal(transformedTrace)) {
		tr.updateResultsFromTransformedTrace(transformedTrace);
		return true; // hit
	}
	return false; // no hit
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
static u32 c_entitiesCulledByABSBounds;
void RFE_AddEntity(rEntityImpl_c *ent) {
	rf_currentEntity = ent;
	model_c *model = (model_c*)ent->getModel();
	if(model == 0) {
		return;
	}
	if(ent->isHidden())
		return;
	if(rf_camera.isThirdPerson()) {
		if(ent->isFirstPersonOnly())
			return;
	} else {
		if(ent->isThirdPersonOnly())
			return;
	}
	if(rf_camera.getFrustum().cull(ent->getBoundsABS()) == CULL_OUT) {
		c_entitiesCulledByABSBounds++;
		return;
	}
	ent->addDrawCalls();
}
void RFE_AddEntityDrawCalls() {
	if(rf_skipEntities.getInt())
		return;

	c_entitiesCulledByABSBounds = 0;
	for(u32 i = 0; i < rf_entities.size(); i++) {
		rEntityImpl_c *ent = rf_entities[i];
		RFE_AddEntity(ent);
	}
	if(0) {
		g_core->Print("RFE_AddEntityDrawCalls: %i of %i entities culled\n",c_entitiesCulledByABSBounds,rf_entities.size());
	}
	rf_currentEntity = 0;
}
void RFE_DrawEntityAbsBounds() {
	for(u32 i = 0; i < rf_entities.size(); i++) {
		rEntityImpl_c *ent = rf_entities[i];
		const aabb &bb = ent->getBoundsABS();
		rb->drawBBLines(bb);
	}
}
void RFE_ClearEntities() {
	for(u32 i = 0; i < rf_entities.size(); i++) {
		delete rf_entities[i];
		rf_entities[i] = 0;
	}
	rf_entities.clear();
}

bool RF_TraceSceneRay(class trace_c &tr, bool bSkipPlayerModels) {
	bool bHit = RF_RayTraceWorld(tr);
	for(u32 i = 0; i < rf_entities.size(); i++) {
		const rEntityImpl_c *ent = rf_entities[i];
		if(bSkipPlayerModels && ent->isPlayerModel()) {
			continue;
		}
		const aabb &worldBB = ent->getBoundsABS();
		if(tr.getTraceBounds().intersect(worldBB) == false)
			continue;
		if(ent->rayTraceWorld(tr)) {
			bHit = true;
		}
	}
	return bHit;
}
