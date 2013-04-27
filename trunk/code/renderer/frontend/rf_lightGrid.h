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
// rf_lightGrid.h - precomputed lighting grid class
#ifndef __RF_LIGHTGRID_H__
#define __RF_LIGHTGRID_H__

#include <math/vec3.h>
#include <math/aabb.h>
#include <shared/array.h>

class lightGridAPI_i {
public:
	virtual ~lightGridAPI_i() {

	}

	virtual void setupPointLighting(const vec3_c &origin, struct pointLightSample_s &out) const = 0;
};

class q3BSPLightGrid_c : public lightGridAPI_i {
	// bsp->models[0] bounds
	aabb worldBounds;
	vec3_c lightGridOrigin;
	vec3_c lightGridSize;
	vec3_c lightGridInverseSize;
	int lightGridBounds[3];
	u32 numGridPoints;
	arraySTD_c<byte> lightGridData;
public:
	~q3BSPLightGrid_c();
	bool init(const byte *gridData, u32 numGridBytes, const aabb &pWorldBounds, const vec3_c &pGridSize);

	virtual void setupPointLighting(const vec3_c &origin, struct pointLightSample_s &out) const;
};

#endif // __RF_LIGHTGRID_H__
