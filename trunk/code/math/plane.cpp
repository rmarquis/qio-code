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
// plane.cpp
#include "plane.h"
#include "aabb.h"

planeSide_e plane_c::onSide(const aabb &bb) const {
#if 0
	planeSide_e s = onSide(bb.getPoint(0));
	for(u32 i = 1; i < 8; i++) {
		planeSide_e s2 = onSide(bb.getPoint(i));
		if(s2 != s) {
			return SIDE_CROSS;
		}
	}
	return s;
#else
	vec3_t corners[2];
	for (int i = 0; i < 3; i++) {
		if (this->norm[i] < 0) {
			corners[0][i] = bb.mins[i];
			corners[1][i] = bb.maxs[i];
		} else {
			corners[1][i] = bb.mins[i];
			corners[0][i] = bb.maxs[i];
		}
	}
	float dist1 = this->distance(corners[0]);
	float dist2 = this->distance(corners[1]);
	bool front = false;
	if (dist1 >= 0) {
		if (dist2 < 0)
			return SIDE_CROSS;
		return SIDE_FRONT;
	}
	if (dist2 < 0)
		return SIDE_BACK;
	//assert(0);
	return SIDE_ERROR;
#endif
}

enum planeSide_e plane_c::onSide(const class vec3_c &center, float radius) const {
	float d = distance(center);
	if(abs(d) <= radius)
		return SIDE_CROSS;
	if(d < 0)
		return SIDE_BACK;
	return SIDE_FRONT;
}
