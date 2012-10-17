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
// rf_drawCall.cpp - drawCalls managment and sorting
#include "rf_drawCall.h"
#include "rf_entities.h"
#include <api/rbAPI.h>
#include <shared/array.h>

class drawCall_c {
public:
	const char *source; // for debuging, should never be fried
	bool bindVertexColors; // temporary?
	class mtrAPI_i *material;
	class textureAPI_i *lightmap; // for bsp surfaces
	class rVertexBuffer_c *verts;
	class rIndexBuffer_c *indices;
	enum drawCallSort_e sort;
	rEntityAPI_i *entity;
//public:
	
};
static arraySTD_c<drawCall_c> rf_drawCalls;
static u32 rf_numDrawCalls = 0;

void RF_AddDrawCall(rVertexBuffer_c *verts, rIndexBuffer_c *indices,
	class mtrAPI_i *mat, class textureAPI_i *lightmap, drawCallSort_e sort,
		bool bindVertexColors) {
	drawCall_c *n;
	if(rf_numDrawCalls == rf_drawCalls.size()) {
		n = &rf_drawCalls.pushBack();
	} else {
		n = &rf_drawCalls[rf_numDrawCalls];
	}
	n->verts = verts;
	n->indices = indices;
	n->material = mat;
	n->lightmap = lightmap;
	n->sort = sort;
	n->bindVertexColors = bindVertexColors;
	n->entity = rf_currentEntity;
	rf_numDrawCalls++;
}

	
void RF_SortAndIssueDrawCalls() {
	drawCall_c *c = rf_drawCalls.getArray();
	rEntityAPI_i *prevEntity = 0;
	for(u32 i = 0; i < rf_numDrawCalls; i++, c++) {
		if(prevEntity != c->entity) {
			if(c->entity == 0) {
				rb->setupWorldSpace();
			} else {
				rb->setupEntitySpace(c->entity->getAxis(),c->entity->getOrigin());
			}
			prevEntity = c->entity;
		}
		rb->setBindVertexColors(c->bindVertexColors);
		rb->setMaterial(c->material,c->lightmap);
		rb->drawElements(*c->verts,*c->indices);
	}
	rb->setBindVertexColors(false);		
	rf_numDrawCalls = 0;
	if(prevEntity) {
		rb->setupWorldSpace();
		prevEntity = 0;
	}
}