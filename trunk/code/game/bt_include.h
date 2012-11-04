#ifndef __BT_INLCUDE_H__
#define __BT_INLCUDE_H__

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btGeometryUtil.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>

extern btBroadphaseInterface* broadphase;
extern btDefaultCollisionConfiguration* collisionConfiguration;
extern btCollisionDispatcher* dispatcher;
extern btSequentialImpulseConstraintSolver* solver;
extern btDiscreteDynamicsWorld* dynamicsWorld;

class btRigidBody *BT_CreateBoxBody(const float *pos, const float *halfSizes, const float *startVel);
class btRigidBody *BT_CreateRigidBodyWithCModel(const float *pos, const float *angles, const float *startVel, class cMod_i *cModel, float mass = 10.f);
void BT_RemoveRigidBody(class btRigidBody *body);
class btRigidBody* BT_CreateRigidBodyInternal(float mass, const class btTransform& startTransform, class btCollisionShape* shape);
void BT_AddCModelToCompoundShape(btCompoundShape *compound, const class btTransform &localTrans,class cMod_i *cmodel);
class btConvexHullShape *BT_CModelHullToConvex(class cmHull_i *h);

extern float bt_collisionMargin;

#endif // __BT_INLCUDE_H__