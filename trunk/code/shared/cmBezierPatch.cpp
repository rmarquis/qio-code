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
// cmBezierPatch.cpp - simplified bezier patch class for collision detection
#include "cmBezierPatch.h"
#include <api/colMeshBuilderAPI.h>
#include <shared/mapBezierPatch.h>

void cmBezierPatchControlGroup3x3_s::tesselate(u32 level, class colMeshBuilderAPI_i *out) {
	//	calculate how many vertices across/down there are
	arraySTD_c<vec3_c> column[3];
	column[0].resize( level + 1 );
	column[1].resize( level + 1 );
	column[2].resize( level + 1 );

	const f64 w = 0.0 + (1.0 / f64(level) );

	// tesselate along the columns
	for( s32 j = 0; j <= level; j++ ) {
		const f64 f = w * (f64) j;
		for(u32 c = 0; c < 3; c++) {
			G_GetInterpolated_quadraticn(3,column[c][j],verts[c],verts[3+c],verts[6+c],f);
		}
	}

	const u32 idx = out->getNumVerts();
	// tesselate across the rows to get final vertices
	vec3_c f;
	for( s32 j = 0; j <= level; ++j)
	{
		for( s32 k = 0; k <= level; ++k)
		{
			G_GetInterpolated_quadraticn(3,f,column[0][j],column[1][j],column[2][j], w * (f64) k);
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

void cmBezierPatch3x3_c::init(const class cmBezierPatch_c *in) {
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
			cmBezierPatchControlGroup3x3_s cl;
			vec3_c *bezierControls = &cl.verts[0];
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
void cmBezierPatch3x3_c::tesselate(u32 level, class colMeshBuilderAPI_i *out) {
	cmBezierPatchControlGroup3x3_s *g = ctrls3x3.getArray();
	for(u32 i = 0; i < ctrls3x3.size(); i++, g++) {
		g->tesselate(level,out);
	}
}

cmBezierPatch_c::cmBezierPatch_c() {
	width = 0;
	height = 0;
	as3x3 = 0;
}
cmBezierPatch_c::~cmBezierPatch_c() {
	if(as3x3) {
		delete as3x3;
	}
}

void cmBezierPatch_c::tesselate(u32 newLevel, colMeshBuilderAPI_i *out) {
	if(as3x3 == 0) {
		as3x3 = new cmBezierPatch3x3_c;
		as3x3->init(this);
	}
	as3x3->tesselate(newLevel,out);
}
void cmBezierPatch_c::fromMapBezierPatch(const class mapBezierPatch_c *in) {
	verts.resize(in->getNumVerts());
	for(u32 i = 0; i < verts.size(); i++) {
		verts[i] = in->getVert(i).xyz;
	}
	width = in->getWidth();
	height = in->getHeight();
}



