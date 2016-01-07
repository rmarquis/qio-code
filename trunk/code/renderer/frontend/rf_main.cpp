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
// rf_main.cpp
#include "rf_local.h"
#include "rf_drawCall.h"
#include "rf_world.h"
#include "rf_sunLight.h"
#include <api/coreAPI.h>
#include <api/rEntityAPI.h>
#include <api/mtrAPI.h>
#include <shared/autocvar.h>
#include <math/matrix.h>
#include <math/aabb.h>

static aCvar_c rf_enableMultipassRendering("rf_enableMultipassRendering","0");
static aCvar_c rf_shadows("rf_shadows","0");
static aCvar_c rf_debugMaterialNeedsCPUCheck("rf_debugMaterialNeedsCPUCheck","0");
// draw only mirrored/visible trough portal view
static aCvar_c rf_portalOnly("rf_portalOnly","0");
static aCvar_c rf_skipMirrorAndPortalSubViews("rf_skipMirrorAndPortalSubViews","0");
static aCvar_c rf_useLightmapsWithMultipassRendering("rf_useLightmapsWithMultipassRendering","0");
static aCvar_c rf_maxPortalDepth("rf_maxPortalDepth","3");
static aCvar_c rf_sunShadowMap_boundXY("rf_sunShadowMap_boundXY","1024");
static aCvar_c rf_sunShadowMap_boundZ("rf_sunShadowMap_boundZ","256");
static aCvar_c rf_sunShadowMap_allowSplits("rf_sunShadowMap_allowSplits","1");
static aCvar_c rf_sunShadowMap_splitCount("rf_sunShadowMap_splitCount","3");
static aCvar_c rf_sunShadowMap_showFirstSplitBounds("rf_sunShadowMap_showFirstSplitBounds","0");
static aCvar_c rf_sunShadowMap_showSecondSplitBounds("rf_sunShadowMap_showSecondSplitBounds","0");

// it's in rf_proc.cpp
extern aCvar_c rf_proc_useProcDataToOptimizeLighting;

u32 rf_draw3DViewCount = 0;

bool RF_IsUsingDynamicLights() {
	if(rf_enableMultipassRendering.getInt())
		return true;
	return false;
}

bool RF_IsUsingShadowVolumes() {
	// TODO: see if the stencil buffer is supported
	if(rf_shadows.getInt() == 1) {
		return true;
	}
	return false;
}
bool RF_IsUsingShadowMapping() {
	if(rf_shadows.getInt() == 2) {
		return true;
	}
	return false;
}
int RF_GetShadowingMode() {
	int ret = rf_shadows.getInt();
	return ret;
}
bool RF_IsDrawingPrelitGeometry() {
	if(rf_bDrawingPrelitPath)
		return true;
	return false;
}
bool RF_MaterialNeedsCPU(const class mtrAPI_i *mat) {
	if(mat->hasTexGen()) {
		// see if the texGen effect is supported by renderer backend GPU shaders
		if(rb->gpuTexGensSupported() == false) {
			if(rf_debugMaterialNeedsCPUCheck.getInt()) {
				g_core->Print("RF_MaterialNeedsCPU: material %s from file %s needs CPU because GPU texGens are not supported by current backend\n",
					mat->getName(),mat->getSourceFileName());
			}
			return true;
		}
	}
#if 0
	if(mat->hasRGBGen()) {
		if(rf_debugMaterialNeedsCPUCheck.getInt()) {
			g_core->Print("RF_MaterialNeedsCPU: material %s from file %s needs CPU because GPU rgbGens are not supported by current backend\n",
				mat->getName(),mat->getSourceFileName());
		}
		return true;
	}
#endif
	if(rf_debugMaterialNeedsCPUCheck.getInt()) {
		g_core->Print("RF_MaterialNeedsCPU: material %s from file %s dont need CPU calculations\n",
			mat->getName(),mat->getSourceFileName());
	}
	return false;
}
void RF_AddGenericDrawCalls() {
	RF_AddWorldDrawCalls();
	RFE_AddEntityDrawCalls();
	RF_AddStaticModelDrawCalls();
	RF_AddWorldDecalDrawCalls();
	RF_AddWaterDrawCalls();
	RF_AddExplosionDrawCalls();
	RF_AddCustomDrawCalls();
	// this will add a drawcall only if lightgrid debuging is enabled
	RF_AddLightGridDebugModelDrawCalls();
}
void RF_GenerateDepthBufferOnlySceneDrawCalls() {
	rf_bDrawOnlyOnDepthBuffer = true;
	RF_AddGenericDrawCalls();
	rf_bDrawOnlyOnDepthBuffer = false;
}
void RF_Draw3DSubView(u32 firstDrawCall, u32 numDrawCalls) {
	rb->clearDepthBuffer();
	rb->setupProjection3D(&rf_camera.getProjDef());
	if(rf_camera.getProjDef().fovX == 0.f) {
		g_core->RedWarning("RF_Draw3DSubView: fovX is 0! Did you forget to setup projection?\n");
	}
	rb->setup3DView(rf_camera.getOrigin(), rf_camera.getAxis());
	rb->setIsMirror(rf_camera.isMirror());
	// for recursive mirror/portal views
	rb->setPortalDepth(rf_camera.getPortalDepth());
	rb->setPortalClipPlane(rf_camera.getPortalPlane(),rf_camera.isPortal());
	// issue drawcalls to the renderer backend
	RF_IssueDrawCalls(firstDrawCall,numDrawCalls);
	// do a debug drawing on top of everything
	RF_DoDebugDrawing();
}
bool RF_ShouldUseMultipassRendering() {
	if(rf_enableMultipassRendering.getInt())
		return true; // use multipass rendering
	return false; // don't use it
}
bool RF_ShouldUseLightmapsWithMultipassRendering() {
	if(rf_useLightmapsWithMultipassRendering.getInt()) {
		return true;
	}
	return false;
}
bool RF_IsUsingDynamicSunLight() {
	if(RF_HasSunMaterial())
		return true;
	return false;
}
void RF_SetupAndDrawSunShadowMapSplit(const aabb &baseSunBounds, u32 splitNum, float ext) {
	// get the bounds f
	aabb sunBounds = baseSunBounds;
	vec3_c halfSizes(ext,ext,ext);
	sunBounds.capTo(
		aabb(rf_camera.getOrigin() + halfSizes,rf_camera.getOrigin()-halfSizes
		));

	rf_sunShadowBounds[splitNum] = sunBounds;

	if(splitNum == 0) {
		if(rf_sunShadowMap_showFirstSplitBounds.getInt()) {
			RFDL_AddDebugBB(sunBounds,vec3_c(1,0,0),0.5);
		}
	} else if(splitNum == 1) {
		if(rf_sunShadowMap_showSecondSplitBounds.getInt()) {
			RFDL_AddDebugBB(sunBounds,vec3_c(0,1,0),0.5);
		}
	}

	rf_currentSunBounds = sunBounds;
	rf_currentShadowMapLOD = splitNum;
	RF_AddGenericDrawCalls(); // TODO: culling
}
void RF_Generate3DSubView() {
	u32 firstDrawCall = RF_GetCurrentDrawcallsCount();
	if(RF_ShouldUseMultipassRendering() == false) { 
		RF_AddGenericDrawCalls();
	} else {
		if(RF_ShouldUseLightmapsWithMultipassRendering() ) {
			// generate prelit world drawcalls
			// (using lightmaps + dynamic lights)
			rf_bDrawingPrelitPath = true;
			RF_AddGenericDrawCalls();
			rf_bDrawingPrelitPath = false;
		} else {
			// draw on depth buffer
			// (100% dynamic lighting)
			RF_GenerateDepthBufferOnlySceneDrawCalls();
		}	
		if(RF_IsUsingDynamicSunLight()) {
			rf_bDrawingSunLightPass = true;
			rSunLight_c *sl = RF_GetSunLight();
			if(RF_IsUsingShadowVolumes()) {
				sl->addSunLightShadowVolumes();
			} else {
				sl->freeSunLightShadowVolumes();
				if(RF_IsUsingShadowMapping()) {
					aabb worldBounds;
					RF_GetSunWorldBounds(worldBounds);
					if(worldBounds.isValid() == false) {
						RF_GetEntitiesBounds(worldBounds);
					}

					aabb baseSunBounds;
					rf_camera.getFrustum().getBounds(baseSunBounds);
					baseSunBounds.capTo(worldBounds);

					rf_bDrawingSunShadowMapPass = true;
					rf_bDrawOnlyOnDepthBuffer = true;
					if(rf_sunShadowMap_allowSplits.getInt()) {
						if(rf_sunShadowMap_splitCount.getInt() == 2) {
							float lod0Exts = 512;
							float lod1Exts = 4096;

							RF_SetupAndDrawSunShadowMapSplit(baseSunBounds,0,lod0Exts);
							RF_SetupAndDrawSunShadowMapSplit(baseSunBounds,1,lod1Exts);
						} else {
							float lod0Exts = 300;
							float lod1Exts = 3000;
							float lod2Exts = 12000;

							RF_SetupAndDrawSunShadowMapSplit(baseSunBounds,0,lod0Exts);
							RF_SetupAndDrawSunShadowMapSplit(baseSunBounds,1,lod1Exts);
							RF_SetupAndDrawSunShadowMapSplit(baseSunBounds,2,lod2Exts);
						}
					} else {
						aabb sunBounds = baseSunBounds;
						aabb cap;
						vec3_c delta(rf_sunShadowMap_boundXY.getFloat(),rf_sunShadowMap_boundXY.getFloat(),rf_sunShadowMap_boundZ.getFloat());
						cap.fromTwoPoints(rf_camera.getOrigin() + delta,rf_camera.getOrigin()-delta);
						sunBounds.capTo(cap);

						rf_currentSunBounds = sunBounds;
						rf_sunShadowBounds[0] = rf_currentSunBounds;

						RF_AddGenericDrawCalls();
					}
					rf_bDrawingSunShadowMapPass = false;
					rf_bDrawOnlyOnDepthBuffer = false;
					rf_currentShadowMapLOD = -1;
				}
			}
			RF_AddGenericDrawCalls();
			rf_bDrawingSunLightPass = false;
		}
		// add drawcalls of light interactions
		RFL_AddLightInteractionsDrawCalls();
	}
	u32 lastDrawCall = RF_GetCurrentDrawcallsCount();
	u32 numDrawCalls = lastDrawCall - firstDrawCall;
	// sort the drawcalls generated for this subview
	// (opaque surfaces must be drawn first, then translucent ones)
	RF_SortDrawCalls(firstDrawCall,numDrawCalls);
	// check the drawcalls for mirror/portals surfaces.
	// this might call another RF_Generate3DSubView
	// instance recursively
	//if(rf_camera.isPortal() == false) {
	if(rf_camera.getPortalDepth() < rf_maxPortalDepth.getInt()) {
		if(rf_skipMirrorAndPortalSubViews.getInt()==0) {
			RF_CheckDrawCallsForMirrorsAndPortals(firstDrawCall,numDrawCalls);
		}
	}
	// finally, send the drawcalls to the renderer backend
	if(rf_portalOnly.getInt() && (rf_camera.isPortal()==false)) {

	} else {
		RF_Draw3DSubView(firstDrawCall,numDrawCalls);
	}
}
void RF_Draw3DView() {
	rf_draw3DViewCount++;
	RF_Generate3DSubView();
	RF_DrawCallsEndFrame();
}
static aCvar_c rf_printCullEntitySpaceBounds("rf_printCullEntitySpaceBounds","0");

enum cullResult_e RF_CullEntitySpaceBounds(const aabb &bb) {
	if(rf_currentEntity == 0) {
		// if rf_currentEntity is NULL, we're using world space
		return rf_camera.getFrustum().cull(bb);
	}
	const matrix_c &mat = rf_currentEntity->getMatrix();
	aabb transformed;
	mat.transformAABB(bb,transformed);
	if(rf_printCullEntitySpaceBounds.getInt()) {
		g_core->Print("RF_CullEntitySpaceBounds: entitySpace bounds: %f %f %f, %f %f %f\n"
			"worldSpace bounds: %f %f %f, %f %f %f\n",
			bb.mins.x,bb.mins.y,bb.mins.z,bb.maxs.x,bb.maxs.y,bb.maxs.z,
			transformed.mins.x,transformed.mins.y,transformed.mins.z,transformed.maxs.x,transformed.maxs.y,transformed.maxs.z);
	}

	return rf_camera.getFrustum().cull(transformed);
}
void RF_OnRFShadowsCvarModified(const class aCvar_c *cv) {
	if(cv->getInt()) {
		// user has enabled on rf_shadows
		RFL_RecalculateLightsInteractions();
	}
}
void RF_OnRFEnableMultipassRenderingCvarModified(const class aCvar_c *cv) {
	if(cv->getInt()) {
		// user has enabled rf_multipassRendering
		RFL_RecalculateLightsInteractions();
	}
}
void RF_OnRFUseProcDataToOptimizeLightingCvarModified(const class aCvar_c *cv) {
	g_core->RedWarning("Running full light interactions rebuild...\n");
	RFL_RecalculateLightsInteractions();
}
void RF_InitMain() {
	rf_shadows.setExtraModificationCallback(RF_OnRFShadowsCvarModified);
	rf_enableMultipassRendering.setExtraModificationCallback(RF_OnRFEnableMultipassRenderingCvarModified);
	rf_proc_useProcDataToOptimizeLighting.setExtraModificationCallback(RF_OnRFUseProcDataToOptimizeLightingCvarModified);
}

