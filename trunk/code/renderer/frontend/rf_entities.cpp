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
#include "rf_skin.h"
#include <api/coreAPI.h>
#include <api/skelModelAPI.h>
#include <api/skelAnimAPI.h>
#include <api/afDeclAPI.h>
#include <api/modelDeclAPI.h>
#include <api/kfModelAPI.h>
#include <api/q3PlayerModelDeclAPI.h>
#include <shared/autoCvar.h>
#include <shared/boneOrQP.h>
#include <shared/afRagdollHelper.h>
#include <shared/trace.h>
#include <shared/animationController.h>
#include <shared/quake3AnimationConfig.h>
#include <shared/kfAnimCtrl.h>
#include <renderer/rfSurfFlags.h>

static aCvar_c rf_skipEntities("rf_skipEntities","0");
static aCvar_c rf_noEntityDrawCalls("rf_noEntityDrawCalls","0");
static aCvar_c rf_forceKFModelsFrame("rf_forceKFModelsFrame","-1");


class q3AnimCtrl_c {
	kfAnimCtrl_s legs;
	kfAnimCtrl_s torso;
public:	
	void setLegsAnim(u32 animIndex, const rModelAPI_i *model) {
		legs.setAnim(animIndex,model);
	}
	void setTorsoAnim(u32 animIndex, const rModelAPI_i *model) {
		torso.setAnim(animIndex,model);
	}
	void runAnimController(int curGlobalTimeMSec) {
		legs.runAnimController(curGlobalTimeMSec);
		torso.runAnimController(curGlobalTimeMSec);
	}
	const kfAnimCtrl_s &getLegs() const {
		return legs;
	}
	const kfAnimCtrl_s &getTorso() const {
		return torso;
	}
};

rEntityImpl_c::rEntityImpl_c() {
	model = 0;
	staticDecals = 0;
	instance = 0;
	skelAnimCtrl = 0;
	axis.identity();
	matrix.identity();
	bFirstPersonOnly = false;
	bThirdPersonOnly = false;
	bHidden = false;
	bIsPlayerModel = false;
	myRagdollDef = 0;
	ragOrs = 0;
	q3AnimCtrl = 0;
	//skinMatList = 0;
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
	if(skelAnimCtrl) {
		delete skelAnimCtrl;
		skelAnimCtrl = 0;
	}
	if(q3AnimCtrl) {
		delete q3AnimCtrl;
		q3AnimCtrl = 0;
	}
	if(ragOrs) {
		delete ragOrs;
		ragOrs = 0;
	}
	//if(skinMatList) {
	//	delete skinMatList;
	//	skinMatList = 0;
	//}
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
void rEntityImpl_c::updateModelSkin() {
	if(model == 0) {
		//if(skinMatList) {
		//	delete skinMatList;
		//	skinMatList = 0;
		//}
		return;
	}
	if(skinName.length() == 0) {
		//if(skinMatList) {
		//	delete skinMatList;
		//	skinMatList = 0;
		//}
		return;
	}
	if(model->isQ3PlayerModel()) {
		const q3PlayerModelAPI_i *q3p = model->getQ3PlayerModelAPI();
		// in this special case we need to load 3 skin files:
		// skin file for head model, for upper body model, and for lower body model
		rSkinRemap_c *skinHead = RF_RegisterSkinForModel(q3p->getHeadModelName(),skinName);
		rSkinRemap_c *skinTorso = RF_RegisterSkinForModel(q3p->getTorsoModelName(),skinName);
		rSkinRemap_c *skinLegs = RF_RegisterSkinForModel(q3p->getLegsModelName(),skinName);
		if(skinHead || skinTorso || skinLegs) {
			//skinMatList = new rSkinMatList_c;
			instance->appendSkinRemap(skinHead);
			instance->appendSkinRemap(skinTorso);
			instance->appendSkinRemap(skinLegs);
		}
	} else {
		// load single skin file
		rSkinRemap_c *skin = RF_RegisterSkinForModel(model->getName(),skinName);
		if(skin) {
			if(instance) {
				instance->appendSkinRemap(skin);
			} else {
				// if we havent a dynamic model instance,
				// we can't apply skin directly.
				// We need to use external material list
			//skinMatList = new rSkinMatList_c;
			}
		}
	}
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
		// init skelmodel / keyframed model instance
		if(newModel->isSkeletal()) {
			const skelModelAPI_i *skelModel = newModel->getSkelModelAPI();
			instance = new r_model_c;
			instance->initSkelModelInstance(skelModel);
			instance->updateSkelModelInstance(skelModel,skelModel->getBaseFrameABS());
		} else if(newModel->isKeyframed()) {
			const kfModelAPI_i *kfModel = newModel->getKFModelAPI();
			instance = new r_model_c;
			instance->initKeyframedModelInstance(kfModel);
			instance->updateKeyframedModelInstance(kfModel,0);
		} else if(newModel->isQ3PlayerModel()) {
			const q3PlayerModelAPI_i *q3PlayerModel = newModel->getQ3PlayerModelAPI();
			instance = new r_model_c;
			instance->initQ3PlayerModelInstance(q3PlayerModel);
			instance->updateQ3PlayerModelInstance(q3PlayerModel,0,0);
		} else if(newModel->isSprite()) {
			instance = new r_model_c;
			instance->initSprite(newModel->getSpriteMaterial(),newModel->getSpriteRadius());
		}
	}
	model = newModel;
	recalcABSBounds();
	updateModelSkin();
}
void rEntityImpl_c::setAnim(const class skelAnimAPI_i *anim) {
	if(anim == 0 && skelAnimCtrl == 0)
		return; // ignore
	if(this->model == 0)
		return; // ignore
	const class skelModelAPI_i *skelModel = this->model->getSkelModelAPI();
	if(skelAnimCtrl == 0) {
		skelAnimCtrl = new skelAnimController_c;
		skelAnimCtrl->resetToAnim(anim,rf_curTimeMsec);
	} else {
		// TODO
		skelAnimCtrl->setNextAnim(anim,skelModel,rf_curTimeMsec);
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
void rEntityImpl_c::setQ3LegsAnimLocalIndex(int localAnimIndex) {
	if(model->isQ3PlayerModel() == false) {
		g_core->Print("rEntityImpl_c::setQ3LegsAnimLocalIndex: called on non-decl model %s\n",model->getName());
		return;
	}
	if(q3AnimCtrl == 0) {
		q3AnimCtrl = new q3AnimCtrl_c;
	}
	q3AnimCtrl->setLegsAnim(localAnimIndex,model);
}
void rEntityImpl_c::setQ3TorsoAnimLocalIndex(int localAnimIndex) {
	if(model->isQ3PlayerModel() == false) {
		g_core->Print("rEntityImpl_c::setQ3TorsoAnimLocalIndex: called on non-decl model %s\n",model->getName());
		return;
	}
	if(q3AnimCtrl == 0) {
		q3AnimCtrl = new q3AnimCtrl_c;
	}
	q3AnimCtrl->setTorsoAnim(localAnimIndex,model);
}
void rEntityImpl_c::setSkin(const char *newSkinName) {
	if(!stricmp(newSkinName,this->skinName)) {
		return; // no change
	}
	this->skinName = newSkinName;
	updateModelSkin();
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
bool rEntityImpl_c::isQ3PlayerModel() const {
	if(model == 0)
		return false;
	if(model->isQ3PlayerModel())
		return true;
	return false;
}
bool rEntityImpl_c::isAnimated() const {
	if(model == 0)
		return false;
	if(model->getSkelModelAPI())
		return true;
	return false;
}
bool rEntityImpl_c::isSprite() const {
	if(model == 0)
		return false;
	if(model->isSprite())
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
const class r_model_c *rEntityImpl_c::getCurrentRModelInstance() const {
	if(model == 0)
		return 0;
	if(instance)
		return instance;
	model_c *m = dynamic_cast<model_c*>(model);
	if(m == 0) {
		g_core->RedWarning("rEntityImpl_c::getCurrentRModelInstance: dynamic cast failed\n");
		return 0;
	}
	return m->getRModel();
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
void rEntityImpl_c::updateAnimatedEntity() {
	// we have an instance of dynamic model.
	// It might be an instance of skeletal model (.md5mesh, etc)
	// or an instance of keyframed model (.md3, .mdc, .md2, etc...)
	if(model->isSkeletal()) {
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
		} else if(skelAnimCtrl) {
			skelAnimCtrl->runAnimController(rf_curTimeMsec);
			skelAnimCtrl->updateModelAnimation(skelModel);
			instance->updateSkelModelInstance(skelModel,skelAnimCtrl->getCurBones());	
			instance->recalcModelNormals(); // this is slow
		}
	} else if(model->isKeyframed()) {
		const kfModelAPI_i *kfModel = model->getKFModelAPI();
		if(rf_forceKFModelsFrame.getInt() >= 0) {
			u32 fixedFrameIndex = kfModel->fixFrameNum(rf_forceKFModelsFrame.getInt());
			instance->updateKeyframedModelInstance(kfModel,fixedFrameIndex);
		} else {

		}
		instance->recalcModelNormals(); // this is slow
	} else if(model->isQ3PlayerModel()) {
		const q3PlayerModelAPI_i *q3Player = model->getQ3PlayerModelAPI();
		if(rf_forceKFModelsFrame.getInt() >= 0) {
			instance->updateQ3PlayerModelInstance(q3Player,rf_forceKFModelsFrame.getInt(),rf_forceKFModelsFrame.getInt());
		} else {
			if(q3AnimCtrl) {
				q3AnimCtrl->runAnimController(rf_curTimeMsec);
				instance->updateQ3PlayerModelInstance(q3Player,
					q3AnimCtrl->getLegs().curLerp.from,
					q3AnimCtrl->getTorso().curLerp.from);	
			}
		}
		instance->recalcModelNormals(); // this is slow
	} else if(model->isSprite()) {
		instance->updateSprite(rf_camera.getAxis(),model->getSpriteRadius());
	}
}
void rEntityImpl_c::addDrawCalls() {
	if(rf_noEntityDrawCalls.getInt())
		return;
	// this is needed here so rf_drawCalls.cpp know
	// which orientation should be used 
	// (world identity matrix vs. entity pos/angles matrix)
	rf_currentEntity = this;

	if(model->isStatic()) {
		((model_c*)model)->addModelDrawCalls(&surfaceFlags);
		if(staticDecals) {
			staticDecals->addDrawCalls();
		}
	} else if(instance) {
		this->updateAnimatedEntity();
		instance->addDrawCalls(&surfaceFlags);
	}

	// done.
	rf_currentEntity = 0;
}
bool rEntityImpl_c::getBoneWorldOrientation(int localBoneIndex, class matrix_c &out) {
	if(model == 0) {
		out = this->matrix;
		return true; // error 
	}
	if(model->isQ3PlayerModel() && this->q3AnimCtrl) {
		matrix_c localMat;
		model->getTagOrientation(localBoneIndex,this->q3AnimCtrl->getLegs().curLerp,this->q3AnimCtrl->getTorso().curLerp,localMat);
		out = this->matrix * localMat;
		return false; // OK
	}
	skelModelAPI_i *skel = model->getSkelModelAPI();
	if(skel == 0) {
		out = this->matrix;
		return true; // error 
	}
	if(skelAnimCtrl == 0) {
		out = this->matrix;
		return true; // error 
	}
	const boneOrArray_c &curBones = skelAnimCtrl->getCurBones();
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
		tr.updateResultsFromTransformedTrace(transformedTrace,this->getMatrix());
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
static u32 c_entitiesCulledByPortals;
void RFE_AddEntity(rEntityImpl_c *ent, const class frustum_c *customFrustum) {
	model_c *model = (model_c*)ent->getModel();
	if(model == 0) {
		return;
	}
	if(ent->isHidden())
		return;
	if(rf_camera.isThirdPerson()) {
		if(ent->isFirstPersonOnly()) {
			return;
		}
	} else {
		if(ent->isThirdPersonOnly()) {
			// if dynamic shadows are enabled we still need to keep 
			// 3rd person-only models updated
			if(RF_IsUsingShadowVolumes()) {
				ent->updateAnimatedEntity();
			}
			return;
		}
	}
	if(customFrustum) {
		if(customFrustum->cull(ent->getBoundsABS()) == CULL_OUT) {
			c_entitiesCulledByABSBounds++;
			return;
		}
	} else {
		if(rf_camera.getFrustum().cull(ent->getBoundsABS()) == CULL_OUT) {
			c_entitiesCulledByABSBounds++;
			return;
		}
		if(RF_CullBoundsByPortals(ent->getBoundsABS())) {
			c_entitiesCulledByPortals++;		
			return;
		}
	}
	ent->addDrawCalls();
}
static aCvar_c rf_printAddEntityDrawCallsCullStats("rf_printAddEntityDrawCallsCullStats","0");
static aCvar_c rf_printAddedEntityModelNames("rf_printAddedEntityModelNames","0");
void RFE_AddEntityDrawCalls() {
	if(rf_skipEntities.getInt())
		return;

	c_entitiesCulledByABSBounds = 0;
	c_entitiesCulledByPortals = 0;
	for(u32 i = 0; i < rf_entities.size(); i++) {
		rEntityImpl_c *ent = rf_entities[i];
		if(rf_printAddedEntityModelNames.getInt()) {
			g_core->Print("RFE_AddEntityDrawCalls: adding entity %i of %i with model %s\n",i,rf_entities.size(),ent->getModelName());
		}
		RFE_AddEntity(ent);
	}
	if(rf_printAddEntityDrawCallsCullStats.getInt()) {
		g_core->Print("RFE_AddEntityDrawCalls: %i culled by frustum, %i by portals (total entiy count %i)\n",c_entitiesCulledByABSBounds,c_entitiesCulledByPortals,rf_entities.size());
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
u32 RFE_BoxEntities(const class aabb &absBounds, arraySTD_c<rEntityImpl_c*> &out) {
	for(u32 i = 0; i < rf_entities.size(); i++) {
		rEntityImpl_c *ent = rf_entities[i];
		const aabb &entBB = ent->getBoundsABS();
		if(absBounds.intersect(entBB)) {
			out.push_back(ent);
		}
	}	
	return out.size();
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
