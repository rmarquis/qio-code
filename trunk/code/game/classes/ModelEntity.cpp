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


DEFINE_CLASS(ModelEntity, "BaseEntity");

// temporary (?) alias for .map loading (.obj models are using this)
DEFINE_CLASS_ALIAS(ModelEntity, func_static);

ModelEntity::ModelEntity() {
	body = 0;
	cmod = 0;
	cmSkel = 0;
	ragdoll = 0;
	health = 100;
	modelDecl = 0;
	bTakeDamage = false;
}
ModelEntity::~ModelEntity() {
	if(body) {
		destroyPhysicsObject();
	}
	if(ragdoll) {
		delete ragdoll;
	}
}
void ModelEntity::setOrigin(const vec3_c &newXYZ) {
	BaseEntity::setOrigin(newXYZ);
	if(body) {
#if 1
		btTransform transform;
		transform.setFromOpenGLMatrix(this->getMatrix());
		body->setWorldTransform(transform);
		//body->getMotionState()->setWorldTransform(transform);
		body->activate(true);
#else
		this->destroyPhysicsObject();
		this->initRigidBodyPhysics();
#endif
	}
}
void ModelEntity::setAngles(const class vec3_c &newAngles) {
	BaseEntity::setAngles(newAngles);
}
void ModelEntity::setRenderModel(const char *newRModelName) {
	// NOTE: render model pointer isnt stored in game module.
	// It's clientside only.
	this->myEdict->s->rModelIndex = G_RenderModelIndex(newRModelName);
	renderModelName = newRModelName;
	// decl models are present on both client and server
	this->modelDecl = g_declMgr->registerModelDecl(newRModelName);
	if(this->modelDecl) {
		// update animation indexes
		setAnimation(this->animName);
	}

	this->recalcABSBounds();
	this->link();
}
int ModelEntity::getBoneNumForName(const char *boneName) {
	if(cmSkel == 0) {
		cmSkel = cm->registerSkelModel(this->renderModelName);
	}
	if(cmSkel == 0)
		return -1;
	return cmSkel->getBoneNumForName(boneName);
}
void ModelEntity::setAnimation(const char *newAnimName) {
	if(this->modelDecl) {
		int animIndex = this->modelDecl->getAnimIndexForAnimAlias(newAnimName);
		this->myEdict->s->animIndex = animIndex;
	} else {
		this->myEdict->s->animIndex = G_AnimationIndex(newAnimName);
		// nothing else to do right now...
	}
	animName = newAnimName;
}
bool ModelEntity::setColModel(const char *newCModelName) {
	if(newCModelName[0] == '*' && BT_GetSubModelCModel(atoi(newCModelName+1))) {
		this->cmod = BT_GetSubModelCModel(atoi(newCModelName+1));
	} else {
		this->cmod = cm->registerModel(newCModelName);
	}
	if(this->cmod == 0)
		return true; // error
#if 1 
	if(this->cmod->hasCenterOfMassOffset()) {
		// PRIMITIVE fix for inline models loaded from .map files
		vec3_c p = this->getOrigin();
		p += this->cmod->getCenterOfMassOffset();
		this->setOrigin(p);
	}
#endif
	this->myEdict->s->colModelIndex = G_CollisionModelIndex(newCModelName);
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
void ModelEntity::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"model") || !stricmp(key,"rendermodel")) {
		this->setRenderModel(value);
		if(value[0] == '*') {
			this->setColModel(value);
		}
	} else if(!stricmp(key,"cmodel") || !stricmp(key,"collisionmodel")) {
		this->setColModel(value);
	} else if(!stricmp(key,"size")) {
		vec3_c sizes(value);
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
	} else if(!stricmp(key,"ragdoll")) {
		// Doom3 "ragdoll" keyword (name of ArticulatedFigure)
		this->ragdollDefName = value;
	} else if(!stricmp(key,"articulatedFigure")) {
		// I dont really know whats the difference between "ragdoll"
		// and "articulatedFigure" keywords, they are both used in Doom3
		// and seem to have the same meaning
		this->ragdollDefName = value;
	} else if(!stricmp(key,"anim")) {
		this->setAnimation(value);
	} else if(!stricmp(key,"takeDamage")) {
		this->bTakeDamage = atoi(value);
	} else {
		// fallback to parent class keyvalues
		BaseEntity::setKeyValue(key,value);
	}
}
#include "../bt_include.h"
#include <math/matrix.h>
void ModelEntity::runPhysicsObject() {
	if(body) {
		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);
	#if 0
		VectorSet(myEdict->s->origin,trans.getOrigin().x(),trans.getOrigin().y(),trans.getOrigin().z());
		//G_Printf("G_UpdatePhysicsObject: at %f %f %f\n",ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
		btQuaternion q = trans.getRotation();
		// quaterion->angles conversion doesnt work correctly here!
		// You can debug it with "btd_drawWireFrame 1" on local server
		quat_c q2(q.x(),q.y(),q.z(),q.w());
		q2.toAngles(myEdict->s->angles);
	#else
		matrix_c mat;
		trans.getOpenGLMatrix(mat);
		this->setMatrix(mat);
#endif
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
	this->body = BT_CreateRigidBodyWithCModel(this->getOrigin(),this->getAngles(),0,this->cmod);
	this->body->setUserPointer(this);
}
void ModelEntity::initStaticBodyPhysics() {
	if(this->cmod == 0) {
		return;
	}
	// static physics bodies have NULL mass
	this->body = BT_CreateRigidBodyWithCModel(this->getOrigin(),this->getAngles(),0,this->cmod,0);
	this->body->setUserPointer(this);
}
void ModelEntity::initRagdollPhysics() {
	destroyPhysicsObject();
	if(this->ragdollDefName.length() == 0)
		return;
	ragdoll = G_SpawnTestRagdollFromAF(ragdollDefName,this->getOrigin(),this->getAngles());
	if(ragdoll) {
		this->myEdict->s->activeRagdollDefNameIndex = G_RagdollDefIndex(ragdollDefName);
	}
}
void ModelEntity::destroyPhysicsObject() {
	if(body) {
		BT_RemoveRigidBody(body);
		body = 0;
	}
	if(ragdoll) {
		delete ragdoll;
		ragdoll = 0;
		this->myEdict->s->activeRagdollDefNameIndex = 0;
	}
}
void ModelEntity::runFrame() {
	runPhysicsObject();
}
void ModelEntity::getLocalBounds(aabb &out) const {
	if(renderModelName.length() == 0) {
		BaseEntity::getLocalBounds(out);
		return;
	}
	if(renderModelName[0] == '*') {
		out = G_GetInlineModelBounds(atoi(renderModelName.c_str()+1));
		out.extend(0.5f);
	} else {
		if(cmod) {
			cmod->getBounds(out);
		} else {
			out.fromRadius(64.f);
		}
	}
}
#include <shared/trace.h>
bool ModelEntity::traceWorldRay(class trace_c &tr) {
	trace_c transformedTrace;
	tr.getTransformed(transformedTrace,this->getMatrix());
	if(this->traceLocalRay(transformedTrace)) {
		tr.updateResultsFromTransformedTrace(transformedTrace);
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
void ModelEntity::onBulletHit(const vec3_c &dirWorld, int damage) {
	// apply hit damage
	if(bTakeDamage) {
		int prevHealth = health;
		health -= damage;
		g_core->Print("ModelEntity::onBulletHit: prev health %i, health now %i\n",prevHealth,health);
		if(prevHealth > 0 && health <= 0) {
			this->onDeath();
		}
	}
}