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
// cmBrush.h
#ifndef __CMBRUSH_H__
#define __CMBRUSH_H__

#include <shared/array.h>
#include <math/vec3.h>
#include <math/plane.h>

//class cmBrushSide_c {
//friend class cmBrush_c;
//	plane_c pl;
//	
//};

class cmBrush_c {
	arraySTD_c<plane_c> sides;
public:
	void fromBounds(const class aabb &bb);
	void fromPoints(const vec3_c *points, u32 numPoints);
	void fromPoints(const arraySTD_c<vec3_c> &points) {
		fromPoints(points.getArray(),points.size());
	}
	bool hasPlane(const plane_c &pl, const float normalEps = PL_NORM_EPS, const float distEps = PL_DIST_EPS) {
		for(u32 i = 0; i < sides.size(); i++) {
			if(sides[i].compare(pl,normalEps,distEps)) {
				return true;
			}
		}
		return false;
	}

	
	void writeSingleBrushToMapFileVersion2(class writeStreamAPI_i *out);
	void writeSingleBrushToMapFileVersion2(const char *outFName);
};

#endif // __CMBRUSH_H__
