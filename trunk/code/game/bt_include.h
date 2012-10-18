#ifndef __BT_INLCUDE_H__
#define __BT_INLCUDE_H__

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btGeometryUtil.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

extern btBroadphaseInterface* broadphase;
extern btDefaultCollisionConfiguration* collisionConfiguration;
extern btCollisionDispatcher* dispatcher;
extern btSequentialImpulseConstraintSolver* solver;
extern btDiscreteDynamicsWorld* dynamicsWorld;

class btRigidBody *BT_CreateBoxBody(const float *pos, const float *halfSizes, const float *startVel);
void BT_RemoveRigidBody(class btRigidBody *body);
class btRigidBody* BT_CreateRigidBodyInternal(float mass, const class btTransform& startTransform, class btCollisionShape* shape);

#endif // __BT_INLCUDE_H__