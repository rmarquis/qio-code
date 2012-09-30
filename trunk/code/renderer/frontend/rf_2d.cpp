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
// rf_2d.cpp - 2d graphics batching and drawing

#include "rf_2d.h"

tess2d_c::tess2d_c() {
	material = 0;
	curColor[0] = curColor[1] = curColor[2] = curColor[3] = 1.f;
	numVerts = 0;
	numIndexes = 0;
}
void tess2d_c::finishDrawing() {
	if(numVerts == 0 || numIndexes == 0) {
		// two sanity checks
		if(numVerts) {
		}
		if(numIndexes) {

		}
		return;
	}

	material = 0;
	numVerts = 0;
	numIndexes = 0;
}
void tess2d_c::set2DColor(const float *rgba) {	// NULL = 1,1,1,1
	if(curColor[0] != rgba[0] || curColor[1] != rgba[1] || curColor[2] != rgba[2]
	|| curColor[3] != rgba[3]) {
		finishDrawing();
	}
}
void tess2d_c::ensureAlloced(u32 numNeedIndexes, u32 numNeedVerts) {
	if(verts.size() <= numNeedVerts) {
		verts.resize(numNeedVerts+1);
	}
	if(indices.size() <= numNeedIndexes) {
		indices.resize(numNeedIndexes+1);
	}	
}
void tess2d_c::drawStretchPic(float x, float y, float w, float h,
	float s1, float t1, float s2, float t2, class mtrAPI_i *pMaterial) { // NULL = white
	if(material != pMaterial) {
		finishDrawing();
	}
	ensureAlloced(numIndexes + 6, numVerts + 4);
	// add a quad (two triangles)
	indices[numIndexes + 0] = numVerts + 3;
	indices[numIndexes + 1] = numVerts + 0;
	indices[numIndexes + 2] = numVerts + 2;
	indices[numIndexes + 3] = numVerts + 2;
	indices[numIndexes + 4] = numVerts + 0;
	indices[numIndexes + 5] = numVerts + 1;
	// add vertices
	r2dVert_s *v = &verts[numVerts];
	v->set(x,y,s1,t1);
	v++;
	v->set(x+w,y,s2,t1);
	v++;
	v->set(x+w,y+h,s2,t2);
	v++;
	v->set(x,y+h,s1,t2);
	v++;
}



