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
#include "bt_include.h"
#include <api/afDeclAPI.h>
#include <api/declManagerAPI.h>
#include <api/skelAnimAPI.h>
#include <api/modelDeclAPI.h>
#include <api/coreAPI.h>
#include <shared/autoCvar.h>
#include <shared/skelUtils.h>
#include <math/axis.h>


void CU_InitD3TRMBoneConvexPoints(float length, float width, arraySTD_c<vec3_c> &out) {
	float halfLength = length * 0.5f;
	out.resize(5);
	// set vertices
	out[0].set(0.0f, 0.0f, -halfLength);
	out[1].set(0.0f, width * -0.5f, 0.0f);
	out[2].set(width * 0.5f, width * 0.25f, 0.0f);
	out[3].set(width * -0.5f, width * 0.25f, 0.0f);
	out[4].set(0.0f, 0.0f, halfLength);
}

void CU_InitD3TRMCylinderConvexPoints(const aabb &cylBounds, int numSides, arraySTD_c<vec3_c> &out) {
	int n = numSides;
	if ( n < 3 ) {
		n = 3;
	}
	out.resize(n*2);
	vec3_c offset = ( cylBounds.mins + cylBounds.maxs ) * 0.5f;
	vec3_c halfSize = cylBounds.maxs - offset;
	for(u32 i = 0; i < n; i++) {
		// verts
		float angle = (2.0*M_PI) * i / n;
		out[i].x = cos( angle ) * halfSize.x + offset.x;
		out[i].y = sin( angle ) * halfSize.y + offset.y;
		out[i].z = -halfSize.z + offset.z;
		out[n+i].x = out[i].x;
		out[n+i].y = out[i].y;
		out[n+i].z = halfSize.z + offset.z;
	}
}

void CU_InitD3TRMConeConvexPoints(const aabb &coneBounds, int numSides, arraySTD_c<vec3_c> &out) {
	int n = numSides;
	if ( n < 2 ) {
		n = 3;
	}
	out.resize(n + 1);
	vec3_c offset = ( coneBounds.mins + coneBounds.maxs ) * 0.5f;
	vec3_c halfSize = coneBounds.maxs - offset;
	out[n].set( 0.0f, 0.0f, halfSize.z + offset.z );
	for (u32 i = 0; i < n; i++) {
		// verts
		float angle = (2.0*M_PI) * i / n;
		out[i].x = cos( angle ) * halfSize.x + offset.x;
		out[i].y = sin( angle ) * halfSize.y + offset.y;
		out[i].z = -halfSize.z + offset.z;
	}
}


void CU_InitD3TRMDodecahedronConvexPoints(const aabb &dodBounds, arraySTD_c<vec3_c> &out) {
	float s, d;
	vec3_c a, b, c;

	a[0] = a[1] = a[2] = 0.5773502691896257f; // 1.0f / ( 3.0f ) ^ 0.5f;
	b[0] = b[1] = b[2] = 0.3568220897730899f; // ( ( 3.0f - ( 5.0f ) ^ 0.5f ) / 6.0f ) ^ 0.5f;
	c[0] = c[1] = c[2] = 0.9341723589627156f; // ( ( 3.0f + ( 5.0f ) ^ 0.5f ) / 6.0f ) ^ 0.5f;
	d = 0.5f / c[0];
	s = ( dodBounds.maxs[0] - dodBounds.mins[0] ) * d;
	a[0] *= s;
	b[0] *= s;
	c[0] *= s;
	s = ( dodBounds.maxs[1] - dodBounds.mins[1] ) * d;
	a[1] *= s;
	b[1] *= s;
	c[1] *= s;
	s = ( dodBounds.maxs[2] - dodBounds.mins[2] ) * d;
	a[2] *= s;
	b[2] *= s;
	c[2] *= s;

	vec3_c offset = ( dodBounds.mins + dodBounds.maxs ) * 0.5f;

	// set vertices
	out.resize(20);
	out[ 0].set( offset.x + a[0], offset.y + a[1], offset.z + a[2] );
	out[ 1].set( offset.x + a[0], offset.y + a[1], offset.z - a[2] );
	out[ 2].set( offset.x + a[0], offset.y - a[1], offset.z + a[2] );
	out[ 3].set( offset.x + a[0], offset.y - a[1], offset.z - a[2] );
	out[ 4].set( offset.x - a[0], offset.y + a[1], offset.z + a[2] );
	out[ 5].set( offset.x - a[0], offset.y + a[1], offset.z - a[2] );
	out[ 6].set( offset.x - a[0], offset.y - a[1], offset.z + a[2] );
	out[ 7].set( offset.x - a[0], offset.y - a[1], offset.z - a[2] );
	out[ 8].set( offset.x + b[0], offset.y + c[1], offset.z        );
	out[ 9].set( offset.x - b[0], offset.y + c[1], offset.z        );
	out[10].set( offset.x + b[0], offset.y - c[1], offset.z        );
	out[11].set( offset.x - b[0], offset.y - c[1], offset.z        );
	out[12].set( offset.x + c[0], offset.y       , offset.z + b[2] );
	out[13].set( offset.x + c[0], offset.y       , offset.z - b[2] );
	out[14].set( offset.x - c[0], offset.y       , offset.z + b[2] );
	out[15].set( offset.x - c[0], offset.y       , offset.z - b[2] );
	out[16].set( offset.x       , offset.y + b[1], offset.z + c[2] );
	out[17].set( offset.x       , offset.y - b[1], offset.z + c[2] );
	out[18].set( offset.x       , offset.y + b[1], offset.z - c[2] );
	out[19].set( offset.x       , offset.y - b[1], offset.z - c[2] );
}

class afRagdollSpawner_c {
	boneOrArray_c bones;
	const boneDefArray_c *boneDefs;
	const skelAnimAPI_i *anim;

	
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
	vec3_c getAFVec3Value(const afVec3_s &v) {
		if(v.type == AFVEC3_RAWDATA)
			return v.rawData;
		if(v.type == AFVEC3_BONECENTER) {
			int b0 = anim->getLocalBoneIndexForBoneName(v.boneName);
			int b1 = anim->getLocalBoneIndexForBoneName(v.secondBoneName);
			if(b0 != -1 && b1 != -1) {
				vec3_c p0 = bones[b0].mat.getOrigin();
				vec3_c p1 = bones[b1].mat.getOrigin();
				vec3_c ret;
				ret.lerp(p0,p1,0.5f);
				return ret;
			}
		} 
		if(v.type == AFVEC3_JOINT) {
			int boneNum = anim->getLocalBoneIndexForBoneName(v.boneName);
			if(boneNum != -1) {
				return bones[boneNum].mat.getOrigin();
			}
		} 
		return vec3_c(0,0,0);
	}	
	btCollisionShape *createBulletShapeForAFModel(const afModel_s &m) {
		if(m.type == AFM_BOX) {
			aabb bb;
			bb.mins = getAFVec3Value(m.v[0]);
			bb.maxs = getAFVec3Value(m.v[1]);
			// create convex volume
			btAlignedObjectArray<btVector3>	vertices;
			for(u32 i = 0; i < 8; i++) {
				vec3_c p = bb.getPoint(i);
				vertices.push_back(p.floatPtr());
			}
			// this create an internal copy of the vertices
			btConvexHullShape *shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
#if 1
			// This is not needed by physics code itself, but its needed by bt debug drawing.
			// (without it convex shapes edges are messed up)
			shape->initializePolyhedralFeatures();
#endif
			return shape;
		} else if(m.type == AFM_CYLINDER) {
			arraySTD_c<vec3_c> points;
			aabb bb;
			bb.mins = getAFVec3Value(m.v[0]);
			bb.maxs = getAFVec3Value(m.v[1]);
			CU_InitD3TRMCylinderConvexPoints(bb,m.numSides,points);
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
		} else if(m.type == AFM_BONE) {
			vec3_c v0 = getAFVec3Value(m.v[0]);
			vec3_c v1 = getAFVec3Value(m.v[1]);
			axis_c axis;
			// direction of bone
			axis[2] = v1 - v0;
			float length = axis[2].normalize2();
			// axis of bone trace model
			axis[2].makeNormalVectors( axis[0], axis[1] );
			axis[1] = -axis[1];
			// create bone trace model
			arraySTD_c<vec3_c> points;
			CU_InitD3TRMBoneConvexPoints(length,m.width,points);
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
		} else if(m.type == AFM_CONE) {
			vec3_c v0 = getAFVec3Value(m.v[0]);
			vec3_c v1 = getAFVec3Value(m.v[1]);
			aabb bb;
			bb.mins = v0;
			bb.maxs = v1;
			// place the apex at the origin
			bb.mins.z -= bb.maxs.z;
			bb.maxs.z = 0.0f;

			// create cone trace model
			arraySTD_c<vec3_c> points;
			CU_InitD3TRMConeConvexPoints(bb,m.numSides,points);
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
		} else if(m.type == AFM_DODECAHEDRON) {
			aabb bb;
			bb.mins = getAFVec3Value(m.v[0]);
			bb.maxs = getAFVec3Value(m.v[1]);
			// create dodecahedron trace model
			arraySTD_c<vec3_c> points;
			CU_InitD3TRMDodecahedronConvexPoints(bb,points);
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
		g_core->RedWarning("afRagdollSpawner_c::createBulletShapeForAFModel: failed to spawn model of type %i\n",m.type);
		return 0;
	}
public:
	void spawnRagdollFromAF(const char *afName) {
		afDeclAPI_i *af = g_declMgr->registerAFDecl(afName);
		if(af == 0) {
			g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to find articulatedFigure \"%s\"\n",afName);
			return;
		}
		const afPublicData_s *afd = af->getData();
		const modelDeclAPI_i *model = g_declMgr->registerModelDecl(afd->modelName);
		if(model == 0) {
			g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to find model %s (needed for articulatedFigure \"%s\")\n",afd->modelName.c_str(),afName);
			return;
		}
		anim = model->getSkelAnimAPIForAlias("af_pose");
		if(anim == 0) {
			g_core->RedWarning("afRagdollSpawner_c::spawnRagdollFromAF: failed to find \"af_pose\" animation in model \"%s\" of af \"%s\"\n",
				afd->modelName.c_str(),afName);
			return;
		}
		bones.resize(anim->getNumBones());
		anim->buildFrameBonesABS(0,bones);
		// spawn bullet bodies
		arraySTD_c<btCollisionShape*> shapes;
		arraySTD_c<btRigidBody*> bodies;
		arraySTD_c<matrix_c> matrices;
		matrices.resize(afd->bodies.size());
		bodies.resize(afd->bodies.size());
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
			vec3_c p = getAFVec3Value(b.origin);
			matrix_c mat;
			if(m.type == AFM_BONE) {
				vec3_c v0 = getAFVec3Value(m.v[0]);
				vec3_c v1 = getAFVec3Value(m.v[1]);
				axis_c axis;
				axis[2] = v1 - v0;
				axis[2].normalize();
				axis[2].makeNormalVectors( axis[0], axis[1] );
				axis[1] = -axis[1];
				mat.fromAxisAndOrigin(axis,p);
			} else {
				vec3_c angles = b.angles;
				mat.fromAnglesAndOrigin(angles,p);
			}
			matrices[i] = mat;
			tr.setFromOpenGLMatrix(mat);
			float spawnMass = 10.f;
			bodies[i] = localCreateRigidBody(spawnMass,tr,bm);
		}
		// spawn bullet contraints
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
			dynamicsWorld->addConstraint(bc);
		}
	}
};

void G_SpawnTestRagdollFromAF(const char *afName) {
	afRagdollSpawner_c s;
	s.spawnRagdollFromAF(afName);
}