#include "g_local.h"
#include <api/vfsAPI.h>
#include <api/cmAPI.h>
#include <math/quat.h>

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
	dynamicsWorld->setGravity(btVector3(0,0,-500));

	// add ghostPairCallback for character controller collision detection
	dynamicsWorld->getPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
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
#define USE_MOTIONSTATE 1
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
btRigidBody* CreateRigidBody(float mass, const btTransform& startTransform, btCollisionShape* shape)
{
	btAssert((!shape || shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));

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

	dynamicsWorld->addRigidBody(body);

	return body;
}
void QuatToAngles(const float *q, vec3_t angles)
{
	float          q2[4];

	q2[0] = q[0] * q[0];
	q2[1] = q[1] * q[1];
	q2[2] = q[2] * q[2];
	q2[3] = q[3] * q[3];

	angles[PITCH] = RAD2DEG(asin(-2 * (q[2] * q[0] - q[3] * q[1])));
	angles[YAW] = RAD2DEG(atan2(2 * (q[2] * q[3] + q[0] * q[1]), (q2[2] - q2[3] - q2[0] + q2[1])));
	angles[ROLL] = RAD2DEG(atan2(2 * (q[3] * q[0] + q[2] * q[1]), (-q2[2] - q2[3] + q2[0] + q2[1])));
}

void G_UpdatePhysicsObject(gentity_s *ent) {
	btRigidBody *body = ent->body;
	if(body == 0)
		return;
	btTransform trans;
	body->getMotionState()->getWorldTransform(trans);
	VectorSet(ent->s.origin,trans.getOrigin().x(),trans.getOrigin().y(),trans.getOrigin().z());
	//G_Printf("G_UpdatePhysicsObject: at %f %f %f\n",ent->s.origin[0],ent->s.origin[1],ent->s.origin[2]);
	btQuaternion q = trans.getRotation();
	quat_c q2(q.x(),q.y(),q.z(),q.w());
	QuatToAngles(q2,ent->s.angles);
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

void G_TryToJump(btKinematicCharacterController *ch) {
	if(ch->onGround() == false)
		return;
	ch->jump();
}
void BT_FreeCharacter(btKinematicCharacterController *c) {
	if(c == 0)
		return;
	dynamicsWorld->removeCharacter(c);
	delete c;

}
btRigidBody *BT_CreateBoxBody(const float *pos, const float *halfSizes, const float *startVel) {
	btBoxShape* boxShape = new btBoxShape(btVector3(halfSizes[0], halfSizes[1], halfSizes[2]));
	boxShape->initializePolyhedralFeatures();
		
	const btVector3 btStart(pos[0], pos[1], pos[2]);

	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btStart);

	float mass = 10.0f;

	// rigidbody is dynamic if and only if mass is non zero, otherwise static
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic) 
	{
		boxShape->calculateLocalInertia(mass, localInertia);
	}

	btRigidBody *body = CreateRigidBody(mass, startTransform, boxShape);
	body->setLinearFactor(btVector3(1, 1, 1));
		
//	body->getWorldTransform().setOrigin(btStart);

	if(startVel) {
		btVector3 vel(startVel[0], startVel[1], startVel[2]);
		vel *= 150;

		body->setLinearVelocity(vel);
	}
		
	body->setAngularVelocity(btVector3(0,0,0));
	body->setContactProcessingThreshold(1e30);
		
	//enable CCD if the object moves more than 1 meter in one simulation frame
	//rigidBody.setCcdSweptSphereRadius(20);

	//if (g_physUseCCD.integer)
	{
		body->setCcdMotionThreshold(halfSizes[0]);
		body->setCcdSweptSphereRadius(6);
	}
	return body;
}
void BT_CreateBoxEntity(gentity_s *ent, const float *pos, const float *halfSizes, const float *startVel) {
	ent->body = BT_CreateBoxBody(pos,halfSizes,startVel);
	ent->cmod = cm->registerBoxExts(halfSizes);
	ent->body->setUserPointer(ent);
}
gentity_s *BT_CreateBoxEntity(const float *pos, const float *halfSizes, const float *startVel) {
	gentity_s *e = G_Spawn();
	BT_CreateBoxEntity(e,pos,halfSizes,startVel);
	return e;
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
};
#include "../fileformats/bspFileFormat.h"
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
	int len = g_vfs->FS_FOpenFile(buf,&f,FS_READ);
	byte *data = (byte*)malloc(len);
	g_vfs->FS_Read(data,len,f);
	g_vfs->FS_FCloseFile(f);
#endif


	q3Header_s *h = (q3Header_s*)data;
	q3Brush_s *b = (q3Brush_s*)(data + h->lumps[Q3_BRUSHES].fileOfs);
	q3BrushSide_s *sides = (q3BrushSide_s*)(data + h->lumps[Q3_BRUSHSIDES].fileOfs);
	q3BSPMaterial_s *mats = (q3BSPMaterial_s*)(data + h->lumps[Q3_SHADERS].fileOfs);
	q3Plane_s *planes = (q3Plane_s*)(data + h->lumps[Q3_PLANES].fileOfs);
	int numBrushes = h->lumps[Q3_BRUSHES].fileLen / sizeof(q3Brush_s);
	for(int i = 0; i < numBrushes; i++, b++) {
		q3BSPMaterial_s &m = mats[b->materialNum];
		if((m.contentFlags & 1) == false)
			continue;
		btAlignedObjectArray<btVector3> planeEquations;
		q3BrushSide_s *s = sides + b->firstSide;
		for(int j = 0; j < b->numSides; j++, s++) {
			q3Plane_s &plane = planes[s->planeNum];
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