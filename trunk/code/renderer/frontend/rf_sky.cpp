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
// rf_sky.cpp - sky materials (skybox) drawing
#include <api/mtrAPI.h>
#include <api/materialSystemAPI.h>
#include <materialSystem/mat_public.h>
#include <shared/autoCvar.h>
#include <shared/autoCmd.h>
#include "rf_local.h"
#include "rf_surface.h"

static class mtrAPI_i *rf_skyMaterial = 0;
static class mtrAPI_i *rf_sunMaterial = 0;
static r_surface_c *rf_skyboxSurfaces = 0;
static aCvar_c rf_skipSky("rf_skipSky","0");

void RF_InitSky() {
	rf_skyMaterial = 0;
	rf_sunMaterial = 0;
}

bool RF_HasSky() {
	if(rf_skyMaterial == 0)
		return false;
	return true;
}
void RF_ShutdownSky() {
	if(rf_skyboxSurfaces == 0)
		return;
	delete [] rf_skyboxSurfaces;
	rf_skyboxSurfaces = 0;
}
void RF_SetupSkySurface() {
	if(rf_skyboxSurfaces)
		return;
	rf_skyboxSurfaces = new r_surface_c[6];
	for(u32 i = 0; i < 6; i++) {
		r_surface_c &sf = rf_skyboxSurfaces[i];
		sf.add3Indices(0,1,2);
		sf.add3Indices(2,0,3);
		sf.resizeVerts(4);
	}
	rf_skyboxSurfaces[0].setVertXYZTC(0,vec3_c(-10.f,10.f,10.f),0,0);
	rf_skyboxSurfaces[0].setVertXYZTC(1,vec3_c(10.f,10.f,10.f),0,1);
	rf_skyboxSurfaces[0].setVertXYZTC(2,vec3_c(10.f,-10.f,10.f),1,1);
	rf_skyboxSurfaces[0].setVertXYZTC(3,vec3_c(-10.f,-10.f,10.f),1,0);
	// bottom side
	rf_skyboxSurfaces[1].setVertXYZTC(0,vec3_c(10.f,-10.f,-10.f),0,0);
	rf_skyboxSurfaces[1].setVertXYZTC(1,vec3_c(-10.f,-10.f,-10.f),0,1);
	rf_skyboxSurfaces[1].setVertXYZTC(2,vec3_c(-10.f,10.f,-10.f),1,1);
	rf_skyboxSurfaces[1].setVertXYZTC(3,vec3_c(10.f,10.f,-10.f),1,0);
	// right side
	rf_skyboxSurfaces[2].setVertXYZTC(0,vec3_c(10.f,-10.f,10.f),1,0);
	rf_skyboxSurfaces[2].setVertXYZTC(1,vec3_c(10.f,-10.f,-10.f),1,1);
	rf_skyboxSurfaces[2].setVertXYZTC(2,vec3_c(10.f,10.f,-10.f),0,1);
	rf_skyboxSurfaces[2].setVertXYZTC(3,vec3_c(10.f,10.f,10.f),0,0);
	// left side
	rf_skyboxSurfaces[3].setVertXYZTC(0,vec3_c(-10.f,-10.f,-10.f),0,1);
	rf_skyboxSurfaces[3].setVertXYZTC(1,vec3_c(-10.f,-10.f,10.f),0,0);
	rf_skyboxSurfaces[3].setVertXYZTC(2,vec3_c(-10.f,10.f,10.f),1,0);
	rf_skyboxSurfaces[3].setVertXYZTC(3,vec3_c(-10.f,10.f,-10.f),1,1);
	// back side
	rf_skyboxSurfaces[4].setVertXYZTC(0,vec3_c(-10.f,10.f,-10.f),0,1);
	rf_skyboxSurfaces[4].setVertXYZTC(1,vec3_c(-10.f,10.f,10.f),0,0);
	rf_skyboxSurfaces[4].setVertXYZTC(2,vec3_c(10.f,10.f,10.f),1,0);
	rf_skyboxSurfaces[4].setVertXYZTC(3,vec3_c(10.f,10.f,-10.f),1,1);
	// front side
	rf_skyboxSurfaces[5].setVertXYZTC(0,vec3_c(-10.f,-10.f,10.f),1,0);
	rf_skyboxSurfaces[5].setVertXYZTC(1,vec3_c(-10.f,-10.f,-10.f),1,1);
	rf_skyboxSurfaces[5].setVertXYZTC(2,vec3_c(10.f,-10.f,-10.f),0,1);
	rf_skyboxSurfaces[5].setVertXYZTC(3,vec3_c(10.f,-10.f,10.f),0,0);
}
void RF_DrawSingleSkyBox(const skyBoxAPI_i *skyBox) {
	RF_SetupSkySurface();

	const vec3_c &eye = rf_camera.getOrigin();
	rb->beginDrawingSky();
	rb->setupEntitySpace2(vec3_c(0,0,0),eye);
	rf_skyboxSurfaces[0].drawSurfaceWithSingleTexture(skyBox->getUp());
	rf_skyboxSurfaces[1].drawSurfaceWithSingleTexture(skyBox->getDown());
	rf_skyboxSurfaces[2].drawSurfaceWithSingleTexture(skyBox->getRight());
	rf_skyboxSurfaces[3].drawSurfaceWithSingleTexture(skyBox->getLeft());
	rf_skyboxSurfaces[4].drawSurfaceWithSingleTexture(skyBox->getBack());
	rf_skyboxSurfaces[5].drawSurfaceWithSingleTexture(skyBox->getFront());
	// this breaks mirrors, we need to use farthest depth range
	//rb->clearDepthBuffer();
	rb->endDrawingSky();
	rb->setupWorldSpace();
}
static r_surface_c r_skyDomeSurface; 
void RF_DrawSingleSkyDome(mtrAPI_i *mat) {
	const vec3_c &eye = rf_camera.getOrigin();
	r_skyDomeSurface.createSphere(eye,16.f,16,16);
	r_skyDomeSurface.swapIndexes();
	rb->beginDrawingSky();
	rb->setMaterial(mat);
	rb->drawElements(r_skyDomeSurface.getVerts(),r_skyDomeSurface.getIndices());
	// this breaks mirrors, we need to use farthest depth range
//	rb->clearDepthBuffer();
	rb->endDrawingSky();
}
void RF_DrawSky() {
	if(rf_skyMaterial == 0)
		return;
	if(rf_skipSky.getInt())
		return;
	const skyParmsAPI_i *skyParms = rf_skyMaterial->getSkyParms();
	if(skyParms == 0) {
		RF_DrawSingleSkyDome(rf_skyMaterial);
		return; // invalid sky material (missing skyparms keyword in .mtr/.shader file)
	}
	const skyBoxAPI_i *skyBox = skyParms->getNearBox();
	if(skyBox && skyBox->isValid()) {
		RF_DrawSingleSkyBox(skyBox);
		return;
	}
	RF_DrawSingleSkyDome(rf_skyMaterial);
}
void RF_SetSkyMaterial(class mtrAPI_i *newSkyMaterial) {
	rf_skyMaterial = newSkyMaterial;
}
void RF_SetSunMaterial(class mtrAPI_i *newSunMaterial) {
	rf_sunMaterial = newSunMaterial;
}
void RF_SetSkyMaterial(const char *skyMaterialName) {
	rf_skyMaterial = g_ms->registerMaterial(skyMaterialName);
}
class mtrAPI_i *RF_GetSkyMaterial() {
	return rf_skyMaterial;
}
bool RF_HasSunMaterial() {
	if(rf_sunMaterial)
		return true;
	return false;
}
const class mtrAPI_i *RF_GetSunMaterial() {
	return rf_sunMaterial;
}
const class vec3_c &RF_GetSunDirection() {
	static vec3_c dummy;
	if(rf_sunMaterial)
		return rf_sunMaterial->getSunParms()->getSunDir();
	return dummy;
}

void RF_SetSunMaterial_f() {
	if(g_core->Argc() < 2) {
		g_core->Print("Usage: RF_SetSunMaterial_f <material_name>\n");
		return;
	}
	const char *matName = g_core->Argv(1);
	mtrAPI_i *mat = g_ms->registerMaterial(matName);
	if(mat == 0) {
		g_core->Print("NULL material\n");
		return;
	}
	RF_SetSunMaterial(mat);
}

void RF_SetSkyMaterial_f() {
	if(g_core->Argc() < 2) {
		g_core->Print("Usage: RF_SetSkyMaterial_f <material_name>\n");
		return;
	}
	const char *matName = g_core->Argv(1);
	mtrAPI_i *mat = g_ms->registerMaterial(matName);
	if(mat == 0) {
		g_core->Print("NULL material\n");
		return;
	}
	RF_SetSkyMaterial(mat);
}

static aCmd_c rf_setSunMaterial("rf_setSunMaterial",RF_SetSunMaterial_f);
static aCmd_c rf_setSkyMaterial("rf_setSkyMaterial",RF_SetSkyMaterial_f);

