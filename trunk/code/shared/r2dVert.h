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
// r2dVert.h - simple vertex struct for 2d graphics (UI)
#ifndef __R2DVERT_H__
#define __R2DVERT_H__

#include <math/vec2.h>

struct r2dVert_s {
	vec2_c pos;
	vec2_c texCoords;

	void set(float nX, float nY, float nS, float nT) {
		pos.set(nX,nY);
		texCoords.set(nS,nT);
	}
};

#endif // __R2DVERT_H__
