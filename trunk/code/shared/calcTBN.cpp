/*
============================================================================
Copyright (C) 2013 V.

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
// calcTBN.cpp
#include <math/vec3.h>
#include <math/vec2.h>

void R_CalcTBN(const vec3_c &v0, const vec3_c &v1, const vec3_c &v2,
	const vec2_c &st0, const vec2_c &st1, const vec2_c &st2,
	vec3_c &outNorm, vec3_c &outTan, vec3_c &outBin) {

	vec3_c edge0 = v2 - v0;
	vec3_c edge1 = v1 - v0;
	outNorm.crossProduct(edge0,edge1);
	outNorm.normalizeFast();

	vec3_c e0(v1[0] - v0[0], st1[0] - st0[0], st1[1] - st0[1]);
	vec3_c e1(v2[0] - v0[0], st2[0] - st0[0], st2[1] - st0[1]);

	vec3_c cp;
	cp.crossProduct(e0,e1);
	
	vec3_c tangent;
	vec3_c binormal;
	vec3_c normal;
	
		
	if(fabs(cp[0]) > 10e-6) {
		tangent[0]	= -cp[1] / cp[0];
		binormal[0]	= -cp[2] / cp[0];
	}

	e0[0] = v1[1] - v0[1];
	e1[0] = v2[1] - v0[1];

	cp.crossProduct(e0,e1);
	if(fabs(cp[0]) > 10e-6) {
		tangent[1]	= -cp[1] / cp[0];        
		binormal[1]	= -cp[2] / cp[0];
	}

	e0[0] = v1[2] - v0[2];
	e1[0] = v2[2] - v0[2];

	cp.crossProduct(e0,e1);
	if(fabs(cp[0]) > 10e-6) {
		tangent[2]	= -cp[1] / cp[0];        
		binormal[2]	= -cp[2] / cp[0];
	}


	// tangent...
	tangent.normalizeFast();
		
	// binormal...
	binormal.normalizeFast();
	
	//// normal...
	//// compute the cross product TxB
	normal.crossProduct(tangent, binormal);
	//normal.crossProduct(tangent, binormal);

	normal.normalizeFast();

	
	// Gram-Schmidt orthogonalization process for B
	// compute the cross product B=NxT to obtain 
	// an orthogonal basis
	binormal.crossProduct(normal, tangent);

	outTan = tangent;
	outBin = binormal;
//	outNorm = normal;
}
