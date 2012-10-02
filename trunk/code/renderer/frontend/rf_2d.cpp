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
#include <api/rbAPI.h>
#include <api/materialSystemAPI.h>

static tess2d_c r_tess2D; // I dont need to access that class outside this file
r2dCommandsQueue_c r_2dCmds;

//
//	tess2d_c class
//
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
	rb->setMaterial(material);
	rb->setColor4(curColor);
	rb->draw2D(verts.getArray(),numVerts,indices.getArray(),numIndexes);
	material = 0;
	numVerts = 0;
	numIndexes = 0;
}
void tess2d_c::set2DColor(const float *rgba) {	// NULL = 1,1,1,1
	if(rgba == 0) {
		float def[] = { 1, 1, 1, 1 };
		rgba = def;
	}
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
	material = pMaterial;
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

	numVerts += 4;
	numIndexes += 6;
}




//
//	r2dCommandsQueue_c class
//
enum {
	R2DCMD_STOP,
	R2DCMD_SETCOLOR,
	R2DCMD_SETCOLOR_DEFAULT,
	R2DCMD_DRAWPIC,
};
struct setColorData_s {
	float col[4];
};
struct drawPicData_s {
	float pos[2];
	float size[2];
	float tcs[2];
	float tcs2[2];
	class mtrAPI_i *mat;
};
r2dCommandsQueue_c::r2dCommandsQueue_c() {
	data.resize(1);
	data[0] = 0;
	at = 0;
}
void r2dCommandsQueue_c::ensureAlloced(u32 neededSize) {
	if(data.size() + 1 <= neededSize) {
		data.resize(neededSize+4);
	}
}
void r2dCommandsQueue_c::addSetColorCmd(const float *rgba) {
	if(rgba == 0) {
		ensureAlloced(at+1);
		data[at] = R2DCMD_SETCOLOR_DEFAULT;
		at++;
	} else {
		ensureAlloced(at+1+sizeof(setColorData_s));
		data[at] = R2DCMD_SETCOLOR;
		at++;
		setColorData_s *s = (setColorData_s*)&data[at];
		s->col[0] = rgba[0];
		s->col[1] = rgba[1];
		s->col[2] = rgba[2];
		s->col[3] = rgba[3];
		at += sizeof(setColorData_s);
	}
	data[at] = 0;
}
void r2dCommandsQueue_c::addDrawStretchPic(float x, float y, float w, float h,
	float s1, float t1, float s2, float t2, class mtrAPI_i *material) {
	ensureAlloced(at+1+sizeof(drawPicData_s));
	data[at] = R2DCMD_DRAWPIC;
	at++;
	drawPicData_s *dp = (drawPicData_s*)&data[at];
	dp->mat = material;
	dp->pos[0] = x;
	dp->pos[1] = y;
	dp->size[0] = w;
	dp->size[1] = h;
	dp->tcs[0] = s1;
	dp->tcs[1] = t1;
	dp->tcs2[0] = s2;
	dp->tcs2[1] = t2;
	at += sizeof(drawPicData_s);	
	data[at] = 0;
}
void r2dCommandsQueue_c::executeCommands() {
	if(at == 0)
		return;
	u32 stop = at;
	at = 0;
	while(at <= stop) {
		byte type = data[at];
		if(type == R2DCMD_STOP) {
			break;
		} 
		at++;
		if(type == R2DCMD_SETCOLOR) {
			const float *col = (const float*)&data[at];
			r_tess2D.set2DColor(col);
			at += (sizeof(float)*4);
		} else if(type == R2DCMD_SETCOLOR_DEFAULT) {
			r_tess2D.set2DColor(0);
		} else if(type == R2DCMD_DRAWPIC) {
			const drawPicData_s *dp = (const drawPicData_s*)&data[at];
			r_tess2D.drawStretchPic(dp->pos[0],dp->pos[1],dp->size[0],dp->size[1],dp->tcs[0],dp->tcs[1],
				dp->tcs2[0],dp->tcs2[1],dp->mat);
			at += sizeof(drawPicData_s);
		} else {
			printf("r2dCommandsQueue_c::executeCommands: unknown cmd id %i\n",type);
			break;
		}
	}
	at = 0;
	r_tess2D.finishDrawing();
}
void r2dCommandsQueue_c::clear() {
	at = 0;
}
