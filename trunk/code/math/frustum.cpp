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
// frustum.cpp
#include "frustum.h"
#include "axis.h"
#include <shared/autoCvar.h>

static aCvar_c frustum_noCull("frustum_noCull","0");

cullResult_e frustum_c::cull(const aabb &bb) const {
	if(frustum_noCull.getInt()) {
		return CULL_IN;
	}
#if 0
	for(u32 i = 0; i < FRP_NUM_FRUSTUM_PLANES; i++) {	
		if(planes[i].onSide(bb) == SIDE_BACK)
			return CULL_OUT;
	}	
	return CULL_CLIP;
#else
	// I need to differentiate CULL_CLIP and CULL_IN 
	// in order to avoid some reduntant frustum culling 
	// in BSP rendering code
	bool clip = false;
	for(u32 i = 0; i < FRP_NUM_FRUSTUM_PLANES; i++) {	
		int side = planes[i].onSide(bb);
		if(side == SIDE_BACK)
			return CULL_OUT;
		if(side == SIDE_CROSS) {
			clip = true;
		}
	}	
	if(clip) {
		return CULL_CLIP;
	}
	return CULL_IN;
#endif
}
cullResult_e frustum_c::cullSphere(const class vec3_c &p, float radius) const {	
	if(frustum_noCull.getInt()) {
		return CULL_IN;
	}
	bool clip = false;
	for(u32 i = 0; i < FRP_NUM_FRUSTUM_PLANES; i++) {	
		int side = planes[i].onSide(p,radius);
		if(side == SIDE_BACK)
			return CULL_OUT;
		if(side == SIDE_CROSS) {
			clip = true;
		}
	}	
	if(clip) {
		return CULL_CLIP;
	}
	return CULL_IN;
}

void frustum_c::setup(float fovX, float fovY, float zFar, const axis_c &axis, const vec3_c &origin) {
#if 1
	float ang = fovX / 180 * M_PI * 0.5f;
	float xs = sin( ang );
	float xc = cos( ang );

	planes[0].norm = axis[0] * xs;
	planes[0].norm.vectorMA(planes[0].norm,axis[1],xc);

	planes[1].norm = axis[0] * xs;
	planes[1].norm.vectorMA(planes[1].norm,axis[1],-xc);

	ang = fovY / 180 * M_PI * 0.5f;
	xs = sin( ang );
	xc = cos( ang );

	planes[2].norm = axis[0] * xs;
	planes[2].norm.vectorMA(planes[2].norm,axis[2],xc);

	planes[3].norm = axis[0] * xs;
	planes[3].norm.vectorMA(planes[3].norm,axis[2],-xc);

	for (u32 i = 0; i < 4; i++) {
		//planes[i].type = PLANE_NON_AXIAL;
		planes[i].dist = -origin.dotProduct(planes[i].norm);
		//planes[i].setSignBits();
	}
#if 1
	//fifth plane (closing one...)
	planes[4].norm = -axis[0];
	planes[4].dist = -planes[4].norm.dotProduct(origin+(axis[0]*zFar));
	//planes[4].type = PLANE_NON_AXIAL;
	//planes[4].setSignBits();
#endif
#else

#endif
}
void frustum_c::setupExt(float fovX, float viewWidth, float viewHeight, float zFar, const class axis_c &axis, const class vec3_c &origin) {
	float x = viewWidth / tan( fovX / 360 * M_PI );
	float fovY = atan2( viewHeight, x ) * 360 / M_PI;
	setup(fovX,fovY,zFar,axis,origin);
}
