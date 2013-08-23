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
// btp_constraint.cpp
#include "btp_constraint.h"
#include "btp_world.h"
#include "btp_rigidBody.h"
#include "btp_convert.h"
#include <math/matrix.h>

btpConstraintBase_c::btpConstraintBase_c() {
	world = 0;
	bodies[0] = bodies[1] = 0;
}
btpConstraintBase_c::~btpConstraintBase_c() {
	destroyConstraint();
}
void btpConstraintBase_c::removeConstraintReferences() {
	if(world) {
		//world->removeConstraintRef(this);
	}
	if(bodies[0]) {
		bodies[0]->removeConstraintRef(this);
	}
	if(bodies[1]) {
		bodies[1]->removeConstraintRef(this);
	}
}
void btpConstraintBase_c::calcLocalOffsets(const vec3_c &pos, class bulletRigidBody_c *b0, class bulletRigidBody_c *b1, btTransform &frameA, btTransform &frameB) {
	matrix_c cMat;
	cMat.setupOrigin(pos.x,pos.y,pos.z);
	matrix_c e0Mat;
	b0->getPhysicsMatrix(e0Mat);
	matrix_c b0Mat = e0Mat.getInversed()*cMat;
	frameA.setFromOpenGLMatrix(b0Mat);
	frameA.scaleOrigin(QIO_TO_BULLET);

	if(b1) {
		matrix_c e1Mat;
		b1->getPhysicsMatrix(e1Mat);
		matrix_c b1Mat = e1Mat.getInversed()*cMat;
		frameB.setFromOpenGLMatrix(b1Mat);
		frameB.scaleOrigin(QIO_TO_BULLET);
	}
}
class physObjectAPI_i *btpConstraintBase_c::getBody0() const {
	return bodies[0];
}
class physObjectAPI_i *btpConstraintBase_c::getBody1() const {
	return bodies[1];
}
void btpConstraintBall_c::init(const vec3_c &pos, class bulletRigidBody_c *b0, class bulletRigidBody_c *b1, class bulletPhysicsWorld_c *worldPointer) {
	this->world = worldPointer;
	this->bodies[0] = b0;
	this->bodies[1] = b1;

	btTransform frameA;
	btTransform frameB;

	calcLocalOffsets(pos,b0,b1,frameA,frameB);

	if(b1) {
		this->bulletConstraint = new btGeneric6DofConstraint(*b0->getBulletRigidBody(),*b1->getBulletRigidBody(),frameA,frameB,false);
	} else {
		this->bulletConstraint = new btGeneric6DofConstraint(*b0->getBulletRigidBody(),frameA,false);
	}
	// lock linear transforms
	this->bulletConstraint->setLimit(0,0,0);
	this->bulletConstraint->setLimit(1,0,0);
	this->bulletConstraint->setLimit(2,0,0);
	// free the angular axes
	this->bulletConstraint->setLimit(3,-1,0);
	this->bulletConstraint->setLimit(4,-1,0);
	this->bulletConstraint->setLimit(5,-1,0);
	worldPointer->getBTDynamicsWorld()->addConstraint(this->bulletConstraint);

	// add constraint refs to bulletRigidBody_c's
	b0->addConstraintRef(this);
	if(b1)
		b1->addConstraintRef(this);
}
void btpConstraintBall_c::destroyConstraint() {
	if(bulletConstraint == 0) {
		return;
	}
	this->world->getBTDynamicsWorld()->removeConstraint(this->bulletConstraint);
	delete this->bulletConstraint;
	this->bulletConstraint = 0;
	this->removeConstraintReferences();
}

void btpConstraintHinge_c::init(const class vec3_c &pos, const class vec3_c &axis, class bulletRigidBody_c *b0, class bulletRigidBody_c *b1, class bulletPhysicsWorld_c *worldPointer) {
	this->world = worldPointer;
	this->bodies[0] = b0;
	this->bodies[1] = b1;

	btTransform frameA;
	btTransform frameB;

	calcLocalOffsets(pos,b0,b1,frameA,frameB);

	if(b1) {
		this->bulletConstraintHinge = new btHingeConstraint(*b0->getBulletRigidBody(),*b1->getBulletRigidBody(),frameA,frameB,false);
	} else {
		this->bulletConstraintHinge = new btHingeConstraint(*b0->getBulletRigidBody(),frameA,false);
	}
	worldPointer->getBTDynamicsWorld()->addConstraint(this->bulletConstraintHinge);

	// add constraint refs to bulletRigidBody_c's
	b0->addConstraintRef(this);
	if(b1)
		b1->addConstraintRef(this);
}
void btpConstraintHinge_c::destroyConstraint() {
	if(bulletConstraintHinge == 0) {
		return;
	}
	this->world->getBTDynamicsWorld()->removeConstraint(this->bulletConstraintHinge);
	delete this->bulletConstraintHinge;
	this->bulletConstraintHinge = 0;
	this->removeConstraintReferences();
}

