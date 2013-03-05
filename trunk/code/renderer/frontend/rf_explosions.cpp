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
// rf_explosions.cpp
#include "rf_local.h"
#include "rf_surface.h"
#include "rf_drawCall.h"
#include <api/coreAPI.h>
#include <api/mtrAPI.h>
#include <api/materialSystemAPI.h>
#include <shared/autoCmd.h>

class r_explosion_c {
	u32 frameNum;
	float startTime;
	vec3_c pos;
	float radius;
	mtrAPI_i *material;
	 // TODO: merge explosion surfaces?
	r_surface_c *sf;
public:
	r_explosion_c() {
		startTime = -1.f;
		frameNum = 0;
		radius = 0;
		material = 0;
		sf = 0;
	}
	r_explosion_c(const r_explosion_c &) {
		startTime = -1.f;
		frameNum = 0;
		radius = 0;
		material = 0;
		sf = 0;
	}
	~r_explosion_c() {
		delete sf;
	}
	bool isActive() const {
		if(startTime < 0)
			return false;
		return true;
	}
	void initExplosion(const vec3_c &newPos, float newRadius, mtrAPI_i *newMat) {
		this->pos = newPos;
		this->radius = newRadius;
		this->material = newMat;
		this->startTime = rf_curTimeSeconds;

		// see if we have to reinit sprite surface
		if(sf) {
			sf->initSprite(material,radius);
		}
	}
	void runFrame() {
		float elapsedTime = rf_curTimeSeconds - this->startTime;
		this->frameNum = elapsedTime / 0.1f;
		// check for timeout
		if(this->frameNum >= material->getColorMapImageFrameCount()) {
			startTime = -1.f;
			return;
		}
	}
	void addSurfaceDrawCall() {
		if(sf == 0) {
			sf = new r_surface_c;
			sf->initSprite(material,radius);
		}
		// update sprite
		sf->updateSprite(rf_camera.getAxis(),pos,radius);
		// add draw call
		RF_SetForceSpecificMaterialFrame(this->frameNum);
		sf->addDrawCall();
		RF_SetForceSpecificMaterialFrame(-1);
	}
};

static arraySTD_c<r_explosion_c> rf_explosions;

u32 RF_AddExplosion(const vec3_c &pos, float radius, mtrAPI_i *material) {
	// see if we have a free explosion slot
	r_explosion_c *e = rf_explosions.getArray();
	for(u32 i = 0; i < rf_explosions.size(); i++, e++) {
		if(e->isActive() == false) {
			// found an empty slot
			e->initExplosion(pos,radius,material);
			return i;
		}
	}
	rf_explosions.pushBack().initExplosion(pos,radius,material);
	return rf_explosions.size()-1;
}
u32 RF_AddExplosion(const vec3_c &pos, float radius, const char *matName) {
	if(matName == 0 || matName[0] == 0) {
		g_core->RedWarning("RF_AddExplosion: NULL material name, using default\n");
		matName = "default";
	}
	mtrAPI_i *mat = g_ms->registerMaterial(matName);
	return RF_AddExplosion(pos,radius,mat);
}

void RF_AddExplosionDrawCalls() {
	r_explosion_c *e = rf_explosions.getArray();
	for(u32 i = 0; i < rf_explosions.size(); i++, e++) {
		if(e->isActive()) {
			e->runFrame();
			if(e->isActive()) {
				e->addSurfaceDrawCall();
			}
		}
	}
}

// console command for testing explosion effect
static void RF_TestExplosion_f() {
	str matName;
	float radius;

	// Quake3 rocket explosion material
	matName = "rocketExplosion";
	radius = 32.f;

	vec3_c p = rf_camera.getOrigin() + rf_camera.getForward() * 128.f;
	RF_AddExplosion(p,radius,matName);
}

static aCmd_c rf_testExplosion_f("rf_testExplosion",RF_TestExplosion_f);
