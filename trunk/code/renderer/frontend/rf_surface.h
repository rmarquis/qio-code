#ifndef /*
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
// rf_surface.h
#ifndef __RF_SURFACE_H__
#define __RF_SURFACE_H__

#include "../rIndexBuffer.h"
#include "../rVertexBuffer.h"

class r_surface_c {
	str name;
	str matName;
	class mtrAPI_i *mat;
	rVertexBuffer_c verts;
	rIndexBuffer_c indices;
public:
	r_surface_c() {
		mat = 0;
	}


};

#endif // __RF_SURFACE_H__

