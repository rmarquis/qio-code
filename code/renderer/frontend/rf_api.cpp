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
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <api/fontAPI.h>
#include <api/rbAPI.h>
#include <api/rdAPI.h>
#include <api/ddAPI.h>
#include <api/clientAPI.h>
#include <api/imgAPI.h>
#include <api/moduleManagerAPI.h>
#include <api/materialSystemAPI.h>
#include <api/gameAPI.h> // only for debug drawing
#include <api/modelLoaderDLLAPI.h>
#include <api/declManagerAPI.h>
#include <api/mtrStageAPI.h>
#include <api/mtrAPI.h>
#include <api/tikiAPI.h>
#include <math/matrix.h>
#include <math/axis.h>
#include <shared/autoCvar.h>
#include <shared/autoCmd.h>
#include <shared/colorTable.h>
#include "../rVertexBuffer.h"
#include "../rIndexBuffer.h"

#include "rf_2d.h"
#include "rf_world.h"
#include "rf_anims.h"

cameraDef_c rf_camera;
int rf_curTimeMsec;
float rf_curTimeSeconds;

static aCvar_c rf_forceZFar("rf_forceZFar","-1");

class rAPIImpl_c : public rAPI_i {
	moduleAPI_i *materialSystemDLL;
	bool initialized;
	projDef_s projDef;
	axis_c camAxis;
	fontAPI_i *defaultFont;

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
		defaultFont = 0;
	}
	~rAPIImpl_c() {

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
		if(rf_forceZFar.getFloat() >= 0.f) {
			projDef.zFar = rf_forceZFar.getFloat();
		}
	}
	virtual void setRenderTimeMsec(int msec) {
		rf_curTimeMsec = msec;
		rf_curTimeSeconds = float(msec)*0.001f;
		rb->setRenderTimeSeconds(rf_curTimeSeconds);
	}
	virtual void setup3DView(const class vec3_c &newCamPos, const vec3_c &newCamAngles, bool thirdPersonRendering) {
		//camPos = newCamPos;
		//camAngles = newCamAngles;
		camAxis.fromAngles(newCamAngles);
		rf_camera.setup(newCamPos,camAxis,projDef,thirdPersonRendering);
	}
	virtual const class axis_c &getCameraAxis() const {
		return camAxis;
	}
	virtual bool doCameraTrace(class trace_c &tr, bool bSkipPlayerModels) {
		return RF_DoCameraTrace(tr,bSkipPlayerModels);
	}
	//// used while drawing world surfaces and particles
	//virtual void setupWorldSpace() {
	//	rb->setupWorldSpace();
	//}
	//// used while drawing entities
	//virtual void setupEntitySpace(const class axis_c &axis, const class vec3_c &origin) {
	//	rb->setupEntitySpace(axis,origin);
	//}
	//virtual void registerRenderableForCurrentFrame(class iRenderable_c *r) = 0;
	virtual void draw3DView() {
		RF_Draw3DView();
	}
	virtual void setup2DView() {
		defaultFont = g_core->GetDefaultFont();
		rb->setup2DView();
	}
	virtual void set2DColor(const float *rgba) {
		r_2dCmds.addSetColorCmd(rgba);
	}
	virtual void drawStretchPic(float x, float y, float w, float h,
		float s1, float t1, float s2, float t2, class mtrAPI_i *material) {
		r_2dCmds.addDrawStretchPic(x, y, w, h, s1, t1, s2, t2, material);
	}
	fontAPI_i *getAnyWorkingFont() {
		return defaultFont;
	}
	virtual float getStringWidth(const char *s) {
		fontAPI_i *f = getAnyWorkingFont();
		if(f) {
			return f->getStringWidth(s);
		}
		return 0.f;
	}
	virtual float drawChar(float x, float y, char s) {
		fontAPI_i *f = getAnyWorkingFont();
		if(f) {
			return f->drawChar(x,y,s);
		}
		return 0.f;
	}
	virtual void drawString(float x, float y, const char *s) {
		fontAPI_i *f = getAnyWorkingFont();
		if(f) {
			f->drawString(x,y,s);
		}
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
	virtual bool loadWorldMap(const char *mapName)  {
		g_core->Print(S_COLOR_RED"rAPIImpl_c::loadWorldMap: %s\n",mapName);
		if(RF_LoadWorldMap(mapName)) {
			g_core->RedWarning("rAPIImpl_c::loadWorldMap: FAILED TO LOAD %s\n",mapName);
			return true; // map loading error
		}
		RF_LoadWorldMapCubeMaps(mapName);
		if(rd) {
			rd->onRenderWorldMapLoaded();
		}
		return false;
	}
	virtual const char *getLoadedMapName() const {
		return RF_GetWorldMapName();
	}
	virtual bool rayTraceWorldMap(class trace_c &tr) {
		return RF_RayTraceWorld(tr);
	}	
	virtual void setAreaBits(const byte *bytes, u32 numBytes) {
		RF_SetWorldAreaBits(bytes, numBytes);
	}
	virtual void setSkyMaterial(const char *skyMaterialName) {
		RF_SetSkyMaterial(skyMaterialName);
	}
	virtual void setWaterLevel(const char *waterLevel) {
		RF_SetWaterLevel(waterLevel);
	}
	virtual class rEntityAPI_i *allocEntity() {
		return RFE_AllocEntity();
	}
	virtual void removeEntity(class rEntityAPI_i *ent) {
		RFE_RemoveEntity(ent);
	}
	virtual class rStaticModelAPI_i *allocStaticModel() {
		return RF_AllocStaticModel();
	}
	virtual class fontAPI_i *registerFont(const char *name) {
		return RF_RegisterFont(name);
	}
	virtual void removeStaticModel(class rStaticModelAPI_i *m) {
		RF_RemoveStaticModel(m);
	}
	virtual class rLightAPI_i *allocLight() {
		return RFL_AllocLight();
	}
	virtual void removeLight(class rLightAPI_i *ent) {
		RFL_RemoveLight(ent);
	}
	virtual u32 getNumAllocatedLights() const {
		return RFL_GetNumAllocatedLights();
	}
	virtual class rLightAPI_i *getLight(u32 i) {
		return RFL_GetLight(i);
	}
	virtual int addWorldMapDecal(const vec3_c &pos, const vec3_c &normal, float radius, class mtrAPI_i *material) {
		return RF_AddWorldMapDecal(pos,normal,radius,material);
	}
	virtual u32 addExplosion(const vec3_c &pos, float radius, const char *matName) {
		return RF_AddExplosion(pos,radius,matName);
	}
	virtual class mtrAPI_i *registerMaterial(const char *matName) {
		if(g_ms == 0)
			return 0;
		return g_ms->registerMaterial(matName);
	}	
	virtual bool isMaterialOrImagePresent(const char *matName) {
		if(g_ms == 0)
			return false;
		return g_ms->isMaterialOrImagePresent(matName);
	}
	virtual u32 getNumAllocatedModels() const {
		return RF_GetNumAllocatedModels();
	}
	virtual const char *getAllocatedModelName(u32 i) const {
		return RF_GetAllocatedModelName(i);
	}
	virtual class rModelAPI_i *registerModel(const char *modName) {
		return RF_RegisterModel(modName);
	}
	virtual const class skelAnimAPI_i *registerAnimation_getAPI(const char *animName) {
		return RF_RegisterAnimation_GetAPI(animName);
	}
	virtual void addCustomRenderObject(class customRenderObjectAPI_i *api) {
		return RF_AddCustomRenderObject(api);
	}
	virtual void removeCustomRenderObject(class customRenderObjectAPI_i *api) {
		return RF_RemoveCustomRenderObject(api);
	}
	virtual class cubeMapAPI_i *getNearestEnvCubeMapImage(const class vec3_c &p) {
		return RF_FindNearestEnvCubeMap_Image(p);
	}
	// this will use the current camera settings
	virtual void getLookatSurfaceInfo(struct rendererSurfaceRef_s &out) {
		RF_GetLookatSurfaceInfo(out);
	}
	virtual void iterateWorldSolidTriangles(class perTriCallback_i *cb) const {
		RF_IterateWorldSolidTriangles(cb);
	}
	virtual void setWorldSurfaceMaterial(const char *matName, int surfNum, int areaNum) {
		RF_SetWorldSurfaceMaterial(areaNum,surfNum,matName);
	}
	virtual void markUsedMaterials() {
		RF_MarkModelMaterials();
		RFE_MarkEntityMaterials();
		RF_MarkStaticModelMaterials();
	}

	virtual class rDebugDrawer_i *getDebugDrawer() {
		return r_dd;
	}
	virtual u32 addDebugLine(const vec3_c &from, const vec3_c &to, const vec3_c &color, float life) {
		return RFDL_AddDebugLine(from,to,color,life);
	}
	virtual void init(bool bEditorMode) {
		if(initialized) {
			g_core->DropError("rAPIImpl_c::init: already initialized\n");
		}
#ifdef _HAS_ITERATOR_DEBUGGING
		g_core->Print("rAPIImpl_c::init(): _HAS_ITERATOR_DEBUGGING is %i\n",_HAS_ITERATOR_DEBUGGING);
#else
		g_core->Print("rAPIImpl_c::init(): _HAS_ITERATOR_DEBUGGING is not defined.\n");
#endif
		initialized = true;
		AUTOCVAR_RegisterAutoCvars();
		AUTOCMD_RegisterAutoConsoleCommands();
		RF_InitMain();
		loadMaterialSystem();
		rb->init(!bEditorMode);
		RF_InitSky();
		RF_InitDecals();
		RFT_InitTerrain();
		RF_InitFonts();
		defaultFont = g_core->GetDefaultFont();
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
		RF_ShutdownSky();
		RF_ShutdownWater();
		RF_ShutdownExplosions();
		RFT_ShutdownTerrain();
		RF_ShutdownFonts();
		unloadMaterialSystem();
		if(g_declMgr) {
			// g_declMgr is not necessary for engine to start.
			g_declMgr->onRendererShutdown();
		}
		AUTOCVAR_UnregisterAutoCvars();
		AUTOCMD_UnregisterAutoConsoleCommands();
		rb->shutdown(destroyWindow);
	}
	virtual u32 getWinWidth() const {
		return rb->getWinWidth();
	}
	virtual u32 getWinHeight() const {
		return rb->getWinHeight();
	}
	virtual class rbAPI_i *getBackend() const {
		return rb;
	}
	// This will setup a rVertexBuffer and rIndexBuffer and call rb->drawElements
	virtual void rbDrawElements_winding(const class vec3_c *p, const class vec2_c *tc, u32 numPoints, u32 stride) {
		rVertexBuffer_c verts;
		verts.resize(numPoints);
		for(u32 i = 0; i < numPoints; i++) {
			verts.setXYZ(i,*p);
			verts.setTC(i,*tc);
			p = (const vec3_c*)(((const byte*)p)+stride);
			tc = (const vec2_c*)(((const byte*)tc)+stride);
		}
		rIndexBuffer_c indices;
		indices.initU16((numPoints-2)*3);
		u32 idx = 0;
		for(u32 i = 2; i < numPoints; i++) {
			indices.setIndex(idx,0);
			idx++;
			indices.setIndex(idx,i-1);
			idx++;
			indices.setIndex(idx,i);
			idx++;
		}
		rb->drawElements(verts,indices);
	}	
	virtual void rbDrawEditorLightShape(const class aabb &inside, const vec3_c *color) {
		vec3_t corners[4];
		float midZ = inside.getMins()[2] + (inside.getMaxs()[2] - inside.getMins()[2]) / 2;

		corners[0][0] = inside.getMins()[0];
		corners[0][1] = inside.getMins()[1];
		corners[0][2] = midZ;

		corners[1][0] = inside.getMins()[0];
		corners[1][1] = inside.getMaxs()[1];
		corners[1][2] = midZ;

		corners[2][0] = inside.getMaxs()[0];
		corners[2][1] = inside.getMaxs()[1];
		corners[2][2] = midZ;

		corners[3][0] = inside.getMaxs()[0];
		corners[3][1] = inside.getMins()[1];
		corners[3][2] = midZ;

		vec3_c top, bottom;

		top[0] = inside.getMins()[0] + ((inside.getMaxs()[0] - inside.getMins()[0]) / 2);
		top[1] = inside.getMins()[1] + ((inside.getMaxs()[1] - inside.getMins()[1]) / 2);
		top[2] = inside.getMaxs()[2];

		bottom = top;
		bottom[2] = inside.getMins()[2];
		rVertexBuffer_c verts;
		vec3_c curColor(1,1,1);
		if(color)
			curColor = *color;
		vec3_c vSave = curColor;
		u16 indices[] = {
			// top sides
			0, 1, 2,
			0, 2, 3,
			0, 3, 4,
			0, 4, 1,
			// bottom sides
			2, 1, 5,
			3, 2, 5,
			4, 3, 5,
			1, 4, 5
		};
		verts.addVertexXYZColor3f(top,curColor);
		for (u32 i = 0; i < 4; i++) {
			curColor *= 0.95;
			verts.addVertexXYZColor3f(corners[i],curColor);
		}
		curColor *= 0.95;
		verts.addVertexXYZColor3f(bottom,curColor);

		rIndexBuffer_c pIndices;
		pIndices.addU16Array(indices,sizeof(indices)/sizeof(indices[0]));
		// get an unique name so this material is used only here
		mtrAPI_i *mat = registerMaterial("(1 0.92016 0.80106)");
		// HACK, so vertex colors are working...
		((mtrStageAPI_i*)mat->getFirstColorMapStage())->setRGBGenVerex();
		rb->setMaterial(mat,0,0);
		rb->drawElements(verts,pIndices);
	}
};
void RF_PrintGFXInfo_f() {
	const char *info = rb->getBackendInfoStr();
	g_core->Print(info);
}

static aCmd_c rf_printGFXInfo("gfxInfo",RF_PrintGFXInfo_f);
static aCmd_c rf_printGFXInfo2("printGFXInfo",RF_PrintGFXInfo_f);

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
clAPI_s *g_client = 0;
rbAPI_i *rb = 0;
moduleManagerAPI_i *g_moduleMgr = 0;
materialSystemAPI_i *g_ms = 0;
modelLoaderDLLAPI_i *g_modelLoader = 0;
// game module api - only for debug drawing on non-dedicated server
gameAPI_s *g_game = 0;
declManagerAPI_i *g_declMgr = 0;
imgAPI_i *g_img = 0;
tikiAPI_i *g_tikiMgr = 0;
rdAPI_i *rd = 0;

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
	g_iFaceMan->registerIFaceUser(&g_declMgr,DECL_MANAGER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_img,IMG_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_client,CLIENT_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_tikiMgr,TIKI_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rd,RD_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_RENDERER;
}

