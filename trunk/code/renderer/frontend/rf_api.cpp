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
// rf_api.cpp - renderer DLL entry point

#include "rf_local.h"
#include "rf_drawCall.h"
#include "rf_model.h"
#include "rf_anims.h"
#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>
#include <api/ddAPI.h>
#include <api/moduleManagerAPI.h>
#include <api/materialSystemAPI.h>
#include <api/gameAPI.h> // only for debug drawing
#include <api/modelLoaderDLLAPI.h>
#include <math/matrix.h>
#include <math/axis.h>
#include <shared/autoCvar.h>

#include "rf_2d.h"
#include "rf_world.h"
#include "rf_anims.h"

cameraDef_c rf_camera;
int rf_curTimeMsec;

class rAPIImpl_c : public rAPI_i {
	moduleAPI_i *materialSystemDLL;
	bool initialized;
	projDef_s projDef;

	void unloadMaterialSystem() {
		if(materialSystemDLL == 0) {
			// should never happen
			g_core->Print("rAPIImpl_c::unloadMaterialSystem: materialSystemDLL is already unloaded\n");
			return;
		}
		g_ms->shutdownMaterialsSystem();
		g_moduleMgr->unload(&materialSystemDLL);
	}
	void loadMaterialSystem() {
		if(materialSystemDLL) {
			// should never happen
			g_core->Print("rAPIImpl_c::unloadMaterialSystem: materialSystemDLL is already loaded\n");
			return;
		}
		materialSystemDLL = g_moduleMgr->load("materialSystem");
		if(materialSystemDLL == 0) {
			g_core->DropError("Cannot load materialSystem DLL");
		}
		g_iFaceMan->registerIFaceUser(&g_ms,MATERIALSYSTEM_API_IDENTSTR);
		g_ms->initMaterialsSystem();
	}
public:
	rAPIImpl_c() {
		materialSystemDLL = 0;
		initialized = false;
	}
	// functions called every frame
	virtual void beginFrame() {
		rb->beginFrame();
		rb->setColor4(0);
	}
	virtual void setupProjection3D(const projDef_s *pd) {
		if(pd == 0) {
			projDef.setDefaults();
		} else {
			projDef = *pd;
		}
		rb->setupProjection3D(&projDef);
	}
	virtual void setRenderTimeMsec(int msec) {
		rf_curTimeMsec = msec;
	}
	virtual void setup3DView(const class vec3_c &newCamPos, const vec3_c &newCamAngles) {
		//camPos = newCamPos;
		//camAngles = newCamAngles;
		axis_c camAxis;
		camAxis.fromAngles(newCamAngles);
		rf_camera.setup(newCamPos,camAxis,projDef);
		rb->setup3DView(newCamPos, camAxis);
	}
	// used while drawing world surfaces and particles
	virtual void setupWorldSpace() {
		rb->setupWorldSpace();
	}
	// used while drawing entities
	virtual void setupEntitySpace(const class axis_c &axis, const class vec3_c &origin) {
		rb->setupEntitySpace(axis,origin);
	}
	//virtual void registerRenderableForCurrentFrame(class iRenderable_c *r) = 0;
	virtual void draw3DView() {
		// generate drawcalls
		RF_AddWorldDrawCalls();
		RFE_AddEntityDrawCalls();
		RF_AddWorldDecalDrawCalls();
		// first draw sky (without writing to the depth buffer)
		RF_DrawSky();
		// sort and issue drawcalls (transparency rendering)
		RF_SortAndIssueDrawCalls();
		// do a debug drawing on top of everything
		RF_DoDebugDrawing();
	}
	virtual void setup2DView() {
		draw3DView(); // fixme
		rb->setup2DView();
	}
	virtual void set2DColor(const float *rgba) {
		r_2dCmds.addSetColorCmd(rgba);
	}
	virtual void drawStretchPic(float x, float y, float w, float h,
		float s1, float t1, float s2, float t2, class mtrAPI_i *material) {
		r_2dCmds.addDrawStretchPic(x, y, w, h, s1, t1, s2, t2, material);
	}
	virtual void endFrame() {
		setup2DView();
		r_2dCmds.executeCommands();
		rb->endFrame();
	}
	// misc functions
	virtual void clearEntities() {
		RFE_ClearEntities();
	}
	virtual void loadWorldMap(const char *mapName)  {
		g_core->Print(S_COLOR_RED"rAPIImpl_c::loadWorldMap: %s\n",mapName);
		RF_LoadWorldMap(mapName);
	}
	virtual bool rayTraceWorldMap(class trace_c &tr) {
		return RF_RayTraceWorld(tr);
	}	
	virtual void setAreaBits(const byte *bytes, u32 numBytes) {
		RF_SetWorldAreaBits(bytes, numBytes);
	}
	virtual class rEntityAPI_i *allocEntity() {
		return RFE_AllocEntity();
	}
	virtual void removeEntity(class rEntityAPI_i *ent) {
		RFE_RemoveEntity(ent);
	}
	virtual int addWorldMapDecal(const vec3_c &pos, const vec3_c &normal, float radius, class mtrAPI_i *material) {
		return RF_AddWorldMapDecal(pos,normal,radius,material);
	}
	virtual class mtrAPI_i *registerMaterial(const char *matName) {
		if(g_ms == 0)
			return 0;
		return g_ms->registerMaterial(matName);
	}	
	virtual class rModelAPI_i *registerModel(const char *modName) {
		return RF_RegisterModel(modName);
	}
	virtual const class skelAnimAPI_i *registerAnimation_getAPI(const char *animName) {
		return RF_RegisterAnimation_GetAPI(animName);
	}
	virtual class rDebugDrawer_i *getDebugDrawer() {
		return r_dd;
	}
	virtual u32 addDebugLine(const vec3_c &from, const vec3_c &to, const vec3_c &color, float life) {
		return RFDL_AddDebugLine(from,to,color,life);
	}
	virtual void init() {
		if(initialized) {
			g_core->DropError("rAPIImpl_c::init: already initialized\n");
		}
		initialized = true;
		AUTOCVAR_RegisterAutoCvars();
		loadMaterialSystem();
		rb->init();
		RF_InitSky();
		RF_InitDecals();
	}
	virtual void endRegistration() {

	}
	virtual void shutdown(bool destroyWindow) {
		if(initialized == false) {
			g_core->DropError("rAPIImpl_c::shutdown: not initialized\n");
		}
		initialized = false;	
		RF_ClearWorldMap();
		RFE_ClearEntities();
		RF_ClearAnims();
		RF_ClearModels();
		RF_ShutdownDecals();
		unloadMaterialSystem();
		AUTOCVAR_UnregisterAutoCvars();
		if(destroyWindow) {
			rb->shutdown();
		}
	}
	virtual u32 getWinWidth() const {
		return rb->getWinWidth();
	}
	virtual u32 getWinHeight() const {
		return rb->getWinHeight();
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
rbAPI_i *rb = 0;
moduleManagerAPI_i *g_moduleMgr = 0;
materialSystemAPI_i *g_ms = 0;
modelLoaderDLLAPI_i *g_modelLoader = 0;
// game module api - only for debug drawing on non-dedicated server
gameAPI_s *g_game = 0;

// exports
static rAPIImpl_c g_staticRFAPI;
rAPI_i *rf = &g_staticRFAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)rf,RENDERER_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rb,RENDERER_BACKEND_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_moduleMgr,MODULEMANAGER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_ms,MATERIALSYSTEM_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_game,GAME_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_modelLoader,MODELLOADERDLL_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_RENDERER;
}

