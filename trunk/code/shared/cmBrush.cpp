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
// cmBrush.cpp
#include "cmBrush.h"
#include <math/aabb.h>
#include <api/coreAPI.h>

void cmBrush_c::fromBounds(const class aabb &bb) {
	sides.resize(6);
	for(u32 i = 0; i < 3; i++) {
		vec3_c normal(0,0,0);

		normal[i] = -1;
		this->sides[i].set(normal, bb.maxs[i]);

		normal[i] = 1;
		this->sides[3+i].set(normal, -bb.mins[i]);
	}
}
void cmBrush_c::fromPoints(const vec3_c *points, u32 numPoints) {
	// simple bruce method (slow)
	for(u32 i = 0; i < numPoints; i++) {
		for(u32 j = 0; j < numPoints; j++) {
			for(u32 k = 0; k < numPoints; k++) {
				const vec3_c &p0 = points[i];
				const vec3_c &p1 = points[j];
				const vec3_c &p2 = points[k];
				plane_c pl;
				if(pl.fromThreePoints(p0,p1,p2)) {
					continue; // triangle was degenerate
				}
				bool add = true;
				if(hasPlane(pl) == false) {
					// all of the points must be on of behind the plane
					for(u32 l = 0; l < numPoints; l++) {
						if(l == i || l == j || l == k)
							continue; // we know that these points are on this plane
						planeSide_e side = pl.onSide(points[l]);
						if(side == SIDE_BACK) {
							add = false;
							break;
						}
					}
				}
				if(add) {
					sides.push_back(pl);
				}
			}
		}
	}
}
#include <shared/parser.h>
bool cmBrush_c::parseBrushQ3(class parser_c &p) {
	// old brush format
	// Number of sides isnt explicitly specified,
	// so parse until the closing brace is hit
	while(p.atWord("}") == false) {
		// ( 2304 -512 1024 ) ( 2304 -768 1024 ) ( -2048 -512 1024 ) german/railgun_flat 0 0 0 0.5 0.5 0 0 0
		vec3_c p0;
		if(p.getFloatMat_braced(p0,3)) {
			g_core->RedWarning("cmBrush_c::parseBrushQ3: failed to read old brush def first point in file %s at line %i\n",p.getDebugFileName(),p.getCurrentLineNumber());
			return true; // error
		}
		vec3_c p1;
		if(p.getFloatMat_braced(p1,3)) {
			g_core->RedWarning("cmBrush_c::parseBrushQ3: failed to read old brush def second point in file %s at line %i\n",p.getDebugFileName(),p.getCurrentLineNumber());
			return true; // error
		}
		vec3_c p2;
		if(p.getFloatMat_braced(p2,3)) {
			g_core->RedWarning("cmBrush_c::parseBrushQ3: failed to read old brush def third point in file %s at line %i\n",p.getDebugFileName(),p.getCurrentLineNumber());
			return true; // error
		}
		//str matName = p.getWord();
		p.skipLine();

		// check for extra surfaceParms (MoHAA-specific ?)
		if(p.atWord_dontNeedWS("+")) {
			if(p.atWord("surfaceparm")) {
				const char *surfaceParmName = p.getToken();

			} else {
				g_core->RedWarning("cmBrush_c::parseBrushQ3: unknown extra parameters %s\n",p.getToken());
				p.skipLine();
			}
		}
	
		plane_c pl;
		pl.fromThreePoints(p0,p1,p2);

		sides.push_back(pl);
	}
	return false; // no error
}
#include <api/writeStreamAPI.h>
#include <shared/fileStreamHelper.h>
void cmBrush_c::writeSingleBrushToMapFileVersion2(class writeStreamAPI_i *out) {
	out->writeText("Version 2\n");
	out->writeText("// entity 0\n");
	out->writeText("{\n"); // open entity
	out->writeText("\"classname\" \"worldspawn\"\n");
	out->writeText("// primitive 0\n");
	out->writeText("{\n"); // open primitive
	out->writeText("\tbrushDef3\n");
	out->writeText("\t{\n"); // open brushdef
	for(u32 i = 0; i < sides.size(); i++) {
		const plane_c &pl = sides[i];
		plane_c writePlane = pl.getOpposite();
		// fake S/T axes for texturing
		//vec3_c sAxis = pl.norm.getPerpendicular();
		//vec3_c tAxis;
		//tAxis.crossProduct(sAxis,pl.norm);
		out->writeText("\t\t ( %f %f %f %f ) ( ( 0.0078125 0 0 ) ( 0 0.0078125 0 ) ) \"textures/common/clip\"\n",
			writePlane.norm.x,writePlane.norm.y,writePlane.norm.z,writePlane.dist);
	}
	out->writeText("\t}\n"); // close brushdef
	out->writeText("}\n"); // close primitive
	out->writeText("}\n"); // close entity
}

void cmBrush_c::writeSingleBrushToMapFileVersion2(const char *outFName) {
	fileStreamHelper_c s;
	if(s.beginWriting(outFName)) { 
		g_core->RedWarning("cmBrush_c::writeSingleBrushToMapFileVersion2: cannot open %s for writing\n",outFName);
		return;
	}
	writeSingleBrushToMapFileVersion2(&s);
}
