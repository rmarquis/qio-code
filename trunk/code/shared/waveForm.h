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
// waveForm.h
#ifndef __WAVEFORM_H__
#define __WAVEFORM_H__

enum genFunc_e {
	GF_NONE,
	GF_SIN,
	GF_SQUARE,
	GF_TRIANGLE,
	GF_SAWTOOTH,
	GF_INVERSE_SAWTOOTH,
	GF_NOISE,
};

enum {
	BASETABLE_SIZE = 1024,
	BASETABLE_MASK = BASETABLE_SIZE-1,
};

class waveForm_c {
friend class texMod_c; // HACK
	genFunc_e type;
	float base, amplitude, phase, frequency;
	static float sinTable[BASETABLE_SIZE];
	static float squareTable[BASETABLE_SIZE];
	static float triangleTable[BASETABLE_SIZE];
	static float sawToothTable[BASETABLE_SIZE];
	static float inverseSawToothTable[BASETABLE_SIZE];
	static bool initialized;
public:
	static void initTables();
	static float genericSinValue(float at);
	bool parseParameters(class parser_c &p);
	bool parse(class parser_c &p);
	const float *getTable() const;
	float evaluate(float curTimeSeconds, float extraPhase = 0.f) const;
};

#endif // __WAVEFORM_H__
