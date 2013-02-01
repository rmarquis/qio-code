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
// rf_bezier.cpp - bezier patch class
#include "rf_bezier.h"
#include <shared/parser.h>
#include <api/materialSystemAPI.h>

void bezierPatchControlGroup3x3_s::tesselate(u32 level, class r_surface_c *out) {
	//	calculate how many vertices across/down there are
	arraySTD_c<rVert_c> column[3];
	column[0].resize( level + 1 );
	column[1].resize( level + 1 );
	column[2].resize( level + 1 );

	const f64 w = 0.0 + (1.0 / f64(level) );

	// tesselate along the columns
	for( s32 j = 0; j <= level; j++ ) {
		const f64 f = w * (f64) j;

		column[0][j] = verts[0].getInterpolated_quadratic(verts[3], verts[6], f );
		column[1][j] = verts[1].getInterpolated_quadratic(verts[4], verts[7], f );
		column[2][j] = verts[2].getInterpolated_quadratic(verts[5], verts[8], f );
	}

	const u32 idx = out->getNumVerts();
	// tesselate across the rows to get final vertices
	rVert_c f;
	for( s32 j = 0; j <= level; ++j)
	{
		for( s32 k = 0; k <= level; ++k)
		{
			f = column[0][j].getInterpolated_quadratic(column[1][j], column[2][j], w * (f64) k);
			out->addVert( f );
		}
	}

	//ibo.reallocate(ibo.size()+6*level*level);
	// connect
	for( s32 j = 0; j < level; ++j)
	{
		for( s32 k = 0; k < level; ++k)
		{
			const s32 inx = idx + ( k * ( level + 1 ) ) + j;

			out->addIndex( inx + 0 );
			out->addIndex( inx + (level + 1 ) + 0 );
			out->addIndex( inx + (level + 1 ) + 1 );

			out->addIndex( inx + 0 );
			out->addIndex( inx + (level + 1 ) + 1 );
			out->addIndex( inx + 1 );
		}
	}
}

void bezierPatch3x3_c::init(const class r_bezierPatch_c *in) {
	// number of biquadratic patches
	const u32 biquadWidth = (in->width - 1)/2;
	const u32 biquadHeight = (in->height - 1)/2;

	// create space for a temporary array of the patch's control points
	int numControlPoints = ( in->width * in->height );

	// loop through the biquadratic patches
	for(u32 j = 0; j < biquadHeight; j++) {
		for(u32 k = 0; k < biquadWidth; k++) {
			// set up this patch
			const s32 inx = j*in->width*2 + k*2;
			bezierPatchControlGroup3x3_s cl;
			rVert_c *bezierControls = &cl.verts[0];
			// setup bezier control points for this patch
			bezierControls[0] = in->verts[ inx + 0 ];
			bezierControls[1] = in->verts[ inx + 1 ];
			bezierControls[2] = in->verts[ inx + 2 ];
			bezierControls[3] = in->verts[ inx + in->width + 0 ];
			bezierControls[4] = in->verts[ inx + in->width + 1 ];
			bezierControls[5] = in->verts[ inx + in->width + 2 ];
			bezierControls[6] = in->verts[ inx + in->width * 2 + 0];
			bezierControls[7] = in->verts[ inx + in->width * 2 + 1];
			bezierControls[8] = in->verts[ inx + in->width * 2 + 2];
			this->ctrls3x3.push_back(cl);
		}
	}	
}
void bezierPatch3x3_c::tesselate(u32 level, class r_surface_c *out) {
	bezierPatchControlGroup3x3_s *g = ctrls3x3.getArray();
	for(u32 i = 0; i < ctrls3x3.size(); i++, g++) {
		g->tesselate(level,out);
	}
	out->recalcBB();
}

r_bezierPatch_c::r_bezierPatch_c() {
	sf = 0;
	width = 0;
	height = 0;
	mat = 0;
	lightmap = 0;
	as3x3 = 0;
}
r_bezierPatch_c::~r_bezierPatch_c() {
	if(as3x3) {
		delete as3x3;
	}
	if(sf) {
		delete sf;
	}
}
void r_bezierPatch_c::setMaterial(const char *matName) {
	this->mat = g_ms->registerMaterial(matName);
}
void r_bezierPatch_c::tesselate(u32 newLevel) {
	if(sf == 0) {
		sf = new r_surface_c;
	}
	sf->clear();
	sf->setMaterial(this->mat);
	sf->setLightmap(this->lightmap);
	if(as3x3 == 0) {
		as3x3 = new bezierPatch3x3_c;
		as3x3->init(this);
	}
	as3x3->tesselate(newLevel,sf);
	sf->createVBO();
	sf->createIBO();
}
void r_bezierPatch_c::addDrawCall() {
	sf->addDrawCall();
}
bool r_bezierPatch_c::traceRay(class trace_c &tr) {
	if(sf == 0) {
		// patch must be instanced
		tesselate(4);
	}
	// fallback to r_surface_c::traceRay
	return sf->traceRay(tr);
}
const aabb &r_bezierPatch_c::getBB() const {
	return sf->getBB();
}
#include <shared/mapBezierPatch.h>
bool r_bezierPatch_c::fromMapBezierPatch(const mapBezierPatch_c *p) {
	this->width = p->getWidth();
	this->height = p->getHeight();
	this->setMaterial(p->getMatName());
	verts.resize(p->getNumVerts());
	for(u32 i = 0; i < verts.size(); i++) {
		const simpleVert_s &s = p->getVert(i);
		verts[i].xyz = s.xyz;
		verts[i].tc = s.tc;
	}
	return false;
}
bool r_bezierPatch_c::fromString(const char *pDefStart, const char *pDefEnd) {
	mapBezierPatch_c mapPatch;
	if(mapPatch.fromString(pDefStart,pDefEnd)) {
		return true;
	}
	this->fromMapBezierPatch(&mapPatch);
	return false;
}