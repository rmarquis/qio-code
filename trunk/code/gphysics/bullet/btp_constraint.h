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
#ifndef __BTP_CONSTRAINT_H__
#define __BTP_CONSTRAINT_H__

#include <api/physConstraintAPI.h>

class btpConstraintBase_c : public physConstraintAPI_i {
protected:
	class bulletPhysicsWorld_c *world;
	class bulletRigidBody_c *bodies[2];

	virtual class physObjectAPI_i *getBody0() const;
	virtual class physObjectAPI_i *getBody1() const;

	void calcLocalOffsets(const class vec3_c &pos, class bulletRigidBody_c *b0, class bulletRigidBody_c *b1, class btTransform &frameA, class btTransform &frameB);
	void removeConstraintReferences();
public:
	btpConstraintBase_c();
	virtual ~btpConstraintBase_c();

	virtual void destroyConstraint() { };
};

class btpConstraintBall_c : public btpConstraintBase_c {
	class btGeneric6DofConstraint *bulletConstraint;
public:
	void init(const class vec3_c &pos, class bulletRigidBody_c *b0, class bulletRigidBody_c *b1, class bulletPhysicsWorld_c *worldPointer);
	virtual void destroyConstraint();
};

class btpConstraintHinge_c : public btpConstraintBase_c {
	class btHingeConstraint *bulletConstraintHinge;
public:
	void init(const class vec3_c &pos, const class vec3_c &axis, class bulletRigidBody_c *b0, class bulletRigidBody_c *b1, class bulletPhysicsWorld_c *worldPointer);
	virtual void destroyConstraint();
};

#endif // __BTP_CONSTRAINT_H__
