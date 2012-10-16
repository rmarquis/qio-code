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
// rf_drawCall.h - drawCalls managment and sorting
#ifndef __RF_DRAWCALL_H__
#define __RF_DRAWCALL_H__

#include "../drawCallSort.h"

void RF_AddDrawCall(class rVertexBuffer_c *verts, class rIndexBuffer_c *indices,
	class mtrAPI_i *mat, class textureAPI_i *lightmap, enum drawCallSort_e sort,
		bool bindVertexColors);

void RF_SortAndIssueDrawCalls();

#endif // __RF_DRAWCALL_H__
