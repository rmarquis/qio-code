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
#include "aabb.h"
#include <stdlib.h>

// cohen-sutherland clipping outcodes
enum {
	CLIP_RIGHT	=	(1<<0),
	CLIP_LEFT	=	(1<<1),
	CLIP_TOP	=	(1<<2),
	CLIP_BOTTOM	=	(1<<3),
	CLIP_FRONT	=	(1<<4),
	CLIP_BACK	=	(1<<5),
};

void aabb::getRandomPointInside(class vec3_c &o) const {
	float fX = (rand()%1000) * 0.001f;
	float fY = (rand()%1000) * 0.001f;
	float fZ = (rand()%1000) * 0.001f;
	vec3_c sizes = getSizes();
	o.x = mins.x + sizes.x * fX;
	o.y = mins.y + sizes.y * fY;
	o.z = mins.z + sizes.z * fZ;
}
// calculates the cohen-sutherland outcode for a point and this bounding box.
char aabb::calcOutCode( const vec3_c &point ) const {	
	char outcode = 0;
	if( point[0] > maxs.x ) {
		outcode |= CLIP_RIGHT;
	} else if( point[0] < mins.x ) {
		outcode |= CLIP_LEFT;
	}
	if( point[1] > maxs.y ) {
		outcode |= CLIP_TOP;
	} else if( point[1] < mins.y ) {
		outcode |= CLIP_BOTTOM;
	}
	if( point[2] > maxs.z ) {
		outcode |= CLIP_BACK;
	} else if( point[2] < mins.z ) {
		outcode |= CLIP_FRONT;
	}
	return outcode;
}

bool aabb::intersect(const vec3_c &from, const vec3_c &to, vec3_c &outIntercept) const {
	char outcode1 = calcOutCode( from );
	if( outcode1 == 0 ) {
		// point inside bounding box
		outIntercept = from;
		return true;
	}

	char outcode2 = calcOutCode( to );
	if( outcode2 == 0 ) {
		// point inside bounding box
		outIntercept = to;
		return true;
	}

	if( (outcode1 & outcode2) > 0 ) {
		// both points on same side of box
		return false;
	}

	// check intersections
	if( outcode1 & (CLIP_RIGHT | CLIP_LEFT) ) {
		if( outcode1 & CLIP_RIGHT ) {
			outIntercept.x = maxs.x;
		} else {
			outIntercept.x = mins.x;
		}
		float x1 = to.x - from.x;
		float x2 = outIntercept.x - from.x;
		outIntercept.y = from.y + x2 * (to.y - from.y) / x1;
		outIntercept.z = from.z + x2 * (to.z - from.z) / x1;

		if( outIntercept.y <= maxs.y && outIntercept.y >= mins.y && outIntercept.z <= maxs.z && outIntercept.z >= mins.z ) {
			return true;
		}
	}

	if( outcode1 & (CLIP_TOP | CLIP_BOTTOM) ) {
		if( outcode1 & CLIP_TOP ) {
			outIntercept.y = maxs.y;
		} else {
			outIntercept.y = mins.y;
		}
		float y1 = to.y - from.y;
		float y2 = outIntercept.y - from.y;
		outIntercept.x = from.x + y2 * (to.x - from.x) / y1;
		outIntercept.z = from.z + y2 * (to.z - from.z) / y1;

		if( outIntercept.x <= maxs.x && outIntercept.x >= mins.x && outIntercept.z <= maxs.z && outIntercept.z >= mins.z ) {
			return true;
		}
	}

	if( outcode1 & (CLIP_FRONT | CLIP_BACK) ) {
		if( outcode1 & CLIP_BACK ) {
			outIntercept.z = maxs.z;
		} else {
			outIntercept.z = mins.z;
		}
		float z1 = to.z - from.z;
		float z2 = outIntercept.z - from.z;
		outIntercept.x = from.x + z2 * (to.x - from.x) / z1;
		outIntercept.y = from.y + z2 * (to.y - from.y) / z1;

		if( outIntercept.x <= maxs.x && outIntercept.x >= mins.x && outIntercept.y <= maxs.y && outIntercept.y >= mins.y ) {
			return true;
		}
	}

	// nothing found
	return false;
}
