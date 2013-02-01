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
// rf_drawCall.cpp - drawCalls managment and sorting
#include "rf_local.h"
#include "rf_drawCall.h"
#include "rf_entities.h"
#include "../rVertexBuffer.h"
#include <api/coreAPI.h>
#include <api/rbAPI.h>
#include <api/mtrAPI.h>
#include <api/materialSystemAPI.h>
#include <api/occlusionQueryAPI.h>
#include <api/rLightAPI.h>
#include <shared/array.h>
#include <shared/autoCvar.h>

aCvar_c rf_noLightmaps("rf_noLightmaps","0");
aCvar_c rf_noVertexColors("rf_noVertexColors","0");
aCvar_c rf_ignoreSpecificMaterial("rf_ignoreSpecificMaterial","");
aCvar_c rf_ignoreSpecificMaterial2("rf_ignoreSpecificMaterial2","");
aCvar_c rf_forceSpecificMaterial("rf_forceSpecificMaterial","");
aCvar_c rf_ignoreShadowVolumeDrawCalls("rf_ignoreShadowVolumeDrawCalls","0");

class drawCall_c {
public:
	const char *source; // for debuging, should never be fried
	bool bindVertexColors; // temporary?
	bool drawOnlyOnDepthBuffer;
	class mtrAPI_i *material;
	class textureAPI_i *lightmap; // for bsp surfaces
	const class rVertexBuffer_c *verts;
	const class rIndexBuffer_c *indices;
	enum drawCallSort_e sort;
	class rEntityAPI_i *entity;
	class rLightAPI_i *curLight;
	const class rPointBuffer_c *points; // ONLY for shadow volumes
//public:
	
};
static arraySTD_c<drawCall_c> rf_drawCalls;
static u32 rf_numDrawCalls = 0;
bool rf_bDrawOnlyOnDepthBuffer = false;

void RF_AddDrawCall(const rVertexBuffer_c *verts, const rIndexBuffer_c *indices,
	class mtrAPI_i *mat, class textureAPI_i *lightmap, drawCallSort_e sort,
		bool bindVertexColors) {
	// developers can supress manually some materials for debugging purposes
	if(rf_ignoreSpecificMaterial.strLen() && rf_ignoreSpecificMaterial.getStr()[0] != '0') {
		if(!stricmp(rf_ignoreSpecificMaterial.getStr(),mat->getName())) {
			return;
		}
	}
	if(rf_ignoreSpecificMaterial2.strLen() && rf_ignoreSpecificMaterial2.getStr()[0] != '0') {
		if(!stricmp(rf_ignoreSpecificMaterial2.getStr(),mat->getName())) {
			return;
		}
	}
	// developers can force a specific material on ALL surfaces as well
	if(rf_forceSpecificMaterial.strLen() && rf_forceSpecificMaterial.getStr()[0] != '0') {
		mat = g_ms->registerMaterial(rf_forceSpecificMaterial.getStr());
	}
	// ignore blendfunc surfaces if we're using dynamic lights
	if(rf_curLightAPI) {
		if(mat->hasBlendFunc() || mat->hasAlphaTest()) {
			return;
		}
	}
	drawCall_c *n;
	if(rf_numDrawCalls == rf_drawCalls.size()) {
		n = &rf_drawCalls.pushBack();
	} else {
		n = &rf_drawCalls[rf_numDrawCalls];
	}
	// if we're drawing only on depth buffer
	if(rf_bDrawOnlyOnDepthBuffer) {
		if(mat->hasBlendFunc() || mat->hasAlphaTest()) {
			sort = DCS_BLEND_AFTER_LIGHTING;
			n->drawOnlyOnDepthBuffer = false;
			//return;
		} else {
			n->drawOnlyOnDepthBuffer = true;
		}
	} else {
		n->drawOnlyOnDepthBuffer = false;
	}
	n->verts = verts;
	n->indices = indices;
	n->material = mat;
	if(rf_noLightmaps.getInt()) {
		n->lightmap = 0;
	} else {
		n->lightmap = lightmap;
	}
	n->sort = sort;
	if(rf_noVertexColors.getInt()) {
		n->bindVertexColors = false;
	} else {
		n->bindVertexColors = bindVertexColors;
	}
	n->entity = rf_currentEntity;
	n->curLight = rf_curLightAPI;
	rf_numDrawCalls++;
}
void RF_AddShadowVolumeDrawCall(const class rPointBuffer_c *points, const class rIndexBuffer_c *indices) {
	if(rf_curLightAPI == 0) {
		// should never happen..
		g_core->RedWarning("RF_AddShadowVolumeDrawCall: rf_curLightAPI is NULL!!!\n");
		return;
	}
	if(rf_ignoreShadowVolumeDrawCalls.getInt()) {
		return;
	}
	drawCall_c *n;
	if(rf_numDrawCalls == rf_drawCalls.size()) {
		n = &rf_drawCalls.pushBack();
	} else {
		n = &rf_drawCalls[rf_numDrawCalls];
	}
	n->verts = 0;
	n->points = points;
	n->indices = indices;
	n->material = 0;
	n->lightmap = 0;
	n->sort = DCS_SPECIAL_SHADOWVOLUME;
	n->bindVertexColors = false;
	n->drawOnlyOnDepthBuffer = false;
	n->entity = rf_currentEntity;
	n->curLight = rf_curLightAPI;
	rf_numDrawCalls++;
}
u32 RF_GetCurrentDrawcallsCount() {
	return rf_numDrawCalls;
}

int compareDrawCall(const void *v0, const void *v1) {
	const drawCall_c *c0 = (const drawCall_c *)v0;
	const drawCall_c *c1 = (const drawCall_c *)v1;

	if(c1->sort == DCS_BLEND_AFTER_LIGHTING && c1->sort != c0->sort) {
		return -1; // c0 first
	}
	if(c0->sort == DCS_BLEND_AFTER_LIGHTING && c1->sort != c0->sort) {
		return 1; // c1 first
	}

	if(c0->curLight) {
		if(c1->curLight == 0) {
			return 1; // c1 first
		}
		if(c0->curLight > c1->curLight) {
			return 1;
		}
		if(c0->curLight < c1->curLight) {
			return -1;
		}
		// c0->curLight == c1->curLight
		// light shadow volumes are drawn before light interactions
		if(c0->sort == DCS_SPECIAL_SHADOWVOLUME) {
			if(c1->sort == DCS_SPECIAL_SHADOWVOLUME)
				return 0; // equal
			return -1; // c0 first
		} else if(c1->sort == DCS_SPECIAL_SHADOWVOLUME) {
			return 1; // c1 first
		}
	} else if(c1->curLight) {
		return -1; // c0 first
	}
	if(c0->sort > c1->sort) {
		return 1; // c1 first
	} else if(c0->sort < c1->sort) {
		return -1; // c0 first
	}
#if 1
	if(c0->sort == DCS_PORTAL) {
		// fix for q3dm0 where teleporter portal
		// was visible in mirror...
		vec3_c p = rf_camera.getPVSOrigin();
		vec3_c center0, center1;
		c0->verts->getCenter(*c0->indices,center0);
		c1->verts->getCenter(*c1->indices,center1);
		if(p.distSQ(center0) < p.distSQ(center1)) {
			return -1;
		}
		return 1;
	}
#endif
	// sorts are equal, sort by material pointer
	if(c0->material > c1->material) {
		return -1;
	} else if(c0->material < c1->material) {
		return 1;
	}
	// materials are equal too
	return 0;
}
void RF_SortDrawCalls(u32 firstDrawCall, u32 numDrawCalls) {
	// sort the needed part of the drawCalls array
	qsort(rf_drawCalls.getArray()+firstDrawCall,numDrawCalls,sizeof(drawCall_c),compareDrawCall);
}
void RF_Generate3DSubView();
static aCvar_c light_printLightsCulledByGPUOcclusionQueries("light_printLightsCulledByGPUOcclusionQueries","0");
void RF_IssueDrawCalls(u32 firstDrawCall, u32 numDrawCalls) {
	// issue the drawcalls
	drawCall_c *c = (rf_drawCalls.getArray()+firstDrawCall);
	rEntityAPI_i *prevEntity = 0;
	rLightAPI_i *prevLight = 0;
	for(u32 i = 0; i < numDrawCalls; i++, c++) {
		if(prevEntity != c->entity) {
			if(c->entity == 0) {
				rb->setupWorldSpace();
			} else {
				rb->setupEntitySpace(c->entity->getAxis(),c->entity->getOrigin());
			}
			prevEntity = c->entity;
		}
		if(prevLight != c->curLight) {
			if(prevLight == 0) {
				// depth pass finished
				RFL_AssignLightOcclusionQueries();
			}
			rb->setCurLight(c->curLight);
			prevLight = c->curLight;
		}
		if(c->curLight && RFL_GPUOcclusionQueriesForLightsEnabled() && (c->curLight->getBCameraInside()==false)) {
			// see if the light is culled by GPU occlusion query
			class occlusionQueryAPI_i *oq = c->curLight->getOcclusionQuery();
			if(oq) {
				u32 passed;
#if 1
				if(oq->isResultAvailable()) {
					passed = oq->getNumSamplesPassed();
				} else {
					passed = oq->getPreviousResult();
				}
#else
				passed = oq->waitForLatestResult();
#endif
				if(passed == 0) {
					if(light_printLightsCulledByGPUOcclusionQueries.getInt()) {
						g_core->Print("Skipping drawcall with light %i\n",c->curLight);
					}
					continue;
				}
			}
		}
		rb->setCurrentDrawCallSort(c->sort);
		rb->setBindVertexColors(c->bindVertexColors);
		rb->setBDrawOnlyOnDepthBuffer(c->drawOnlyOnDepthBuffer);
		rb->setMaterial(c->material,c->lightmap);
		if(c->verts) {
			// draw surface
			rb->drawElements(*c->verts,*c->indices);
		} else {
			// draw shadow volume points
			rb->drawIndexedShadowVolume(c->points,c->indices);
		}
	}
	rb->setBindVertexColors(false);	
	rb->setCurLight(0);	
	if(prevEntity) {
		rb->setupWorldSpace();
		prevEntity = 0;
	}
}

// from Q3 SDK, code for mirrors
void R_MirrorPoint(vec3_t in, vec3_t surfaceOrigin, vec3_t cameraOrigin, vec3_t surfaceAxis[3], vec3_t cameraAxis[3], vec3_t out) {
	int             i;
	vec3_t          local;
	vec3_t          transformed;
	float           d;

	VectorSubtract(in, surfaceOrigin, local);

	VectorClear(transformed);
	for(i = 0; i < 3; i++)
	{
		d = DotProduct(local, surfaceAxis[i]);
		VectorMA(transformed, d, cameraAxis[i], transformed);
	}

	VectorAdd(transformed, cameraOrigin, out);
}
void R_MirrorVector(vec3_t in, vec3_t surfaceAxis[3], vec3_t cameraAxis[3], vec3_t out) {
	VectorClear(out);
	for(u32 i = 0; i < 3; i++) {
		float d = DotProduct(in, surfaceAxis[i]);
		VectorMA(out, d, cameraAxis[i], out);
	}
}
void R_MirrorAxis(const axis_c &in, vec3_t surfaceAxis[3], vec3_t cameraAxis[3], axis_c &out) {
	for(u32 i = 0; i < 3; i++) {
		R_MirrorVector(in[i],surfaceAxis,cameraAxis,out[i]);
	}
}
void RF_CheckDrawCallsForMirrorsAndPortals(u32 firstDrawCall, u32 numDrawCalls) {
	// drawcalls are already sorted
	// search for DCS_PORTAL
	for(u32 i = firstDrawCall; i < numDrawCalls; i++) {
		drawCall_c &dc = rf_drawCalls.getArray()[i];
		if(dc.sort > DCS_PORTAL) {
			return; // dont have to check any more
		}
		static rVertexBuffer_c pointsTransformed;
		dc.verts->getReferencedPoints(pointsTransformed,*dc.indices);
		if(dc.entity) {
			pointsTransformed.transform(dc.entity->getMatrix());
		}
		plane_c surfacePlane;
		if(pointsTransformed.getPlane(surfacePlane)) {
			g_core->RedWarning("RF_CheckDrawCallsForMirrorsAndPortals: mirror/portal surface not planar!\n");
			continue;
		}
		float cameraDist = surfacePlane.distance(rf_camera.getOrigin());
		if(cameraDist > 0) {
			continue; // camera is behind surface plane
		}
		//g_core->Print("Found DCS_PORTAL drawCall with material %s (abs index %i)\n",dc.material->getName(),i);
		// do the automatic mirror for now 
		if(dc.material->isMirrorMaterial() || !stricmp(dc.material->getName(),"textures/common/mirror2")) {
			//vec3_c center;
			//dc.verts->getCenter(*dc.indices, center);
			vec3_c surfaceOrigin = surfacePlane.norm * -surfacePlane.dist;
			//g_core->Print("Center %f %f %f, origin %f %f %f\n",center.x,center.y,center.z,surfaceOrigin.x,surfaceOrigin.y,surfaceOrigin.z);

			axis_c surfaceAxis;
			surfaceAxis[0] = surfacePlane.norm;
			surfacePlane.norm.makeNormalVectors(surfaceAxis[1],surfaceAxis[2]);
			surfaceAxis[2] = - surfaceAxis[2]; // makeNormalVectors returns "down" instead of "up"
			// store previous camera def
			cameraDef_c prevCamera = rf_camera;
			const axis_c &prevAxis = prevCamera.getAxis();
			const vec3_c &prevEye = prevCamera.getOrigin();

			axis_c cameraAxis;
			cameraAxis[0] = -surfaceAxis[0];
			cameraAxis[1] = surfaceAxis[1];
			cameraAxis[2] = surfaceAxis[2];
			//g_core->Print("Cam axis: %s\n",cameraAxis.toString());

			vec3_c newEye;
			axis_c newAxis;
			R_MirrorPoint(prevEye, surfaceOrigin, surfaceOrigin, surfaceAxis, cameraAxis, newEye);

			plane_c portalPlane;
			portalPlane.norm = -cameraAxis[0];
			portalPlane.dist = -surfaceOrigin.dotProduct(portalPlane.norm);

			R_MirrorAxis(prevAxis,surfaceAxis,cameraAxis,newAxis);

			rf_camera.setup(newEye,newAxis,rf_camera.getProjDef(),true);
			rf_camera.setPortalPlane(portalPlane);
			rf_camera.setIsMirror(true);
			rf_camera.setPVSOrigin(prevCamera.getPVSOrigin());

			RF_Generate3DSubView();
			// restore previous camera def
			rf_camera = prevCamera;
		}
	}
}
void RF_DrawCallsEndFrame() {
	rf_numDrawCalls = 0;
}
