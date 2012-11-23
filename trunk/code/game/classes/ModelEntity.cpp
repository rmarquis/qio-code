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

DEFINE_CLASS(ModelEntity, "BaseEntity");

// temporary (?) alias for .map loading (.obj models are using this)
DEFINE_CLASS_ALIAS(ModelEntity, func_static);

ModelEntity::ModelEntity() {
	body = 0;
	cmod = 0;
	cmSkel = 0;
}
ModelEntity::~ModelEntity() {
	if(body) {
		destroyPhysicsObject();
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
	this->myEdict->s->animIndex = G_AnimationIndex(newAnimName);
	// nothing else to do right now...
}
bool ModelEntity::setColModel(const char *newCModelName) {
	this->cmod = cm->registerModel(newCModelName);
	if(this->cmod == 0)
		return true; // error
	this->myEdict->s->colModelIndex = G_CollisionModelIndex(newCModelName);
	return false;
}
bool ModelEntity::setColModel(class cMod_i *newCModel) {
	return setColModel(newCModel->getName());
}
void ModelEntity::debugDrawCMObject(class rDebugDrawer_i *dd) {
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
		cmBBExts_i *cmBB = cm->registerBoxExts(sizes);
		this->setColModel(cmBB);
	} else {
		// fallback to parent class keyvalues
		BaseEntity::setKeyValue(key,value);
	}
}
#include "../bt_include.h"
#include <math/matrix.h>
void ModelEntity::runPhysicsObject() {
	if(body == 0)
		return;
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
void ModelEntity::destroyPhysicsObject() {
	if(body == 0)
		return;
	BT_RemoveRigidBody(body);
	body = 0;
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
