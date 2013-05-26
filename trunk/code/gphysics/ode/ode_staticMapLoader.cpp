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
// ode_staticMapLoader.cpp
#include "ode_convert.h"
#include "ode_staticMapLoader.h"
#include "ode_world.h"
#include "ode_cMod2OdeGeom.h"
#include <api/vfsAPI.h>
#include <api/cmAPI.h>
#include <api/coreAPI.h>
#include <shared/bspPhysicsDataLoader.h>
#include <shared/str.h>
#include <shared/cmSurface.h>

static class bspPhysicsDataLoader_c *g_bspPhysicsLoader = 0;
static class odeStaticMapLoader_c *g_staticMap = 0;

void ODE_ConvertWorldPoly(u32 surfNum, u32 contentFlags) {
	if((contentFlags & 1) == 0)
		return;
	cmSurface_c newSF;
	g_bspPhysicsLoader->getTriangleSurface(surfNum,newSF);
	g_staticMap->addWorldSurface(newSF);
}
odeStaticMapLoader_c::odeStaticMapLoader_c() {

}
bool odeStaticMapLoader_c::loadFromBSPFile(const char *fname) {
	bspPhysicsDataLoader_c l;		
	if(l.loadBSPFile(fname)) {
		return true;
	}
	g_bspPhysicsLoader = &l;
	g_staticMap = this;
	// load static world geometry
	l.iterateModelTriSurfs(0,ODE_ConvertWorldPoly);
	l.iterateModelBezierPatches(0,ODE_ConvertWorldPoly);

	buildODEWorldTriMesh();
	return false;
}
bool odeStaticMapLoader_c::loadFromMAPFile(const char *fname) {
	cMod_i *m = cm->registerModel(fname);
	if(m == 0)
		return true; // error

	// get raw triangles data for ODE
	m->getRawTriSoupData(&mainWorldSurface);

	buildODEWorldTriMesh();

	return false;
}
void odeStaticMapLoader_c::buildODEWorldTriMesh() {	
	g_core->Print("odeStaticMapLoader_c::buildODEWorldTriMesh: %i indices, %i verts\n",mainWorldSurface.getNumIndices(),mainWorldSurface.getNumVerts());  
	
	if(mainWorldSurface.getNumTris() == 0)
		return;

	mainWorldSurface.swapIndexes();
	mainWorldSurface.prepareScaledVerts(QIO_TO_BULLET);

	worldTriMeshData = dGeomTriMeshDataCreate();
	dGeomTriMeshDataBuildSingle(worldTriMeshData, mainWorldSurface.getScaledVerticesBase(), 3 * sizeof(float),
		mainWorldSurface.getNumVerts(), mainWorldSurface.getIndices(), mainWorldSurface.getNumIndices(), 3 * sizeof(dTriIndex));
	worldTriMesh = dCreateTriMesh(myPhysWorld->getODESpace(), worldTriMeshData, 0, 0, 0);
}
void odeStaticMapLoader_c::addWorldSurface(class cmSurface_c &sf) {
	mainWorldSurface.addSurface(sf);
}
bool odeStaticMapLoader_c::loadMap(const char *mapName, class odePhysicsWorld_c *pWorld) {
	this->myPhysWorld = pWorld;
	str path = "maps/";
	path.append(mapName);
	path.setExtension("bsp");
	if(g_vfs->FS_FileExists(path)) {
		return loadFromBSPFile(path);
	}
	path.setExtension("proc");
	if(g_vfs->FS_FileExists(path)) {
		//return loadPROCFile(path);
	}
	path.setExtension("map");
	if(g_vfs->FS_FileExists(path)) {
		return loadFromMAPFile(path);
	}
	return false; // no error
}
void odeStaticMapLoader_c::freeMemory() {
	//for(u32 i = 0; i < shapes.size(); i++) {
	//	delete shapes[i];
	//}
	//shapes.clear();
	for(u32 i = 0; i < surfs.size(); i++) {
		delete surfs[i];
	}
	surfs.clear();

}
