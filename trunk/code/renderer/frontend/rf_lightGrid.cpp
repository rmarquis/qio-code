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
// rf_lightGrid.cpp - precomputed lighting grid class
#include "rf_lightGrid.h"
#include <api/coreAPI.h>
#include "../pointLightSample.h"

q3BSPLightGrid_c::~q3BSPLightGrid_c() {

}
bool q3BSPLightGrid_c::init(const byte *pGridData, u32 numGridBytes, const aabb &pWorldBounds, const vec3_c &pGridSize) {
	this->lightGridSize = pGridSize;
	this->worldBounds = pWorldBounds;
	
	lightGridInverseSize = 1.f / lightGridSize;

	for (u32 i = 0; i < 3; i++) {
		lightGridOrigin[i] = lightGridSize[i] * ceil( worldBounds.mins[i] / lightGridSize[i] );
		float tmp = lightGridSize[i] * floor( worldBounds.maxs[i] / lightGridSize[i] );
		lightGridBounds[i] = (tmp - lightGridOrigin[i])/lightGridSize[i] + 1;
	}

	numGridPoints = lightGridBounds[0] * lightGridBounds[1] * lightGridBounds[2];

	if (numGridBytes != numGridPoints * 8) {
		g_core->RedWarning("q3BSPLightGrid_c::init: light grid mismatch\n");
		return true; // error
	}

	lightGridData.resize(numGridBytes);
	memcpy(lightGridData.getArray(),pGridData,numGridBytes);
	return false;
}


void q3BSPLightGrid_c::setupPointLighting(const vec3_c &origin, struct pointLightSample_s &out) const {
	vec3_c lightOrigin = origin - this->lightGridOrigin;
	int pos[3];
	float frac[3];
	for(u32 i = 0; i < 3; i++) {
		float v = lightOrigin[i] * this->lightGridInverseSize[i];
		pos[i] = floor( v );
		frac[i] = v - pos[i];
		if(pos[i] < 0) {
			pos[i] = 0;
		} else if(pos[i] >= this->lightGridBounds[i] - 1) {
			pos[i] = this->lightGridBounds[i] - 1;
		}
	}

	out.ambientLight.zero();
	out.directedLight.zero();
	out.lightDir.zero();

	int gridStep[3];
	// trilerp the light value
	gridStep[0] = 8;
	gridStep[1] = 8 * this->lightGridBounds[0];
	gridStep[2] = 8 * this->lightGridBounds[0] * this->lightGridBounds[1];
	u32 pointDataOffset = pos[0] * gridStep[0] + pos[1] * gridStep[1] + pos[2] * gridStep[2];
	if(pointDataOffset >= this->lightGridData.size()) {
		g_core->RedWarning("q3BSPLightGrid_c::setupPointLighting: bad pointDataOffset %i (grid data size is %i)\n",
			pointDataOffset,this->lightGridData.size());
		return;
	}
	const byte *pointData = this->lightGridData.getArray() + pointDataOffset;

	float totalFactor = 0;
	for (u32 i = 0; i < 8; i++ ) {
		float factor = 1.0;
		const byte *data = pointData;
		for (u32 j = 0; j < 3; j++) {
			if ( i & (1<<j) ) {
				factor *= frac[j];
				data += gridStep[j];
			} else {
				factor *= (1.0f - frac[j]);
			}
		}

		if ( !(data[0]+data[1]+data[2]) ) {
			continue;	// ignore samples in walls
		}
		totalFactor += factor;

		out.ambientLight[0] += factor * data[0];
		out.ambientLight[1] += factor * data[1];
		out.ambientLight[2] += factor * data[2];

		out.directedLight[0] += factor * data[3];
		out.directedLight[1] += factor * data[4];
		out.directedLight[2] += factor * data[5];

		float lat = float(data[7]);
		float lng = float(data[6]);
		lat *= M_PI/128.f;
		lng *= M_PI/128.f;

		// decode X as cos( lat ) * sin( long )
		// decode Y as sin( lat ) * sin( long )
		// decode Z as cos( long )
		vec3_c normal(
			cos(lat) * sin(lng),
			sin(lat) * sin(lng),
			cos(lng));

		out.lightDir += normal * factor;
	}

	if ( totalFactor > 0 && totalFactor < 0.99 ) {
		totalFactor = 1.0f / totalFactor;
		out.ambientLight *= totalFactor;
		out.directedLight *= totalFactor;
	}
	out.lightDir.normalize();
}


