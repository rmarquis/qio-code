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
// btp_constraint.h
#ifndef __ODE_CONSTRAINT_H__
#define __ODE_CONSTRAINT_H__

#include <api/physConstraintAPI.h>

#include "ode_headers.h"

class odeConstraintBase_c : public physConstraintAPI_i {
protected:
	class odePhysicsWorld_c *world;
	class odeRigidBody_c *bodies[2];

	virtual class physObjectAPI_i *getBody0() const;
	virtual class physObjectAPI_i *getBody1() const;

	void calcLocalOffsets(const class vec3_c &pos, class odeRigidBody_c *b0, class odeRigidBody_c *b1, class matrix_c &frameA, class matrix_c &frameB);
	void removeConstraintReferences();
public:
	odeConstraintBase_c();
	virtual ~odeConstraintBase_c();

	virtual void destroyConstraint() { };
};

class odeConstraintBall_c : public odeConstraintBase_c {
	dJointID odeJoint;
public:
	void init(const class vec3_c &pos, class odeRigidBody_c *b0, class odeRigidBody_c *b1, class odePhysicsWorld_c *worldPointer);
	virtual void destroyConstraint();
};

class odeConstraintHinge_c : public odeConstraintBase_c {
	dJointID odeJoint;
public:
	void init(const class vec3_c &pos, const class vec3_c &axis, class odeRigidBody_c *b0, class odeRigidBody_c *b1, class odePhysicsWorld_c *worldPointer);
	virtual void destroyConstraint();
};

#endif // __ODE_CONSTRAINT_H__
