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
// rf_water.cpp - water rendering frontend
#include "rf_surface.h"
#include <shared/autoCvar.h>

static aCvar_c rf_skipGlobalWater("rf_skipGlobalWater","0");

static float rf_waterLevel = -1.f;
static bool rf_hasGlobalWaterLevel = false;
static r_surface_c *rf_waterSurface = 0;

void RF_SetWaterLevel(const char *waterLevel) {
	if(waterLevel == 0 || waterLevel[0] == 0 || !stricmp(waterLevel,"none")) {
		rf_hasGlobalWaterLevel = false;
		rf_waterLevel = -1.f;
		if(rf_waterSurface) {
			delete rf_waterSurface;
			rf_waterSurface = 0;
		}
		return;
	}
	rf_hasGlobalWaterLevel = true;
	rf_waterLevel = atof(waterLevel);
	rf_waterSurface = new r_surface_c;
	float sizeXY = 16384*1.5;
	rf_waterSurface->createFlatGrid(sizeXY,sizeXY/128);
	rf_waterSurface->scaleTexCoords(sizeXY/256);
	rf_waterSurface->translateXYZ(vec3_c(0,0,rf_waterLevel));
}

void RF_AddWaterDrawCalls() {
	if(rf_waterSurface == 0)
		return;
	if(rf_skipGlobalWater.getInt())
		return;
	//rf_waterSurface->setMaterial("textures/misc_outside/canal_sludge");
	//rf_waterSurface->setMaterial("textures/misc_outside/canal_sludge");
	rf_waterSurface->setMaterial("qiotests/cpuwatertest1");
	rf_waterSurface->addDrawCall();
}

void RF_ShutdownWater() {
	if(rf_waterSurface) {
		delete rf_waterSurface;
		rf_waterSurface = 0;
	}
	rf_waterLevel = -1.f;
	rf_hasGlobalWaterLevel = false;
}


