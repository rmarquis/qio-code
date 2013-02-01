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
// mapBezierPatch.h - bezier patch data loaded from .map file
#ifndef __MAPBEZIERPATCH_H__
#define __MAPBEZIERPATCH_H__

#include <shared/simpleVert.h>
#include <shared/array.h>
#include <shared/str.h>

class mapBezierPatch_c {
	u32 width;
	u32 height;
	arraySTD_c<simpleVert_s> verts;
	str matName;
public:
	// parse .map file brushDef2/brushDef3 data
	bool parse(class parser_c &p);
	bool fromString(const char *pDefStart, const char *pDefEnd);
	// create bezier patch vertices/indices data at lowest possible level of detail
	// (without any tesselation)
	void getLowestDetailCMSurface(class cmSurface_c *out) const;

	u32 getNumVerts() const {
		return verts.size();
	}
	const simpleVert_s &getVert(u32 vertIndex) const {
		return verts[vertIndex];
	}
	u32 getHeight() const {
		return height;
	}
	u32 getWidth() const {
		return width;
	}
	const char *getMatName() const {
		return matName;
	}
};

#endif // __MAPBEZIERPATCH_H__
