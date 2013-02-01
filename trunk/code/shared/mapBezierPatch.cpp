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
// mapBezierPatch.cpp - bezier patch data loaded from .map file
#include "mapBezierPatch.h"
#include <shared/parser.h>
#include <api/coreAPI.h>
#include <shared/cmSurface.h>

bool mapBezierPatch_c::parse(class parser_c &p) {
	bool isPatchDef3 = false;
	if(p.atWord("patchDef3")) {
		isPatchDef3 = true;
	} else if(p.atWord("patchDef2")) {
	
	} else {
		g_core->RedWarning("mapBezierPatch_c::fromString: unknown patch type %s\n",p.getToken());
		return true;
	}
	if(p.atWord("{") == false) {
		g_core->RedWarning("mapBezierPatch_c::fromString: Expected '{' to folow patchDef*, found %s\n",p.getToken());
		return true;
	}
	p.getToken(this->matName);
	if(p.atWord("(") == false) {
		g_core->RedWarning("mapBezierPatch_c::fromString: Expected '(' to folow patchDef*, found %s\n",p.getToken());
		return true;
	}
	width = p.getInteger();
	height = p.getInteger();
	if(isPatchDef3) {
		p.getInteger(); // skip horizontal subdivisions
		p.getInteger(); // skip vertical subdivisions
	}
	p.getToken(); // skip contents
	p.getToken(); // skip flags
	p.getToken(); // skip "values"
	if(p.atWord(")") == false) {
		g_core->RedWarning("mapBezierPatch_c::fromString: Expected ')' to folow patchDef*, found %s\n",p.getToken());
		return true;
	}
	if(p.atWord("(") == false) {
		g_core->RedWarning("mapBezierPatch_c::fromString: Expected '(' to folow patchDef* points, found %s\n",p.getToken());
		return true;
	}
	verts.resize(width*height);
	for(u32 i = 0; i < width; i++) {
		if(p.atWord("(") == false) {
			g_core->RedWarning("mapBezierPatch_c::fromString: Expected '(' to folow patchDef* points, found %s\n",p.getToken());
			return true;
		}
		for(u32 j = 0; j < height; j++) {
			if(p.atWord("(") == false) {
				g_core->RedWarning("mapBezierPatch_c::fromString: Expected '(' to folow patchDef* points, found %s\n",p.getToken());
				return true;
			}
			simpleVert_s &vp = this->verts[j*width+i];
			p.getFloatMat(vp.xyz,3);
			p.getFloatMat(vp.tc,2);
			if(p.atWord(")") == false) {
				g_core->RedWarning("mapBezierPatch_c::fromString: Expected ')' to folow patchDef* points, found %s\n",p.getToken());
				return true;
			}
		}
		if(p.atWord(")") == false) {
			g_core->RedWarning("mapBezierPatch_c::fromString: Expected ')' to folow patchDef* points, found %s\n",p.getToken());
			return true;
		}
	}
	if(p.atWord(")") == false) {
		g_core->RedWarning("mapBezierPatch_c::fromString: Expected ')' after patchDef* points, found %s\n",p.getToken());
		return true;
	}
	if(p.atWord("}") == false) {
		g_core->RedWarning("mapBezierPatch_c::fromString: Expected '}' closing patchDef*, found %s\n",p.getToken());
		return true;
	}
	return false;
}
bool mapBezierPatch_c::fromString(const char *pDefStart, const char *pDefEnd) {
	parser_c p;
	p.setup(pDefStart);
	bool parseError = this->parse(p);
	return parseError;
}
void mapBezierPatch_c::getLowestDetailCMSurface(class cmSurface_c *out) const {
	// copy vertices
	out->setNumVerts(verts.size());
	for(u32 i = 0; i < verts.size(); i++) {
		out->setVert(i,verts[i]);
	}
	// create indices
	out->setNumIndices((width-1)*(height-1)*6);
	u32 idx = 0;
	for(u32 i = 1; i < width; i++) {
		for(u32 j = 1; j < height; j++) {
			u32 i0 = j*width+i;
			u32 i1 = j*width+(i-1);
			u32 i2 = (j-1)*width+i;
			u32 i3 = (j-1)*width+(i-1);

			out->setIndex(idx,i2);
			idx++;
			out->setIndex(idx,i1);
			idx++;
			out->setIndex(idx,i0);
			idx++;

			out->setIndex(idx,i1);
			idx++;
			out->setIndex(idx,i2);
			idx++;
			out->setIndex(idx,i3);
			idx++;
		}
	}
}