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
// brushside.h
#ifndef __SHARED_BRUSHSIDE_H__
#define __SHARED_BRUSHSIDE_H__

#include <shared/str.h>
#include <math/vec3.h>
#include <math/plane.h>
#include <math/aabb.h>

// old brush format
struct texdef_t {
	str matName;
	float	shift[2];
	float	rotate;
	float	scale[2];
	int		contents;
	int		flags;
	int		value;

	
	texdef_t() {
		scale[0] = scale[1] = 1.f;
		rotate = 0.f;
		shift[0] = shift[1] = 0.f;
	}
	void setName(const char *s) {
		matName = s;
	}
	const char *getName() const {
		return matName;
	}
};
// new brush format
struct brushprimit_texdef_s {
	float	coords[2][3];
};

#include <shared/texturedWinding.h>



extern int g_allocatedCounter_face;

class face_s
{
friend class brush_c;
friend class edBrush_c;

public:
	// face order (linked list)
	class face_s *next;
	// used for vertex movement
	class face_s *original;		
	// face shape def
	vec3_c planepts[3];
	plane_c plane;
	// texturing
	texdef_t texdef;
	brushprimit_texdef_s	brushprimit_texdef;
	vec3_t d_color;
	class mtrAPI_i *d_texture;
	// derived data
	texturedWinding_c *face_winding;
public:
	face_s();
	~face_s();

	void calculatePlaneFromPoints();
	void convertFaceToBrushPrimitFace();
	face_s *cloneFace();
	void fitTexture(int nHeight, int nWidth );
	void calcTextureVectors(float STfromXYZ[2][4]) const;
	void calcTextureCoordinates (class texturedVertex_c &out) const;
	void calcBrushPrimitTextureCoordinates(texturedWinding_c *w);
	void rotateFaceTexture_BrushPrimit(int nAxis, float fDeg, const vec3_c &vOrigin);
	void moveTexture_BrushPrimit(const vec3_c &delta);
	void shiftTexture_BrushPrimit(int x, int y);
	face_s *cloneFace() const;
	u32 getMemorySize() const;


	void setMatName(const char *s) {
		texdef.matName = s;
		//matName = s;
	}
	const char *getMatName() const {
	///	return matName;
		return texdef.matName;
	}
	face_s *getNextFace() {
		return next;
	}
};

#endif // __SHARED_BRUSHSIDE_H__