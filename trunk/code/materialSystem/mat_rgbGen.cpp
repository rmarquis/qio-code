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
// mat_rgbGen.cpp
#include "mat_rgbGen.h"
#include <shared/parser.h>
#include <shared/ast.h>

rgbGen_c::~rgbGen_c() {
	if(type == RGBGEN_AST) {
		ast->destroyAST();
	}
}
bool rgbGen_c::parse(class parser_c &p) {
	if(p.atWord("wave")) {
		type = RGBGEN_WAVE;
		wave.parse(p);
	} else if(p.atWord("const")) {
		type = RGBGEN_CONST;
		p.getFloatMat_braced(constValue,3);
	} else if(p.atWord("vertex")) {
		type = RGBGEN_VERTEX;
	} else if(p.atWord("identity")) {
		type = RGBGEN_IDENTITY;
	} else if(p.atWord("static")) {
		// seem to be used in MoHAA static models.
		// It might be related to MoHAA precomputed static models lighting (vertex lighting)
		type = RGBGEN_STATIC;
	} else if(p.atWord("lightingSpherical")) {
		// MoHAA-specific spherical lighting?
		type = RGBGEN_LIGHTINGSPHERICAL;
	} else if(p.atWord("identityLighting")) {
		type = RGBGEN_IDENTITYLIGHTING;
	} else if(p.atWord("exactVertex")) {
		type = RGBGEN_EXACTVERTEX;
	} else if(p.atWord("constLighting")) {
		type = RGBGEN_CONSTLIGHTING; // added for Call Of Duty??
	} else if(p.atWord("lightingdiffuse")) {
		type = RGBGEN_LIGHTINGDIFFUSE; // added for RTCW? 
	} else if(p.atWord("entity")) {
		// this is used by Quake3 railgun model to set shader color from cgame code
	} else {
		str tok = p.getToken();
		g_core->RedWarning("rgbGen_c::parse: unknown rgbGen %s\n",tok.c_str());
		return true;
	}
	return false;
}
void rgbGen_c::setRGBGenAST(class astAPI_i *newAST) {
	type = RGBGEN_AST;
	ast = newAST;
}
void rgbGen_c::evaluateRGBGen(const class astInputAPI_i *in, float *out3Floats) const {
	if(type == RGBGEN_AST) {
		float result = ast->execute(in);
		out3Floats[0] = result;
		out3Floats[1] = result;
		out3Floats[2] = result;
	}
}



