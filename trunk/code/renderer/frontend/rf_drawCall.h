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
// rf_drawCall.h - drawCalls managment and sorting
#ifndef __RF_DRAWCALL_H__
#define __RF_DRAWCALL_H__

#include "../drawCallSort.h"
#include <math/aabb.h>

void RF_AddDrawCall(const class rVertexBuffer_c *verts, const class rIndexBuffer_c *indices,
	class mtrAPI_i *mat, class textureAPI_i *lightmap, enum drawCallSort_e sort,
		bool bindVertexColors, class textureAPI_i *deluxemap = 0, const class vec3_c *extraRGB = 0);
void RF_AddShadowVolumeDrawCall(const class rPointBuffer_c *points, const class rIndexBuffer_c *indices);

void RF_SetForceSpecificMaterialFrame(int newFrameNum);

// NOTE: RF_SortAndIssueDrawCalls might be called more than once
// in a single renderer frame when there are active mirrors/portals views
void RF_IssueDrawCalls(u32 firstDrawCall, u32 numDrawCalls);
void RF_SortDrawCalls(u32 firstDrawCall, u32 numDrawCalls);
void RF_CheckDrawCallsForMirrorsAndPortals(u32 firstDrawCall, u32 numDrawCalls);
void RF_DrawCallsEndFrame(); // sets the current drawCalls count to 0
u32 RF_GetCurrentDrawcallsCount();

// for depth pass
extern bool rf_bDrawOnlyOnDepthBuffer;
// for lightmaps/deluxemaps pass (which is replacing depth pass)
extern bool rf_bDrawingPrelitPath;
// for point light shadow mapping
extern int rf_currentShadowMapCubeSide;
extern int rf_currentShadowMapW;
extern int rf_currentShadowMapH;
// for sun lighting pass
extern bool rf_bDrawingSunLightPass;
// for directional light (sun) shadow mapping
extern bool rf_bDrawingSunShadowMapPass;
//extern class matrix_c rf_sunProjection;
//extern class matrix_c rf_sunMatrix;
extern aabb rf_currentSunBounds;

#endif // __RF_DRAWCALL_H__
