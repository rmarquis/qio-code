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
// btp_rigidBody.h
#ifndef __ODE_RIGIDBODY_H__
#define __ODE_RIGIDBODY_H__

#include <api/physAPI.h> 
#include <api/physObjectAPI.h> 
#include "ode_headers.h"

class odeRigidBody_c : public physObjectAPI_i {
	class odeColShape_c *shape;
	dBodyID body;
	dGeomID geom;
	class BaseEntity *myEntity;
	//arraySTD_c<odeConstraintBase_c*> constraints;

	float bounciness;
public:
	odeRigidBody_c();
	~odeRigidBody_c();

	void init(class odePhysicsWorld_c *world, class odeColShape_c *newShape, const struct physObjectDef_s &def); 

	virtual void setOrigin(const class vec3_c &newPos);
	virtual const class vec3_c getRealOrigin() const;
	virtual void getCurrentMatrix(class matrix_c &out) const;
	virtual void getPhysicsMatrix(class matrix_c &out) const;
	virtual void applyCentralForce(const class vec3_c &velToAdd);
	virtual void applyCentralImpulse(const class vec3_c &impToAdd);
	// linear velocity access (in Quake units)
	virtual const class vec3_c getLinearVelocity() const;
	virtual void setLinearVelocity(const class vec3_c &newVel);
	// angular velocity access
	virtual const vec3_c getAngularVelocity() const;
	virtual void setAngularVelocity(const class vec3_c &newAVel);
	// misc
	virtual void setKinematic();
	virtual bool isDynamic() const;
	virtual void setEntityPointer(class BaseEntity *ent);
	virtual BaseEntity *getEntityPointer() const;
	// water physics
	virtual void runWaterPhysics(float curWaterLevel);

	dBodyID getODEBodyID() {
		return body;
	}
	inline float getBounciness() const {
		return bounciness;
	}
};

#endif // __ODE_RIGIDBODY_H__
