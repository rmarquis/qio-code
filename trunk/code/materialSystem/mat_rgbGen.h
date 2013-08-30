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
// mat_rgbGen.h
#ifndef __MAT_RGBGEN_H__
#define __MAT_RGBGEN_H__

#include <shared/waveForm.h>
#include "mat_public.h"

class rgbGen_c {
	rgbGen_e type;
	union {
		waveForm_c wave;
		float constValue[3];
		class astAPI_i *ast;
	};
public:
	rgbGen_c() {
		type = RGBGEN_NONE;
	}
	~rgbGen_c();

	bool parse(class parser_c &p);
	void setRGBGenAST(class astAPI_i *newAST);
	void evaluateRGBGen(const class astInputAPI_i *in, float *out3Floats) const;

	bool isNone() const {
		return (type == RGBGEN_NONE);
	}
	bool isConst() const {
		return (type == RGBGEN_CONST);
	}
	inline rgbGen_e getType() const {
		return type;
	}
	const waveForm_c &getWaveForm() const {
		return this->wave;
	}
	float getConstRedValueAsByte() const {
		return constValue[0] * 255.f;
	}
	float getConstBlueValueAsByte() const {
		return constValue[2] * 255.f;
	}
	float getConstGreenValueAsByte() const {
		return constValue[1] * 255.f;
	}
	const float *getConstValues() const {
		return constValue;
	}
};

#endif // __MAT_RGBGEN_H__

