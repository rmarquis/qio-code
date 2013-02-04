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
// btp_world.cpp
#include "btp_world.h"
#include "btp_shape.h"
#include "btp_headers.h"
#include "btp_rigidBody.h"
#include <shared/physObjectDef.h>
#include <api/cmAPI.h>

#ifdef DEBUG
#pragma comment( lib, "BulletCollision_debug.lib" )
#pragma comment( lib, "BulletDynamics_debug.lib" )
#pragma comment( lib, "LinearMath_debug.lib" )
#else
#pragma comment( lib, "BulletCollision.lib" )
#pragma comment( lib, "BulletDynamics.lib" )
#pragma comment( lib, "LinearMath.lib" )
#endif

bulletPhysicsWorld_c::bulletPhysicsWorld_c(const char *newDebugName) {
	this->debugName = newDebugName;
	broadphase = 0;
	collisionConfiguration = 0;
	dispatcher = 0;
	solver = 0;
	dynamicsWorld = 0;
}

void bulletPhysicsWorld_c::init(const vec3_c &newGravity) {
	this->gravity = newGravity;
	// Build the broadphase
	broadphase = new btDbvtBroadphase();

	// Set up the collision configuration and dispatcher
	collisionConfiguration = new btDefaultCollisionConfiguration();

	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	// The actual physics solver
	solver = new btSequentialImpulseConstraintSolver;

	// The world.
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,0,-10));

	// add ghostPairCallback for character controller collision detection
	dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
}
void bulletPhysicsWorld_c::runFrame(float frameTime) {
	//runVehicles();
	dynamicsWorld->stepSimulation(frameTime,2);
}
void bulletPhysicsWorld_c::shutdown() {
	// free physics data allocated in bulletPhysicsWorld_c::init()
    delete dynamicsWorld;
	dynamicsWorld = 0;
    delete solver;
	solver = 0;
    delete dispatcher;
	dispatcher = 0;
    delete collisionConfiguration;
	collisionConfiguration = 0;
    delete broadphase;
	broadphase = 0;
}
bulletColShape_c *bulletPhysicsWorld_c::registerShape(const cMod_i *cmodel, bool isStatic) {
	str shapeName;
	// static trimeshes must be handled other way then dynamic ones
	if(isStatic && (cmodel->isTriMesh() || cmodel->isCompound())) {
		shapeName = "|static|";
	} else {
		shapeName = "|dynamic|";
	}
	shapeName.append(cmodel->getName());
	bulletColShape_c *ret = shapes.getEntry(shapeName);
	if(ret) {
		return ret;
	}
	ret = new bulletColShape_c;
	ret->setName(shapeName);
	ret->init(cmodel,isStatic);
	shapes.addObject(ret);
	return ret;
}
// mass 0 means that object is static (non moveable)
physObjectAPI_i *bulletPhysicsWorld_c::createPhysicsObject(const struct physObjectDef_s &def) {
	if(def.collisionModel == 0) {
		
		return 0;
	}
	bool isStatic = (def.mass == 0.f);
	bulletColShape_c *colShape = registerShape(def.collisionModel,isStatic);
	if(colShape == 0) {

		return 0;
	}
	bulletRigidBody_c *body = new bulletRigidBody_c;
	body->init(colShape,def);
	return body;
}
void bulletPhysicsWorld_c::setGravity(const vec3_c &newGravity) {
	gravity = newGravity;
}
const vec3_c &bulletPhysicsWorld_c::getGravity() const {
	return gravity;
}



