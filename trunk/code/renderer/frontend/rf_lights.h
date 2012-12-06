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
// rf_lights.h
#ifndef __RF_LIGHTS_H__
#define __RF_LIGHTS_H__

#include <math/vec3.h>
#include <math/aabb.h>
#include <api/rLightAPI.h>

class rLightImpl_c : public rLightAPI_i {
	vec3_c pos;
	float radius;

	aabb absBounds;

	virtual void setOrigin(const class vec3_c &newXYZ);
	virtual void setRadius(float newRadius);
public:
	rLightImpl_c();

};

#endif // __RF_LIGHTS_H__
