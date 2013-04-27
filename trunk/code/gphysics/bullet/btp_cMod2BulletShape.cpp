/*
============================================================================
Copyright (C) 2013 V.

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
// btp_cMod2BulletShape.cpp
#include "btp_headers.h"
#include "btp_convert.h"
#include <api/coreAPI.h>
#include <api/cmAPI.h>
#include <shared/cmSurface.h>

// brush converting
static btAlignedObjectArray<btVector3> planeEquations;
static void BT_AddBrushPlane(const float q3Plane[4]) {
	btVector3 planeEq;
	planeEq.setValue(q3Plane[0],q3Plane[1],q3Plane[2]);
	planeEq[3] = q3Plane[3];
	planeEquations.push_back(planeEq);
}
void BT_ConvertVerticesArrayFromQioToBullet(btAlignedObjectArray<btVector3> &vertices) {
	for(u32 i = 0; i < vertices.size(); i++) {
		vertices[i] *= QIO_TO_BULLET;
	}
}
btConvexHullShape *BT_ConvexHullShapeFromVerticesArray(const btAlignedObjectArray<btVector3> &vertices) {
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
btConvexHullShape *BT_CModelHullToConvex(const cmHull_i *h, const vec3_c *ofs = 0) {
	planeEquations.clear();
	h->iterateSidePlanes(BT_AddBrushPlane);
	// convert plane equations -> vertex cloud
	btAlignedObjectArray<btVector3>	vertices;
	btGeometryUtil::getVerticesFromPlaneEquations(planeEquations,vertices);
	if(ofs) {
		for(u32 i = 0; i < vertices.size(); i++) {
			vertices[i] -= btVector3(ofs->floatPtr());
		}
	}
	BT_ConvertVerticesArrayFromQioToBullet(vertices);
	btConvexHullShape *shape = BT_ConvexHullShapeFromVerticesArray(vertices);
	return shape;
}
btConvexHullShape *BT_CModelTriMeshToConvex(const class cmTriMesh_i *triMesh, const vec3_c *ofs) {
	btAlignedObjectArray<btVector3>	vertices;
	vertices.resize(triMesh->getNumVerts());
	for(u32 i = 0; i < triMesh->getNumVerts(); i++) {
		vec3_c p = triMesh->getVerts()[i];
		if(ofs) {
			p -= *ofs;
		}	
		vertices[i] = (p*QIO_TO_BULLET).floatPtr();
	}
	btConvexHullShape *shape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());
	return shape;
}
btBvhTriangleMeshShape *BT_CMSurfaceToBHV(const class cmSurface_c *sf) {
	if(sf->getNumIndices() == 0 || sf->getNumVerts() == 0) {
		g_core->RedWarning("BT_CMSurfaceToBHV: ignoring empty mesh\n");
		return 0;
	}
	btTriangleIndexVertexArray *mesh = new btTriangleIndexVertexArray;

	sf->prepareScaledVerts(QIO_TO_BULLET);

	btIndexedMesh subMesh;
	subMesh.m_numTriangles = sf->getNumTris();
	subMesh.m_numVertices = sf->getNumVerts();
	subMesh.m_vertexStride = sizeof(vec3_c);
	subMesh.m_vertexType = PHY_FLOAT;
	subMesh.m_vertexBase = (const byte*)sf->getScaledVerts();
	subMesh.m_indexType = PHY_INTEGER;
	subMesh.m_triangleIndexBase = (const byte*)sf->getIndices();
	subMesh.m_triangleIndexStride = sizeof(int)*3;
	mesh->addIndexedMesh(subMesh);

	btBvhTriangleMeshShape* shape;
	if(sf->getNumTris() < 13) {
		// dont build BHV for really small meshes
		shape = new btBvhTriangleMeshShape(mesh,false,false);
	} else {
		// this function is slow, so tell user what we're doing
		if(sf->getNumTris() > 1024) {
			g_core->Print("BT_CMSurfaceToBHV: building BVH for cmSurface with %i tris and %i vertices...\n",sf->getNumTris(),sf->getNumVerts());
		}
		shape = new btBvhTriangleMeshShape(mesh,true);
		if(sf->getNumTris() > 1024) {
			g_core->Print("... done!\n");
		}
	}
	return shape;
}
btBvhTriangleMeshShape *BT_CModelTriMeshToBHV(const class cmTriMesh_i *triMesh) {
	if(triMesh->getNumIndices() == 0 || triMesh->getNumVerts() == 0) {
		g_core->RedWarning("BT_CModelTriMeshToBHV: ignoring empty mesh\n");
		return 0;
	}
	btTriangleIndexVertexArray *mesh = new btTriangleIndexVertexArray;

	triMesh->precacheScaledVerts(QIO_TO_BULLET);

	btIndexedMesh subMesh;
	subMesh.m_numTriangles = triMesh->getNumTris();
	subMesh.m_numVertices = triMesh->getNumVerts();
	subMesh.m_vertexStride = sizeof(vec3_c);
	subMesh.m_vertexType = PHY_FLOAT;
	subMesh.m_vertexBase = (const byte*)triMesh->getScaledVerts();
	subMesh.m_indexType = PHY_INTEGER;
	subMesh.m_triangleIndexBase = (const byte*)triMesh->getIndices();
	subMesh.m_triangleIndexStride = sizeof(int)*3;
	mesh->addIndexedMesh(subMesh);

	btBvhTriangleMeshShape* shape;
	if(triMesh->getNumTris() < 13) {
		// dont build BHV for really small meshes
		shape = new btBvhTriangleMeshShape(mesh,false,false);
	} else {
		// this function is slow, so tell user what we're doing
		if(triMesh->getNumTris() > 1024) {
			g_core->Print("BT_CreateBHVTriMeshForCMSurface: building BVH for cmSurface with %i tris and %i vertices...\n",triMesh->getNumTris(),triMesh->getNumVerts());
		}
		shape = new btBvhTriangleMeshShape(mesh,true);
		if(triMesh->getNumTris() > 1024) {
			g_core->Print("... done!\n");
		}
	}
	return shape;
}


btCollisionShape *BT_CModelToBulletCollisionShape(const class cMod_i *cModel, bool bIsStatic, class vec3_c *extraCenterOfMassOffset) {
	if(cModel->isCompound()) {
		const cmCompound_i *cmCompound = cModel->getCompound();
		u32 numSubShapes = cmCompound->getNumSubShapes();
		if(numSubShapes == 1) {
			return BT_CModelToBulletCollisionShape(cmCompound->getSubShapeN(0),bIsStatic,extraCenterOfMassOffset);
		}
		btCompoundShape *btCompound = new btCompoundShape;
		for(u32 i = 0; i < numSubShapes; i++) {
			const cMod_i *cmSubModel = cmCompound->getSubShapeN(i);
			btCollisionShape *btSubmodel = BT_CModelToBulletCollisionShape(cmSubModel,bIsStatic,extraCenterOfMassOffset);
			if(btSubmodel == 0)
				continue;
			btTransform id;
			id.setIdentity();
			btCompound->addChildShape(id,btSubmodel);
		}
		return btCompound;
	} else if(cModel->isHull()) {
		return BT_CModelHullToConvex(cModel->getHull(),extraCenterOfMassOffset);
	} else if(cModel->isTriMesh()) {
		if(bIsStatic) {
			return BT_CModelTriMeshToBHV(cModel->getTriMesh());
		} else {
			return BT_CModelTriMeshToConvex(cModel->getTriMesh(),extraCenterOfMassOffset);
		}
	}
	return 0;
}
