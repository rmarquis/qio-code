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
// frustumExt.cpp
#include "frustumExt.h"
#include "frustum.h"
#include <shared/cmWinding.h>

frustumExt_c::frustumExt_c(const class frustum_c &fr) {
	planes.resize(FRP_NUM_FRUSTUM_PLANES);
	for(u32 i = 0; i < FRP_NUM_FRUSTUM_PLANES; i++) {
		planes[i] = fr.getPlane(i);
	}
}

cullResult_e frustumExt_c::cull(const aabb &bb) const {
	//return CULL_IN;
#if 0
	for(u32 i = 0; i < planes.size(); i++) {	
		if(planes[i].onSide(bb) == SIDE_BACK)
			return CULL_OUT;
	}	
	return CULL_CLIP;
#else
	// I need to differentiate CULL_CLIP and CULL_IN 
	// in order to avoid some reduntant frustum culling 
	// in BSP rendering code
	bool clip = false;
	for(u32 i = 0; i < planes.size(); i++) {	
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

void frustumExt_c::adjustFrustum(const frustumExt_c &other, const vec3_c &eye, const class cmWinding_c &points, const plane_c &plane) {
	cmWinding_c copy;
	plane_c cap;
	if(plane.onSide(eye) != SIDE_BACK) {
		copy = points.getReversed();
		cap = plane.getOpposite();
	} else {
		copy = points;
		cap = plane;
	}
	for(int i = 0; i < other.size(); i++) {
		const plane_c &pl = other[i];
		copy.clipWindingByPlane(pl);
	}
	if(copy.size() < 3) {
		this->clear();
		return;
	}
	// build fr's planes clockwise
	this->planes.resize(copy.size());
	int prev = copy.size()-1;
	for(int i = 0; i < copy.size(); i++) {
		this->planes[i].fromThreePoints(eye, copy[prev], copy[i]);
		prev = i;
	}
	this->planes.push_back(cap);
}
void frustumExt_c::fromPointAndWinding(const vec3_c &eye, const class cmWinding_c &points, const plane_c &plane) {
	cmWinding_c copy;
	plane_c cap;
	if(plane.onSide(eye) != SIDE_BACK) {
		copy = points.getReversed();
		cap = plane.getOpposite();
	} else {
		copy = points;
		cap = plane;
	}
	// build fr's planes clockwise
	this->planes.resize(copy.size());
	int prev = copy.size()-1;
	for(int i = 0; i < copy.size(); i++) {
		this->planes[i].fromThreePoints(eye, copy[prev], copy[i]);
		prev = i;
	}
	this->planes.push_back(cap);
}

