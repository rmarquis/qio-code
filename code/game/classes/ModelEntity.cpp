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

ModelEntity::ModelEntity() {
	body = 0;
	cmod = 0;
}
ModelEntity::~ModelEntity() {
	if(body) {
		BT_RemoveRigidBody(body);
	}
}
void ModelEntity::setOrigin(const vec3_c &newXYZ) {
	BaseEntity::setOrigin(newXYZ);
}
void ModelEntity::setAngles(const class vec3_c &newAngles) {
	BaseEntity::setAngles(newAngles);
}
void ModelEntity::setRenderModel(const char *newRModelName) {
	// NOTE: render model pointer isnt stored in game module.
	// It's clientside only.
	this->myEdict->s->rModelIndex = G_RenderModelIndex(newRModelName);
}
void ModelEntity::setColModel(const char *newCModelName) {
	this->cmod = cm->registerModel(newCModelName);
	this->myEdict->s->colModelIndex = G_CollisionModelIndex(newCModelName);
}
void ModelEntity::setColModel(class cMod_i *newCModel) {
	setColModel(newCModel->getName());
}
void ModelEntity::debugDrawCMObject(class rDebugDrawer_i *dd) {
	if(cmod == 0)
		return;
	if(cmod->isCapsule()) {
		cmCapsule_i *c = cmod->getCapsule();
		dd->drawCapsuleZ(getOrigin(), c->getHeight(), c->getRadius());
	} else if(cmod->isBBExts()) {
		cmBBExts_i *bb = cmod->getBBExts();
		dd->drawBBExts(getOrigin(),getAngles(),bb->getHalfSizes());
	}
}	
void ModelEntity::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"model")) {
		this->setRenderModel(value);
	} else {
		// fallback to parent class keyvalues
		BaseEntity::setKeyValue(key,value);
	}
}
#include "../bt_include.h"
void ModelEntity::runPhysicsObject() {
	if(body == 0)
		return;
	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	VectorSet(myEdict->s->origin,trans.getOrigin().x(),trans.getOrigin().y(),trans.getOrigin().z());
	//G_Printf("G_UpdatePhysicsObject: at %f %f %f\n",ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
	btQuaternion q = trans.getRotation();
	quat_c q2(q.x(),q.y(),q.z(),q.w());
	q2.toAngles(myEdict->s->angles);
}

void ModelEntity::createBoxPhysicsObject(const float *pos, const float *halfSizes, const float *startVel) {
	this->body = BT_CreateBoxBody(pos,halfSizes,startVel);
	this->setColModel(cm->registerBoxExts(halfSizes));
	this->body->setUserPointer(this);
}
void ModelEntity::runFrame() {
	runPhysicsObject();
}