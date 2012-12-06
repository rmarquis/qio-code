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
// rf_lights.cpp
#include "rf_lights.h"
#include <shared/array.h>

rLightImpl_c::rLightImpl_c() {
	radius = 512.f;
}
void rLightImpl_c::setOrigin(const class vec3_c &newXYZ) {
	if(pos.compare(newXYZ)) {
		return; // no change
	}
	pos = newXYZ;
	absBounds.fromPointAndRadius(pos,radius);
}
void rLightImpl_c::setRadius(float newRadius) {
	if(radius == newRadius) {
		return; // no change
	}
	radius = newRadius;
	absBounds.fromPointAndRadius(pos,radius);
}


static arraySTD_c<rLightImpl_c*> rf_lights;

class rLightAPI_i *RFL_AllocLight() {
	rLightImpl_c *light = new rLightImpl_c;
	rf_lights.push_back(light);
	return light;
}
void RFL_RemoveLight(class rLightAPI_i *light) {
	rLightImpl_c *rlight = (rLightImpl_c*)light;
	rf_lights.remove(rlight);
	delete rlight;
}

void RF_RenderSceneWithDynamicLighting() {
	/*
	// draw on depth buffer / draw lightmapped
	if(RF_WorldHasLightmaps()) {
		RF_DrawLightmapped();
		// draw entities as well??? use lightgrid?
	} else {
		RF_DrawOnDepthBuffer();
	}
	for(u32 i = 0; i < rf_lights.size(); i++) {
		rLightImpl_c *l = rf_lights[i];
		if(l->isCulled())
			continue;
		RF_RenderLightInteractions();
	}
	
	*/
}