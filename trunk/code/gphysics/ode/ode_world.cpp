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
#include "ode_world.h"
#include "ode_shape.h"
#include "ode_rigidBody.h"
#include "ode_characterController.h"
#include "ode_convert.h"
#include "ode_constraint.h"
#include <shared/physObjectDef.h>
#include <api/cmAPI.h>
#include <api/coreAPI.h>

#ifdef _DEBUG
#pragma comment( lib, "ode_singled.lib" )
#else
#pragma comment( lib, "ode_single.lib" )
#endif

odePhysicsWorld_c::odePhysicsWorld_c(const char *newDebugName) {
	this->debugName = newDebugName;
//	broadphase = 0;
//	collisionConfiguration = 0;
///	dispatcher = 0;
//	solver = 0;
//	dynamicsWorld = 0;
}
odePhysicsWorld_c::~odePhysicsWorld_c() {
	this->shutdown();
}

void odePhysicsWorld_c::init(const vec3_c &newGravity) {
	this->gravity = newGravity;

    dInitODE ();
	world = dWorldCreate();
	space = dSimpleSpaceCreate(0);
	contactGroup = dJointGroupCreate(0);
	dWorldSetGravity(world, 0,0,-5);
	//dWorldSetGravity(world, gravity.x*QIO_TO_ODE,gravity.y*QIO_TO_ODE,gravity.z*QIO_TO_ODE);
	//dWorldSetERP(world, 0.2);
	//dWorldSetCFM(world, 1e-5);
	//dWorldSetContactMaxCorrectingVel(world, 0.9);
	//dWorldSetContactSurfaceLayer(world, 0.001);
	//dWorldSetAutoDisableFlag(world, 1);

	dWorldSetCFM (world,1e-5);
	dWorldSetAutoDisableFlag (world,1);

#if 1
	dWorldSetAutoDisableAverageSamplesCount( world, 10 );
#endif

	dWorldSetLinearDamping(world, 0.00001);
	dWorldSetAngularDamping(world, 0.005);
	dWorldSetMaxAngularSpeed(world, 200);

	dWorldSetContactMaxCorrectingVel (world,0.1);
	dWorldSetContactSurfaceLayer (world,0.001);

    //dCreatePlane(space, 0, 0, 1, 0);
}
bool odePhysicsWorld_c::loadMap(const char *mapName) {
	return staticWorld.loadMap(mapName,this);
}
#define ODE_MAX_CONTACTS 32
dWorldID tmpWorld;
dJointGroupID tmpGroup;
static void ODE_NearCallback (void *data, dGeomID o1, dGeomID o2)
{
	// Temporary index for each contact
	int i;

	// Get the dynamics body for each geom
	dBodyID b1 = dGeomGetBody(o1);
	dBodyID b2 = dGeomGetBody(o2);
	
	class odeRigidBody_c *ob1;
	class odeRigidBody_c *ob2;

	float extraBounciness = 0.f;
	if(b1) {
		ob1 = (class odeRigidBody_c *)dBodyGetData(b1);
		if(ob1) {
			extraBounciness += ob1->getBounciness();
		}
	} else {
		ob1 = 0;
	}
	if(b2) {
		ob2 = (class odeRigidBody_c *)dBodyGetData(b2);
		if(ob2) {
			extraBounciness += ob2->getBounciness();
		}
	} else {
		ob2 = 0;
	}


	// Create an array of dContact objects to hold the contact joints
	dContact contact[ODE_MAX_CONTACTS];

	// Now we set the joint properties of each contact. Going into the full details here would require a tutorial of its
	// own. I'll just say that the members of the dContact structure control the joint behaviour, such as friction,
	// velocity and bounciness. See section 7.3.7 of the ODE manual and have fun experimenting to learn more.  
	for (i = 0; i < ODE_MAX_CONTACTS; i++)
	{
		contact[i].surface.mode = dContactBounce | dContactSoftCFM;
		//| dContactApprox1;
		// dInfinity here results with two high friction for trimeshes vs trimesh contacts
		contact[i].surface.mu = 10.f;//dInfinity;
		contact[i].surface.mu2 = 0;
		contact[i].surface.bounce = 0.01+extraBounciness;
		contact[i].surface.bounce_vel = 0.1;
		contact[i].surface.soft_cfm = 0.01;
	}

	// Here we do the actual collision test by calling dCollide. It returns the number of actual contact points or zero
	// if there were none. As well as the geom IDs, max number of contacts we also pass the address of a dContactGeom
	// as the fourth parameter. dContactGeom is a substructure of a dContact object so we simply pass the address of
	// the first dContactGeom from our array of dContact objects and then pass the offset to the next dContactGeom
	// as the fifth paramater, which is the size of a dContact structure. That made sense didn't it?  
	if (int numc = dCollide(o1, o2, ODE_MAX_CONTACTS, &contact[0].geom, sizeof(dContact)))
	{
		// To add each contact point found to our joint group we call dJointCreateContact which is just one of the many
		// different joint types available.  
		for (i = 0; i < numc; i++)
		{
			// dJointCreateContact needs to know which world and joint group to work with as well as the dContact
			// object itself. It returns a new dJointID which we then use with dJointAttach to finally create the
			// temporary contact joint between the two geom bodies.
			dJointID c = dJointCreateContact(tmpWorld, tmpGroup, contact + i);
			dJointAttach(c, b1, b2);
		}
	}
}

void odePhysicsWorld_c::runFrame(float frameTime) {
	if(frameTime <= 0)
		return;
//	if(dynamicsWorld == 0)
///		return;
	//runVehicles();
////	dynamicsWorld->stepSimulation(frameTime,2);
	tmpWorld = world;
	tmpGroup = contactGroup;
	unsigned int numSubSteps = 5;
	float subStepTime = frameTime / numSubSteps;
	for(unsigned int i = 0; i < numSubSteps; i++) {
		dSpaceCollide(space, 0, &ODE_NearCallback);
		dWorldQuickStep(world, subStepTime);
		dJointGroupEmpty(contactGroup);
	}
}
void odePhysicsWorld_c::shutdown() {
	// see if we forgot to free some rigid bodies or constraings
	//if(bodies.size()) {
	//	g_core->RedWarning("odePhysicsWorld_c::shutdown: forgot to free %i bodies\n",bodies.size());
	//	while(bodies.size()) {
	//		this->destroyPhysicsObject(bodies[0]);
	//	}	
	//}
	//if(constraints.size()) {
	//	g_core->RedWarning("odePhysicsWorld_c::shutdown: forgot to free %i constraints\n",constraints.size());
	//	while(constraints.size()) {
	//		this->destroyPhysicsConstraint(constraints[0]);
	//	}
	//}
	//if(characters.size()) {
	//	g_core->RedWarning("odePhysicsWorld_c::shutdown: forgot to free %i characters\n",characters.size());
	//	while(characters.size()) {
	//		this->freeCharacter(characters[0]);
	//	}
	//}
	//// free static world data
	//staticWorld.freeMemory();
	// free physics data allocated in odePhysicsWorld_c::init()

	if(contactGroup) {
		dJointGroupDestroy(contactGroup);
		contactGroup = 0;
	}
	if(space) {
		dSpaceDestroy(space);
		space = 0;
	}
	if(world) {
		dWorldDestroy(world);
		world = 0;
	}
}
odeColShape_c *odePhysicsWorld_c::registerShape(const cMod_i *cmodel, bool isStatic) {
	str shapeName;
	// static trimeshes must be handled other way then dynamic ones
	if(isStatic && (cmodel->isTriMesh() || cmodel->isCompound())) {
		shapeName = "|static|";
	} else {
		shapeName = "|dynamic|";
	}
	shapeName.append(cmodel->getName());
	odeColShape_c *ret = shapes.getEntry(shapeName);
	if(ret) {
	//	return ret;
	}
	ret = new odeColShape_c;
	ret->setName(shapeName);
	ret->init(space,cmodel,isStatic);
	shapes.addObject(ret);
	return ret;
}
// mass 0 means that object is static (non moveable)
physObjectAPI_i *odePhysicsWorld_c::createPhysicsObject(const struct physObjectDef_s &def) {
	if(def.collisionModel == 0) {
		
		return 0;
	}
	bool isStatic = (def.mass == 0.f);
	odeColShape_c *colShape = registerShape(def.collisionModel,isStatic);
	if(colShape == 0) {

		return 0;
	}
	if(colShape->getODEGeom() == 0) {

		return 0;
	}
	odeRigidBody_c *body = new odeRigidBody_c;
	body->init(this,colShape,def);
	bodies.push_back(body);
	return body;
}
void odePhysicsWorld_c::destroyPhysicsObject(class physObjectAPI_i *p) {
	if(p == 0)
		return;
	odeRigidBody_c *pBody = dynamic_cast<odeRigidBody_c*>(p);
	this->bodies.remove(pBody);
	delete pBody;
}
class physConstraintAPI_i *odePhysicsWorld_c::createConstraintBall(const vec3_c &pos, physObjectAPI_i *b0API, physObjectAPI_i *b1API) {
	odeRigidBody_c *b0 = dynamic_cast<odeRigidBody_c*>(b0API);
	odeRigidBody_c *b1 = dynamic_cast<odeRigidBody_c*>(b1API);
	if(b0 == 0) {
		if(b1) {
			b0 = b1;
			b1 = 0;
		} else {
			g_core->RedWarning("odePhysicsWorld_c::createConstraintBall: failed to create constraint because both bodies are NULL\n");
			return 0;
		}
	}
	odeConstraintBall_c *ballConstraint = new odeConstraintBall_c;
	ballConstraint->init(pos,b0,b1,this);
	this->constraints.push_back(ballConstraint);
	return ballConstraint;
}
class physConstraintAPI_i *odePhysicsWorld_c::createConstraintHinge(const vec3_c &pos, const vec3_c &axis, physObjectAPI_i *b0API, physObjectAPI_i *b1API) {
	odeRigidBody_c *b0 = dynamic_cast<odeRigidBody_c*>(b0API);
	odeRigidBody_c *b1 = dynamic_cast<odeRigidBody_c*>(b1API);
	if(b0 == 0) {
		if(b1) {
			b0 = b1;
			b1 = 0;
		} else {
			g_core->RedWarning("odePhysicsWorld_c::createConstraintBall: failed to create constraint because both bodies are NULL\n");
			return 0;
		}
	}
	odeConstraintHinge_c *hingeConstraint = new odeConstraintHinge_c;
	hingeConstraint->init(pos,axis,b0,b1,this);
	this->constraints.push_back(hingeConstraint);
	return hingeConstraint;
}
void odePhysicsWorld_c::destroyPhysicsConstraint(physConstraintAPI_i *p) {
	odeConstraintBase_c *base = dynamic_cast<odeConstraintBase_c*>(p);
	if(base == 0) {
		g_core->RedWarning("odePhysicsWorld_c::destroyPhysicsConstraint: dynamic cast to constraintBase failed. This should never happen!\n");
		return;
	}
	this->constraints.remove(base);
	base->destroyConstraint();
	delete base;
}
class physCharacterControllerAPI_i *odePhysicsWorld_c::createCharacter(const class vec3_c &pos, float characterHeight, float characterWidth) {
	odeCharacterController_c *newChar = new odeCharacterController_c;
	newChar->init(this,pos,characterHeight,characterWidth);
	this->characters.push_back(newChar);
	return newChar;
}
void odePhysicsWorld_c::freeCharacter(class physCharacterControllerAPI_i *p) {
	odeCharacterController_c *pChar = dynamic_cast<odeCharacterController_c*>(p);
	this->characters.remove(pChar);
	delete pChar;
}
void odePhysicsWorld_c::setGravity(const vec3_c &newGravity) {
	gravity = newGravity;
}
const vec3_c &odePhysicsWorld_c::getGravity() const {
	return gravity;
}
#include <shared/trace.h>
bool odePhysicsWorld_c::traceRay(class trace_c &tr) {
	//btVector3 rayFrom;
	//rayFrom = (btVector3(tr.getStartPos().x*QIO_TO_ODE,tr.getStartPos().y*QIO_TO_ODE,tr.getStartPos().z*QIO_TO_ODE));
	//btVector3 rayTo;
	//rayTo = (btVector3(tr.getTo().x*QIO_TO_ODE,tr.getTo().y*QIO_TO_ODE,tr.getTo().z*QIO_TO_ODE));
	//btRayCallback_c rayCallback(rayFrom,rayTo);
	//dynamicsWorld->rayTest(rayFrom,rayTo,rayCallback);
	//if(rayCallback.hasHit()) {
	//	tr.setHitPos(vec3_c(rayCallback.m_hitPointWorld.m_floats)*ODE_TO_QIO);
	//}
	//if(rayCallback.m_collisionObject) {
	//	void *uPtr = rayCallback.m_collisionObject->getUserPointer();
	//	if(uPtr) {
	//		tr.setHitEntity((BaseEntity*)uPtr);
	//	}
	//}
	//return rayCallback.hasHit();
	return false;
}


