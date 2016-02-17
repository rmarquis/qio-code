/*
============================================================================
Copyright (C) 2016 V.

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
#include <math/vec3.h>
#include <math/vec2.h>

class texCoordCalc_c {
	vec3_c uAxis;
	vec3_c vAxis;
	float texScaleX;
	float texScaleY;
public:
	texCoordCalc_c() {
		uAxis = vec3_c(1,0,0);
		vAxis = vec3_c(0,1,0);
		texScaleX = 64.f;
		texScaleY = 64.f;
	}
	void setTexScaleX(float f) {
		texScaleX = f;
	}
	void setTexScaleY(float f) {
		texScaleY = f;
	}
	void calcTexCoord(const vec3_c &p, vec2_c &tc) const {
		float s = p.dotProduct(uAxis);
		float t = p.dotProduct(vAxis);
		tc.x = s / texScaleX;
		tc.y = t / texScaleY;
	}
};