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
// bt_include.h - local header for Bullet Physics
#ifndef __BT_INLCUDE_H__
#define __BT_INLCUDE_H__

#include <btBulletDynamicsCommon.h>
#include <LinearMath/btGeometryUtil.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>
#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>

extern btBroadphaseInterface* broadphase;
extern btDefaultCollisionConfiguration* collisionConfiguration;
extern btCollisionDispatcher* dispatcher;
extern btSequentialImpulseConstraintSolver* solver;
extern btDiscreteDynamicsWorld* dynamicsWorld;

class btRigidBody *BT_CreateBoxBody(const float *pos, const float *halfSizes, const float *startVel);
class btRigidBody *BT_CreateRigidBodyWithCModel(const float *pos, const float *angles, const float *startVel, class cMod_i *cModel, float mass = 10.f, bool bUseDynamicConvexForTrimeshCMod = false);
void BT_RemoveRigidBody(class btRigidBody *body);
class btRigidBody* BT_CreateRigidBodyInternal(float mass, const class btTransform& startTransform, class btCollisionShape* shape);
void BT_AddCModelToCompoundShape(btCompoundShape *compound, const class btTransform &localTrans,class cMod_i *cmodel);
class btConvexHullShape *BT_CModelHullToConvex(class cmHull_i *h);
btConvexHullShape *BT_AABBMinsMaxsToConvex(const class aabb &bb);
void BT_ConvertVerticesArrayFromQioToBullet(btAlignedObjectArray<btVector3> &vertices);

extern float bt_collisionMargin;

#endif // __BT_INLCUDE_H__