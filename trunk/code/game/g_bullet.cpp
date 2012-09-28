#include "g_local.h"

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btGeometryUtil.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

#pragma comment( lib, "BulletCollision_debug.lib" )
#pragma comment( lib, "BulletDynamics_debug.lib" )
#pragma comment( lib, "LinearMath_debug.lib" )

btBroadphaseInterface* broadphase = 0;
btDefaultCollisionConfiguration* collisionConfiguration = 0;
btCollisionDispatcher* dispatcher = 0;
btSequentialImpulseConstraintSolver* solver = 0;
btDiscreteDynamicsWorld* dynamicsWorld = 0;


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
	dynamicsWorld->setGravity(btVector3(0,-10,0));
}
void G_ShudownBullet() {
    // Clean up behind ourselves like good little programmers
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
void G_RunPhysics() {
	float frameTime = level.frameTime;
	dynamicsWorld->stepSimulation(frameTime,10);
}
void BT_CreateWorldBrush(btAlignedObjectArray<btVector3> &vertices) {
	float mass = 0.f;
	btTransform startTransform;
	//can use a shift
	startTransform.setIdentity();
	//this create an internal copy of the vertices
	btCollisionShape* shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
	//m_demoApp->m_collisionShapes.push_back(shape);

	//m_demoApp->localCreateRigidBody(mass, startTransform,shape);

	//rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0,0,0);
	if (isDynamic)
		shape->calculateLocalInertia(mass,localInertia);

	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
#define USE_MOTIONSTATE 1
#ifdef USE_MOTIONSTATE
	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);

	btRigidBody::btRigidBodyConstructionInfo cInfo(mass,myMotionState,shape,localInertia);

	btRigidBody* body = new btRigidBody(cInfo);
	//body->setContactProcessingThreshold(m_defaultContactProcessingThreshold);

#else
	btRigidBody* body = new btRigidBody(mass,0,shape,localInertia);	
	body->setWorldTransform(startTransform);
#endif//

	dynamicsWorld->addRigidBody(body);

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

btKinematicCharacterController* BT_CreateCharacter(float stepHeight,
	vec3_t pos, float characterHeight,  float characterWidth)
{
   btPairCachingGhostObject* ghostObject = new btPairCachingGhostObject();
   btConvexShape* characterShape = new btCapsuleShape(characterWidth,characterHeight);
   btTransform trans;
   trans.setIdentity();
   btVector3 vPos(pos[0],pos[1],pos[2]);
   trans.setOrigin(vPos);
   ghostObject->setWorldTransform(trans);
   ghostObject->setCollisionShape(characterShape);
   btKinematicCharacterController *character = new btKinematicCharacterController (ghostObject,
		characterShape,stepHeight,1);
   character->setUpAxis(2);

   dynamicsWorld->addCollisionObject( ghostObject, btBroadphaseProxy::CharacterFilter,
                     btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

   dynamicsWorld->addCharacter(character);
	dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
   return character;
};
#include "../qcommon/qfiles.h"
void G_LoadMap(const char *mapName) {
#if 0
	FILE *f = fopen(mapName,"rb");
	if(f == 0) {
		char buf[256];
		strcpy(buf,"baseqio/");
		strcat(buf,mapName);
		strcat(buf,".bsp");
		f = fopen(buf,"rb");
	}
	if(f == 0)
		return 0;
	fseek(f,0,SEEK_END);
	int len = ftell(f);
	rewind(f);
	byte *data = (byte*)malloc(len);
	fread(data,1,len,f);
	fclose(f);
#else
	char buf[256];
	strcpy(buf,"maps/");
	strcat(buf,mapName);
	strcat(buf,".bsp");
	fileHandle_t f;
	int len = trap_FS_FOpenFile(buf,&f,fsMode_t::FS_READ);
	byte *data = (byte*)malloc(len);
	trap_FS_Read(data,len,f);
	trap_FS_FCloseFile(f);
#endif


	dheader_t *h = (dheader_t*)data;
	dbrush_t *b = (dbrush_t*)(data + h->lumps[LUMP_BRUSHES].fileofs);
	dbrushside_t *sides = (dbrushside_t*)(data + h->lumps[LUMP_BRUSHSIDES].fileofs);
	dshader_t *mats = (dshader_t*)(data + h->lumps[LUMP_SHADERS].fileofs);
	dplane_t *planes = (dplane_t*)(data + h->lumps[LUMP_PLANES].fileofs);
	int numBrushes = h->lumps[LUMP_BRUSHES].filelen / sizeof(dbrush_t);
	for(int i = 0; i < numBrushes; i++, b++) {
		dshader_t &m = mats[b->shaderNum];
		if((m.contentFlags & 1) == false)
			continue;
		btAlignedObjectArray<btVector3> planeEquations;
		dbrushside_t *s = sides + b->firstSide;
		for(int j = 0; j < b->numSides; j++, s++) {
			dplane_t &plane = planes[s->planeNum];
			btVector3 planeEq;
			planeEq.setValue(plane.normal[0],plane.normal[1],plane.normal[2]);
			planeEq[3] = -plane.dist;
			planeEquations.push_back(planeEq);
		}
		btAlignedObjectArray<btVector3>	vertices;
		btGeometryUtil::getVerticesFromPlaneEquations(planeEquations,vertices);
		BT_CreateWorldBrush(vertices);
	}

	free(data);

}