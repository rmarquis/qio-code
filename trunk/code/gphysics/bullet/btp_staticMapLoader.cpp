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
// btp_staticMapLoader.cpp
#include "btp_convert.h"
#include "btp_staticMapLoader.h"
#include "btp_world.h"
#include "btp_cMod2BulletShape.h"
#include <api/vfsAPI.h>
#include <api/cmAPI.h>
#include <shared/bspPhysicsDataLoader.h>
#include <shared/str.h>
#include <shared/cmSurface.h>

static class bspPhysicsDataLoader_c *g_bspPhysicsLoader = 0;
static class btpStaticMapLoader_c *g_staticMap = 0;

// brush converting
static btAlignedObjectArray<btVector3> planeEquations;
static void BT_AddBrushPlane(const float q3Plane[4]) {
	btVector3 planeEq;
	planeEq.setValue(q3Plane[0],q3Plane[1],q3Plane[2]);
	// q3 plane equation is Ax + By + Cz - D = 0, so negate D
	planeEq[3] = -q3Plane[3];
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
	BT_ConvertVerticesArrayFromQioToBullet(vertices);
	g_staticMap->createWorldBrush(vertices);
}
void BT_ConvertWorldPoly(u32 surfNum, u32 contentFlags) {
	if((contentFlags & 1) == 0)
		return;
#if 0
	cmSurface_c *newSF = new cmSurface_c;
	g_bspPhysicsLoader->getTriangleSurface(surfNum,*newSF);
	g_staticMap->createWorldSurface(newSF);
#else
	cmSurface_c newSF;
	g_bspPhysicsLoader->getTriangleSurface(surfNum,newSF);
	g_staticMap->addWorldSurface(newSF);
#endif
}
void BT_ConvertWorldBezierPatch(u32 surfNum, u32 contentFlags) {
	if((contentFlags & 1) == 0)
		return;
	cmSurface_c newSF;
	// convert bezier patches to trimesh data
	g_bspPhysicsLoader->convertBezierPatchToTriSurface(surfNum,3,newSF);
	g_staticMap->addWorldSurface(newSF);
}
// for SourceEngine displacement surfaces
void BT_ConvertWorldDisplacementSurface(u32 surfNum, u32 contentFlags) {
	//if((contentFlags & 1) == 0)
	//	return;
	cmSurface_c newSF;
	// convert bezier patches to trimesh data
	g_bspPhysicsLoader->convertDisplacementToTriSurface(surfNum,newSF);
	g_staticMap->addWorldSurface(newSF);
}
// for SourceEngine static props
void BT_ConvertStaticProp(u32 staticPropNum, u32 contentFlags) {
	//if((contentFlags & 1) == 0)
	//	return;
	cmSurface_c newSF;
	// convert bezier patches to trimesh data
	g_bspPhysicsLoader->convertStaticPropToSurface(staticPropNum,newSF);
	g_staticMap->addWorldSurface(newSF);
}
btBvhTriangleMeshShape *BT_CMSurfaceToBHV(const class cmSurface_c *sf);
btpStaticMapLoader_c::btpStaticMapLoader_c() {
	mainWorldShape = 0;
	mainWorldBody = 0;
	mainWorldSurface_shape = 0;
	mainWorldSurface_body = 0;
}
btpStaticMapLoader_c::~btpStaticMapLoader_c() {

}
bool btpStaticMapLoader_c::loadFromBSPFile(const char *fname) {
	bspPhysicsDataLoader_c l;		
	if(l.loadBSPFile(fname)) {
		return true;
	}
	g_bspPhysicsLoader = &l;
	g_staticMap = this;
	// load static world geometry
	if(l.isCoD1BSP() || l.isHLBSP()) {
		// HL bsps dont have brush data
		// COD bsps have brush data, but we havent reverse engineered it fully yet
		l.iterateModelTriSurfs(0,BT_ConvertWorldPoly);
	} else {
		l.iterateModelBrushes(0,BT_ConvertWorldBrush);
		l.iterateModelBezierPatches(0,BT_ConvertWorldBezierPatch);
		l.iterateModelDisplacementSurfaces(0,BT_ConvertWorldDisplacementSurface);
	}
	l.iterateStaticProps(BT_ConvertStaticProp);
	if(mainWorldSurface.getNumIndices()) {
		mainWorldSurface_shape = BT_CMSurfaceToBHV(&mainWorldSurface);
		mainWorldSurface_body = new btRigidBody(0,0,mainWorldSurface_shape,btVector3(0,0,0));	
		myPhysWorld->getBTDynamicsWorld()->addRigidBody(mainWorldSurface_body);
	}
	return false;
}
bool btpStaticMapLoader_c::loadFromPROCFile(const char *fname) {
	if(mainWorldSurface.loadDoom3ProcFileWorldModel(fname))
	{
		return true;
	}
	if(mainWorldSurface.getNumIndices()) {
		mainWorldSurface_shape = BT_CMSurfaceToBHV(&mainWorldSurface);
		mainWorldSurface_body = new btRigidBody(0,0,mainWorldSurface_shape,btVector3(0,0,0));	
		myPhysWorld->getBTDynamicsWorld()->addRigidBody(mainWorldSurface_body);
	}
	return false;
}
bool btpStaticMapLoader_c::loadFromMAPFile(const char *fname) {
	if(cm == 0)
		return true; // error, cm not present
	cMod_i *m = cm->registerModel(fname);
	if(m == 0)
		return true; // error
	mainWorldShape = BT_CModelToBulletCollisionShape(m,true);
	if(mainWorldShape == 0) {
		return true; // error
	}
	mainWorldBody = new btRigidBody(0,0,mainWorldShape,btVector3(0,0,0));	
	myPhysWorld->getBTDynamicsWorld()->addRigidBody(mainWorldBody);
	return false;
}
void btpStaticMapLoader_c::createWorldBrush(const btAlignedObjectArray<btVector3> &vertices) {
	// create convex hull shape
	class btConvexHullShape *shape = BT_ConvexHullShapeFromVerticesArray(vertices);
	this->shapes.push_back(shape);
	// create static body
	btRigidBody* body = new btRigidBody(0,0,shape,btVector3(0,0,0));	
	myPhysWorld->getBTDynamicsWorld()->addRigidBody(body);
	this->bodies.push_back(body);
}
void btpStaticMapLoader_c::createWorldSurface(class cmSurface_c *sf) {
	surfs.push_back(sf);
	// create trimesh shape
	class btBvhTriangleMeshShape *shape = BT_CMSurfaceToBHV(sf);
	this->shapes.push_back(shape);
	// create static body
	btRigidBody* body = new btRigidBody(0,0,shape,btVector3(0,0,0));	
	myPhysWorld->getBTDynamicsWorld()->addRigidBody(body);
	this->bodies.push_back(body);
}
void btpStaticMapLoader_c::addWorldSurface(class cmSurface_c &sf) {
	mainWorldSurface.addSurface(sf);
}
void btpStaticMapLoader_c::createEmptyMap() {
	// add only single ground plane
	mainWorldShape = new btStaticPlaneShape(btVector3(0,0,1),0);
	mainWorldBody = new btRigidBody(0,0,mainWorldShape,btVector3(0,0,0));	
	myPhysWorld->getBTDynamicsWorld()->addRigidBody(mainWorldBody);
}
bool btpStaticMapLoader_c::loadMap(const char *mapName, class bulletPhysicsWorld_c *pWorld) {
	this->myPhysWorld = pWorld;
	if(!stricmp(mapName,"_empty")) {
		createEmptyMap();
		return false;
	}
	str path = "maps/";
	path.append(mapName);
	path.setExtension("bsp");
	if(g_vfs->FS_FileExists(path)) {
		return loadFromBSPFile(path);
	}
	path.setExtension("proc");
	if(g_vfs->FS_FileExists(path)) {
		return loadFromPROCFile(path);
	}
	path.setExtension("map");
	if(g_vfs->FS_FileExists(path)) {
		return loadFromMAPFile(path);
	}
	// create default empty map
	createEmptyMap();
	return false; // no error
}
void btpStaticMapLoader_c::freeMemory() {
	for(u32 i = 0; i < shapes.size(); i++) {
		delete shapes[i];
	}
	shapes.clear();
	for(u32 i = 0; i < surfs.size(); i++) {
		delete surfs[i];
	}
	surfs.clear();
	for(u32 i = 0; i < bodies.size(); i++) {
		myPhysWorld->getBTDynamicsWorld()->removeRigidBody(bodies[i]);
		delete bodies[i];
	}
	bodies.clear();
	if(mainWorldShape) {
		delete mainWorldShape;
		mainWorldShape = 0;
	}
	if(mainWorldBody) {
		myPhysWorld->getBTDynamicsWorld()->removeRigidBody(mainWorldBody);
		delete mainWorldBody;
		mainWorldBody = 0;
	}
	if(mainWorldSurface_shape) {
		delete mainWorldSurface_shape;
		mainWorldSurface_shape = 0;
	}
	if(mainWorldSurface_body) {
		myPhysWorld->getBTDynamicsWorld()->removeRigidBody(mainWorldSurface_body);
		delete mainWorldSurface_body;
		mainWorldSurface_body = 0;
	}
}
