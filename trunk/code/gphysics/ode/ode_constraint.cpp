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
#include "ode_constraint.h"
#include "ode_world.h"
#include "ode_rigidBody.h"
#include "ode_convert.h"
#include <math/matrix.h>

odeConstraintBase_c::odeConstraintBase_c() {
	world = 0;
	bodies[0] = bodies[1] = 0;
}
odeConstraintBase_c::~odeConstraintBase_c() {
	destroyConstraint();
}
void odeConstraintBase_c::removeConstraintReferences() {
	if(world) {
		//world->removeConstraintRef(this);
	}
	if(bodies[0]) {
	//	bodies[0]->removeConstraintRef(this);
	}
	if(bodies[1]) {
	//	bodies[1]->removeConstraintRef(this);
	}
}
void odeConstraintBase_c::calcLocalOffsets(const vec3_c &pos, class odeRigidBody_c *b0, class odeRigidBody_c *b1, matrix_c &frameA, matrix_c &frameB) {
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
class physObjectAPI_i *odeConstraintBase_c::getBody0() const {
	return bodies[0];
}
class physObjectAPI_i *odeConstraintBase_c::getBody1() const {
	return bodies[1];
}
void odeConstraintBall_c::init(const vec3_c &pos, class odeRigidBody_c *b0, class odeRigidBody_c *b1, class odePhysicsWorld_c *worldPointer) {
	this->world = worldPointer;
	this->bodies[0] = b0;
	this->bodies[1] = b1;

	//matrix_c frameA;
	//matrix_c frameB;

	//calcLocalOffsets(pos,b0,b1,frameA,frameB);

	odeJoint = dJointCreateBall(worldPointer->getODEWorld(), 0);

	if(b1) {
		dJointAttach(odeJoint,b0->getODEBodyID(),b1->getODEBodyID());
	} else {
		dJointAttach(odeJoint,b0->getODEBodyID(),0);
	}
	vec3_c posScaled = pos * QIO_TO_BULLET;
	dJointSetBallAnchor(odeJoint,posScaled.x,posScaled.y,posScaled.z);
}
void odeConstraintBall_c::destroyConstraint() {
	if(odeJoint == 0) {
		return;
	}
	dJointDestroy (odeJoint);
	this->removeConstraintReferences();
}

void odeConstraintHinge_c::init(const class vec3_c &pos, const class vec3_c &axis, class odeRigidBody_c *b0, class odeRigidBody_c *b1, class odePhysicsWorld_c *worldPointer) {
	this->world = worldPointer;
	this->bodies[0] = b0;
	this->bodies[1] = b1;

	//matrix_c frameA;
	//matrix_c frameB;

	//calcLocalOffsets(pos,b0,b1,frameA,frameB);
	odeJoint = dJointCreateHinge(worldPointer->getODEWorld(), 0);

	if(b1) {
		dJointAttach(odeJoint,b0->getODEBodyID(),b1->getODEBodyID());
	} else {
		dJointAttach(odeJoint,b0->getODEBodyID(),0);
	}
	vec3_c posScaled = pos * QIO_TO_BULLET;
	dJointSetHingeAnchor(odeJoint,posScaled.x,posScaled.y,posScaled.z);
	dJointSetHingeAxis(odeJoint,axis.x,axis.y,axis.z);
}
void odeConstraintHinge_c::destroyConstraint() {
	if(odeJoint == 0) {
		return;
	}
	dJointDestroy (odeJoint);
	this->removeConstraintReferences();
}

