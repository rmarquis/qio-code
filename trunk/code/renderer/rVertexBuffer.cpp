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
// rVertexBuffer.cpp
#include "rVertexBuffer.h"
#include "rIndexBuffer.h"
#include <math/plane.h>

void rVertexBuffer_c::calcEnvironmentTexCoords(const vec3_c &viewerOrigin) {
	rVert_c *v = this->getArray();
	for(u32 i = 0; i < this->numVerts; i++, v++) {
		v->calcEnvironmentTexCoords(viewerOrigin);
	}
}
void rVertexBuffer_c::calcEnvironmentTexCoordsForReferencedVertices(const class rIndexBuffer_c &ibo, const class vec3_c &viewerOrigin) {
	static arraySTD_c<byte> bVertexCalculated;
	if(this->numVerts > bVertexCalculated.size()) {
		bVertexCalculated.resize(this->numVerts);
	}
	memset(bVertexCalculated.getArray(),0,this->numVerts);
	for(u32 i = 0; i < ibo.getNumIndices(); i++) {
		u32 index = ibo[i];
		if(bVertexCalculated[index] == 0) {
			this->data[index].calcEnvironmentTexCoords(viewerOrigin);
			bVertexCalculated[index] = 1;
		}
	}
}

void rVertexBuffer_c::setVertexColorsToConstValue(byte val) {
	rVert_c *v = this->getArray();
	for(u32 i = 0; i < this->numVerts; i++, v++) {
		v->color[0] = v->color[1] = v->color[2] = v->color[3] = val;
	}
}
void rVertexBuffer_c::setVertexColorsToConstValues(byte *rgbVals) {
	rVert_c *v = this->getArray();
	for(u32 i = 0; i < this->numVerts; i++, v++) {
		v->color[0] = rgbVals[0];
		v->color[1] = rgbVals[1];
		v->color[2] = rgbVals[2];
	}
}
void rVertexBuffer_c::setVertexAlphaToConstValue(byte val) {
	rVert_c *v = this->getArray();
	for(u32 i = 0; i < this->numVerts; i++, v++) {
		v->color[3] = val;
	}
}
bool rVertexBuffer_c::getPlane(class plane_c &pl) const {
	if(size() < 3)
		return true; // error
	const rVert_c *v = this->getArray();
	pl.fromThreePoints(v[0].xyz,v[1].xyz,v[2].xyz);
	for(u32 i = 0; i < size(); i++, v++) {
		float d = pl.distance(v->xyz);
		if(abs(d) > 0.1f) {
			return true; // error
		}
	}
	return false;
}
bool rVertexBuffer_c::getPlane(const class rIndexBuffer_c &ibo, class plane_c &pl) const {
	if(ibo.getNumIndices() < 3)
		return true; // error
	bool planeOK = false;
	const rVert_c *v = this->getArray();
	for(u32 i = 2; i < ibo.getNumIndices(); i++) {
		u32 i0 = ibo[i-2];
		u32 i1 = ibo[i-1];
		u32 i2 = ibo[i];
		if(pl.fromThreePoints(v[i0].xyz,v[i1].xyz,v[i2].xyz)==false) {
			planeOK = true;
			break;
		}
	}
	if(planeOK == false) {
		// all of the planes were denegarate
		// this should never happen...
		return true; // error
	}
	for(u32 i = 0; i < ibo.getNumIndices(); i++) {
		u32 index = ibo[i];
		float d = pl.distance(v[index].xyz);
		if(abs(d) > 0.1f) {
			return true; // error
		}
	}
	return false;
}
void rVertexBuffer_c::getCenter(const class rIndexBuffer_c &ibo, class vec3_c &out) const {
	if(ibo.getNumIndices() == 0) {
		out.set(0,0,0);
		return;
	}
	if(this->numVerts == 0) {
		out.set(0,0,0);
		return;
	}
	static arraySTD_c<byte> bVertexChecked;
	if(this->numVerts > bVertexChecked.size()) {
		bVertexChecked.resize(this->numVerts);
	}
	u32 numUniqueVertices = 0;
	double cX = 0;
	double cY = 0;
	double cZ = 0;
	memset(bVertexChecked.getArray(),0,this->numVerts);
	const rVert_c *v = this->getArray();
	for(u32 i = 0; i < ibo.getNumIndices(); i++) {
		u32 index = ibo[i];
		if(bVertexChecked[index] == 0) {
			const vec3_c &p = v[index].xyz;
			cX += p.x;
			cY += p.y;
			cZ += p.z;
			bVertexChecked[index] = 1;
			numUniqueVertices++;
		}
	}
	cX /= double(numUniqueVertices);
	cY /= double(numUniqueVertices);
	cZ /= double(numUniqueVertices);
	out.set(cX,cY,cZ);
}

