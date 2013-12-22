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
// cg_tempLights.cpp - temporary clientside-only light objects
#include "cg_local.h"
#include <shared/array.h>
#include <math/vec3.h>
#include <api/rLightAPI.h>
#include <api/rAPI.h>

class cgTempLight_c {
	class rLightAPI_i *light;
	vec3_c origin;
	float radius;
	int curLife;
	int totalLife;
public:
	void runTempLight() {
		if(light == 0)
			return;
		curLife -= cg.frametime;
		if(curLife < 0) {
			rf->removeLight(light);
			light = 0;
		}
	}
	void setupTempLight(const vec3_c &nPos, float nRadius, int nTotalLife) {
		light = rf->allocLight();
		light->setOrigin(nPos);
		light->setRadius(nRadius);
		origin = nPos;
		curLife = totalLife = nTotalLife;
		radius = nRadius;
	}
};

static arraySTD_c<cgTempLight_c> cg_tempLights;
static u32 cg_numTempLights = 0;

void CG_CreateTempLight(const vec3_c &pos, float radius, int totalLife) {
	cgTempLight_c newLight;
	newLight.setupTempLight(pos,radius,totalLife);
	if(cg_numTempLights == cg_tempLights.size()) {
		cg_tempLights.push_back(newLight);
	} else {
		cg_tempLights[cg_numTempLights] = newLight;
	}
	cg_numTempLights++;
}

void CG_RunTempLights() {
	for(u32 i = 0; i < cg_numTempLights; i++) {
		cgTempLight_c &l = cg_tempLights[i];
		l.runTempLight();
	}
}





