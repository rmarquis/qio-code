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
// frustum.h - simple frustum class (defined by 6 planes)
#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__

#include "plane.h"
#include <shared/cullResult.h>

enum {
	FRP_RIGHT_PLANE,
	FRP_LEFT_PLANE,
	FRP_BOTTOM_PLANE,
	FRP_TOP_PLANE,
	FRP_FAR_PLANE,
	FRP_NUM_FRUSTUM_PLANES
};

class frustum_c {
	//vec3_c eye;
	plane_c planes[FRP_NUM_FRUSTUM_PLANES];
public:
	cullResult_e cull(const class aabb &bb) const;
	cullResult_e cullSphere(const class vec3_c &p, float radius) const;
	void setup(float fovX, float fovY, float zFar, const class axis_c &axis, const class vec3_c &origin);
	void setupExt(float fovX, float viewWidth, float viewHeight, float zFar, const class axis_c &axis, const class vec3_c &origin);
	void getBounds(class aabb &out) const;

	const plane_c &getPlane(u32 i) const {
		return planes[i];
	}
	plane_c &getPlane(u32 i) {
		return planes[i];
	}
};

#endif // __FRUSTUM_H__



