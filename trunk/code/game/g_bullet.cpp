#include "g_local.h"
#include <api/cmAPI.h>
#include <math/quat.h>
#include <math/aabb.h>
#include "classes/ModelEntity.h"
#include <shared/cmSurface.h>
#include <api/coreAPI.h>

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
	btContactSolverInfo &csi = dynamicsWorld->getSolverInfo();
	//matrix_c tmp;
	//tmp.identity();
	//aabb bb;
	//bb.addPoint(8,8,8);
	//bb.addPoint(-8,-8,64);
	//BT_IsInSolid(tmp, bb);
}
// this is a (temporary?) fix to objects (especially barrels) jittering.
float bt_collisionMargin = 4.f;

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
	dynamicsWorld->removeCollisionObject(c->getGhostObject());
	delete c->getGhostObject();
	delete c;
}
//btRigidBody *BT_CreateBoxBody(const float *pos, const float *halfSizes, const float *startVel) {
//	btBoxShape* boxShape = new btBoxShape(btVector3(halfSizes[0], halfSizes[1], halfSizes[2]));
//	boxShape->initializePolyhedralFeatures();
//		
//	const btVector3 btStart(pos[0], pos[1], pos[2]);
//
//	btTransform startTransform;
//	startTransform.setIdentity();
//	startTransform.setOrigin(btStart);
//
//	float mass = 10.0f;
//
//	// rigidbody is dynamic if and only if mass is non zero, otherwise static
//	bool isDynamic = (mass != 0.f);
//
//	btVector3 localInertia(0, 0, 0);
//	if (isDynamic) 
//	{
//		boxShape->calculateLocalInertia(mass, localInertia);
//	}
//
//	btRigidBody *body = BT_CreateRigidBodyInternal(mass, startTransform, boxShape);
//	body->setLinearFactor(btVector3(1, 1, 1));
//		
////	body->getWorldTransform().setOrigin(btStart);
//
//	if(startVel) {
//		btVector3 vel(startVel[0], startVel[1], startVel[2]);
//		vel *= 150;
//
//		body->setLinearVelocity(vel);
//	}
//		
//	body->setAngularVelocity(btVector3(0,0,0));
//	body->setContactProcessingThreshold(1e30);
//		
//	//enable CCD if the object moves more than 1 meter in one simulation frame
//	//rigidBody.setCcdSweptSphereRadius(20);
//
//	//if (g_physUseCCD.integer)
//	{
//		body->setCcdMotionThreshold(halfSizes[0]);
//		body->setCcdSweptSphereRadius(6);
//	}
//	return body;
//}
btRigidBody *BT_CreateRigidBodyWithCModel(const float *pos, const float *angles, const float *startVel, cMod_i *cModel) {
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
	}
	
	

		
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
		shape->calculateLocalInertia(mass, localInertia);
	}

	btRigidBody *body = BT_CreateRigidBodyInternal(mass, startTransform, shape);
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
	//if((contentFlags & 1) == 0)
	//	return;
	planeEquations.clear();
	g_bspPhysicsLoader->iterateBrushPlanes(brushNum,BT_AddBrushPlane);
	// convert plane equations -> vertex cloud
	btAlignedObjectArray<btVector3>	vertices;
	btGeometryUtil::getVerticesFromPlaneEquations(planeEquations,vertices);
	BT_CreateWorldBrush(vertices);
}
void BT_ConvertWorldBezierPatch(u32 surfNum, u32 contentFlags) {
	if((contentFlags & 1) == 0)
		return;
	cmSurface_c *newSF = new cmSurface_c;
	bt_cmSurfs.push_back(newSF); // we'll need to free it later
	g_bspPhysicsLoader->convertBezierPatchToTriSurface(surfNum,2,*newSF);
	BT_CreateWorldTriMesh(*newSF);
}
arraySTD_c<aabb> g_inlineModelBounds;
const class aabb &G_GetInlineModelBounds(u32 inlineModelNum) {
	return g_inlineModelBounds[inlineModelNum];
}
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
	bspPhysicsDataLoader_c l;
	if(l.loadBSPFile(mapName)) {

		return;
	}
	g_bspPhysicsLoader = &l;
	// load world model
	l.iterateModelBrushes(0,BT_ConvertWorldBrush);
	l.iterateModelBezierPatches(0,BT_ConvertWorldBezierPatch);
	// load inline models - TODO
	for(u32 i = 0; i < l.getNumInlineModels(); i++) {
		aabb bb;
		l.getInlineModelBounds(i,bb);
		g_inlineModelBounds.push_back(bb);
	}

	l.clear();
	g_bspPhysicsLoader = 0;
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

