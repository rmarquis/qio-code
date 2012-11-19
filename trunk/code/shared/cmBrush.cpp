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
		this->sides[i].pl.set(normal, bb.maxs[i]);

		normal[i] = 1;
		this->sides[3+i].pl.set(normal, -bb.mins[i]);
	}
}
#include "cmWinding.h"
void cmBrush_c::fromPoints(const vec3_c *points, u32 numPoints) {
	cmWinding_c w;
	w.addPointsUnique(points,numPoints);
	points = w.getPoints().getArray();
	numPoints = w.size();
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
					if(add) {
						sides.push_back(cmBrushSide_c(pl));
					}
				}
			}
		}
	}
}
#include <shared/trace.h>
#define BRUSH_EPSILON 0.00000001f
bool cmBrush_c::traceRay(class trace_c &tr) {
    float startFraction = -1.0f;
    float endFraction = 1.0f;
    bool startsOut = false;
    bool endsOut = false;
	const plane_c *clipPlane = 0;
    for (int i = 0; i < sides.size(); i++) {
		const plane_c *pl = &sides[i].pl;
		float startDistance = pl->distance(tr.getFrom());
		float endDistance = pl->distance(tr.getTo());

        if (startDistance > 0)
            startsOut = true;
        if (endDistance > 0)
            endsOut = true;

        // make sure the trace isn't completely on one side of the brush
        if (startDistance > 0 && endDistance > 0) {   
            return false; // both are in front of the plane, its outside of this brush
        }
        if (startDistance <= 0 && endDistance <= 0) {   
            continue; // both are behind this plane, it will get clipped by another one
        }

        if (startDistance > endDistance) {
           // line is entering into the brush
            float fraction = (startDistance - BRUSH_EPSILON) / (startDistance - endDistance);
            if (fraction > startFraction) {
				clipPlane = pl;
                startFraction = fraction;
			}
        } else {   
			// line is leaving the brush
            float fraction = (startDistance + BRUSH_EPSILON) / (startDistance - endDistance);
            if (fraction < endFraction) {
                endFraction = fraction;
			}
        }
    }
    if (startsOut == false) {
		if (endsOut == false) {
           // tr.setAllSolid(true);
		}
		//tr.setStartSolid(true);
		tr.setFraction(0);
		return true;
    }
    if (startFraction < endFraction) {
        if (startFraction > -1 && startFraction < 1.f) {
            if (startFraction < 0)
                startFraction = 0;
			tr.setFraction(startFraction);
			//tr.setPlane(*clipPlane);
			return true;
        }
    }
	return false; // no hit
}	
bool cmBrush_c::hasSideWithMaterial(const char *matName) const {
	for(u32 i = 0; i < sides.size(); i++) {
		if(!Q_stricmp(sides[i].matName,matName))
			return true;
	}
	return false;
}
#include <shared/cmWinding.h>
bool cmBrush_c::calcBounds() {
	this->bounds.clear();
	for(u32 i = 0; i < sides.size(); i++) {
		const plane_c &iPlane = sides[i].pl;

		cmWinding_c winding;
		winding.createBaseWindingFromPlane(iPlane);
		for(u32 j = 0; j < sides.size(); j++) {
			if(i == j)
				continue;
			const plane_c &jPlane = sides[j].pl;
			winding.clipWindingByPlane(jPlane.getOpposite());
		}
		winding.addPointsToBounds(this->bounds);
	}
	return false; // ok
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
		cmBrushSide_c &newSide = sides.pushBack();
		newSide.matName = p.getToken();
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
		newSide.pl.fromThreePointsINV(p0,p1,p2);
	}
	return false; // no error
}
bool cmBrush_c::parseBrushD3(class parser_c &p) {
	// new brush format (with explicit plane equations)
	// Number of sides isnt explicitly specified,
	// so parse until the closing brace is hit
	if(p.atWord("{") == false) {
		g_core->RedWarning("brush_c::parseBrushD3: expected { to follow brushDef3, found %s at line %i of file %s\n",
			p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
			return true;
	}
	while(p.atWord_dontNeedWS("}") == false) {
		//  ( -0 -0 -1 1548 ) ( ( 0.0078125 0 -4.0625004768 ) ( -0 0.0078125 0.03125 ) ) "textures/common/clip" 0 0 0
		plane_c sidePlane;

		if(p.atWord("(") == false) {
			g_core->RedWarning("brush_c::parseBrushD3: expected ( to follow brushSide plane equation, found %s at line %i of file %s\n",
				p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
			return true;
		}

		p.getFloatMat(sidePlane.norm,3);
		sidePlane.dist = p.getFloat();

		if(p.atWord(")") == false) {
			g_core->RedWarning("brush_c::parseBrushD3: expected ) after brushSide plane equation, found %s at line %i of file %s\n",
				p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
			return true;
		}

		if(p.atWord("(") == false) {
			g_core->RedWarning("brush_c::parseBrushD3: expected ( to follow brushSide texMat, found %s at line %i of file %s\n",
				p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
			return true;
		}

		vec3_c mat[2];
		if(p.getFloatMat_braced(mat[0],3)) {
			g_core->RedWarning("brush_c::parseBrushD3: failed to read brush texMat at line %i of file %s\n",
				p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
			return true;
		}
		if(p.getFloatMat_braced(mat[1],3)) {
			g_core->RedWarning("brush_c::parseBrushD3: failed to read brush texMat at line %i of file %s\n",
				p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
			return true;
		}

		if(p.atWord(")") == false) {
			g_core->RedWarning("brush_c::parseBrushD3: expected ) after brushSide texMat, found %s at line %i of file %s\n",
				p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
			return true;
		}
		cmBrushSide_c &newSide = sides.pushBack();
		newSide.pl = sidePlane;
		// get material name
		newSide.matName = p.getToken();
		// after material name we usually have 3 zeroes
	
		p.skipLine();
	}
	if(p.atWord("}") == false) {
		g_core->RedWarning("brush_c::parseBrushD3: expected } after brushDef3, found %s at line %i of file %s\n",
			p.getToken(),p.getCurrentLineNumber(),p.getDebugFileName());
		return true;
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
		const plane_c &pl = sides[i].pl;
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
