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
// g_ragdoll.cpp
#include "g_local.h"
#include "g_ragdoll.h"
#include "bt_include.h"
#include <api/afDeclAPI.h>
#include <api/declManagerAPI.h>
#include <api/skelAnimAPI.h>
#include <api/modelDeclAPI.h>
#include <api/coreAPI.h>
#include <shared/autoCvar.h>
#include <shared/skelUtils.h>
#include <shared/afRagdollHelper.h>
#include <math/axis.h>

class ragdoll_c : public ragdollAPI_i {
friend class afRagdollSpawner_c;
	// ragdoll definition
	const afDeclAPI_i *af;
	const afPublicData_s *afd;
	//arraySTD_c<matrix_c> afPose;
	// Bullet Physics ragdoll representation
	arraySTD_c<btCollisionShape*> shapes;
	arraySTD_c<btRigidBody*> bodies;
	arraySTD_c<btTypedConstraint*> constraints;
	// bodies transforms, updated every frame
	arraySTD_c<matrix_c> curTransforms;
public:
	ragdoll_c() {
		af = 0;
		afd = 0;
	}
	~ragdoll_c() {
		for(u32 i = 0; i < constraints.size(); i++) {
			dynamicsWorld->removeConstraint(constraints[i]);
			delete constraints[i];
		}
		constraints.clear();
		for(u32 i = 0; i < bodies.size(); i++) {
			dynamicsWorld->removeRigidBody(bodies[i]);
			delete bodies[i];
		}
		bodies.clear();
		for(u32 i = 0; i < shapes.size(); i++) {
			shapes[i];
			delete shapes[i];
		}
		shapes.clear();
	}
	virtual const arraySTD_c<matrix_c> &getCurWorldMatrices() const {
		return curTransforms;
	}
	virtual void updateWorldTransforms() {
		for(u32 i = 0; i < bodies.size(); i++) {
			btRigidBody *b = bodies[i];
			b->getWorldTransform().getOpenGLMatrix(curTransforms[i]);
		}
	}
};

class afRagdollSpawner_c : public afRagdollHelper_c {
	btRigidBody* localCreateRigidBody (btScalar mass, const btTransform& startTransform, btCollisionShape* shape)
	{
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0,0,0);
		if (isDynamic)
			shape->calculateLocalInertia(mass,localInertia);

		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,shape,localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		dynamicsWorld->addRigidBody(body);
		return body;
	}
	btCollisionShape *createBulletShapeForAFModel(const afModel_s &m) {
		arraySTD_c<vec3_c> points;
		if(createConvexPointSoupForAFModel(m,points))
			return 0; // error
		// create convex volume
		btAlignedObjectArray<btVector3>	vertices;
		for(u32 i = 0; i < points.size(); i++) {
			vertices.push_back(points[i].floatPtr());
		}
		// this create an internal copy of the vertices
		btConvexHullShape *shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
#if 1
		// This is not needed by physics code itself, but its needed by bt debug drawing.
		// (without it convex shapes edges are messed up)
		shape->initializePolyhedralFeatures();
#endif
		return shape;
	}
public:
	ragdoll_c *spawnRagdollFromAF(const char *afName, const vec3_c &pos) {
		if(setupRagdollHelper(afName)) {
			g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to setup afRagdollHelper for AF \"%s\"\n",afName);
			return 0;
		}
		matrix_c extraWorldTransform;
		extraWorldTransform.setupOrigin(pos.x,pos.y,pos.z);
		ragdoll_c *ret = new ragdoll_c;
		ret->af = af;
		ret->afd = afd;
		bones.resize(anim->getNumBones());
		anim->buildFrameBonesABS(0,bones);
		// spawn bullet bodies
		arraySTD_c<btCollisionShape*> &shapes = ret->shapes;
		arraySTD_c<btRigidBody*> &bodies = ret->bodies;
		arraySTD_c<matrix_c> matrices;
		matrices.resize(afd->bodies.size());
		bodies.resize(afd->bodies.size());
		ret->curTransforms.resize(afd->bodies.size());
		for(u32 i = 0; i < afd->bodies.size(); i++) {
			const afBody_s &b = afd->bodies[i];
			const afModel_s &m = b.model;

			btCollisionShape *bm = createBulletShapeForAFModel(m);
			if(bm == 0) {
				g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to spawn model for body \"%s\" (%i) of AF \"%s\"\n",
					b.name.c_str(),i,afName);
				bodies[i] = 0;
				continue;
			}
			btTransform tr;
			tr.setIdentity();
			matrix_c mat;
			getBodyTransform(i,mat);
			// add world transform
			mat = extraWorldTransform * mat;
			matrices[i] = mat;
			tr.setFromOpenGLMatrix(mat);
			float spawnMass = 10.f;
			bodies[i] = localCreateRigidBody(spawnMass,tr,bm);
		}
		// spawn bullet contraints
		arraySTD_c<btTypedConstraint*> &constraints = ret->constraints;
		constraints.resize(afd->constraints.size());
		for(u32 i = 0; i < afd->constraints.size(); i++) {
			const afConstraint_s &c = afd->constraints[i];
			int b0Index = afd->getLocalIndexForBodyName(c.body0Name);
			if(b0Index == -1) {
				g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to find first body (\"%s\") of constraint \"%s\" (%i) of AF \"%s\"\n",
					c.body0Name.c_str(),c.name.c_str(),i,afName);
				continue;
			}
			int b1Index = afd->getLocalIndexForBodyName(c.body1Name);
			if(b0Index == -1) {
				g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to find second body (\"%s\") of constraint \"%s\" (%i) of AF \"%s\"\n",
					c.body1Name.c_str(),c.name.c_str(),i,afName);
				continue;
			}
			btRigidBody *b0 = bodies[b0Index];
			if(b0 == 0) {

				continue;
			}
			btRigidBody *b1 = bodies[b1Index];
			if(b1 == 0) {

				continue;
			}
			vec3_c anchor = getAFVec3Value(c.anchor);
			matrix_c cMat;
			cMat.setupOrigin(anchor.x,anchor.y,anchor.z);
			// add world transform
			cMat = extraWorldTransform * cMat;

			matrix_c b0Mat = matrices[b0Index].getInversed()*cMat;
			matrix_c b1Mat = matrices[b1Index].getInversed()*cMat;
			btTransform frameA, frameB;
			frameA.setFromOpenGLMatrix(b0Mat);
			frameB.setFromOpenGLMatrix(b1Mat);
			btGeneric6DofConstraint *bc = new btGeneric6DofConstraint(*b0,*b1,frameA,frameB,false);
			// lock linear transforms
			bc->setLimit(0,0,0);
			bc->setLimit(1,0,0);
			bc->setLimit(2,0,0);
			// free the angular axes
			bc->setLimit(3,-1,0);
			bc->setLimit(4,-1,0);
			bc->setLimit(5,-1,0);
			constraints[i] = bc;
			dynamicsWorld->addConstraint(bc);
		}
		return ret;
	}
};

class ragdollAPI_i *G_SpawnTestRagdollFromAF(const char *afName, const vec3_c &pos) {
	afRagdollSpawner_c s;
	return s.spawnRagdollFromAF(afName, pos);
}