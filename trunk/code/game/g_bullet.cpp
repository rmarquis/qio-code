#include "g_local.h"
#include <api/vfsAPI.h>
#include <api/cmAPI.h>
#include <math/quat.h>
#include <math/aabb.h>
#include "classes/ModelEntity.h"
#include <shared/cmSurface.h>

#include "bt_include.h"

#pragma comment( lib, "BulletCollision_debug.lib" )
#pragma comment( lib, "BulletDynamics_debug.lib" )
#pragma comment( lib, "LinearMath_debug.lib" )

btBroadphaseInterface* broadphase = 0;
btDefaultCollisionConfiguration* collisionConfiguration = 0;
btCollisionDispatcher* dispatcher = 0;
btSequentialImpulseConstraintSolver* solver = 0;
btDiscreteDynamicsWorld* dynamicsWorld = 0;

// they need to be alloced as long as they are used
static arraySTD_c<btTriangleIndexVertexArray*> bt_trimeshes;
static arraySTD_c<cmSurface_c*> bt_cmSurfs;


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
}
void G_RunPhysics() {
	BT_RunVehicles();
	float frameTime = level.frameTime;
	dynamicsWorld->stepSimulation(frameTime,10);
	//BT_RemoveRigidBody(BT_CreateBoxBody(vec3_c(0,0,0),vec3_c(8,8,8),0));
}
#define USE_MOTIONSTATE 1
void BT_CreateWorldBrush(btAlignedObjectArray<btVector3> &vertices) {
	float mass = 0.f;
	btTransform startTransform;
	//can use a shift
	startTransform.setIdentity();
	//this create an internal copy of the vertices
	btCollisionShape* shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());

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
void BT_CreateWorldTriMesh(const cmSurface_c &sf) {
	float mass = 0.f;
	btTransform startTransform;
	//can use a shift
	startTransform.setIdentity();
	//this create an internal copy of the vertices
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
	btCollisionShape* shape = new btBvhTriangleMeshShape(mesh,true);


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

	btRigidBody *body = BT_CreateRigidBodyInternal(mass, startTransform, boxShape);
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
edict_s *BT_CreateBoxEntity(const float *pos, const float *halfSizes, const float *startVel) {
	ModelEntity *e = new ModelEntity;
	e->createBoxPhysicsObject(pos,halfSizes,startVel);
	return e->getEdict();
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
#include <shared/cmBezierPatch.h>
void G_LoadMap(const char *mapName) {
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

		return;
	}
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
	const q3Brush_s *b = h->getBrushes();
	const q3Plane_s *planes = h->getPlanes();
	u32 numBrushes = h->getModels()->numBrushes;
	for(u32 i = 0; i < numBrushes; i++, b++) {
		const q3BSPMaterial_s *m = h->getMat(b->materialNum);
		if((m->contentFlags & 1) == false)
			continue;
		btAlignedObjectArray<btVector3> planeEquations;
		for(int j = 0; j < b->numSides; j++) {
			const q3BrushSide_s *s = h->getBrushSide(b->firstSide+j);
			const q3Plane_s &plane = planes[s->planeNum];
			btVector3 planeEq;
			planeEq.setValue(plane.normal[0],plane.normal[1],plane.normal[2]);
			planeEq[3] = -plane.dist;
			planeEquations.push_back(planeEq);
		}
		btAlignedObjectArray<btVector3>	vertices;
		btGeometryUtil::getVerticesFromPlaneEquations(planeEquations,vertices);
		BT_CreateWorldBrush(vertices);
	}
	// load only bezier patches (brushes are used for collision detection instead of planar surfaces)
	u32 numSurfs = h->getModels()->numSurfaces;
	const q3Surface_s *sf = h->getSurfaces();
	for(u32 i = 0; i < numSurfs; i++) {
		if(sf->surfaceType == Q3MST_PATCH) {
			cmBezierPatch_c bp;
			const q3Vert_s *v = h->getVerts() + sf->firstVert;
			for(u32 j = 0; j < sf->numVerts; j++, v++) {
				bp.addVertex(v->xyz);
			}
			bp.setHeight(sf->patchHeight);
			bp.setWidth(sf->patchWidth);
			cmSurface_c *cmSF = new cmSurface_c;
			bp.tesselate(2,cmSF);
			BT_CreateWorldTriMesh(*cmSF);
			bt_cmSurfs.push_back(cmSF); // we'll need to free it later
		}
		sf = h->getNextSurface(sf);
	}
	free(data);
}