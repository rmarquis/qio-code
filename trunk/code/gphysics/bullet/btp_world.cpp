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
#include "btp_characterController.h"
#include "btp_convert.h"
#include "btp_constraint.h"
#include <shared/physObjectDef.h>
#include <api/cmAPI.h>
#include <api/coreAPI.h>

#ifdef _DEBUG
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
bulletPhysicsWorld_c::~bulletPhysicsWorld_c() {
	this->shutdown();
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
	dynamicsWorld->setGravity((newGravity*QIO_TO_BULLET).floatPtr());

	// add ghostPairCallback for character controller collision detection
	dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
}
bool bulletPhysicsWorld_c::loadMap(const char *mapName) {
	return staticWorld.loadMap(mapName,this);
}
void bulletPhysicsWorld_c::runFrame(float frameTime) {
	if(dynamicsWorld == 0)
		return;
	//runVehicles();
	dynamicsWorld->stepSimulation(frameTime,2);
}
void bulletPhysicsWorld_c::shutdown() {
	// see if we forgot to free some rigid bodies or constraings
	if(bodies.size()) {
		g_core->RedWarning("bulletPhysicsWorld_c::shutdown: forgot to free %i bodies\n",bodies.size());
		while(bodies.size()) {
			this->destroyPhysicsObject(bodies[0]);
		}	
	}
	if(constraints.size()) {
		g_core->RedWarning("bulletPhysicsWorld_c::shutdown: forgot to free %i constraints\n",constraints.size());
		while(constraints.size()) {
			this->destroyPhysicsConstraint(constraints[0]);
		}
	}
	if(characters.size()) {
		g_core->RedWarning("bulletPhysicsWorld_c::shutdown: forgot to free %i characters\n",characters.size());
		while(characters.size()) {
			this->freeCharacter(characters[0]);
		}
	}
	// free static world data
	staticWorld.freeMemory();
	// free physics data allocated in bulletPhysicsWorld_c::init()
	if(dynamicsWorld) {
		delete dynamicsWorld;
		dynamicsWorld = 0;
	}
	if(solver) {
		delete solver;
		solver = 0;
	}
	if(dispatcher) {
		delete dispatcher;
		dispatcher = 0;
	}
	if(collisionConfiguration) {
		delete collisionConfiguration;
		collisionConfiguration = 0;
	}
	if(broadphase) {
		delete broadphase;
		broadphase = 0;
	}
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
	if(colShape->getBulletCollisionShape() == 0) {

		return 0;
	}
	bulletRigidBody_c *body = new bulletRigidBody_c;
	body->init(colShape,def,this);
	this->dynamicsWorld->addRigidBody(body->getBulletRigidBody());
	this->bodies.push_back(body);
	return body;
}
void bulletPhysicsWorld_c::destroyPhysicsObject(class physObjectAPI_i *p) {
	if(p == 0)
		return;
	bulletRigidBody_c *pBody = dynamic_cast<bulletRigidBody_c*>(p);
	this->bodies.remove(pBody);
	delete pBody;
}
class physConstraintAPI_i *bulletPhysicsWorld_c::createConstraintBall(const vec3_c &pos, physObjectAPI_i *b0API, physObjectAPI_i *b1API) {
	bulletRigidBody_c *b0 = dynamic_cast<bulletRigidBody_c*>(b0API);
	bulletRigidBody_c *b1 = dynamic_cast<bulletRigidBody_c*>(b1API);
	if(b0 == 0) {
		if(b1) {
			b0 = b1;
			b1 = 0;
		} else {
			g_core->RedWarning("bulletPhysicsWorld_c::createConstraintBall: failed to create constraint because both bodies are NULL\n");
			return 0;
		}
	}
	btpConstraintBall_c *ballConstraint = new btpConstraintBall_c;
	ballConstraint->init(pos,b0,b1,this);
	this->constraints.push_back(ballConstraint);
	return ballConstraint;
}
class physConstraintAPI_i *bulletPhysicsWorld_c::createConstraintHinge(const vec3_c &pos, const vec3_c &axis, physObjectAPI_i *b0API, physObjectAPI_i *b1API) {
	bulletRigidBody_c *b0 = dynamic_cast<bulletRigidBody_c*>(b0API);
	bulletRigidBody_c *b1 = dynamic_cast<bulletRigidBody_c*>(b1API);
	if(b0 == 0) {
		if(b1) {
			b0 = b1;
			b1 = 0;
		} else {
			g_core->RedWarning("bulletPhysicsWorld_c::createConstraintBall: failed to create constraint because both bodies are NULL\n");
			return 0;
		}
	}
	btpConstraintHinge_c *hingeConstraint = new btpConstraintHinge_c;
	hingeConstraint->init(pos,axis,b0,b1,this);
	this->constraints.push_back(hingeConstraint);
	return hingeConstraint;
}
void bulletPhysicsWorld_c::destroyPhysicsConstraint(physConstraintAPI_i *p) {
	btpConstraintBase_c *base = dynamic_cast<btpConstraintBase_c*>(p);
	if(base == 0) {
		g_core->RedWarning("bulletPhysicsWorld_c::destroyPhysicsConstraint: dynamic cast to constraintBase failed. This should never happen!\n");
		return;
	}
	this->constraints.remove(base);
	base->destroyConstraint();
	delete base;
}
class physCharacterControllerAPI_i *bulletPhysicsWorld_c::createCharacter(const class vec3_c &pos, float characterHeight, float characterWidth) {
	btpCharacterController_c *newChar = new btpCharacterController_c;
	newChar->init(this,pos,characterHeight,characterWidth);
	this->characters.push_back(newChar);
	return newChar;
}
void bulletPhysicsWorld_c::freeCharacter(class physCharacterControllerAPI_i *p) {
	btpCharacterController_c *pChar = dynamic_cast<btpCharacterController_c*>(p);
	this->characters.remove(pChar);
	delete pChar;
}
void bulletPhysicsWorld_c::setGravity(const vec3_c &newGravity) {
	gravity = newGravity;
}
const vec3_c &bulletPhysicsWorld_c::getGravity() const {
	return gravity;
}
#include <shared/trace.h>
class btRayCallback_c : public btCollisionWorld::RayResultCallback {
public:
	btRayCallback_c(const btVector3&	rayFromWorld,const btVector3&	rayToWorld)
	:m_rayFromWorld(rayFromWorld),
	m_rayToWorld(rayToWorld)
	{
	}

	btVector3	m_rayFromWorld;//used to calculate hitPointWorld from hitFraction
	btVector3	m_rayToWorld;

	btVector3	m_hitNormalWorld;
	btVector3	m_hitPointWorld;
		
	virtual	btScalar	addSingleResult(btCollisionWorld::LocalRayResult& rayResult,bool normalInWorldSpace)
	{
		//caller already does the filter on the m_closestHitFraction
		btAssert(rayResult.m_hitFraction <= m_closestHitFraction);
		
		m_closestHitFraction = rayResult.m_hitFraction;
		m_collisionObject = rayResult.m_collisionObject;
		if (normalInWorldSpace)
		{
			m_hitNormalWorld = rayResult.m_hitNormalLocal;
		} else
		{
			///need to transform normal into worldspace
			m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
		}
		m_hitPointWorld.setInterpolate3(m_rayFromWorld,m_rayToWorld,rayResult.m_hitFraction);
		return rayResult.m_hitFraction;
	}
};
bool bulletPhysicsWorld_c::traceRay(class trace_c &tr) {
	btVector3 rayFrom;
	rayFrom = (btVector3(tr.getStartPos().x*QIO_TO_BULLET,tr.getStartPos().y*QIO_TO_BULLET,tr.getStartPos().z*QIO_TO_BULLET));
	btVector3 rayTo;
	rayTo = (btVector3(tr.getTo().x*QIO_TO_BULLET,tr.getTo().y*QIO_TO_BULLET,tr.getTo().z*QIO_TO_BULLET));
	btRayCallback_c rayCallback(rayFrom,rayTo);
	dynamicsWorld->rayTest(rayFrom,rayTo,rayCallback);
	if(rayCallback.hasHit()) {
		tr.setHitPos(vec3_c(rayCallback.m_hitPointWorld.m_floats)*BULLET_TO_QIO);
	}
	if(rayCallback.m_collisionObject) {
		void *uPtr = rayCallback.m_collisionObject->getUserPointer();
		if(uPtr) {
			tr.setHitEntity((BaseEntity*)uPtr);
		}
	}
	return rayCallback.hasHit();
}


