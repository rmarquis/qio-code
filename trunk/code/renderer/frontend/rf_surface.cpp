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
// rf_surface.cpp - static surface class
#include "rf_surface.h"
#include "rf_drawCall.h"
#include <api/rbAPI.h>
#include <shared/trace.h>
#include <api/mtrAPI.h>

void r_surface_c::drawSurface() {
	rb->setBindVertexColors(true);
	rb->setMaterial(this->mat,this->lightmap);
	rb->drawElements(this->verts,this->indices);
	rb->setBindVertexColors(false);
}

void r_surface_c::addDrawCall() {
	RF_AddDrawCall(&this->verts,&this->indices,this->mat,this->lightmap,this->mat->getSort(),true);
}

bool r_surface_c::traceRay(class trace_c &tr) {
	if(tr.getTraceBounds().intersect(this->bounds) == false)
		return false;
	bool hasHit = false;
	for(u32 i = 0; i < indices.getNumIndices(); i+=3) {
		u32 i0 = indices[i+0];
		u32 i1 = indices[i+1];
		u32 i2 = indices[i+2];
		const rVert_c &v0 = verts[i0];
		const rVert_c &v1 = verts[i1];
		const rVert_c &v2 = verts[i2];
		if(tr.clipByTriangle(v0.xyz,v1.xyz,v2.xyz,true)) {
			hasHit = true;
		}
	}
	return hasHit;
}
