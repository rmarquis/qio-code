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
// rf_lightGrid_debug.cpp - lightgrid debugging
#include "rf_lightGrid.h"
#include "rf_surface.h"
#include "rf_world.h"
#include "rf_local.h"
#include "rf_drawCall.h"
#include <shared/autoCvar.h>
#include <api/coreAPI.h>
#include "../pointLightSample.h"

static aCvar_c rf_debugLightGrid("rf_debugLightGrid","0");

r_surface_c rf_lightGridDebugSurface;

//void CalcVertexLighting(const rIndexBuffer_c &indices, rVertexBuffer_c &verts, const vec3_c &dir, const vec3_c &color) {
//	for(u32 i = 0; i < indices.getNumIndices(); i+=3) {
//		u32 i0 = indices[i];
//		u32 i1 = indices[i+1];
//		u32 i2 = indices[i+2];
//
//	}
//}



void CalcVertexDirLighting(rVertexBuffer_c &verts, const vec3_c &dir, const vec3_c &color) {
	for(u32 i = 0; i < verts.size(); i++) {
		rVert_c &v = verts[i];
		float dot = v.normal.dotProduct(dir);
		if(dot < 0)
			continue;
		v.color[0] = color.x * dot;
		v.color[1] = color.y * dot;
		v.color[2] = color.z * dot;
		v.color[3] = 255;
	}
}
void CalcVertexGridLighting(rVertexBuffer_c &verts, const pointLightSample_s &in) {
	// always set alhpa to 255
	verts.setVertexAlphaToConstValue(255);
	// set ambient colors (for all vertices)
	verts.setVertexColorsToConstValuesVec3255(in.ambientLight);
	// set dir colors (for vertices facing lights)
	CalcVertexDirLighting(verts,in.lightDir,in.directedLight);
}

void RF_AddLightGridDebugModelDrawCalls() {
	if(rf_debugLightGrid.getInt() == 0)
		return;
	const lightGridAPI_i *worldLightGrid = RF_GetWorldLightGridAPI();
	if(worldLightGrid == 0) {
		return;
	}
	rf_currentEntity = 0;
	vec3_c p = rf_camera.getForward() * 64.f + rf_camera.getOrigin();
	rf_lightGridDebugSurface.createBox(8.f,p);
	rf_lightGridDebugSurface.setMaterial("defaultMaterial");
	pointLightSample_s point;
	worldLightGrid->setupPointLighting(p,point);
	g_core->Print("AmbientLight %f %f %f, dirLight %f %f %f, dir %f %f %f\n",point.ambientLight.x,point.ambientLight.y,point.ambientLight.z,
		point.directedLight.x,point.directedLight.y,point.directedLight.z,
		point.lightDir.x,point.lightDir.y,point.lightDir.z);
	CalcVertexGridLighting(rf_lightGridDebugSurface.getVerts(),point);
	rf_lightGridDebugSurface.addDrawCall(true);
}


