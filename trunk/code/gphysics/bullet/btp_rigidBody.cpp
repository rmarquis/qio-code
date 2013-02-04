/*
============================================================================
Copyright (C) 2013 V.

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
// btp_rigidBody.cpp
#include "btp_rigidBody.h"
#include "btp_shape.h"
#include "btp_headers.h"
#include <shared/physObjectDef.h>

bulletRigidBody_c::bulletRigidBody_c() {

}
bulletRigidBody_c::~bulletRigidBody_c() {

}
void bulletRigidBody_c::init(class bulletColShape_c *newShape, const struct physObjectDef_s &def) {
	shape = newShape;
	btCollisionShape *btShape = shape->getBulletCollisionShape();
	bool isStatic = def.isStatic();
	btVector3 localInertia(0,0,0);
	if(isStatic == false) {
		btShape->calculateLocalInertia(def.mass,localInertia);
	}
	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(def.transform);
	btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform,shape->getCenterOfMassTransform());
	btRigidBody::btRigidBodyConstructionInfo cInfo(def.mass,myMotionState,btShape,localInertia);
	bulletRigidBody = new btRigidBody(cInfo);
}

void bulletRigidBody_c::getCurrentMatrix(const class matrix_c &out) const {

}
void bulletRigidBody_c::applyCentralForce(const class vec3_c &velToAdd) {
}
void bulletRigidBody_c::applyCentralImpulse(const class vec3_c &impToAdd) {

}
// linear velocity access (in Quake units)
const class vec3_c bulletRigidBody_c::getLinearVelocity() const {
	return vec3_c(0,0,0);
}
void bulletRigidBody_c::setLinearVelocity(const class vec3_c &newVel) {

}
// angular velocity access
const vec3_c bulletRigidBody_c::getAngularVelocity() const {
	return vec3_c(0,0,0);
}
void bulletRigidBody_c::setAngularVelocity(const class vec3_c &newAVel) {
}
// water physics
void bulletRigidBody_c::runWaterPhysics(float curWaterLevel) {

}
