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
#ifndef __BTP_WORLD_H__
#define __BTP_WORLD_H__

#include <shared/hashTableTemplate.h>
#include <shared/str.h>
#include <api/physAPI.h>



class bulletPhysicsWorld_c : public physWorldAPI_i {
	str debugName;
	vec3_c gravity;
	// Bullet shapes cache (single shape can be used by several rigid bodies)
	hashTableTemplateExt_c<class bulletColShape_c> shapes;
	// single rigid bodies (NOTE: they can be connected by constraints)
	arraySTD_c<class bulletRigidBody_c*> bodies;
	// ragdolls
	arraySTD_c<class bulletRagdoll_c*> ragdolls;
	// vehicles 
	arraySTD_c<class bulletVehicle_c*> vehicles;
	// BulletPhysics variables
	class btBroadphaseInterface *broadphase;
	class btDefaultCollisionConfiguration *collisionConfiguration;
	class btCollisionDispatcher *dispatcher;
	class btSequentialImpulseConstraintSolver *solver;
	class btDiscreteDynamicsWorld *dynamicsWorld;

	bulletColShape_c *registerShape(const class cMod_i *cmodel, bool isStatic);
public:
	bulletPhysicsWorld_c(const char *newDebugName);

	virtual void init(const vec3_c &newGravity);
	virtual void runFrame(float frameTime);
	virtual void shutdown();

	// mass 0 means that object is static (non moveable)
	virtual physObjectAPI_i *createPhysicsObject(const struct physObjectDef_s &def);
	virtual void setGravity(const vec3_c &newGravity);
	virtual const vec3_c &getGravity() const;
};

#endif // __BTP_WORLD_H__
