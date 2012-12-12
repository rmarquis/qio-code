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
#include <api/coreAPI.h>
#include <api/rbAPI.h>
#include <api/mtrAPI.h>
#include <api/materialSystemAPI.h>
#include <shared/array.h>
#include <shared/autoCvar.h>

aCvar_c rf_noLightmaps("rf_noLightmaps","0");
aCvar_c rf_noVertexColors("rf_noVertexColors","0");
aCvar_c rf_ignoreSpecificMaterial("rf_ignoreSpecificMaterial","");
aCvar_c rf_ignoreSpecificMaterial2("rf_ignoreSpecificMaterial2","");
aCvar_c rf_forceSpecificMaterial("rf_forceSpecificMaterial","");

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
	drawCall_c *n;
	if(rf_numDrawCalls == rf_drawCalls.size()) {
		n = &rf_drawCalls.pushBack();
	} else {
		n = &rf_drawCalls[rf_numDrawCalls];
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
	n->drawOnlyOnDepthBuffer = rf_bDrawOnlyOnDepthBuffer;
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

int compareDrawCall(const void *v0, const void *v1) {
	const drawCall_c *c0 = (const drawCall_c *)v0;
	const drawCall_c *c1 = (const drawCall_c *)v1;
	if(c0->curLight) {
		if(c1->curLight == 0) {
			return 1; // c1 first
		}
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
	// sorts are equal, sort by material pointer
	if(c0->material > c1->material) {
		return -1;
	} else if(c0->material < c1->material) {
		return 1;
	}
	// materials are equal too
	return 0;
}

void RF_SortAndIssueDrawCalls() {
	qsort(rf_drawCalls.getArray(),rf_numDrawCalls,sizeof(drawCall_c),compareDrawCall);
	drawCall_c *c = rf_drawCalls.getArray();
	rEntityAPI_i *prevEntity = 0;
	for(u32 i = 0; i < rf_numDrawCalls; i++, c++) {
		if(prevEntity != c->entity) {
			if(c->entity == 0) {
				rb->setupWorldSpace();
			} else {
				rb->setupEntitySpace(c->entity->getAxis(),c->entity->getOrigin());
			}
			prevEntity = c->entity;
		}
		rb->setCurLight(c->curLight);
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
	rf_numDrawCalls = 0;
	if(prevEntity) {
		rb->setupWorldSpace();
		prevEntity = 0;
	}
}