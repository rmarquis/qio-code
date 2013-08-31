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
	// Doom3 ast texmods
	TCMOD_D3_ROTATE,
	TCMOD_D3_SCALE,
	TCMOD_D3_SHEAR,
	TCMOD_D3_SCROLL,
	TCMOD_D3_CENTERSCALE,
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
		class astAPI_i *astRotation;
		class astAPI_i *astScale[2];
		class astAPI_i *astShear[2];
		class astAPI_i *astScroll[2];
		waveForm_c wave;
	};
	static void applyRotationToMatrix(class matrix_c &mat, float rot);

	void clear();
public:
	texMod_c();
	texMod_c(const texMod_c &other);
	texMod_c &operator =(const texMod_c &other);
	~texMod_c();

	bool parse(class parser_c &p);
	// appends a tcmod transform to given matrix
	void appendTransform(class matrix_c &mat, float timeNowSeconds, const class astInputAPI_i *in);
	// D3 scriptable texmods
	void setD3TexModRotate(class astAPI_i *ast) {
		type = TCMOD_D3_ROTATE;
		astRotation = ast;
	}
	void setD3TexModScale(class astAPI_i *val0, class astAPI_i *val1) {
		type = TCMOD_D3_SCALE;
		astScale[0] = val0;
		astScale[1] = val1;
	}
	void setD3TexModShear(class astAPI_i *val0, class astAPI_i *val1) {
		type = TCMOD_D3_SHEAR;
		astShear[0] = val0;
		astShear[1] = val1;
	}
	void setD3TexModScroll(class astAPI_i *val0, class astAPI_i *val1) {
		type = TCMOD_D3_SCROLL;
		astScroll[0] = val0;
		astScroll[1] = val1;
	}
	void setD3TexModCenterScale(class astAPI_i *val0, class astAPI_i *val1) {
		type = TCMOD_D3_CENTERSCALE;
		astScale[0] = val0;
		astScale[1] = val1;
	}
};

class texModArray_c : public arraySTD_c<texMod_c> {
public:
	void calcTexMatrix(class matrix_c &out, float timeNowSeconds, const class astInputAPI_i *in);
	// D3 scriptable texmods
	void addD3TexModRotate(class astAPI_i *value);
	void addD3TexModScale(class astAPI_i *val0, class astAPI_i *val1);
	void addD3TexModShear(class astAPI_i *val0, class astAPI_i *val1);
	void addD3TexModScroll(class astAPI_i *val0, class astAPI_i *val1);
	void addD3TexModCenterScale(class astAPI_i *val0, class astAPI_i *val1);
};


#endif // __MAT_TEXMODS_H__
