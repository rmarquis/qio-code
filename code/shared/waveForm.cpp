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
// waveForm.cpp
#include "waveForm.h"
#include <api/coreAPI.h> // only for Print/RedWarning
#include <shared/parser.h>
#include <math/math.h>

float waveForm_c::sinTable[BASETABLE_SIZE];
float waveForm_c::squareTable[BASETABLE_SIZE];
float waveForm_c::triangleTable[BASETABLE_SIZE];
float waveForm_c::sawToothTable[BASETABLE_SIZE];
float waveForm_c::inverseSawToothTable[BASETABLE_SIZE];
bool waveForm_c::initialized = false;

void waveForm_c::initTables() {
	if(initialized)
		return;
	initialized = true;
	for(int i = 0; i < BASETABLE_SIZE; i++) {
		sinTable[i] = sin(DEG2RAD(i * 360.0f / ((float)(BASETABLE_SIZE - 1))));
		squareTable[i] = (i < BASETABLE_SIZE / 2) ? 1.0f : -1.0f;
		sawToothTable[i] = (float)i / BASETABLE_SIZE;
		inverseSawToothTable[i] = 1.0f - sawToothTable[i];

		if(i < BASETABLE_SIZE / 2) {
			if(i < BASETABLE_SIZE / 4) {
				triangleTable[i] = (float)i / (BASETABLE_SIZE / 4);
			} else {
				triangleTable[i] = 1.0f - triangleTable[i - BASETABLE_SIZE / 4];
			}
		} else {
			triangleTable[i] = -triangleTable[i - BASETABLE_SIZE / 2];
		}
	}
}

bool waveForm_c::parseParameters(class parser_c &p) {
	// BASE, AMP, PHASE, FREQ
	this->base = p.getFloat();
	this->amplitude = p.getFloat();
	this->phase = p.getFloat();
	this->frequency = p.getFloat();
	return false; // OK
}
bool waveForm_c::parse(class parser_c &p) {
	if(p.atWord("sin")) {
		this->type = GF_SIN;
	} else if(p.atWord("square")) {
		this->type = GF_SQUARE;
	} else if(p.atWord("triangle")) {
		this->type = GF_TRIANGLE;
	} else if(p.atWord("sawtooth")) {
		this->type = GF_SAWTOOTH;
	} else if(p.atWord("inversesawtooth")) {
		this->type = GF_INVERSE_SAWTOOTH;
	} else if(p.atWord("noise")) {
		this->type = GF_NOISE;
	} else {
		g_core->RedWarning("waveForm_c::parse: unknown waveForm func type %s in file %s at line %i\n",p.getToken(),p.getDebugFileName(),p.getCurrentLineNumber());
		return true; // error
	}
	return parseParameters(p);
}
const float *waveForm_c::getTable() const {
	if(type == GF_SIN) {
		return sinTable;
	}
	if(type == GF_SQUARE) {
		return squareTable;
	}
	if(type == GF_TRIANGLE) {
		return triangleTable;
	}
	if(type == GF_SAWTOOTH) {
		return sawToothTable;
	}
	if(type == GF_INVERSE_SAWTOOTH) {
		return inverseSawToothTable;
	}
	g_core->RedWarning("waveForm_c::getTable: type %i not handled\n",type);
	return 0;
}
float waveForm_c::evaluate(float curTimeSeconds, float extraPhase) const {
	const float *table = getTable();
	if(table == 0)
		return 0;
	float val = table[ int( ( ( phase + extraPhase + curTimeSeconds * frequency ) * BASETABLE_SIZE ) ) & BASETABLE_MASK ];
	val *= amplitude;
	val += base;
	return val;
}
float waveForm_c::genericSinValue(float at) {
	int off = float( float(BASETABLE_SIZE) / (M_PI*2.f) ) * at;
	return sinTable[ off & BASETABLE_MASK ];
}



