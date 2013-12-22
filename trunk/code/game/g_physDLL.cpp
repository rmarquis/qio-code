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
// g_physDLL.cpp
#include <api/iFaceMgrAPI.h>
#include <api/moduleManagerAPI.h>
#include <api/physAPI.h>
#include <api/loadingScreenMgrAPI.h>
#include <api/cmAPI.h>
#include <api/coreAPI.h>
#include <math/vec3.h>
#include <shared/autoCvar.h>
#include "g_local.h"

class physWorldStub_c : public physWorldAPI_i {
	vec3_c g;
public:
	virtual void init(const class vec3_c &newGravity) {
	}
	virtual bool loadMap(const char *mapName) {
		return false;
	}
	virtual void runFrame(float frameTime) {
	}
	virtual void shutdown() {
	}
	virtual class physObjectAPI_i *createPhysicsObject(const struct physObjectDef_s &def) {
		return 0;
	}
	class physConstraintAPI_i *createConstraintBall(const vec3_c &pos, physObjectAPI_i *b0, physObjectAPI_i *b1) {
		return 0;
	}
	class physConstraintAPI_i *createConstraintHinge(const vec3_c &pos, const vec3_c &axis, physObjectAPI_i *b0, physObjectAPI_i *b1) {
		return 0;
	}
	void destroyPhysicsConstraint(physConstraintAPI_i *p) {

	}
	virtual void destroyPhysicsObject(class physObjectAPI_i *p) {
	}
	virtual class physCharacterControllerAPI_i *createCharacter(const class vec3_c &pos, float characterHeight,  float characterWidth) {
		return 0;
	}
	virtual void freeCharacter(class physCharacterControllerAPI_i *p) {
	}
	virtual class physVehicleAPI_i *createVehicle(const vec3_c &pos, const vec3_c &angles, class cMod_i *cm) {
		return 0;
	}
	virtual void removeVehicle(class physVehicleAPI_i *v) {
	}
	virtual void setGravity(const vec3_c &newGravity) {
	}
	virtual const vec3_c &getGravity() const {
		return g;
	}
	virtual bool traceRay(class trace_c &tr) {
		return false;
	}
};
static physWorldStub_c g_physWorldStub;
class physWorldAPI_i *g_physWorld = &g_physWorldStub;
class physDLLAPI_i *g_physAPI = 0;
static class moduleAPI_i *g_physDLL = 0;

static aCvar_c g_runPhysics("g_runPhysics","1");

void G_InitPhysicsEngine() {
	//g_physDLL = g_moduleMgr->load("gphysics_ode");
	g_physDLL = g_moduleMgr->load("gphysics_bullet");
	if(g_physDLL == 0) {
		g_core->RedWarning("G_InitPhysicsEngine: physics module not avaible\n");
	} else {
		g_iFaceMan->registerIFaceUser(&g_physAPI,GPHYSICS_API_IDENTSTR);	
		g_physAPI->initPhysicsSystem();
		g_physWorld = g_physAPI->allocWorld("mainPhysicsWorld");
		g_physWorld->init(vec3_c(0,0,-800.f));
	}
}
void G_LoadMap(const char *mapName) {
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString("G_LoadMap: \"%s\"...",mapName);
	}
	cm->loadMap(mapName);
	g_physWorld->loadMap(mapName);
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" done.\n");
	}
}
void G_ShutdownPhysicsEngine() {
	if(g_physDLL) {
		g_physWorld->shutdown();
		g_physAPI->freeWorld(g_physWorld);
		g_physAPI->shutdownPhysicsSystem();
		g_moduleMgr->unload(&g_physDLL);
	}
}
void G_RunPhysics() {
	if(g_runPhysics.getInt() == 0)
		return;
	g_physWorld->runFrame(level.frameTime);
}
