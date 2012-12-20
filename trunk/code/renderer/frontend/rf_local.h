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
// rf_local.h - local header for renderer frontend module
#ifndef __RF_LOCAL_H__
#define __RF_LOCAL_H__

#include "../cameraDef.h"
#include <shared/array.h>

// rf_main.cpp
void RF_InitMain();
void RF_Draw3DView();
void RF_AddGenericDrawCalls();
bool RF_IsUsingDynamicLights();
bool RF_IsUsingShadowVolumes();
enum cullResult_e RF_CullEntitySpaceBounds(const class aabb &bb);

// rf_debugDrawing.cpp
void RF_DoDebugDrawing();
void RFDL_DrawDebugLines();
u32 RFDL_AddDebugLine(const vec3_c &from, const vec3_c &to, const vec3_c &color, float life);

// rf_entities.cpp
class rEntityAPI_i *RFE_AllocEntity();
void RFE_RemoveEntity(class rEntityAPI_i *ent);
void RFE_AddEntityDrawCalls();
void RFE_ClearEntities();
void RFE_DrawEntityAbsBounds();
bool RF_TraceSceneRay(class trace_c &tr, bool bSkipPlayerModels);
u32 RFE_BoxEntities(const class aabb &absBounds, arraySTD_c<class rEntityImpl_c*> &out);
void RFE_AddEntity(class rEntityImpl_c *ent);

// rf_lights.cpp
class rLightAPI_i *RFL_AllocLight();
void RFL_RemoveLight(class rLightAPI_i *ent);
void RFL_AddLightInteractionsDrawCalls();
void RFL_RecalculateLightsInteractions();

// rf_sky.cpp
void RF_InitSky();
void RF_DrawSky();
bool RF_HasSky();
void RF_SetSkyMaterial(class mtrAPI_i *newSkyMaterial);
void RF_SetSkyMaterial(const char *skyMaterialName);

// rf_decals.cpp
void RF_InitDecals();
void RF_ShutdownDecals();
void RF_AddWorldDecalDrawCalls();
class simpleDecalBatcher_c *RF_GetWorldDecalBatcher();

// rf_anims.cpp
void RF_ClearAnims();

// rf_map.cpp - load world map directly from .map file
class r_model_c *RF_LoadMAPFile(const char *fname);

// rf_water.cpp
void RF_SetWaterLevel(const char *waterLevel);
void RF_AddWaterDrawCalls();
void RF_ShutdownWater();

extern class cameraDef_c rf_camera;
extern int rf_curTimeMsec;
extern class rLightAPI_i *rf_curLightAPI;
// NULL == worldspawn
extern class rEntityAPI_i *rf_currentEntity;

#endif // __RF_LOCAL_H__
