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
// BaseEntity.h - base class for all entities

#include "../g_local.h"
#include "BaseEntity.h"
#include <api/ddAPI.h>
#include <api/cmAPI.h>
#include <math/vec3.h>
#include <math/quat.h>

// use this to force BaseEntity::ctor() to use a specific edict instead of allocating a new one
static edict_s *be_forcedEdict = 0;
void BE_SetForcedEdict(edict_s *nfe) {
	be_forcedEdict = nfe;
}

BaseEntity::BaseEntity() {
	body = 0;
	cmod = 0;
	if(be_forcedEdict) {
		myEdict = be_forcedEdict;
		be_forcedEdict = 0;
	} else {
		myEdict = G_Spawn();
	}
	myEdict->ent = this;
}
BaseEntity::~BaseEntity() {
	G_FreeEntity(myEdict);
}

void BaseEntity::setOrigin(const vec3_c &newXYZ) {
	myEdict->s.origin = newXYZ;
}
void BaseEntity::setAngles(const class vec3_c &newAngles) {
	myEdict->s.angles = newAngles;
}
const vec3_c &BaseEntity::getOrigin() const {
	return myEdict->s.origin;
}
const vec3_c &BaseEntity::getAngles() const {
	return myEdict->s.angles;
}
void BaseEntity::debugDrawCMObject(class rDebugDrawer_i *dd) {
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
#include "../bt_include.h"
void BaseEntity::runPhysicsObject() {
	if(body == 0)
		return;
	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	VectorSet(myEdict->s.origin,trans.getOrigin().x(),trans.getOrigin().y(),trans.getOrigin().z());
	//G_Printf("G_UpdatePhysicsObject: at %f %f %f\n",ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
	btQuaternion q = trans.getRotation();
	quat_c q2(q.x(),q.y(),q.z(),q.w());
	q2.toAngles(myEdict->s.angles);
}

void BaseEntity::createBoxPhysicsObject(const float *pos, const float *halfSizes, const float *startVel) {
	this->body = BT_CreateBoxBody(pos,halfSizes,startVel);
	this->cmod = cm->registerBoxExts(halfSizes);
	this->body->setUserPointer(this);
}
