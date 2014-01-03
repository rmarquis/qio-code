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
#include "btp_convert.h"
#include "btp_world.h"
#include "btp_constraint.h"
#include <shared/physObjectDef.h>

bulletRigidBody_c::bulletRigidBody_c() {
	myWorld = 0;
	shape = 0;
	bulletRigidBody = 0;
	myEntity = 0;
}
bulletRigidBody_c::~bulletRigidBody_c() {
	if(bulletRigidBody == 0)
		return;
	arraySTD_c<btpConstraintBase_c*> copy = constraints;
	for(u32 i = 0; i < copy.size(); i++) {
		copy[i]->destroyConstraint();
	}
	myWorld->getBTDynamicsWorld()->removeRigidBody(bulletRigidBody);
	delete bulletRigidBody;
	bulletRigidBody = 0;
}
void bulletRigidBody_c::init(class bulletColShape_c *newShape, const struct physObjectDef_s &def, class bulletPhysicsWorld_c *pWorld) {
	myWorld = pWorld;
	shape = newShape;
	btCollisionShape *btShape = shape->getBulletCollisionShape();
	bool isStatic = def.isStatic();
	btVector3 localInertia(0,0,0);
	if(isStatic == false) {
		btShape->calculateLocalInertia(def.mass,localInertia);
	}
	btTransform startTransform;
	const matrix_c &com = shape->getCenterOfMassTransform();
	matrix_c bulletMat = def.transform;
	bulletMat.scaleOriginXYZ(QIO_TO_BULLET);	
	bulletMat = com * bulletMat;
	startTransform.setFromOpenGLMatrix(bulletMat);
	btRigidBody::btRigidBodyConstructionInfo cInfo(def.mass,0,btShape,localInertia);
	bulletRigidBody = new btRigidBody(cInfo);
	bulletRigidBody->setDamping(0.05, 0.85);
	bulletRigidBody->setDeactivationTime(0.8);
	bulletRigidBody->setSleepingThresholds(1.6, 2.5);
	//btShape->setMargin(0.f);
	bulletRigidBody->setWorldTransform(startTransform);
	bulletRigidBody->setRestitution(def.bounciness);
	bulletRigidBody->setUserPointer(myEntity);
}
void bulletRigidBody_c::setOrigin(const class vec3_c &newPos) {
	btTransform trans;
	trans = bulletRigidBody->getWorldTransform();
	btVector3 posScaled(newPos.x*QIO_TO_BULLET,newPos.y*QIO_TO_BULLET,newPos.z*QIO_TO_BULLET);
	trans.setOrigin(posScaled);
	bulletRigidBody->setWorldTransform(trans);
}
const class vec3_c bulletRigidBody_c::getRealOrigin() const {
	btTransform trans;
	trans = bulletRigidBody->getWorldTransform();
	class matrix_c mat;
	trans.getOpenGLMatrix(mat);
	mat.scaleOriginXYZ(BULLET_TO_QIO);
	return mat.getOrigin();
}
void bulletRigidBody_c::setMatrix(const class matrix_c &newMatrix) {
	matrix_c m = newMatrix;
	m.scaleOrigin(QIO_TO_BULLET);
	if(this->shape->hasCenterOfMassTransform()) {
		m = m * this->shape->getCenterOfMassTransform();
	}
	btTransform trans;
	trans.setFromOpenGLMatrix(m);
	bulletRigidBody->setWorldTransform(trans);
}
void bulletRigidBody_c::getCurrentMatrix(class matrix_c &out) const {
	btTransform trans;
	trans = bulletRigidBody->getWorldTransform();
	trans.getOpenGLMatrix(out);
	if(this->shape->hasCenterOfMassTransform()) {
		out = out * this->shape->getCenterOfMassTransform().getInversed();
	}
	out.scaleOriginXYZ(BULLET_TO_QIO);
}
void bulletRigidBody_c::getPhysicsMatrix(class matrix_c &out) const {
	btTransform trans;
	trans = bulletRigidBody->getWorldTransform();
	trans.getOpenGLMatrix(out);
	// dont add center of mass transform
	out.scaleOriginXYZ(BULLET_TO_QIO);
}
void bulletRigidBody_c::applyCentralForce(const class vec3_c &velToAdd) {
	bulletRigidBody->activate(true);
	bulletRigidBody->applyCentralForce((velToAdd*QIO_TO_BULLET).floatPtr());
}
void bulletRigidBody_c::applyCentralImpulse(const class vec3_c &impToAdd) {
	bulletRigidBody->activate(true);
	bulletRigidBody->applyCentralImpulse((impToAdd*QIO_TO_BULLET).floatPtr());
}
void bulletRigidBody_c::applyTorque(const class vec3_c &torqueToAdd) {
	bulletRigidBody->activate(true);
	bulletRigidBody->applyTorque((torqueToAdd).floatPtr());
}
void bulletRigidBody_c::applyPointImpulse(const class vec3_c &val, const class vec3_c &point) {
	bulletRigidBody->activate(true);
	bulletRigidBody->applyImpulse((val*QIO_TO_BULLET).floatPtr(),(point*QIO_TO_BULLET).floatPtr());
}
// linear velocity access (in Quake units)
const class vec3_c bulletRigidBody_c::getLinearVelocity() const {
	return vec3_c(bulletRigidBody->getLinearVelocity())*BULLET_TO_QIO;
}
void bulletRigidBody_c::setLinearVelocity(const class vec3_c &newVel) {
	bulletRigidBody->setLinearVelocity((newVel*QIO_TO_BULLET).floatPtr());
}
// angular velocity access
const vec3_c bulletRigidBody_c::getAngularVelocity() const {
	return bulletRigidBody->getAngularVelocity();
}
void bulletRigidBody_c::setAngularVelocity(const class vec3_c &newAVel) {
	bulletRigidBody->setAngularVelocity(newAVel.floatPtr());
}
void bulletRigidBody_c::setKinematic() {
	myWorld->getBTDynamicsWorld()->removeRigidBody(bulletRigidBody);
	bulletRigidBody->setCollisionFlags(bulletRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	bulletRigidBody->setActivationState(DISABLE_DEACTIVATION);
	bulletRigidBody->activate();
	myWorld->getBTDynamicsWorld()->addRigidBody(bulletRigidBody);
}
bool bulletRigidBody_c::isDynamic() const {
	if(bulletRigidBody->isStaticObject())
		return false;
	return true;
}
void bulletRigidBody_c::setEntityPointer(class BaseEntity *ent) {
	myEntity = ent;
	if(bulletRigidBody) {
		bulletRigidBody->setUserPointer(myEntity);
	}
}
BaseEntity *bulletRigidBody_c::getEntityPointer() const {
	return myEntity;
}
// water physics
void bulletRigidBody_c::runWaterPhysics(float curWaterLevel) {

}
