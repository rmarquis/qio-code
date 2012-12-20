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
	} else if(p.atWord("lightingSpherical")) {
		// MoHAA-specific spherical lighting?
	} else {
		return true;
	}
	return false;
}


