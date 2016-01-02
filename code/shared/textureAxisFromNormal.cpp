/*
============================================================================
Copyright (C) 2015 V.

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

float baseAxis[18][3] = {
	{0,0,1}, {1,0,0}, {0,-1,0},			// floor
	{0,0,-1}, {1,0,0}, {0,-1,0},		// ceiling
	{1,0,0}, {0,1,0}, {0,0,-1},			// west wall
	{-1,0,0}, {0,1,0}, {0,0,-1},		// east wall
	{0,1,0}, {1,0,0}, {0,0,-1},			// south wall
	{0,-1,0}, {1,0,0}, {0,0,-1}			// north wall
};
void MOD_TextureAxisFromNormal(const vec3_c &normal, vec3_c &xv, vec3_c &yv) {
	float best = 0;
	int bestaxis = 0;
	
	for (u32 i = 0; i < 6; i++) {
		float dot = normal.dotProduct(baseAxis[i*3]);
		if (dot > best) {
			best = dot;
			bestaxis = i;
		}
	}
	
	xv = baseAxis[bestaxis*3+1];
	yv = baseAxis[bestaxis*3+2];
}
