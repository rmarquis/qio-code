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
#include <api/coreAPI.h>
#include <api/cmAPI.h>

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

#if 1
#define TRYTOFIX_INTERNAL_EDGES
#endif

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

	dynamicsWorld->getSolverInfo().m_splitImpulse = true;

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
	} else {
		return 0;
	}
	
	

		
	const btVector3 btStart(pos[0], pos[1], pos[2]);

	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btStart);

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
	/*	vel *= 150;*/

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
arraySTD_c<aabb> g_inlineModelBounds;
const class aabb &G_GetInlineModelBounds(u32 inlineModelNum) {
	return g_inlineModelBounds[inlineModelNum];
}
void BT_SpawnStaticCompoundModel(class cmCompound_i *cm) {
	for(u32 i = 0; i < cm->getNumSubShapes(); i++) {
		cMod_i *m = cm->getSubShapeN(i);
		// set the mass to 0, so the body is static
		btRigidBody *staticBody = BT_CreateRigidBodyWithCModel(vec3_origin,vec3_origin,0,m,0);
	}
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
		str fixed = "maps/";
		fixed.append(mapName);
		fixed.setExtension("map");
		cMod_i *worldCMod = cm->registerModel(fixed);
		if(worldCMod) {
			if(worldCMod->isCompound()) {
				BT_SpawnStaticCompoundModel(worldCMod->getCompound());
			} else {
				BT_CreateRigidBodyWithCModel(vec3_origin,vec3_origin,0,worldCMod,0);
			}
		}
		return;
	}
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

