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
// btp_world.h
#ifndef __ODE_WORLD_H__
#define __ODE_WORLD_H__

#include <shared/hashTableTemplate.h>
#include <shared/str.h>
#include <api/physAPI.h>
#include "ode_staticMapLoader.h"
#include "ode_headers.h"

class odePhysicsWorld_c : public physWorldAPI_i {
	str debugName;
	vec3_c gravity;
	// static world collision
	odeStaticMapLoader_c staticWorld;
	// Bullet shapes cache (single shape can be used by several rigid bodies)
	hashTableTemplateExt_c<class odeColShape_c> shapes;
	// single rigid bodies (NOTE: they can be connected by constraints)
	arraySTD_c<class odeRigidBody_c*> bodies;
	// character controllers
	arraySTD_c<class odeCharacterController_c*> characters;
	// constraints
	arraySTD_c<class odeConstraintBase_c*> constraints;
	// ragdolls
	//arraySTD_c<class bulletRagdoll_c*> ragdolls;
	// vehicles 
	//arraySTD_c<class bulletVehicle_c*> vehicles;
	// BulletPhysics variables
	dWorldID world;
	dSpaceID space;
	dJointGroupID contactGroup;

	odeColShape_c *registerShape(const class cMod_i *cmodel, bool isStatic);

public:
	odePhysicsWorld_c(const char *newDebugName);
	~odePhysicsWorld_c();

	virtual void init(const vec3_c &newGravity);
	virtual bool loadMap(const char *mapName);
	virtual void runFrame(float frameTime);
	virtual void shutdown();

	// mass 0 means that object is static (non moveable)
	virtual physObjectAPI_i *createPhysicsObject(const struct physObjectDef_s &def);
	virtual void destroyPhysicsObject(class physObjectAPI_i *p);

	virtual class physConstraintAPI_i *createConstraintBall(const vec3_c &pos, physObjectAPI_i *b0, physObjectAPI_i *b1);
	virtual class physConstraintAPI_i *createConstraintHinge(const vec3_c &pos, const vec3_c &axis, physObjectAPI_i *b0, physObjectAPI_i *b1);
	virtual void destroyPhysicsConstraint(physConstraintAPI_i *p);

	virtual class physCharacterControllerAPI_i *createCharacter(const class vec3_c &pos, float characterHeight,  float characterWidth);
	virtual void freeCharacter(class physCharacterControllerAPI_i *p);

	virtual class physVehicleAPI_i *createVehicle(const vec3_c &pos, const vec3_c &angles, class cMod_i *cm) {
		return 0;
	}
	virtual void removeVehicle(class physVehicleAPI_i *v) {

	}

	virtual void setGravity(const vec3_c &newGravity);
	virtual const vec3_c &getGravity() const;
	virtual bool traceRay(class trace_c &tr);

	dWorldID getODEWorld() {
		return world;
	}
	dSpaceID getODESpace() {
		return space;
	}
};

#endif // __ODE_WORLD_H__
