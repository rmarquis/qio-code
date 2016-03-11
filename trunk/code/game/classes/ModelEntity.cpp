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
// ModelEntity.h - base class for all entities

#include "../g_local.h"
#include "ModelEntity.h"
#include <api/ddAPI.h>
#include <api/cmAPI.h>
#include <math/vec3.h>
#include <math/quat.h>
#include "../bt_include.h"
#include "../g_ragdoll.h"
#include <api/declManagerAPI.h>
#include <api/modelDeclAPI.h>
#include <api/afDeclAPI.h>
#include <api/coreAPI.h>
#include <api/physObjectAPI.h>
#include <api/physAPI.h>
#include <api/skelAnimAPI.h>
#include <api/entityDeclAPI.h>
#include <api/entDefAPI.h>
#include <api/vfsAPI.h>
#include <api/tikiAPI.h>
#include <shared/physObjectDef.h>
#include <shared/keyValuesListener.h>
#include <shared/boneOrQP.h>
#include <shared/autoCvar.h>
#include <shared/afRagdollHelper.h>
#include <shared/skelUtils.h>

DEFINE_CLASS(ModelEntity, "BaseEntity");
DEFINE_CLASS_ALIAS(ModelEntity, brushmodel);
DEFINE_CLASS_ALIAS(ModelEntity, func_moveable);
DEFINE_CLASS_ALIAS(ModelEntity, idAnimated);
// load Doom3 game/pdas map to test it
DEFINE_CLASS_ALIAS(ModelEntity, idMoveablePDAItem);
DEFINE_CLASS_ALIAS(ModelEntity, idPDAItem);
DEFINE_CLASS_ALIAS(ModelEntity, idMoveableItem);
DEFINE_CLASS_ALIAS(ModelEntity, idAFEntity_Generic);
DEFINE_CLASS_ALIAS(ModelEntity, idSpawnableEntity);
DEFINE_CLASS_ALIAS(ModelEntity, idFuncSplat);

static aCvar_c g_verboseSetAnimationCalls("g_verboseSetAnimationCalls","0");
static aCvar_c g_verboseSetDamageZoneCalls("g_verboseSetDamageZoneCalls","0");

class damageZonesList_c {
	class dmgZone_c {
		str name;
		arraySTD_c<u32> boneNumbers;
		float multipler;
	public:
		dmgZone_c(const char *newName = "unnamedZone") {
			name = newName;
			multipler = 1.f;
		}
		bool hasName(const char *s) const {
			return !_stricmp(name.c_str(),s);
		}
		void setBoneIndices(const arraySTD_c<u32> &newIndices) {
			boneNumbers = newIndices;
		}
		bool isBoneReferenced(u32 boneNumber) const {
			if(boneNumbers.isOnList(boneNumber))
				return true;
			return false;
		}
		void removeBone(u32 boneNum) {
			boneNumbers.remove(boneNum);
		}
		const char *getName() const {
			return name.c_str();
		}
	};
	arraySTD_c<dmgZone_c> zones;
public:
	int findBoneZone(u32 boneNumber) const {
		for(u32 i = 0; i < zones.size(); i++) {
			if(zones[i].isBoneReferenced(boneNumber)) {
				return i;
			}
		}
		return -1;
	}
	int findZone(const char *zoneName) const {
		for(u32 i = 0; i < zones.size(); i++) {
			if(zones[i].hasName(zoneName))
				return i;
		}
		return -1; // not found
	}
	const char *getZoneName(int idx) const {
		if(idx < 0)
			return "defaultZone";
		if(idx >= zones.size())
			return "zone_index_out_of_range";
		return zones[idx].getName();
	}
	void setZone(const char *zoneName, const arraySTD_c<u32> &boneIndices) {
		int idx = findZone(zoneName);
		if(idx < 0) {
			idx = zones.size();
			zones.push_back(dmgZone_c(zoneName));
		}
		for(u32 i = 0; i < boneIndices.size(); i++) {
			u32 boneIndex = boneIndices[i];
			int boneZone = findBoneZone(boneIndex);
			if(boneZone >= 0) {
				if(g_verboseSetDamageZoneCalls.getInt()) {
					g_core->Print("damageZonesList_c::setZone: bone %i is already referenced\n",boneIndex);
				}
				// simulate Doom3 behaviour - overwrite joint group
				zones[boneZone].removeBone(boneIndex);
			}
		}
		zones[idx].setBoneIndices(boneIndices);
		if(g_verboseSetDamageZoneCalls.getInt()) {
			g_core->Print("damageZonesList_c::setZone: %s\n",zoneName);
		}
	}
};

ModelEntity::ModelEntity() {
	body = 0;
	cmod = 0;
	cmSkel = 0;
	tiki = 0;
	ragdoll = 0;
	health = 100;
	modelDecl = 0;
	bTakeDamage = false;
	initialRagdolPose = 0;
	bUseRModelToCreateDynamicCVXShape = false;
	bUseDynamicConvexForTrimeshCMod = false;
	bPhysicsBodyKinematic = false;
	bRigidBodyPhysicsEnabled = true;
	linearVelocity.set(0,0,0);
	pvsBoundsSkinWidth = 0.f;
	mass = 10.f;
	physBounciness = 0.f;
	damageZones = 0;
	legsAnimationTime = 0;
}
ModelEntity::~ModelEntity() {
	if(body) {
		destroyPhysicsObject();
	}
	if(ragdoll) {
		delete ragdoll;
	}
	if(initialRagdolPose) {
		delete initialRagdolPose;
	}
	if(damageZones) {
		delete damageZones;
	}
}
const class vec3_c &ModelEntity::getPhysicsOrigin() const {
	return body->getRealOrigin();
}
void ModelEntity::setOrigin(const vec3_c &newXYZ) {
	BaseEntity::setOrigin(newXYZ);
	if(body) {
#if 1
		body->setOrigin(newXYZ);
#else
		this->destroyPhysicsObject();
		this->initRigidBodyPhysics();
#endif
	}
}
void ModelEntity::setAngles(const class vec3_c &newAngles) {
	BaseEntity::setAngles(newAngles);
	if(body) {
		//body->setAngles(newAngles);
		body->setMatrix(this->getMatrix());
	}
}
void ModelEntity::setRenderModel(const char *newRModelName) {
	// NOTE: render model pointer isnt stored in game module.
	// It's clientside only.
	this->myEdict->s->rModelIndex = G_RenderModelIndex(newRModelName);
	renderModelName = newRModelName;
	// decl models are present on both client and server
	if(newRModelName[0] != '*') {
		// try to load a decl model (Doom3, Quake4)
		if(g_declMgr) {
			this->modelDecl = g_declMgr->registerModelDecl(newRModelName);
		} else {
			this->modelDecl = 0;
		}
		// try to load a TIKI model (FAKK, MoHAA)
		if(g_tikiMgr) {
			this->tiki = g_tikiMgr->registerModel(newRModelName);
		} else {
			this->tiki = 0;
		}
	} else {
		this->modelDecl = 0;
	}
	if(this->modelDecl) {
		// update animation indexes
		setAnimation(this->animName);
	}

	this->recalcABSBounds();
	this->link();
}
bool ModelEntity::hasRenderModel(const char *checkRModelName) const {
	if(!_stricmp(this->renderModelName,checkRModelName))
		return true;
	return false;
}
void ModelEntity::setRenderModelSkin(const char *newSkinName) {
	this->myEdict->s->rSkinIndex = G_RenderSkinIndex(newSkinName);
}
void ModelEntity::setRenderModelAttachment(u32 index, const char *boneName, const char *modelName) {
	this->myEdict->s->attachments[index].modelIndex = G_RenderModelIndex(modelName);
	this->myEdict->s->attachments[index].boneIndex = G_RenderModelIndex(boneName);// use as bone index
}
void ModelEntity::setSpriteModel(const char *newSpriteMaterial, float newSpriteRadius) {
	// "sprites/plasma1|sprite|radius,32"
	str newRenderModelName = newSpriteMaterial;
	newRenderModelName.append("|sprite");
	newRenderModelName.append(va("|radius,%f",newSpriteRadius));
	this->setRenderModel(newRenderModelName);
}
int ModelEntity::getBoneNumForName(const char *boneName) {
	if(this->modelDecl) {
		// this is working for tags as well
		return this->modelDecl->getBoneNumForName(boneName);
	}
	if(cmSkel == 0) {
		if(cm) {
			cmSkel = cm->registerSkelModel(this->renderModelName);
		}
	}
	if(cmSkel) {
		return cmSkel->getBoneNumForName(boneName);
	}
	return -1;
}
void ModelEntity::setInternalAnimationIndex(int newAnimIndex) {
	if(this->myEdict->s->animIndex == newAnimIndex) {
		return;
	}
	this->myEdict->s->animIndex = newAnimIndex;
	animName = va("_internalAnim%i",newAnimIndex);
}
bool ModelEntity::hasDeclAnimation(const char *animName) const {
	if(modelDecl == 0)
		return false; // model is not a decl
	if(modelDecl->getSkelAnimAPIForAlias(animName))
		return true; // animation is present
	return false; // animation not found
}
// returns the animation index (for networking)
int ModelEntity::findAnimationIndex(const char *newAnimName) {
	int newAnimIndex;
	if(this->tiki) {
		// internal TIKI animation alias (eg. "walk" or "idle")
		newAnimIndex = this->tiki->findAnim(newAnimName);
	} else if(this->modelDecl) {
		newAnimIndex = this->modelDecl->getAnimIndexForAnimAlias(newAnimName);
	} else {
		// this is a animation file path (eg. models/test/myanim.md5anim)
		newAnimIndex = G_AnimationIndex(newAnimName);
		// nothing else to do right now...
	}
	return newAnimIndex;
}
void ModelEntity::setAnimation(const char *newAnimName) {
	if(g_verboseSetAnimationCalls.getInt()) {
		g_core->Print("ModelEntity::setAnimation: %s\n",newAnimName);
	}
	int newIndex = findAnimationIndex(newAnimName);;
	// save the animation string name
	animName = newAnimName;
	if(newIndex == this->myEdict->s->animIndex)
		return;
	legsAnimationTime = 0;
	// get the animation index (for networking)
	this->myEdict->s->animIndex = newIndex;
}
int ModelEntity::getCurrentAnimationTotalTimeMs() const {
	if(tiki) {
		return tiki->getAnimTotalTimeMs(this->myEdict->s->animIndex);
	}
	return 0; // TODO?
}
void ModelEntity::setTorsoAnimation(const char *newAnimName) {
	if(g_verboseSetAnimationCalls.getInt()) {
		g_core->Print("ModelEntity::setTorsoAnimation: %s\n",newAnimName);
	}
	// get the animation index (for networking)
	this->myEdict->s->torsoAnim = findAnimationIndex(newAnimName);
	// save the animation string name
	torsoAnimName = newAnimName;
}
void ModelEntity::getCurrentBonesArray(class boneOrArray_c &out) {
	if(modelDecl) {
		const skelAnimAPI_i *anim = modelDecl->getSkelAnimAPIForLocalIndex(this->myEdict->s->animIndex);
		if(anim == 0) {
			g_core->RedWarning("ModelEntity::getCurrentBonesArray: failed to get anim %i from decl %s\n",myEdict->s->animIndex,modelDecl->getModelDeclName());
			return;
		}
		out.resize(anim->getNumBones());
		anim->buildFrameBonesABS(0,out);
	} else {
		// TODO
		g_core->RedWarning("ModelEntity::getCurrentBonesArray: failed\n");
	}
}
bool ModelEntity::setColModel(const char *newCModelName) {
	if(cm == 0)
		return true;
	this->cmod = cm->registerModel(newCModelName);
	if(this->cmod == 0)
		return true; // error
	this->myEdict->s->colModelIndex = G_CollisionModelIndex(newCModelName);
	recalcABSBounds();
	return false;
}
void ModelEntity::setRagdollName(const char *ragName) {
	if(ragdoll) {
		delete ragdoll;
		ragdoll = 0;
		this->ragdollDefName = ragName;
		initRagdollPhysics();
	} else {
		this->ragdollDefName = ragName;
	}
}
bool ModelEntity::setColModel(class cMod_i *newCModel) {
	return setColModel(newCModel->getName());
}
void ModelEntity::debugDrawCollisionModel(class rDebugDrawer_i *dd) {
	if(cmod == 0)
		return;
	if(cmod->isCapsule()) {
		cmCapsule_i *c = cmod->getCapsule();
		dd->drawCapsuleZ(getCModelOrigin(), c->getHeight(), c->getRadius());
	} else if(cmod->isBBExts()) {
		cmBBExts_i *bb = cmod->getBBExts();
		dd->drawBBExts(getOrigin(),getAngles(),bb->getHalfSizes());
	}
}		
bool ModelEntity::hasPhysicsObject() const {
	if(body)
		return true;
	return false;
}
bool ModelEntity::hasCollisionModel() const {
	if(cmod)
		return true;
	return false;
}
bool ModelEntity::isDynamic() const {
	if(body == 0)
		return false;
	return body->isDynamic();
}
void ModelEntity::setDamageZone(const char *zoneName, const char *value) {
	if(damageZones == 0)
		damageZones = new damageZonesList_c;
	if(modelDecl) {
		arraySTD_c<str> names;
		arraySTD_c<u32> numbers;
		str(value).tokenize(names);
		UTIL_ContainedJointNamesArrayToJointIndexes(names,numbers,modelDecl->getSkelAnimAPIForLocalIndex(0));
		damageZones->setZone(zoneName,numbers);
	} else {

	}
}
bool ModelEntity::hasDamageZones() const {
	if(damageZones == 0)
		return false;
	return true;
}
int ModelEntity::findBoneDamageZone(int boneNum) const {
	if(damageZones == 0)
		return -1;
	return damageZones->findBoneZone(boneNum);
}
const char *ModelEntity::getDamageZoneName(int zoneNum) const {
	if(damageZones == 0)
		return "defaultZone";
	return damageZones->getZoneName(zoneNum);
}
void ModelEntity::printDamageZones() const {
	if(modelDecl == 0)
		return;
	if(damageZones == 0)
		return;
	for(u32 i = 0; i < modelDecl->getNumBones(); i++) {
		int zone = damageZones->findBoneZone(i);
		if(zone < 0) {
			g_core->Print("Bone %i (%s) has no zone\n",i,modelDecl->getBoneName(i));
		} else {
			g_core->Print("Bone %i (%s) is in zone %i (%s)\n",i,modelDecl->getBoneName(i),zone,damageZones->getZoneName(zone));
		}
	}
}
void ModelEntity::setKeyValue(const char *key, const char *value) {
	if(!_stricmp(key,"model") || !_stricmp(key,"rendermodel") || !_stricmp(key,"world_model") || !_stricmp(key,"model_world")) {
		str fixed;
		// this is a fix for FAKK and MoHAA - .tik model paths are set without "models/" directory
		if(g_vfs->FS_FileExists(value)==false) {
			fixed = "models/";
			fixed.append(value);
			if(g_vfs->FS_FileExists(fixed)) {
				value = fixed;
			}
		}
		this->setRenderModel(value);
		if(value[0] == '*') {
			this->setColModel(value);
		}
	} else if(!_stricmp(key,"model2")) {
		this->setRenderModel(value);
	} else if(!_stricmp(key,"cmodel") || !_stricmp(key,"collisionmodel")) {
		this->setColModel(value);
	} else if(!_stricmp(key,"size")) {
		if(cm) {
			sizes = vec3_c(value);
#if 0
			cmBBExts_i *cmBB = cm->registerBoxExts(sizes);
#else
			aabb bb;
			bb.maxs.x = sizes.x * 0.5f;
			bb.maxs.y = sizes.y * 0.5f;
			bb.maxs.z = sizes.z;
			bb.mins.x = -bb.maxs.x;
			bb.mins.y = -bb.maxs.y;
			bb.mins.z = 0;
			cMod_i *cmBB = cm->registerAABB(bb);
#endif
			this->setColModel(cmBB);
		} else {

		}
	} else if(!_stricmp(key,"ragdoll")) {
		// Doom3 "ragdoll" keyword (name of ArticulatedFigure)
		this->ragdollDefName = value;
	} else if(!_stricmp(key,"articulatedFigure")) {
		// I dont really know whats the difference between "ragdoll"
		// and "articulatedFigure" keywords, they are both used in Doom3
		// and seem to have the same meaning
		this->ragdollDefName = value;
	} else if(!_stricmp(key,"anim")) {
		this->setAnimation(value);
	} else if(!_stricmp(key,"takeDamage")) {
		this->bTakeDamage = atoi(value);
	} else if(!_strnicmp(key,"_articulatedFigureBody",strlen("_articulatedFigureBody"))) {
		// this is a saved pos/quat of single AF (ragdoll) body
		const char *p = key + strlen("_articulatedFigureBody");
		const char *indexStr = p;
		while(isdigit(*p)) {
			p++;
		}
		str tmp;
		tmp.setFromTo(indexStr,p);
		u32 bodyIndex = atoi(tmp);
		
		if(initialRagdolPose == 0) {
			initialRagdolPose = new boneOrQPArray_t;
		}
		if(initialRagdolPose->size() < bodyIndex + 1) {
			initialRagdolPose->resize(bodyIndex+1);
		}
		if(!_stricmp(p,"pos")) {
			vec3_c vec3Value(value);
			initialRagdolPose->setVec3(bodyIndex,vec3Value);
		} else if(!_stricmp(p,"quat")) {
			quat_c quatValue(value);
			initialRagdolPose->setQuat(bodyIndex,quatValue);
		}
	} else if(!_stricmp(key,"bUseRModelToCreateDynamicCVXShape")) {
		this->bUseRModelToCreateDynamicCVXShape = atoi(value);
		this->bUseDynamicConvexForTrimeshCMod = true;
	} else if(!_strnicmp(key,"damage_zone ",12)) {
		const char *zoneName = key + 12;
		setDamageZone(zoneName,value);
	} else if(!_stricmp(key,"scale") || !_stricmp(key,"modelscale")) {
		float f = atof(value);
		this->myEdict->s->scale.set(f,f,f);
	} else if(!_stricmp(key,"anim")) {
		this->setAnimation(value);
	} else {
		// fallback to parent class keyvalues
		BaseEntity::setKeyValue(key,value);
	}
}
void ModelEntity::iterateKeyValues(class keyValuesListener_i *listener) const {
	if(this->renderModelName.length()) {
		listener->addKeyValue("model",renderModelName.c_str());
	}
	if(this->cmod) {
		listener->addKeyValue("cmodel",cmod->getName());
	}
	if(this->ragdollDefName.length()) {
		listener->addKeyValue("articulatedFigure",ragdollDefName.c_str());
	}
	// if we have an active ragdoll, write the positions and quaternions
	// of it's physical bodies
	if(this->ragdoll) {
		const arraySTD_c<matrix_c> &mats = this->ragdoll->getCurWorldMatrices();
		for(u32 i = 0; i < mats.size(); i++) {
			vec3_c pos = mats[i].getOrigin();
			quat_c quat = mats[i].getQuat();
			str tmp;
			tmp = va("_articulatedFigureBody%iPos",i);
			listener->addKeyValue(tmp,pos);
			tmp = va("_articulatedFigureBody%iQuat",i);
			listener->addKeyValue(tmp,quat);
		}
	}

	// call the "iterateKeyValues" of base class
	BaseEntity::iterateKeyValues(listener);
}
void ModelEntity::setPhysicsObjectKinematic(bool newBKinematic) {
	if(this->bPhysicsBodyKinematic == newBKinematic) {
		return;
	}
	this->bPhysicsBodyKinematic = newBKinematic;
	if(body) {
		g_core->RedWarning("ModelEntity::setPhysicsObjectKinematic: TODO: update rigid body\n");
	}
}
void ModelEntity::setRigidBodyPhysicsEnabled(bool bRBPhysEnable) {
	if(this->bRigidBodyPhysicsEnabled == bRBPhysEnable) {
		return;
	}
	this->bRigidBodyPhysicsEnabled = bRBPhysEnable;
	//if(bRigidBodyPhysicsEnabled) {
	//	initRigidBodyPhysics();
	//} else {
	//	destroyPhysicsObject();
	//}
}
void ModelEntity::setPhysBounciness(float newBounciness) {
	physBounciness = newBounciness;
	// TODO: update rigid body
}
#include "../bt_include.h"
#include <math/matrix.h>
void ModelEntity::runPhysicsObject() {
	if(body) {
		matrix_c mat;
		body->getCurrentMatrix(mat);
		this->setMatrix(mat);

		if(0) {
			vec3_c tf = body->getTotalForce();
			g_core->Print("Entity %i total force: %f %f %f\n",getEntNum(),tf.x,tf.y,tf.z);
		}
	} else if(ragdoll) {
		ragdoll->updateWorldTransforms();
		// copy current bodies transforms to entityState
		const arraySTD_c<matrix_c> &mats = ragdoll->getCurWorldMatrices();
		entityState_s *out = myEdict->s;
		for(u32 i = 0; i < mats.size(); i++) {
			const matrix_c &m = mats[i];
			quat_c q = m.getQuat();
			vec3_c p = m.getOrigin();
			// ensure that quat_c::calcW() will recreate the same rotation
			if(q.w > 0) {
				q.negate();
			}
			netBoneOr_s &bor = out->boneOrs[i];
			bor.quatXYZ = q.floatPtr(); // quat_c layout is: XYZW so that's ok
			bor.xyz = p;
		}
	}
}
bool ModelEntity::initRagdollRenderAndPhysicsObject(const char *afName) {
	afDeclAPI_i *af = g_declMgr->registerAFDecl(afName);
	if(af == 0)
		return true; // error, articulatedFigure decl not found
	setRenderModel(af->getDefaultRenderModelName());
	setRagdollName(afName);
	initRagdollPhysics();
	if(ragdoll == 0) {
		return true; // something went wrong in g_ragdoll.cpp code
	}
	return false; // no error
}
void ModelEntity::initRigidBodyPhysics() {
	if(this->cmod == 0) {
		return;
	}
	if(this->bRigidBodyPhysicsEnabled == false) {
		return; // rigid body physics was disabled for this entity
	}
	if(body) {
		destroyPhysicsObject();
	}
	this->body = g_physWorld->createPhysicsObject(physObjectDef_s(this->getOrigin(),this->getAngles(),this->getScale(),
		this->cmod,this->mass,bUseDynamicConvexForTrimeshCMod,physBounciness));
	if(this->body == 0) {
		g_core->RedWarning("ModelEntity::initRigidBodyPhysics: BT_CreateRigidBodyWithCModel failed for cModel %s\n",this->cmod->getName());
		return;
	}
	this->body->setEntityPointer(this);
	if(bPhysicsBodyKinematic) {
		this->body->setKinematic();
//#if 1
//		// FIXME!!! For some resons my kinematic/static entities (func_door) don't collide with items/boxes (BUT they do collide with player controller)
//		//this->body->setCollisionFlags(this->body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
//		this->body->setCollisionFlags(this->body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
//		this->body->setActivationState(DISABLE_DEACTIVATION);
//#else
//		this->destroyPhysicsObject();
//		return;
//#endif
	}
}
void ModelEntity::initStaticBodyPhysics() {
	if(this->cmod == 0) {
		return;
	}
	// static physics bodies have NULL mass
	this->body = g_physWorld->createPhysicsObject(physObjectDef_s(this->getOrigin(),this->getAngles(),this->getScale(),
		this->cmod,0,false));
	this->body->setEntityPointer(this);
}
void ModelEntity::initRagdollPhysics() {
	destroyPhysicsObject();
	if(this->ragdollDefName.length() == 0)
		return;
	ragdoll = G_SpawnTestRagdollFromAF(ragdollDefName,this->getOrigin(),this->getAngles());
	if(ragdoll) {
		this->myEdict->s->activeRagdollDefNameIndex = G_RagdollDefIndex(ragdollDefName);
	} else {
		g_core->RedWarning("ModelEntity::initRagdollPhysics(): failed to spawn ragdoll %s.\n",ragdollDefName.c_str());
		return;
	}
	if(this->initialRagdolPose) {
		// set the ragdoll pose
		ragdoll->setPose(*this->initialRagdolPose);
		// we no longer need it
		delete this->initialRagdolPose;
		this->initialRagdolPose = 0;
	} else {
		boneOrArray_c currentBones;
		getCurrentBonesArray(currentBones);
		if(currentBones.size() && modelDecl) {
			// transform bones from entity space to world space
			currentBones.transform(this->getMatrix());
			ragdoll->setPoseFromRenderModelBonesArray(currentBones,modelDecl->getSkelAnimAPIForLocalIndex(0));
		}
	}
}
void ModelEntity::postSpawn() {
	if(this->ragdollDefName.length()) {
		// create a networked ragdoll
		this->initRagdollPhysics();
	} else if(cmod) {
		// create a rigid body (physics prop)
		initRigidBodyPhysics();
	} else if(bUseRModelToCreateDynamicCVXShape) {
		this->setColModel(this->renderModelName);
		initRigidBodyPhysics();
	}
	BaseEntity::postSpawn();
}
void ModelEntity::applyCentralForce(const vec3_c &forceToAdd) {
	if(this->body == 0)
		return;
	this->body->applyCentralForce(forceToAdd.floatPtr());
}
void ModelEntity::applyCentralImpulse(const vec3_c &impToAdd) {
	if(this->body == 0)
		return;
	this->body->applyCentralImpulse(impToAdd);
}
void ModelEntity::applyPointImpulse(const vec3_c &impToAdd, const vec3_c &pointAbs) {
	if(this->body == 0)
		return;
	vec3_c pointLocal;
	this->getMatrix().getInversed().transformPoint(pointAbs,pointLocal);
	this->body->applyPointImpulse(impToAdd,pointLocal);
}
void ModelEntity::applyTorque(const vec3_c &torqueToAdd) {
	if(this->body == 0)
		return;
	this->body->applyTorque(torqueToAdd);
}
const vec3_c ModelEntity::getLinearVelocity() const {
	if(this->body == 0) {
		return linearVelocity;
	}
	linearVelocity = body->getLinearVelocity();
	return this->linearVelocity;
}
void ModelEntity::setLinearVelocity(const vec3_c &newVel) {
	this->linearVelocity = newVel;
	if(this->body == 0)
		return;
	return body->setLinearVelocity(newVel);
}
const vec3_c ModelEntity::getAngularVelocity() const {
	if(this->body == 0)
		return vec3_c(0,0,0);
	return body->getAngularVelocity();
}
void ModelEntity::setAngularVelocity(const vec3_c &newAVel) {
	if(this->body == 0)
		return;
	return body->setAngularVelocity(newAVel);
}
void ModelEntity::runWaterPhysics(float curWaterLevel) {
	if(this->body == 0)
		return;
	//cmod->getVolume(); // TODO
	float deltaZ = curWaterLevel - this->getOrigin().z;
	float frac;
	if(deltaZ < 50) {
		frac = deltaZ / 50.f;
	} else {
		frac = 1.f;
	}
	float force = 350*frac;
	this->body->setLinearVelocity(this->body->getLinearVelocity()*0.9f);
	this->body->setAngularVelocity(this->body->getAngularVelocity()*0.9f);
	this->applyCentralForce(vec3_c(0,0,force));
}
void ModelEntity::destroyPhysicsObject() {
	if(body) {
		g_physWorld->destroyPhysicsObject(body);
		body = 0;
	}
	if(ragdoll) {
		delete ragdoll;
		ragdoll = 0;
		this->myEdict->s->activeRagdollDefNameIndex = 0;
	}
}
void ModelEntity::updateAnimations() {
	u32 prevLegsAnimationTime = legsAnimationTime;
	legsAnimationTime += level.frameTimeMs;
	if(tiki) {
		const tikiAnim_i *legsAnim = tiki->getAnim(this->myEdict->s->animIndex);
		if(legsAnim) {
			legsAnim->iterateCommands(TCS_SERVER,prevLegsAnimationTime,legsAnimationTime,0);
		}
	}
}
void ModelEntity::runFrame() {
	runPhysicsObject();
	updateAnimations();
}
void ModelEntity::getLocalBounds(aabb &out) const {
	if(renderModelName.length() == 0) {
		BaseEntity::getLocalBounds(out);
		return;
	}
	if(cmod) {
		cmod->getBounds(out);
	} else {
		out.fromRadius(64.f);
	}
	out.extend(pvsBoundsSkinWidth);
}
bool ModelEntity::getBoneWorldOrientation(u32 tagNum, class matrix_c &out) {
	if(modelDecl) {
		// TODO
	}
	out = this->getMatrix();
	return true; // error
}
#include <shared/trace.h>
bool ModelEntity::traceWorldRay(class trace_c &tr) {
	trace_c transformedTrace;
	tr.getTransformed(transformedTrace,this->getMatrix());
	if(this->traceLocalRay(transformedTrace)) {
		tr.updateResultsFromTransformedTrace(transformedTrace,this->getMatrix());
		return true;
	}
	return false;
}
bool ModelEntity::traceLocalRay(class trace_c &tr) {
	if(cmod) {
		if(cmod->traceRay(tr)) {
			tr.setHitEntity(this);
			return true;
		}
		return false;
	}
	//////tr.setFraction(tr.getFraction()-0.01f);
	//////tr.setHitEntity(this);
	//////return true;
	return false;
}
void ModelEntity::onDeath() {
	if(health > 0) {
		health = -1; // ensure that this entity is dead
	}
	if(ragdollDefName.length() && (ragdoll == 0)) {
		this->initRagdollPhysics();
	}
	if(ragdoll == 0) {
		delete this;
	}
}
void ModelEntity::damage(int damageCount) {
	if(bTakeDamage) {
		int prevHealth = health;
		health -= damageCount;
		g_core->Print("ModelEntity::damage: prev health %i, health now %i\n",prevHealth,health);
		if(prevHealth > 0 && health <= 0) {
			this->onDeath();
		}
	}
}
void ModelEntity::applyDamageFromDef(const char *defName, const class trace_c *tr) {
	entityDeclAPI_i *damageDef = g_declMgr->registerEntityDecl(defName);
	if(damageDef == 0) {
		g_core->RedWarning("ModelEntity::applyDamageFromDef: couldn't register damage def %s\n",defName);
		return;
	}
	const entDefAPI_i *e = damageDef->getEntDefAPI();
	int damageValue;
	if(e->getKeyValue("damage",damageValue)) {
		g_core->RedWarning("ModelEntity::applyDamageFromDef: no 'damage' key set in %s\n",defName);
		damageValue = 0;
	}
	if(tr) {
		this->onBulletHit(tr->getHitPos(),tr->getHitPlaneNormal(),damageValue);
	} else {
		this->damage(damageValue);
	}
}
void ModelEntity::onBulletHit(const vec3_c &hitPosWorld, const vec3_c &dirWorld, int damageCount) {
	// apply hit damage
	this->damage(damageCount);
	// apply hit force
	this->applyPointImpulse(dirWorld.getNormalized() * 250.f, hitPosWorld);
}