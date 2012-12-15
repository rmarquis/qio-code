/*
============================================================================
Copyright (C) 2012 V.

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
// g_bullet.cpp
#include "g_local.h"
#include <api/cmAPI.h>
#include <math/quat.h>
#include <math/aabb.h>
#include "classes/ModelEntity.h"
#include <shared/cmSurface.h>
#include <shared/autoCvar.h>
#include <api/coreAPI.h>
#include <api/cmAPI.h>
#include <api/loadingScreenMgrAPI.h>
#include "bt_include.h"

#ifdef DEBUG
#pragma comment( lib, "BulletCollision_debug.lib" )
#pragma comment( lib, "BulletDynamics_debug.lib" )
#pragma comment( lib, "LinearMath_debug.lib" )
#else
#pragma comment( lib, "BulletCollision.lib" )
#pragma comment( lib, "BulletDynamics.lib" )
#pragma comment( lib, "LinearMath.lib" )
#endif



btBroadphaseInterface* broadphase = 0;
btDefaultCollisionConfiguration* collisionConfiguration = 0;
btCollisionDispatcher* dispatcher = 0;
btSequentialImpulseConstraintSolver* solver = 0;
btDiscreteDynamicsWorld* dynamicsWorld = 0;

// they need to be alloced as long as they are used
static arraySTD_c<btTriangleIndexVertexArray*> bt_trimeshes;
static arraySTD_c<cmSurface_c*> bt_cmSurfs;
class cMod_i *bt_worldCMod = 0;

#if 0
#define TRYTOFIX_INTERNAL_EDGES
#endif

#define TEST_RAGDOLL_SCALE 64.f

#define CONSTRAINT_DEBUG_SIZE 0.2f*TEST_RAGDOLL_SCALE

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif

#ifndef M_PI_4
#define M_PI_4     0.785398163397448309616
#endif

class RagDoll
{
	enum
	{
		BODYPART_PELVIS = 0,
		BODYPART_SPINE,
		BODYPART_HEAD,

		BODYPART_LEFT_UPPER_LEG,
		BODYPART_LEFT_LOWER_LEG,

		BODYPART_RIGHT_UPPER_LEG,
		BODYPART_RIGHT_LOWER_LEG,

		BODYPART_LEFT_UPPER_ARM,
		BODYPART_LEFT_LOWER_ARM,

		BODYPART_RIGHT_UPPER_ARM,
		BODYPART_RIGHT_LOWER_ARM,

		BODYPART_COUNT
	};

	enum
	{
		JOINT_PELVIS_SPINE = 0,
		JOINT_SPINE_HEAD,

		JOINT_LEFT_HIP,
		JOINT_LEFT_KNEE,

		JOINT_RIGHT_HIP,
		JOINT_RIGHT_KNEE,

		JOINT_LEFT_SHOULDER,
		JOINT_LEFT_ELBOW,

		JOINT_RIGHT_SHOULDER,
		JOINT_RIGHT_ELBOW,

		JOINT_COUNT
	};

	btDynamicsWorld* m_ownerWorld;
	btCollisionShape* m_shapes[BODYPART_COUNT];
	btRigidBody* m_bodies[BODYPART_COUNT];
	btTypedConstraint* m_joints[JOINT_COUNT];

	btRigidBody* localCreateRigidBody (btScalar mass, const btTransform& startTransform, btCollisionShape* shape)
	{
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			shape->calculateLocalInertia(mass,localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,shape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		m_ownerWorld->addRigidBody(body);

		return body;
	}

public:
	RagDoll (btDynamicsWorld* ownerWorld, const btVector3& positionOffset)
		: m_ownerWorld (ownerWorld)
	{
		// Setup the geometry
		m_shapes[BODYPART_PELVIS] = new btCapsuleShape(btScalar(0.15*TEST_RAGDOLL_SCALE), btScalar(0.20*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_SPINE] = new btCapsuleShape(btScalar(0.15*TEST_RAGDOLL_SCALE), btScalar(0.28*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_HEAD] = new btCapsuleShape(btScalar(0.10*TEST_RAGDOLL_SCALE), btScalar(0.05*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_LEFT_UPPER_LEG] = new btCapsuleShape(btScalar(0.07*TEST_RAGDOLL_SCALE), btScalar(0.45*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_LEFT_LOWER_LEG] = new btCapsuleShape(btScalar(0.05*TEST_RAGDOLL_SCALE), btScalar(0.37*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_RIGHT_UPPER_LEG] = new btCapsuleShape(btScalar(0.07*TEST_RAGDOLL_SCALE), btScalar(0.45*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_RIGHT_LOWER_LEG] = new btCapsuleShape(btScalar(0.05*TEST_RAGDOLL_SCALE), btScalar(0.37*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_LEFT_UPPER_ARM] = new btCapsuleShape(btScalar(0.05*TEST_RAGDOLL_SCALE), btScalar(0.33*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_LEFT_LOWER_ARM] = new btCapsuleShape(btScalar(0.04*TEST_RAGDOLL_SCALE), btScalar(0.25*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_RIGHT_UPPER_ARM] = new btCapsuleShape(btScalar(0.05*TEST_RAGDOLL_SCALE), btScalar(0.33*TEST_RAGDOLL_SCALE));
		m_shapes[BODYPART_RIGHT_LOWER_ARM] = new btCapsuleShape(btScalar(0.04*TEST_RAGDOLL_SCALE), btScalar(0.25*TEST_RAGDOLL_SCALE));

		// Setup all the rigid bodies
		btTransform offset; offset.setIdentity();
		offset.setOrigin(positionOffset);

		btTransform transform;
		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(0.), btScalar(1.*TEST_RAGDOLL_SCALE), btScalar(0.)));
		m_bodies[BODYPART_PELVIS] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_PELVIS]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(0.), btScalar(1.2*TEST_RAGDOLL_SCALE), btScalar(0.)));
		m_bodies[BODYPART_SPINE] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_SPINE]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(0.), btScalar(1.6*TEST_RAGDOLL_SCALE), btScalar(0.)));
		m_bodies[BODYPART_HEAD] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_HEAD]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(-0.18*TEST_RAGDOLL_SCALE), btScalar(0.65*TEST_RAGDOLL_SCALE), btScalar(0.)));
		m_bodies[BODYPART_LEFT_UPPER_LEG] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_LEFT_UPPER_LEG]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(-0.18*TEST_RAGDOLL_SCALE), btScalar(0.2*TEST_RAGDOLL_SCALE), btScalar(0.)));
		m_bodies[BODYPART_LEFT_LOWER_LEG] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_LEFT_LOWER_LEG]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(0.18*TEST_RAGDOLL_SCALE), btScalar(0.65*TEST_RAGDOLL_SCALE), btScalar(0.)));
		m_bodies[BODYPART_RIGHT_UPPER_LEG] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_RIGHT_UPPER_LEG]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(0.18*TEST_RAGDOLL_SCALE), btScalar(0.2*TEST_RAGDOLL_SCALE), btScalar(0.)));
		m_bodies[BODYPART_RIGHT_LOWER_LEG] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_RIGHT_LOWER_LEG]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(-0.35*TEST_RAGDOLL_SCALE), btScalar(1.45*TEST_RAGDOLL_SCALE), btScalar(0.)));
		transform.getBasis().setEulerZYX(0,0,M_PI_2);
		m_bodies[BODYPART_LEFT_UPPER_ARM] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_LEFT_UPPER_ARM]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(-0.7*TEST_RAGDOLL_SCALE), btScalar(1.45*TEST_RAGDOLL_SCALE), btScalar(0.)));
		transform.getBasis().setEulerZYX(0,0,M_PI_2);
		m_bodies[BODYPART_LEFT_LOWER_ARM] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_LEFT_LOWER_ARM]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(0.35), btScalar(1.45), btScalar(0.)));
		transform.getBasis().setEulerZYX(0,0,-M_PI_2);
		m_bodies[BODYPART_RIGHT_UPPER_ARM] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_RIGHT_UPPER_ARM]);

		transform.setIdentity();
		transform.setOrigin(btVector3(btScalar(0.7*TEST_RAGDOLL_SCALE), btScalar(1.45*TEST_RAGDOLL_SCALE), btScalar(0.)));
		transform.getBasis().setEulerZYX(0,0,-M_PI_2);
		m_bodies[BODYPART_RIGHT_LOWER_ARM] = localCreateRigidBody(btScalar(1.), offset*transform, m_shapes[BODYPART_RIGHT_LOWER_ARM]);

		// Setup some damping on the m_bodies
		for (int i = 0; i < BODYPART_COUNT; ++i)
		{
			m_bodies[i]->setDamping(0.05, 0.85);
			m_bodies[i]->setDeactivationTime(0.8);
			m_bodies[i]->setSleepingThresholds(1.6, 2.5);
		}

		// Now setup the constraints
		btHingeConstraint* hingeC;
		btConeTwistConstraint* coneC;

		btTransform localA, localB;

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(btVector3(btScalar(0.), btScalar(0.15*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(btVector3(btScalar(0.), btScalar(-0.15*TEST_RAGDOLL_SCALE), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_SPINE], localA, localB);
		hingeC->setLimit(btScalar(-M_PI_4), btScalar(M_PI_2));
		m_joints[JOINT_PELVIS_SPINE] = hingeC;
		hingeC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_ownerWorld->addConstraint(m_joints[JOINT_PELVIS_SPINE], true);


		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,M_PI_2); localA.setOrigin(btVector3(btScalar(0.), btScalar(0.30*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,M_PI_2); localB.setOrigin(btVector3(btScalar(0.), btScalar(-0.14*TEST_RAGDOLL_SCALE), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_HEAD], localA, localB);
		coneC->setLimit(M_PI_4, M_PI_4, M_PI_2);
		m_joints[JOINT_SPINE_HEAD] = coneC;
		coneC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_ownerWorld->addConstraint(m_joints[JOINT_SPINE_HEAD], true);


		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,-M_PI_4*5); localA.setOrigin(btVector3(btScalar(-0.18*TEST_RAGDOLL_SCALE), btScalar(-0.10*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,-M_PI_4*5); localB.setOrigin(btVector3(btScalar(0.), btScalar(0.225*TEST_RAGDOLL_SCALE), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_LEFT_UPPER_LEG], localA, localB);
		coneC->setLimit(M_PI_4, M_PI_4, 0);
		m_joints[JOINT_LEFT_HIP] = coneC;
		coneC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_HIP], true);

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(btVector3(btScalar(0.), btScalar(-0.225*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(btVector3(btScalar(0.), btScalar(0.185*TEST_RAGDOLL_SCALE), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_LEG], *m_bodies[BODYPART_LEFT_LOWER_LEG], localA, localB);
		hingeC->setLimit(btScalar(0), btScalar(M_PI_2));
		m_joints[JOINT_LEFT_KNEE] = hingeC;
		hingeC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_KNEE], true);


		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,M_PI_4); localA.setOrigin(btVector3(btScalar(0.18*TEST_RAGDOLL_SCALE), btScalar(-0.10*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,M_PI_4); localB.setOrigin(btVector3(btScalar(0.), btScalar(0.225*TEST_RAGDOLL_SCALE), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_PELVIS], *m_bodies[BODYPART_RIGHT_UPPER_LEG], localA, localB);
		coneC->setLimit(M_PI_4, M_PI_4, 0);
		m_joints[JOINT_RIGHT_HIP] = coneC;
		coneC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_HIP], true);

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(btVector3(btScalar(0.), btScalar(-0.225*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(btVector3(btScalar(0.), btScalar(0.185*TEST_RAGDOLL_SCALE), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_LEG], *m_bodies[BODYPART_RIGHT_LOWER_LEG], localA, localB);
		hingeC->setLimit(btScalar(0), btScalar(M_PI_2));
		m_joints[JOINT_RIGHT_KNEE] = hingeC;
		hingeC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_KNEE], true);


		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,M_PI); localA.setOrigin(btVector3(btScalar(-0.2*TEST_RAGDOLL_SCALE), btScalar(0.15*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,M_PI_2); localB.setOrigin(btVector3(btScalar(0.), btScalar(-0.18*TEST_RAGDOLL_SCALE), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_LEFT_UPPER_ARM], localA, localB);
		coneC->setLimit(M_PI_2, M_PI_2, 0);
		coneC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_joints[JOINT_LEFT_SHOULDER] = coneC;
		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_SHOULDER], true);

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(btVector3(btScalar(0.), btScalar(0.18*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(btVector3(btScalar(0.), btScalar(-0.14*TEST_RAGDOLL_SCALE), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_LEFT_UPPER_ARM], *m_bodies[BODYPART_LEFT_LOWER_ARM], localA, localB);
//		hingeC->setLimit(btScalar(-M_PI_2), btScalar(0));
		hingeC->setLimit(btScalar(0), btScalar(M_PI_2));
		m_joints[JOINT_LEFT_ELBOW] = hingeC;
		hingeC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_ownerWorld->addConstraint(m_joints[JOINT_LEFT_ELBOW], true);



		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,0,0); localA.setOrigin(btVector3(btScalar(0.2*TEST_RAGDOLL_SCALE), btScalar(0.15*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,0,M_PI_2); localB.setOrigin(btVector3(btScalar(0.), btScalar(-0.18*TEST_RAGDOLL_SCALE), btScalar(0.)));
		coneC = new btConeTwistConstraint(*m_bodies[BODYPART_SPINE], *m_bodies[BODYPART_RIGHT_UPPER_ARM], localA, localB);
		coneC->setLimit(M_PI_2, M_PI_2, 0);
		m_joints[JOINT_RIGHT_SHOULDER] = coneC;
		coneC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_SHOULDER], true);

		localA.setIdentity(); localB.setIdentity();
		localA.getBasis().setEulerZYX(0,M_PI_2,0); localA.setOrigin(btVector3(btScalar(0.), btScalar(0.18*TEST_RAGDOLL_SCALE), btScalar(0.)));
		localB.getBasis().setEulerZYX(0,M_PI_2,0); localB.setOrigin(btVector3(btScalar(0.), btScalar(-0.14*TEST_RAGDOLL_SCALE), btScalar(0.)));
		hingeC =  new btHingeConstraint(*m_bodies[BODYPART_RIGHT_UPPER_ARM], *m_bodies[BODYPART_RIGHT_LOWER_ARM], localA, localB);
//		hingeC->setLimit(btScalar(-M_PI_2), btScalar(0));
		hingeC->setLimit(btScalar(0), btScalar(M_PI_2));
		m_joints[JOINT_RIGHT_ELBOW] = hingeC;
		hingeC->setDbgDrawSize(CONSTRAINT_DEBUG_SIZE);

		m_ownerWorld->addConstraint(m_joints[JOINT_RIGHT_ELBOW], true);
	}

	virtual	~RagDoll ()
	{
		int i;

		// Remove all constraints
		for ( i = 0; i < JOINT_COUNT; ++i)
		{
			m_ownerWorld->removeConstraint(m_joints[i]);
			delete m_joints[i]; m_joints[i] = 0;
		}

		// Remove all bodies and shapes
		for ( i = 0; i < BODYPART_COUNT; ++i)
		{
			m_ownerWorld->removeRigidBody(m_bodies[i]);
			
			delete m_bodies[i]->getMotionState();

			delete m_bodies[i]; m_bodies[i] = 0;
			delete m_shapes[i]; m_shapes[i] = 0;
		}
	}
};

#ifdef TRYTOFIX_INTERNAL_EDGES

extern ContactAddedCallback		gContactAddedCallback;

///User can override this material combiner by implementing gContactAddedCallback and setting body0->m_collisionFlags |= btCollisionObject::customMaterialCallback;
inline btScalar	calculateCombinedFriction(float friction0,float friction1)
{
	return 0.f;
	btScalar friction = friction0 * friction1;

	const btScalar MAX_FRICTION  = 10.f;
	if (friction < -MAX_FRICTION)
		friction = -MAX_FRICTION;
	if (friction > MAX_FRICTION)
		friction = MAX_FRICTION;
	return friction;

}

inline btScalar	calculateCombinedRestitution(float restitution0,float restitution1)
{
	return restitution0 * restitution1;
}
static bool CustomMaterialCombinerCallback(btManifoldPoint& cp,	const btCollisionObject* colObj0,int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1)
{

	if (1)
	{
		btAdjustInternalEdgeContacts(cp,colObj1,colObj0, partId1,index1);
		//btAdjustInternalEdgeContacts(cp,colObj1,colObj0, partId1,index1, BT_TRIANGLE_CONVEX_BACKFACE_MODE);
		//btAdjustInternalEdgeContacts(cp,colObj1,colObj0, partId1,index1, BT_TRIANGLE_CONVEX_DOUBLE_SIDED+BT_TRIANGLE_CONCAVE_DOUBLE_SIDED);
	}
return false;
	/*float friction0 = colObj0->getFriction();
	float friction1 = colObj1->getFriction();
	float restitution0 = colObj0->getRestitution();
	float restitution1 = colObj1->getRestitution();

	if (colObj0->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
	{
		friction0 = 1.0;//partId0,index0
		restitution0 = 0.f;
	}
	if (colObj1->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK)
	{
		if (index1&1)
		{
			friction1 = 1.0f;//partId1,index1
		} else
		{
			friction1 = 0.f;
		}
		restitution1 = 0.f;
	}

	cp.m_combinedFriction = calculateCombinedFriction(friction0,friction1);
	cp.m_combinedRestitution = calculateCombinedRestitution(restitution0,restitution1);

	//this return value is currently ignored, but to be on the safe side: return false if you don't calculate friction
	return true;*/
}

#endif // TRYTOFIX_INTERNAL_EDGES

void G_InitBullet() {
	// Build the broadphase
	broadphase = new btDbvtBroadphase();

	// Set up the collision configuration and dispatcher
	collisionConfiguration = new btDefaultCollisionConfiguration();

	dispatcher = new btCollisionDispatcher(collisionConfiguration);

	// The actual physics solver
	solver = new btSequentialImpulseConstraintSolver;

	// The world.
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
	dynamicsWorld->setGravity(btVector3(0,0,-500));

	// add ghostPairCallback for character controller collision detection
	dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

	//dynamicsWorld->getSolverInfo().m_splitImpulse = true;

#ifdef TRYTOFIX_INTERNAL_EDGES
	// enable internal edges fix
	gContactAddedCallback = CustomMaterialCombinerCallback;
#endif // TRYTOFIX_INTERNAL_EDGES
}
void G_ShudownBullet() {
	// free vehicles 
	BT_ShutdownVehicles();
	for(u32 i = 0; i < bt_trimeshes.size(); i++) {
		delete bt_trimeshes[i];
	}
	bt_trimeshes.clear();
	for(u32 i = 0; i < bt_cmSurfs.size(); i++) {
		delete bt_cmSurfs[i];
	}
	bt_cmSurfs.clear();
	// free global physics data
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
	
	bt_worldCMod = 0;
}
aCvar_c bt_runPhysics("bt_runPhysics","1");
aCvar_c bt_printTimeStep("bt_printTimeStep","0");

void G_RunPhysics() {
	if(bt_runPhysics.getInt() == 0)
		return;

	if(bt_printTimeStep.getInt()) {
		g_core->Print("Frametime: %f\n",level.frameTime);
	}
	BT_RunVehicles();
	float frameTime = level.frameTime;
	dynamicsWorld->stepSimulation(frameTime,2);
	//BT_RemoveRigidBody(BT_CreateBoxBody(vec3_c(0,0,0),vec3_c(8,8,8),0));
	//btContactSolverInfo &csi = dynamicsWorld->getSolverInfo();
	//matrix_c tmp;
	//tmp.identity();
	//aabb bb;
	//bb.addPoint(8,8,8);
	//bb.addPoint(-8,-8,64);
	//BT_IsInSolid(tmp, bb);
}
// this is a (temporary?) fix to objects (especially barrels) jittering.
#if 1
float bt_collisionMargin = 4.f;
#else
// NOTE: collision margin can't be as high as 4.f 
// because it makes entities seem to float in air above ground
float bt_collisionMargin = CONVEX_DISTANCE_MARGIN;
#endif

#define USE_MOTIONSTATE 1
void BT_CreateWorldBrush(btAlignedObjectArray<btVector3> &vertices) {
	if(vertices.size() == 0)
		return;
	float mass = 0.f;
	btTransform startTransform;
	//can use a shift
	startTransform.setIdentity();
	//this create an internal copy of the vertices
	btConvexHullShape* shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
	shape->setMargin(bt_collisionMargin);
#if 0
	// Bullet debug drawind does not work without it
	shape->initializePolyhedralFeatures(); // causes crash on 20kdm2
#endif

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);

	// TODO: free this rigid body in BT_Shutdown() ?
	btRigidBody* body = new btRigidBody(cInfo);
	//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
	btRigidBody* body = new btRigidBody(mass,0,shape,localInertia);	
	body->setWorldTransform(startTransform);
#endif//
	dynamicsWorld->addRigidBody(body);
}

static class btCollisionObject *bt_queryCollisionObject = 0;
static arraySTD_c<const btCollisionObject*> bt_intersected;
class myBTContactResultCallback_c : public btCollisionWorld::ContactResultCallback {
	virtual	btScalar	addSingleResult(btManifoldPoint& cp,	const btCollisionObject* colObj0,int partId0,int index0,const btCollisionObject* colObj1,int partId1,int index1) {
		const btCollisionObject *other;
		if(colObj0 == bt_queryCollisionObject) {
			other = colObj0;
		} else {
			other = colObj1;
		}
		bt_intersected.push_back(other);
		return 0.f;
	}
};

bool BT_IsInSolid(const matrix_c &mat, const aabb &bb) {
	btAlignedObjectArray<btVector3> vertices;
	for(u32 i = 0; i < 8; i++) {
		vec3_c p = bb.getPoint(i);
		mat.transformPoint(p);
		vertices.push_back(btVector3(p.x,p.y,p.z));
	}
	bt_intersected.clear();
	btConvexHullShape *shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
	btCollisionObject collisionObject;
	bt_queryCollisionObject = &collisionObject;
	collisionObject.setCollisionShape(shape);
	myBTContactResultCallback_c resultCallback;
	dynamicsWorld->contactTest(&collisionObject,resultCallback);
	delete shape;
	if(bt_intersected.size()) 
		return true;
	return false;
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

bool BT_TraceRay(class trace_c &tr) {
	btVector3 rayFrom;
	rayFrom = (btVector3(tr.getStartPos().x,tr.getStartPos().y,tr.getStartPos().z));
	btVector3 rayTo;
	rayTo = (btVector3(tr.getTo().x,tr.getTo().y,tr.getTo().z));
	btRayCallback_c rayCallback(rayFrom,rayTo);
	dynamicsWorld->rayTest(rayFrom,rayTo,rayCallback);
	tr.setHitPos(rayCallback.m_hitPointWorld.m_floats);
	if(rayCallback.m_collisionObject) {
		void *uPtr = rayCallback.m_collisionObject->getUserPointer();
		if(uPtr) {
			tr.setHitEntity((BaseEntity*)uPtr);
		}
	}
	return rayCallback.hasHit();
}
btBvhTriangleMeshShape *BT_CreateBHVTriMeshForCMSurface(const cmSurface_c &sf) {
	btTriangleIndexVertexArray *mesh = new btTriangleIndexVertexArray;
	bt_trimeshes.push_back(mesh);

	btIndexedMesh subMesh;
	subMesh.m_numTriangles = sf.getNumTris();
	subMesh.m_numVertices = sf.getNumVerts();
	subMesh.m_vertexStride = sizeof(vec3_c);
	subMesh.m_vertexType = PHY_FLOAT;
	subMesh.m_vertexBase = sf.getVerticesBase();
	subMesh.m_indexType = PHY_INTEGER;
	subMesh.m_triangleIndexBase = sf.getIndicesBase();
	subMesh.m_triangleIndexStride = sizeof(int)*3;
	mesh->addIndexedMesh(subMesh);
	btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(mesh,true);

#ifdef TRYTOFIX_INTERNAL_EDGES
	btTriangleInfoMap* triangleInfoMap = new btTriangleInfoMap();
	//now you can adjust some thresholds in triangleInfoMap  if needed.
	//btGenerateInternalEdgeInfo fills in the btTriangleInfoMap and stores it as a user pointer of trimeshShape (trimeshShape->setUserPointer(triangleInfoMap))
	btGenerateInternalEdgeInfo(shape,triangleInfoMap);
	shape->setTriangleInfoMap(triangleInfoMap);
#endif // TRYTOFIX_INTERNAL_EDGES

	return shape;
}
void BT_CreateWorldTriMesh(const cmSurface_c &sf) {
	float mass = 0.f;
	btTransform startTransform;
	//can use a shift
	startTransform.setIdentity();

	btBvhTriangleMeshShape* shape = BT_CreateBHVTriMeshForCMSurface(sf);

	shape->setMargin(bt_collisionMargin);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);

	// TODO: free this rigid body in BT_Shutdown() ?
	btRigidBody* body = new btRigidBody(cInfo);
	//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
	btRigidBody* body = new btRigidBody(mass,0,shape,localInertia);	
	body->setWorldTransform(startTransform);
#endif//

	dynamicsWorld->addRigidBody(body);
}
btRigidBody* BT_CreateRigidBodyInternal(float mass, const btTransform& startTransform, btCollisionShape* shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

	shape->setMargin(bt_collisionMargin);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState, shape, localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	body->setContactProcessingThreshold(BT_LARGE_FLOAT);

#else
	btRigidBody* body = new btRigidBody(mass, 0, shape, localInertia);
	body->setWorldTransform(startTransform);
#endif//

#ifdef TRYTOFIX_INTERNAL_EDGES
	//enable custom material callback
	body->setCollisionFlags(body->getCollisionFlags()  | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
#endif 

	dynamicsWorld->addRigidBody(body);

	return body;
}
void G_RunCharacterController(vec3_t dir, btKinematicCharacterController *ch, vec3_t newPos) {
	// set the forward direction of the character controller
	btVector3 walkDir(dir[0],dir[1],dir[2]);
	ch->setWalkDirection(walkDir);

	btVector3 c = ch->getGhostObject()->getWorldTransform().getOrigin();
	newPos[0] = c.x();
	newPos[1] = c.y();
	newPos[2] = c.z();
}

bool G_TryToJump(btKinematicCharacterController *ch) {
	if(ch->onGround() == false)
		return false; // didnt jump
	ch->jump(); // jumped
	return true;
}
bool BT_IsCharacterOnGround(btKinematicCharacterController *ch) {
	return ch->onGround();
}
void BT_FreeCharacter(btKinematicCharacterController *c) {
	if(c == 0)
		return;
	dynamicsWorld->removeCharacter(c);
	dynamicsWorld->removeCollisionObject(c->getGhostObject());
	delete c->getGhostObject();
	delete c;
}
btRigidBody *BT_CreateRigidBodyWithCModel(const float *pos, const float *angles, const float *startVel, cMod_i *cModel, float mass) {
	if(cModel == 0) {
		g_core->RedWarning("BT_CreateRigidBodyWithCModel: NULL cmodel\n");
		return 0;
	}

	btCollisionShape *shape;
	if(cModel->isBBExts()) {
		vec3_c halfSizes = cModel->getBBExts()->getHalfSizes();
		btBoxShape *boxShape = new btBoxShape(btVector3(halfSizes[0], halfSizes[1], halfSizes[2]));
		shape = boxShape;
	} else if(cModel->isCompound()) {
		btCompoundShape *compound = new btCompoundShape;
		BT_AddCModelToCompoundShape(compound,btTransform::getIdentity(),cModel);
		shape = compound;
	} else if(cModel->isHull()) {
		shape = BT_CModelHullToConvex(cModel->getHull());
	} else if(cModel->isTriMesh()) {
		const cmSurface_c *sf = cModel->getTriMesh()->getCMSurface();
		shape = BT_CreateBHVTriMeshForCMSurface(*sf);
	} else if(cModel->isBBMinsMaxs()) {
		aabb bb;
		cModel->getBounds(bb);
		shape = BT_AABBMinsMaxsToConvex(bb);
	} else {
		return 0;
	}

	if(shape == 0)
		return 0;
	
	

		
	const btVector3 btStart(pos[0], pos[1], pos[2]);

	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btStart);


	btRigidBody *body = BT_CreateRigidBodyInternal(mass, startTransform, shape);
	body->setLinearFactor(btVector3(1, 1, 1));
		
//	body->getWorldTransform().setOrigin(btStart);

	if(startVel) {
		btVector3 vel(startVel[0], startVel[1], startVel[2]);
	/*	vel *= 150;*/

		body->setLinearVelocity(vel);
	}
		
	body->setAngularVelocity(btVector3(0,0,0));
	body->setContactProcessingThreshold(1e30);

	//aabb bb;
	//cModel->getBounds(bb);
	//vec3_c center = bb.getCenter();
	//if(center.lenSQ()) {
	//	btTransform comt;
	//	comt.setIdentity();
	//	comt.setOrigin(center);
	//	body->setCenterOfMassTransform(comt);
	//}
	//	
	//enable CCD if the object moves more than 1 meter in one simulation frame
	//rigidBody.setCcdSweptSphereRadius(20);

	//if (g_physUseCCD.integer)
	{
		body->setCcdMotionThreshold(32.f);
		body->setCcdSweptSphereRadius(6);
	}
	return body;
}
void BT_RemoveRigidBody(class btRigidBody *body) {
	btMotionState *s = body->getMotionState();
	if(s) {
		delete s;
		body->setMotionState(0);
	}
	btCollisionShape *shape = body->getCollisionShape();
	if(shape) {
		delete shape;
		body->setCollisionShape(0);
	}
	dynamicsWorld->removeRigidBody(body);
	delete body;
}
btKinematicCharacterController* BT_CreateCharacter(float stepHeight,
	vec3_t pos, float characterHeight,  float characterWidth)
{
	btPairCachingGhostObject* ghostObject = new btPairCachingGhostObject();
	btConvexShape* characterShape = new btCapsuleShapeZ(characterWidth,characterHeight);
	btTransform trans;
	trans.setIdentity();
	btVector3 vPos(pos[0],pos[1],pos[2]);
	trans.setOrigin(vPos);
	ghostObject->setWorldTransform(trans);
	ghostObject->setCollisionShape(characterShape);
	btKinematicCharacterController *character = new btKinematicCharacterController (ghostObject, characterShape, stepHeight,2);
	character->setMaxSlope(DEG2RAD(70));
	character->setJumpSpeed(200);
	character->setFallSpeed(800);
	character->setGravity(600);
	//character->setUseGhostSweepTest(

	dynamicsWorld->addCollisionObject( ghostObject, btBroadphaseProxy::CharacterFilter, 
		btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter | btBroadphaseProxy::CharacterFilter);

	dynamicsWorld->addCharacter(character);
	return character;
}
#include <shared/bspPhysicsDataLoader.h>
bspPhysicsDataLoader_c *g_bspPhysicsLoader;

// brush converting
btAlignedObjectArray<btVector3> planeEquations;
void BT_AddBrushPlane(const float q3Plane[4]) {
	btVector3 planeEq;
	planeEq.setValue(q3Plane[0],q3Plane[1],q3Plane[2]);
	// q3 plane equation is Ax + By + Cz - D = 0, so negate D
	planeEq[3] = -q3Plane[3];
	planeEquations.push_back(planeEq);
}
void BT_AddBrushPlane2(const float q3Plane[4]) {
	btVector3 planeEq;
	planeEq.setValue(q3Plane[0],q3Plane[1],q3Plane[2]);
	planeEq[3] = q3Plane[3];
	planeEquations.push_back(planeEq);
}
void BT_ConvertWorldBrush(u32 brushNum, u32 contentFlags) {
	if((contentFlags & 1) == 0)
		return;
	planeEquations.clear();
	g_bspPhysicsLoader->iterateBrushPlanes(brushNum,BT_AddBrushPlane);
	// convert plane equations -> vertex cloud
	btAlignedObjectArray<btVector3>	vertices;
	btGeometryUtil::getVerticesFromPlaneEquations(planeEquations,vertices);
	BT_CreateWorldBrush(vertices);
}
void BT_ConvertWorldTriSurf(u32 surfNum, u32 contentFlags) {
	if((contentFlags & 1) == 0)
		return;
	cmSurface_c *newSF = new cmSurface_c;
	bt_cmSurfs.push_back(newSF); // we'll need to free it later
	g_bspPhysicsLoader->getTriangleSurface(surfNum,*newSF);
	BT_CreateWorldTriMesh(*newSF);
}
void BT_ConvertWorldBezierPatch(u32 surfNum, u32 contentFlags) {
	if((contentFlags & 1) == 0)
		return;
	cmSurface_c *newSF = new cmSurface_c;
	bt_cmSurfs.push_back(newSF); // we'll need to free it later
	g_bspPhysicsLoader->convertBezierPatchToTriSurface(surfNum,2,*newSF);
	BT_CreateWorldTriMesh(*newSF);
}
void BT_TestSpawnInternalRagDoll(const vec3_c &at) {
	RagDoll *d = new RagDoll(dynamicsWorld,btVector3(at.x,at.y,at.z));
}
arraySTD_c<aabb> g_inlineModelBounds;
const class aabb &G_GetInlineModelBounds(u32 inlineModelNum) {
	if(bt_worldCMod) {
		cmCompound_i *c = bt_worldCMod->getSubModel(inlineModelNum);
		if(c == 0) {
			return aabb();
		}
		// FIXME, this is dirty!
		aabb ret;
		c->getBounds(ret);
		return ret;
	}
	return g_inlineModelBounds[inlineModelNum];
}
class cMod_i *BT_GetSubModelCModel(u32 inlineModelNum) {
	if(bt_worldCMod) {
		u32 realSubModelNum = inlineModelNum-1;
		cmCompound_i *c = bt_worldCMod->getSubModel(realSubModelNum);
		return c;
	}
	return 0;
}
void BT_SpawnStaticCompoundModel(class cmCompound_i *cm) {
	for(u32 i = 0; i < cm->getNumSubShapes(); i++) {
		cMod_i *m = cm->getSubShapeN(i);
		// set the mass to 0, so the body is static
		btRigidBody *staticBody = BT_CreateRigidBodyWithCModel(vec3_origin,vec3_origin,0,m,0);
	}
}
void G_LoadMap(const char *mapName) {
	bt_worldCMod = 0;
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString("G_LoadMap: \"%s\"...",mapName);
	}
	if(!stricmp(mapName,"_empty")) {
		const float worldSize = 4096.f;
		aabb bb;
		bb.fromTwoPoints(vec3_c(-worldSize,-worldSize,-16.f),vec3_c(worldSize,worldSize,0));
		btAlignedObjectArray<btVector3>	vertices;
		for(u32 i = 0; i < 8; i++) {
			vec3_c p = bb.getPoint(i);
			vertices.push_back(btVector3(p.x,p.y,p.z));
		}
		BT_CreateWorldBrush(vertices);
	} else {
		bspPhysicsDataLoader_c l;
		if(l.loadBSPFile(mapName)) {
			str fixed = "maps/";
			fixed.append(mapName);
			fixed.setExtension("map");
			bt_worldCMod = cm->registerModel(fixed);
			if(bt_worldCMod) {
				if(bt_worldCMod->isCompound()) {
					BT_SpawnStaticCompoundModel(bt_worldCMod->getCompound());
				} else {
					BT_CreateRigidBodyWithCModel(vec3_origin,vec3_origin,0,bt_worldCMod,0);
				}
			}
		} else {
			g_bspPhysicsLoader = &l;
			// load world model
			if(l.isCoD1BSP()) {
				l.iterateModelTriSurfs(0,BT_ConvertWorldTriSurf);
			} else {
				l.iterateModelBrushes(0,BT_ConvertWorldBrush);
				l.iterateModelBezierPatches(0,BT_ConvertWorldBezierPatch);
			}
			// load inline models - TODO
			for(u32 i = 0; i < l.getNumInlineModels(); i++) {
				aabb bb;
				l.getInlineModelBounds(i,bb);
				g_inlineModelBounds.push_back(bb);
			}

			l.clear();
			g_bspPhysicsLoader = 0;
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" done.\n");
	}
}
btConvexHullShape *BT_CModelHullToConvex(cmHull_i *h) {
	planeEquations.clear();
	h->iterateSidePlanes(BT_AddBrushPlane2);
	// convert plane equations -> vertex cloud
	btAlignedObjectArray<btVector3>	vertices;
	btGeometryUtil::getVerticesFromPlaneEquations(planeEquations,vertices);
#if 0
	for(u32 i = 0; i < vertices.size(); i++) {
		// TODO: use rotation too
		vertices[i] -= localTrans.getOrigin();
	}
#endif
	if(vertices.size() == 0)
		return 0;
	// this create an internal copy of the vertices
	btConvexHullShape *shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
#if 1
	// This is not needed by physics code itself, but its needed by bt debug drawing.
	// (without it convex shapes edges are messed up)
	shape->initializePolyhedralFeatures();
#endif
	return shape;
}

btConvexHullShape *BT_AABBMinsMaxsToConvex(const aabb &bb) {
	btAlignedObjectArray<btVector3>	vertices;
	vertices.resize(8);
	for(u32 i = 0; i < 8; i++) {
		vertices[i] = bb.getPoint(i).floatPtr();
	}
	if(vertices.size() == 0)
		return 0;
	// this create an internal copy of the vertices
	btConvexHullShape *shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
#if 1
	// This is not needed by physics code itself, but its needed by bt debug drawing.
	// (without it convex shapes edges are messed up)
	shape->initializePolyhedralFeatures();
#endif
	return shape;
}
void BT_AddCModelToCompoundShape(btCompoundShape *compound, const class btTransform &localTrans, class cMod_i *cmodel) {
	if(cmodel->isHull()) {
		cmHull_i *h = cmodel->getHull();
		btConvexHullShape *shape = BT_CModelHullToConvex(h);
		compound->addChildShape(localTrans,shape);
	} else if(cmodel->isCompound()) {
		cmCompound_i *cmCompound = cmodel->getCompound();
		for(u32 i = 0; i < cmCompound->getNumSubShapes(); i++) {
			cMod_i *sub = cmCompound->getSubShapeN(i);
			BT_AddCModelToCompoundShape(compound,localTrans,sub);
		}
	}
}

