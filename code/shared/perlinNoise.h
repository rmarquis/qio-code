/*
============================================================================
Copyright (C) 2016 V.

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
// perlinNoise.h
#ifndef __SHARED_PERLIN_H__
#define __SHARED_PERLIN_H__

#include "../api/noise2DAPI.h"

#include <stdlib.h>
#define PN_SAMPLE_SIZE 1024

struct pnDef_s {
	int octaves;
	float freq;
	float amp;
	int seed;

	pnDef_s() {
		octaves = 2;
		freq = 4;
		amp = 1;
		seed = 94;
	}
	bool operator != (const pnDef_s &o) const {
		if(this->octaves != o.octaves)
			return true;
		if(this->amp != o.amp)
			return true;
		if(this->freq != o.freq)
			return true;
		if(this->seed != o.seed)
			return true;
		return false;
	}
};

class pn_c : public noise2DAPI_i {
	// variables
	int   mOctaves;
	float mFrequency;
	float mAmplitude;
	int   mSeed;

	// tables
	int p[PN_SAMPLE_SIZE + PN_SAMPLE_SIZE + 2];
	float g3[PN_SAMPLE_SIZE + PN_SAMPLE_SIZE + 2][3];
	float g2[PN_SAMPLE_SIZE + PN_SAMPLE_SIZE + 2][2];
	float g1[PN_SAMPLE_SIZE + PN_SAMPLE_SIZE + 2];
	bool  mStart;

	// functions
	float perlinNoise2D(float vec[2]);
	float noise1(float arg);
	float noise2(float vec[2]);
	float noise3(float vec[3]);
	void normalize2(float v[2]);
	void normalize3(float v[3]);
	void init();
public:
	void setupPerlin(int octaves,float freq,float amp,int seed) {
		mOctaves = octaves;
		mFrequency = freq;
		mAmplitude = amp;
		mSeed = seed;
		mStart = true;
	}
	void setupPerlin(const pnDef_s &d) {
		setupPerlin(d.octaves,d.freq,d.amp,d.seed);
	}
	void setupPerlin(const pnDef_s *d) {
		if(d == 0) {
			pnDef_s tmp;
			setupPerlin(tmp);
		} else {
			setupPerlin(*d);
		}
	}


	virtual float sampleNoise2D(float x,float y) {
		float vec[2];
		vec[0] = x;
		vec[1] = y;
		return perlinNoise2D(vec);
	};
};

#endif // __SHARED_PERLIN_H__
