/*
============================================================================
Copyright (C) 2012-2014 V.

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
#include "rf_sunLight.h"
#include "rf_drawCall.h"
#include <api/coreAPI.h>
#include <api/skelModelAPI.h>
#include <api/skelAnimAPI.h>
#include <api/afDeclAPI.h>
#include <api/modelDeclAPI.h>
#include <api/kfModelAPI.h>
#include <api/q3PlayerModelDeclAPI.h>
#include <api/vfsAPI.h>
#include <api/tikiAPI.h>
#include <shared/autoCvar.h>
#include <shared/boneOrQP.h>
#include <shared/afRagdollHelper.h>
#include <shared/trace.h>
#include <shared/animationController.h>
#include <shared/quake3AnimationConfig.h>
#include <shared/kfAnimCtrl.h>
#include <renderer/rfSurfFlags.h>
#include <shared/entityType.h>

static aCvar_c rf_skipEntities("rf_skipEntities","0");
static aCvar_c rf_noEntityDrawCalls("rf_noEntityDrawCalls","0");
static aCvar_c rf_forceKFModelsFrame("rf_forceKFModelsFrame","-1");
static aCvar_c rf_printKFModelsFrame("rf_printKFModelsFrame","0");
static aCvar_c rf_cullEntities("rf_cullEntities","1");
static aCvar_c rf_printEntityTriangleCounts("rf_printEntityTriangleCounts","0");
static aCvar_c rfe_drawFuncStatic("rfe_drawFuncStatic","1");
static aCvar_c rfe_drawPlayers("rfe_drawPlayers","1");
static aCvar_c rfe_drawGeneral("rfe_drawGeneral","1");
static aCvar_c rfe_printAttachments("rfe_printAttachments","0");
// usefull to see how ragdolls are slowing engine down
static aCvar_c rf_dontUpdateRagdollAnimations("rf_dontUpdateRagdollAnimations","0");
// TIKI debugging
static aCvar_c rf_tiki_verboseSetTIKIModelAnimLocalIndex("rf_tiki_verboseSetTIKIModelAnimLocalIndex","0");
static aCvar_c rf_printAnimatedEntityUpdatesSkippedByDontUpdateFlag("rf_printAnimatedEntityUpdatesSkippedByDontUpdateFlag","0");
static aCvar_c rf_wolf_verboseSetAnim("rf_wolf_verboseSetAnim","0");


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
	skelAnimCtrlTorso = 0;
	axis.identity();
	matrix.identity();
	bFirstPersonOnly = false;
	bThirdPersonOnly = false;
	bHidden = false;
	bIsPlayerModel = false;
	myRagdollDef = 0;
	ragOrs = 0;
	q3AnimCtrl = 0;
	absSilChangeCount = 0;
	//skinMatList = 0;
	bCenterLightSampleValid = false;
	finalBones = 0;
	networkingEntityNumber = -1;
	bHasGlobalColor = false;
	bDontUpdateAnimation = false;
	scale.set(1,1,1);
	kfFrameNum = -1;
	worldBoneMat.identity();
	worldBoneMat.setupZRotation(90.f);
}
rEntityImpl_c::~rEntityImpl_c() {
	RFL_RemoveAllReferencesToEntity(this);
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
	if(skelAnimCtrlTorso) {
		delete skelAnimCtrlTorso;
		skelAnimCtrlTorso = 0;
	}
	if(q3AnimCtrl) {
		delete q3AnimCtrl;
		q3AnimCtrl = 0;
	}
	if(ragOrs) {
		delete ragOrs;
		ragOrs = 0;
	}
	if(finalBones) {
		delete finalBones;
		finalBones = 0;
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
	aabb prevABSBounds = this->absBB;
	// update abs bounds
	if(model->isValid() == false) {
		aabb bb;
		bb.fromRadius(16.f);
		matrix.transformAABB(bb,this->absBB);
	} else if(myRagdollDef) {
		this->absBB.fromHalfSize(999999.f); // TODO
	} else if(instance) {
		const aabb &bb = instance->getBounds();
		matrix.transformAABB(bb,this->absBB);
	} else {
		const aabb &bb = model->getBounds();
		matrix.transformAABB(bb,this->absBB);
	}
	// update grid lighting (for Q3 bsps)
	this->bCenterLightSampleValid = (RF_SampleWorldLightGrid(this->absBB.getCenter(),this->centerLightSample)==false);
	// increase shape change counter
	absSilChangeCount++;
	// V: RF_BoxAreas might be expensive - do it only if bounds has changed
	if(absBB.compare(prevABSBounds)==false) {
		// get areas touching abs bounds (for portal culling)
		RF_BoxAreas(absBB,touchingAreas);
	}
}
void rEntityImpl_c::recalcMatrix() {
	// TODO: use axis instead of angles
	matrix.fromAnglesAndOrigin(angles,origin);
	matrix.scale(scale.getX(),scale.getY(),scale.getZ());
	recalcABSBounds();
}
void rEntityImpl_c::setColor(const float *rgba) {
	if(rgba == 0) {
		bHasGlobalColor = 0;
	} else {
		bHasGlobalColor = true;
		entityColor = rgba; // TODO also set alpha
		entityColor *= 255.f; // FIXME
	}
}
void rEntityImpl_c::setOrigin(const vec3_c &newXYZ) {
	if(origin.compare(newXYZ))
		return;
	origin = newXYZ;
	recalcMatrix();
}
void rEntityImpl_c::setAngles(const vec3_c &newAngles) { 
	if(angles.compare(newAngles))
		return;
	angles = newAngles;
	axis.fromAnglesAndScale(angles,scale);
	recalcMatrix();
}
void rEntityImpl_c::setScale(const class vec3_c &newScale) {
	if(scale.compare(newScale))
		return;
	scale = newScale;
	axis.fromAnglesAndScale(angles,scale);
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
		if(model->hasDefaultSkinName()) {
			skinName = model->getDefaultSkinName();
		} else {
			return;
		}
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
		const char *baseName;
		if(model->hasCharacterFile())
			baseName = model->getSkelModelAPI()->getName();
		else 
			baseName = model->getName();

		rSkinRemap_c *skin = RF_RegisterSkinForModel(baseName,skinName);
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
void rEntityImpl_c::setAttachment(u32 which, const char *modelName, const char *boneName) {
	if(modelName == 0 || boneName == 0) {

	} else {
		rModelAPI_i *am = RF_RegisterModel(modelName);
		int boneIndex = this->getModel()->findBone(boneName);
		if(boneIndex == -1) {
			if(skelAnimCtrl && skelAnimCtrl->getAnim()) {
				boneIndex = skelAnimCtrl->getAnim()->getLocalBoneIndexForBoneName(boneName);
			}
		}
		if(rfe_printAttachments.getInt()) {
			g_core->Print("rEntityImpl_c::setAttachment: model %s - bone %s - %i\n",modelName,boneName,boneIndex);
		}
		if(attachments.size() <= which) {
			attachments.resize(which+1);
		}
		attachments[which].model = am;
		attachments[which].boneIndex = boneIndex;
	}
}
void rEntityImpl_c::setModel(class rModelAPI_i *newModel) {
	if(model == newModel) {
		return;
	}
#if 1
	// hack to get static decl models work
	if(newModel->isDeclModel() && newModel->getSkelModelAPI() == 0) {
		rModelAPI_i *staticModel = RF_RegisterModel(newModel->getDeclModelAPI()->getMeshName());
		if(staticModel && staticModel->isValid()) {
			setModel(staticModel);
			return;
		}
	}
#endif
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
	if(skelAnimCtrlTorso) {
		delete skelAnimCtrlTorso;
		skelAnimCtrlTorso = 0;
	}
	surfaceFlags.resize(newModel->getNumSurfaces());
	surfaceFlags.nullMemory();
	if(newModel->isStatic() == false && newModel->isValid()) {
		// init skelmodel / keyframed model instance
		if(newModel->isSkeletal()) {
			const skelModelAPI_i *skelModel = newModel->getSkelModelAPI();
			if(skelModel == 0) {
				g_core->RedWarning("rEntityImpl_c::setModel: valid model has NULL skelModel pointer (%s)\n",newModel->getName());
				return;
			}
			instance = new r_model_c;
			instance->initSkelModelInstance(skelModel);
			// TIKI models has no baseframe (FAKK/MoHAA)
			// I think the TIKIs of EF2 have baseframe
			// (the .skb format of EF2 is newer and has extra baseframe data)
			if(skelModel->getBaseFrameABS().size() != 0) {
				instance->updateSkelModelInstance(skelModel,skelModel->getBaseFrameABS());
			}
#if 1
			else {
				boneOrArray_c nullBones;
				nullBones.resize(skelModel->getNumBones());
				nullBones.identity();
				instance->updateSkelModelInstance(skelModel,nullBones);
			}
#endif
		} else if(newModel->isKeyframed()) {
			const kfModelAPI_i *kfModel;
			//if(newModel->isTIKI()) {
			//	kfModel = newModel->getTIKI();
			//} else {
				kfModel = newModel->getKFModelAPI();
			//}
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
	if(model->isTIKI() && instance) {
		model->getTIKI()->applyMaterialRemapsTo(instance);
	}
	recalcABSBounds();
	updateModelSkin();
}
void rEntityImpl_c::setAnim(const class skelAnimAPI_i *anim, int newFlags) {
	if(anim == 0 && skelAnimCtrl == 0)
		return; // ignore
	if(this->model == 0)
		return; // ignore
	const class skelModelAPI_i *skelModel = this->model->getSkelModelAPI();
	if(skelAnimCtrl == 0) {
		skelAnimCtrl = new skelAnimController_c;
		skelAnimCtrl->resetToAnim(anim,rf_curTimeMsec,newFlags);
	} else {
		// TODO
		skelAnimCtrl->setNextAnim(anim,skelModel,rf_curTimeMsec,newFlags);
	}
}
void rEntityImpl_c::setAnimationFrame(int frameNum) {
	kfFrameNum = frameNum;
}
void rEntityImpl_c::setAnim(const char *animName, int newFlags) {
	if(this->model == 0)
		return; // ignore
	if(this->model->hasWolfAnimConfig()) {
		//str animName = this->model->getName();
		const class skelAnimAPI_i *anim = this->model->findSkelAnim(animName);
		if(rf_wolf_verboseSetAnim.getInt()) {
			g_core->Print("rEntityImpl_c::setAnim[WolfAnim]: model %s anim %s ptr %i\n",
				getModelName(),animName,anim);
		}
		return setAnim(anim,newFlags);
	}
	if(this->model->hasCharacterFile()) {
		//str animName = this->model->getName();
		const class skelAnimAPI_i *anim = this->model->findSkelAnim(animName);
		if(rf_wolf_verboseSetAnim.getInt()) {
			g_core->Print("rEntityImpl_c::setAnim[CharacterFile]: model %s anim %s ptr %i\n",
				getModelName(),animName,anim);
		}
		return setAnim(anim,newFlags);
	}
	if(this->model->isTIKI()) {
		int animIndex = this->model->getTIKI()->findAnim(animName);
		this->setTIKIModelAnimLocalIndex(animIndex,0);
		return;
	}
	class modelDeclAPI_i *dm = this->model->getDeclModelAPI();
	if(dm == 0)
		return;
	int animIndex = dm->getAnimIndexForAnimAlias(animName);
	if(animIndex < 0)
		return;
	this->setDeclModelAnimLocalIndex(animIndex, newFlags);
}
void rEntityImpl_c::setTorsoAnim(const class skelAnimAPI_i *anim, int newFlags) {
	if(anim == 0 && skelAnimCtrlTorso == 0)
		return; // ignore
	if(this->model == 0)
		return; // ignore
	const class skelModelAPI_i *skelModel = this->model->getSkelModelAPI();
	if(skelAnimCtrlTorso == 0) {
		skelAnimCtrlTorso = new skelAnimController_c;
		skelAnimCtrlTorso->resetToAnim(anim,rf_curTimeMsec,newFlags);
	} else {
		// TODO
		skelAnimCtrlTorso->setNextAnim(anim,skelModel,rf_curTimeMsec,newFlags);
	}
}
void rEntityImpl_c::setTIKIModelAnimLocalIndex(int localAnimIndex, int newFlags) {
	if(model->isTIKI() == false) {
		g_core->Print("rEntityImpl_c::setTIKIModelAnimLocalIndex: called on non-TIKI model %s\n",model->getName());
		return;
	}
	const class skelAnimAPI_i *a = model->getTIKI()->getSkelAnim(localAnimIndex);
	if(rf_tiki_verboseSetTIKIModelAnimLocalIndex.getInt()) {
		if(a) {
			g_core->Print("rEntityImpl_c::setTIKIModelAnimLocalIndex: index %i returned %s anim from %s\n",
				localAnimIndex,a->getName(),model->getName());
		} else {
			g_core->Print("rEntityImpl_c::setTIKIModelAnimLocalIndex: index %i returned NULL anim from %s\n",
				localAnimIndex,model->getName());
		}
	}
	this->setAnim(a,newFlags);
}
void rEntityImpl_c::setTIKIModelTorsoAnimLocalIndex(int localAnimIndex, int newFlags) {
	if(model->isTIKI() == false) {
		g_core->Print("rEntityImpl_c::setTIKIModelTorsoAnimLocalIndex: called on non-TIKI model %s\n",model->getName());
		return;
	}
	const class skelAnimAPI_i *a = model->getTIKI()->getSkelAnim(localAnimIndex);
	if(rf_tiki_verboseSetTIKIModelAnimLocalIndex.getInt()) {
		if(a) {
			g_core->Print("rEntityImpl_c::setTIKIModelTorsoAnimLocalIndex: index %i returned %s anim from %s\n",
				localAnimIndex,a->getName(),model->getName());
		} else {
			g_core->Print("rEntityImpl_c::setTIKIModelTorsoAnimLocalIndex: index %i returned NULL anim from %s\n",
				localAnimIndex,model->getName());
		}
	}
	this->setTorsoAnim(a,newFlags);
}
void rEntityImpl_c::setDeclModelAnimLocalIndex(int localAnimIndex, int newFlags) {
	if(model->isDeclModel() == false) {
		g_core->Print("rEntityImpl_c::setDeclModelAnimLocalIndex: called on non-decl model %s\n",model->getName());
		return;
	}
	const class skelAnimAPI_i *a = model->getDeclModelAPI()->getSkelAnimAPIForLocalIndex(localAnimIndex);
	this->setAnim(a,newFlags);
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
	if(!_stricmp(newSkinName,this->skinName)) {
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
void rEntityImpl_c::setVertexColorsRGB(const byte *p) {
	if(instance) {
		instance->setVertexColorsRGB(p);
	} else {
	}
}
bool rEntityImpl_c::hasTIKIModel() const {
	if(model == 0)
		return false;
	if(model->isTIKI())
		return true;
	return false;
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
// for .md3/.md2/.mdc models
bool rEntityImpl_c::isKeyframed() const {
	if(model == 0)
		return false;
	return model->isKeyframed();
}
bool rEntityImpl_c::hasCharacterFile() const {
	if(model == 0)
		return false;
	return model->hasCharacterFile();
}
bool rEntityImpl_c::hasWolfAnimConfig() const {
	if(model == 0)
		return false;
	return model->hasWolfAnimConfig();
}
bool rEntityImpl_c::hasAnim(const char *animName) const {
	if(model == 0)
		return false;
	if(model->hasAnim(animName))
		return true;
	if(strchr(animName,'.')) {
		// FIXME: this might be slow
		return g_vfs->FS_FileExists(animName);
	}
	return false;
}
bool rEntityImpl_c::hasStageWithoutBlendFunc() const {
	if(instance) {
		return instance->hasStageWithoutBlendFunc();
	}
	if(model) {
		return model->hasStageWithoutBlendFunc();
	}
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
void rEntityImpl_c::markEntityMaterials() {
	if(instance) {
		instance->markMaterials();
	}
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

			
	{
		const afPublicData_s *af = this->myRagdollDef->getData();
		const skelModelAPI_i *skelModel = model->getSkelModelAPI();
		const skelAnimAPI_i *anim = model->getDeclModelAFPoseAnim();
		// V: made those static, no sense to realloc memory every frame
		static arraySTD_c<u32> refCounts;
		// resize only if needed (don't do free/new every function call!)
		if(refCounts.size() < skelModel->getNumBones()) {
			refCounts.resize(skelModel->getNumBones());
		}
		refCounts.nullMemory();
		bodyBoneBones.resize(af->bodies.size());
		for(u32 i = 0; i < af->bodies.size(); i++) {
			const afBody_s &b = af->bodies[i];
			arraySTD_c<u32> &boneNumbers = bodyBoneBones[i];
			boneNumbers.resize(0);//set size to 0 but don't free memory - reallocating is slow
			UTIL_ContainedJointNamesArrayToJointIndexes(b.containedJoints,boneNumbers,anim);
			for(u32 j = 0; j < boneNumbers.size(); j++) {
				int boneNum = boneNumbers[j];
				if(refCounts[boneNum]) {
					// it should NEVER happen
					g_core->RedWarning("Bone %i is referenced more than once in AF\n",boneNum);
					continue;
				}
			}
		}
	}
}
void rEntityImpl_c::setRagdollBodyOr(u32 partIndex, const class boneOrQP_c &or) {
	if(ragOrs == 0)
		return;
	class boneOrQP_c &s = (*ragOrs)[partIndex];
	if(!s.compare(or)) {
		bRagdollOrientationDirty = true; // force vertices update
		s = or;
	}
}
void rEntityImpl_c::updateInstanceAttachments() {
	const skelModelAPI_i *skelModel = model->getSkelModelAPI();
	// we need to know where to append new surfaces
	u32 firstSurface = skelModel->getNumSurfs();
	for(u32 i = 0; i < attachments.size(); i++) {
		class rEntityAttachment_c &a = attachments[i];
		if(a.model == 0)
			continue;
		matrix_c m;
		getBoneLocalOrientation(a.boneIndex,m);

		// needed for only RTCW/ET player heads
		// TODO: better fix
		if(this->getModel() && strstr(this->getModel()->getName(),".mds")) {
			vec3_c angles = m.getAngles();
			angles.y*=-1;
			m.fromAnglesAndOrigin(angles,m.getOrigin());
		}
		if(a.model->isKeyframed()) {
			const kfModelAPI_i *kfModel = a.model->getKFModelAPI();
			instance->updateKeyframedModelInstance(kfModel,0,firstSurface);
			instance->transform(m,firstSurface,firstSurface+kfModel->getNumSurfaces());
			firstSurface += kfModel->getNumSurfaces();
		} else if(a.model->isSkeletal()) {
			const skelModelAPI_i *skelModel = a.model->getSkelModelAPI();
			const boneOrArray_c &bones = skelModel->getBaseFrameABS();
			instance->updateSkelModelInstance(skelModel,bones,firstSurface);
			instance->transform(m,firstSurface,firstSurface+skelModel->getNumSurfs());
			firstSurface += skelModel->getNumSurfs();
		} else {
			// TODO
		}
	}
}
void rEntityImpl_c::updateAnimatedEntity() {
	if(model == 0)
		return;
	if(lastAnimatedEntityUpdateFrame == rf_draw3DViewCount)
		return;
	lastAnimatedEntityUpdateFrame = rf_draw3DViewCount;
	if(bDontUpdateAnimation) {
		if(rf_printAnimatedEntityUpdatesSkippedByDontUpdateFlag.getInt()) {
			g_core->Print("[%i] Skipping %s at %f %f %f\n",
				rf_draw3DViewCount,getModelName(),origin.getX(),origin.getY(),origin.getZ());
		}
		return;
	}

	absSilChangeCount++;
	// we have an instance of dynamic model.
	// It might be an instance of skeletal model (.md5mesh, etc)
	// or an instance of keyframed model (.md3, .mdc, .md2, etc...)
	if(model->isSkeletal()) {
		const skelModelAPI_i *skelModel = model->getSkelModelAPI();
		//const skelAnimAPI_i *anim = model->getDeclModelAFPoseAnim();
		// ragdoll controlers ovverides all the animations
		if(myRagdollDef && rf_dontUpdateRagdollAnimations.getInt() == 0) {
			// HACK, USE WORLD TRANSFORMS
			// TODO: better fix?
			rf_currentEntity = 0; 
			setOrigin(vec3_c(0,0,0));
			setAngles(vec3_c(0,0,0));

			// V: recalculate only if something has changed
			if(bRagdollOrientationDirty) {
				bRagdollOrientationDirty = false;
				const afPublicData_s *af = this->myRagdollDef->getData();

				// V: made those static, no sense to realloc memory every frame
				static boneOrArray_c bones;
				// resize only if needed (don't do free/new every function call!)
				if(bones.size() < skelModel->getNumBones()) {
					bones.resize(skelModel->getNumBones());
				}
				for(u32 i = 0; i < af->bodies.size(); i++) {
					const afBody_s &b = af->bodies[i];
					const boneOrQP_c &partOr = (*ragOrs)[i];
					matrix_c bodyMat;
					bodyMat.fromQuatAndOrigin(partOr.getQuat(),partOr.getPos());
					const arraySTD_c<u32> &boneNumbers = bodyBoneBones[i];
					for(u32 j = 0; j < boneNumbers.size(); j++) {
						int boneNum = boneNumbers[j];
						const matrix_c &bpb2b = boneParentBody2Bone[boneNum];
						bones[boneNum].mat = bodyMat * bpb2b;
					}
				}
				// V: update vertex positions, normals, tangents and binormals (TBN approx)
				instance->updateSkelModelInstance(skelModel,bones);	
			}
#if 0
			// V: this very slow when there are many skeletal models in the scene.
			// Right now we're using precomputed skel surfaces tangent/normals,
			// they are set by updateSkelModelInstance call.

			// NOTE: calculating TBNs from scratch took around 17% of total CPU time on game/mars_city1
			// TODO: check if binormal calculations are correct (handeness is ignored righ now)

			// if model needs normals
			if(1) {
				// if model needs TBN
				if(1) {
					instance->recalcModelTBNs(false); // this is slow
				} else {
					instance->recalcModelNormals(); // this is slow
				}
			}
#endif
		} else if(skelAnimCtrl) {
			skelAnimCtrl->runAnimController(rf_curTimeMsec);
			if(skelAnimCtrlTorso && skelAnimCtrlTorso->getAnim()) {
				skelAnimCtrlTorso->runAnimController(rf_curTimeMsec);
				skelAnimCtrlTorso->updateModelAnimation(skelModel,false,&worldBoneMat);
				if(skelAnimCtrlTorso->getAnim()) {
					skelAnimCtrl->updateModelAnimation(skelModel,false,&worldBoneMat);
					arraySTD_c<u32> torsoBones;
					torsoBones.reserve(skelModel->getNumBones()*2);
					skelAnimCtrlTorso->getAnim()->addChildrenOf(torsoBones,"hip");
					if(torsoBones.size() == 0) {
						skelModel->addTorsoBoneIndices(torsoBones);
					}
					if(finalBones == 0) {
						finalBones = new boneOrArray_c;
					}
					*finalBones = skelAnimCtrl->getCurBones();
					finalBones->setBones(skelAnimCtrlTorso->getCurBones(),torsoBones);
					// transform to abs
					finalBones->localBonesToAbsBones(skelModel->getBoneDefs(),&worldBoneMat);
					// V: update vertex positions, normals, tangents and binormals (TBN approx)
					instance->updateSkelModelInstance(skelModel,*finalBones);
					// append .md3/.mdc attachments at the tags
					updateInstanceAttachments();
				} else {
					skelAnimCtrl->updateModelAnimation(skelModel,true,&worldBoneMat);
					if(finalBones) {
						delete finalBones;
						finalBones = 0;
					}
					// V: update vertex positions, normals, tangents and binormals (TBN approx)
					instance->updateSkelModelInstance(skelModel,skelAnimCtrl->getCurBones());
					updateInstanceAttachments();	
				}
			} else {
				skelAnimCtrl->updateModelAnimation(skelModel,true,&worldBoneMat);
				if(finalBones) {
					delete finalBones;
					finalBones = 0;
				}
				// V: update vertex positions, normals, tangents and binormals (TBN approx)
				instance->updateSkelModelInstance(skelModel,skelAnimCtrl->getCurBones());	
				updateInstanceAttachments();	
			}
#if 0
			// V: this very slow when there are many skeletal models in the scene.
			// Right now we're using precomputed skel surfaces tangent/normals,
			// they are set by updateSkelModelInstance call.

			// NOTE: calculating TBNs from scratch took around 17% of total CPU time on game/mars_city1
			// TODO: check if binormal calculations are correct (handeness is ignored righ now)

			// if model needs normals
			if(1) {
				// if model needs TBN
				if(1) {
					instance->recalcModelTBNs(false); // this is slow
				} else {
					instance->recalcModelNormals(); // this is slow
				}
			}
#endif
		} else if(model->isTIKI()) {
			// skeletal TIKIs should always use first animation available,
			// because it's not possible to build a valid .skb/.skd skeleton
			// without an animation
			skelAnimAPI_i *anim = model->getTIKI()->getSkelAnim(0);
			if(anim) {
				if(finalBones == 0) {
					finalBones = new boneOrArray_c;
				}
				singleAnimLerp_s sl;
				sl.frac = sl.from = sl.to = 0;
				finalBones->resize(skelModel->getNumBones());
				anim->buildLoopAnimLerpFrameBonesLocal(sl,*finalBones,skelModel);
				finalBones->localBonesToAbsBones(skelModel->getBoneDefs(),&worldBoneMat);
				// V: update vertex positions, normals, tangents and binormals (TBN approx)
				instance->updateSkelModelInstance(skelModel,*finalBones);	
				updateInstanceAttachments();	
			} else {
				updateInstanceAttachments();
			}
		}
	} else if(model->isKeyframed()) {
		const kfModelAPI_i *kfModel = model->getKFModelAPI();
		if(kfModel == 0) {

		} else {
			if(rf_forceKFModelsFrame.getInt() >= 0) {
				u32 fixedFrameIndex = kfModel->fixFrameNum(rf_forceKFModelsFrame.getInt());
				instance->updateKeyframedModelInstance(kfModel,fixedFrameIndex);
			} else {
				u32 fixedFrameIndex = kfModel->fixFrameNum(kfFrameNum);
				if(rf_printKFModelsFrame.getInt()) {
					g_core->Print("Using frame %i for model %s\n",kfFrameNum,getModelName());
				}
				instance->updateKeyframedModelInstance(kfModel,fixedFrameIndex);
			}
			// if model needs normals
			if(1) {
				// if model needs TBN
				if(1) {
					instance->recalcModelTBNs(false); // this is slow
				} else {
					instance->recalcModelNormals(); // this is slow
				}
			}
		}
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
		// if model needs normals
		if(1) {
			// if model needs TBN
			if(1) {
				instance->recalcModelTBNs(false); // this is slow
			} else {
				instance->recalcModelNormals(); // this is slow
			}
		}
	} else if(model->isSprite()) {
		instance->updateSprite(rf_camera.getAxis(),model->getSpriteRadius());
	}
	recalcABSBounds();

	if(instance) {
		if(this->bCenterLightSampleValid) {
			if(model->isSprite() == false) {
				// it's faster to do this here
			//	instance->calcVertexLightingABS(this->matrix,this->centerLightSample);
				instance->setAmbientLightingVec3_255(this->centerLightSample.ambientLight);
			}
		}
	}
}
u32 rEntityImpl_c::getEntityVertexCount() const {
	if(instance)
		return instance->getTotalVertexCount();
	if(model)
		return model->getTotalVertexCount();
	return 0;
}
u32 rEntityImpl_c::getEntityTriangleCount() const {
	if(instance)
		return instance->getTotalTriangleCount();
	if(model)
		return model->getTotalTriangleCount();
	return 0;
}
void rEntityImpl_c::addDrawCalls() {
	if(rf_noEntityDrawCalls.getInt())
		return;
	if(rf_printEntityTriangleCounts.getInt()) {
		u32 triangleCount = getEntityTriangleCount();
		g_core->Print("Entity %i (%s) has %i triangles\n",this->networkingEntityNumber,this->getModelName(),triangleCount);
	}
	// this is needed here so rf_drawCalls.cpp know
	// which orientation should be used 
	// (world identity matrix vs. entity pos/angles matrix)
	rf_currentEntity = this;

	if(model->isStatic()) {
		vec3_c *extraRGB;
		if(bCenterLightSampleValid && (RF_IsUsingDynamicLights() == false || RF_IsDrawingPrelitGeometry())) {
			extraRGB = &this->centerLightSample.ambientLight;
		} else {
			if(bHasGlobalColor) {
				extraRGB = &entityColor;
			} else {
				extraRGB = 0;
			}
		}
		((model_c*)model)->addModelDrawCalls(&surfaceFlags,extraRGB);
		if(staticDecals) {
			staticDecals->addDrawCalls();
		}
	} else if(instance) {
		// update animated entity (only once per frame)
		if(rf_draw3DViewCount != this->animatedEntityUpdateFrame) {
			this->animatedEntityUpdateFrame = rf_draw3DViewCount;
			this->updateAnimatedEntity();
		}
		vec3_c *extraRGB;
		if(bHasGlobalColor) {
			extraRGB = &entityColor;
		} else {
			extraRGB = 0;
		}
		// instance->addDrawCalls is called for depth pass and each light pass
		instance->addDrawCalls(&surfaceFlags,bCenterLightSampleValid,extraRGB);
	}

	// done.
	rf_currentEntity = 0;
}
bool rEntityImpl_c::getBoneLocalOrientation(int localBoneIndex, class matrix_c &out) {
	if(localBoneIndex < 0) {
		out.identity();
		return true; // error 
	}
	if(model == 0) {
		out.identity();
		return true; // error 
	}
	if(model->isQ3PlayerModel() && this->q3AnimCtrl) {
		model->getTagOrientation(localBoneIndex,this->q3AnimCtrl->getLegs().curLerp,this->q3AnimCtrl->getTorso().curLerp,out);
		return false; // OK
	}
	skelModelAPI_i *skel = model->getSkelModelAPI();
	if(skel == 0) {
		// try to get static tag orientation
		if(model->getTagOrientation(localBoneIndex,out)==false) {
			return false; // ok
		}
		out.identity();
		return true; // error 
	}
	if(skelAnimCtrl == 0) {
		if(finalBones) {
			out = finalBones->getBoneMat(localBoneIndex);
			return false;
		}
		// we still can try to use baseframe (if it's present)
		if(skel) {
			if(localBoneIndex < skel->getBaseFrameABS().size()) {
				out = skel->getBaseFrameABS()[localBoneIndex].mat;
				return false;
			}
		}
		out.identity();
		return true; // error 
	}
	const boneOrArray_c *curBones;
	if(finalBones) {
		curBones = finalBones;
	} else {
		curBones = &skelAnimCtrl->getCurBones();
	}
	if(localBoneIndex < 0 || localBoneIndex >= curBones->size()) {
		g_core->RedWarning("rEntityImpl_c::getBoneWorldOrientation: bone index %i out of range <0,%i)\n",localBoneIndex,curBones->size());
		out.identity();	
		return true;
	}
	out = curBones->getBoneMat(localBoneIndex);
	return false;
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
		matrix_c localMat;
		// try to get static tag orientation
		if(model->getTagOrientation(localBoneIndex,localMat)==false) {
			out = this->matrix * localMat;	
			return false; // ok
		}
		out = this->matrix;
		return true; // error 
	}
	if(skelAnimCtrl == 0) {
		out = this->matrix;
		return true; // error 
	}
	const boneOrArray_c *curBones;
	if(finalBones) {
		curBones = finalBones;
	} else {
		curBones = &skelAnimCtrl->getCurBones();
	}
	if(localBoneIndex < 0 || localBoneIndex >= curBones->size()) {
		g_core->RedWarning("rEntityImpl_c::getBoneWorldOrientation: bone index %i out of range <0,%i)\n",localBoneIndex,curBones->size());
		out = this->matrix;
		return true;
	}
	const matrix_c &localMat = curBones->getBoneMat(localBoneIndex);
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
	RF_AddSunLightInteractionEntity(ent);
	return ent;
}
void RFE_RemoveEntity(class rEntityAPI_i *ent) {
	rEntityImpl_c *rent = (rEntityImpl_c*)ent;
	rf_entities.remove(rent);
	RF_RemoveSunLightInteractionEntity(rent);
	delete rent;
}
static u32 c_entitiesCulledByABSBounds;
static u32 c_entitiesCulledByPortals;
void RFE_AddEntity(rEntityImpl_c *ent, const class frustum_c *customFrustum, bool forceThirdPerson) {
	if(ent == 0) {
		g_core->RedWarning("RFE_AddEntity: NULL entity pointer passed\n");
		return;
	}
	model_c *model = (model_c*)ent->getModel();
	if(model == 0) {
		return;
	}
	if(ent->isHidden())
		return;
	// allow developer to hide certain kinds of entities
	if(rfe_drawFuncStatic.getInt() == 0 && ent->getEntityType() == ET_FUNC_STATIC) {
		return;
	}
	if(rfe_drawPlayers.getInt() == 0 && ent->getEntityType() == ET_PLAYER) {
		return;
	}
	if(rfe_drawGeneral.getInt() == 0 && ent->getEntityType() == ET_GENERAL) {
		return;
	}
	if(rf_camera.isThirdPerson() || forceThirdPerson) {
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
	if(rf_cullEntities.getInt() && !rf_bDrawingSunShadowMapPass) {
		// don't even try to cull first person only entities
		// (player hands and gun viewmodel)
		if(ent->isFirstPersonOnly()==false) {
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
				if(RF_CullBoundsByPortals(ent->getBoundsABS(),ent->getTouchingAreas())) {
					c_entitiesCulledByPortals++;		
					return;
				}
			}
		}
	}
	ent->addDrawCalls();
}
void RFE_IterateEntities(void (*callback)(class rEntityImpl_c *ent)) {
	for(u32 i = 0; i < rf_entities.size(); i++) {
		callback(rf_entities[i]);
	}
}
void RF_GetEntitiesBounds(class aabb &out) {
	for(u32 i = 0; i < rf_entities.size(); i++) {
		out.addBox(rf_entities[i]->getBoundsABS());
	}
}
static aCvar_c rf_printAddEntityDrawCallsCullStats("rf_printAddEntityDrawCallsCullStats","0");
static aCvar_c rf_printAddedEntityModelNames("rf_printAddedEntityModelNames","0");
static aCvar_c rf_printTotalREntitiesCount("rf_printTotalREntitiesCount","0");
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
		if(rf_bDrawingSunShadowMapPass) {
			// force third person view while rendering sun shadows
			RFE_AddEntity(ent,0,true);
		} else {
			RFE_AddEntity(ent);
		}
	}
	if(rf_printAddEntityDrawCallsCullStats.getInt()) {
		g_core->Print("RFE_AddEntityDrawCalls: %i culled by frustum, %i by portals (total entiy count %i)\n",c_entitiesCulledByABSBounds,c_entitiesCulledByPortals,rf_entities.size());
	}
	if(rf_printTotalREntitiesCount.getInt()) {
		g_core->Print("RFE_AddEntityDrawCalls: %i entities\n",rf_entities.size());
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
void RFE_MarkEntityMaterials() {
	for(u32 i = 0; i < rf_entities.size(); i++) {
		rf_entities[i]->markEntityMaterials();
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
	if(RFT_RayTraceTerrain(tr)) {
		bHit = true;
	}
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
bool RF_DoCameraTrace(class trace_c &tr, bool bSkipPlayerModels) {
	vec3_c to = rf_camera.getOrigin() + rf_camera.getForward() * 10000.f;
	tr.setupRay(rf_camera.getOrigin(),to);
	return RF_TraceSceneRay(tr,bSkipPlayerModels);
}
