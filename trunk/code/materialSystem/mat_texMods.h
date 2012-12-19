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
// mat_texmods.h
#ifndef __MAT_TEXMODS_H__
#define __MAT_TEXMODS_H__

#include <shared/array.h>
#include <shared/waveForm.h>

enum texModType_e {
	TCMOD_BAD,
	TCMOD_SCROLL,
	TCMOD_TRANSFORM,
	TCMOD_SCALE,
	TCMOD_STRETCH,
	TCMOD_TURBULENT,
	TCMOD_ROTATE,
	TCMOD_NUM_TEXMODS,
};

class texMod_c {
	texModType_e type;
	union {
		float scroll[2];
		// for tcmod tranform
		struct {
			float matrix[2][2];		// s' = s * m[0][0] + t * m[1][0] + trans[0]
			float translate[2];		// t' = s * m[0][1] + t * m[0][1] + trans[1]
		};
		float scale[2];
		float rotationSpeed;
		waveForm_c wave;
	};
public:
	bool parse(class parser_c &p);
	// appends a tcmod transform to given matrix
	void appendTransform(class matrix_c &mat, float timeNowSeconds);
};

class texModArray_c : public arraySTD_c<texMod_c> {
public:
	void calcTexMatrix(class matrix_c &out, float timeNowSeconds);
};


#endif // __MAT_TEXMODS_H__
