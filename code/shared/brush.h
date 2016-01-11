/*
============================================================================
Copyright (C) 2015 V.

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
// brush.h
#ifndef __SHARED_BRUSH_H__
#define __SHARED_BRUSH_H__

#include "brushside.h"

// temporary?
// lower bits are stronger, and will eat weaker brushes completely
#define	CONTENTS_SOLID			1		// an eye is never valid in a solid

#define	CONTENTS_DETAIL			    0x8000000	  // brushes to be added after vis leafs
#define	CONTENTS_NEGATIVE_CURVE 0x40000000	// reverse inside / outside

#define	CONTENTS_KEEP	(CONTENTS_DETAIL | CONTENTS_NEGATIVE_CURVE)





#define SURF_PATCH        0x20000000
#define	SURF_CURVE_FAKE		0x40000000
#define	SURF_CURVE		    0x80000000
#define	SURF_KEEP		(SURF_CURVE | SURF_CURVE_FAKE | SURF_PATCH)



class brush_c {
protected:
	aabb bounds;
	face_s *brush_faces;
	bool bWindingsGenerationFailed;

public:
	brush_c();
	~brush_c();

	void setupSphere(const vec3_c &mid, u32 sides, float radius, const texdef_t *texdef);
	void setupBox(const vec3_c &mins, const vec3_c &maxs, const texdef_t *texdef);
	void makeFacePlanes();
	void fitTexture(int nHeight, int nWidth);
	void removeEmptyFaces();
	void snapPlanePoints();
	texturedWinding_c *makeFaceWinding (face_s *face);

	
	face_s *getFirstFace() {
		return brush_faces;
	}
};

#endif // __SHARED_BRUSH_H__