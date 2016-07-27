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
// rf_colorAdjust.cpp
#include <math/vec3.h>
float r_colorScale = 3.f;

void RF_AdjustColorRGB(vec3_c &col) {
	col *= r_colorScale;
	col.capEachComponent(255.f);
}
void RF_AdjustColorRGB(byte *rgb) {
	vec3_c col(rgb[0],rgb[1],rgb[2]);
	RF_AdjustColorRGB(col);
	rgb[0] = col.x;
	rgb[1] = col.y;
	rgb[2] = col.z;
}
void RF_AdjustColorRGBA(byte *rgba) {
	for(u32 i = 0; i < 4; i++) {
		float f = rgba[i];
		f *= r_colorScale;
		if(f > 255.f)
			f = 255.f;
		rgba[i] = f;
	}
}