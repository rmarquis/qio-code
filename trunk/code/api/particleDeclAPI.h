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
// particleDeclAPI.h - Doom3 particle decl interface
#ifndef __PARTICLEDECLAPI_H__
#define __PARTICLEDECLAPI_H__

#include <shared/random.h>
#include <math/axis.h>

struct particleInstanceData_s {
	// particle index in the system
	u32 particleIndex;
	// particles life fraction, from 0.0 to 1.0
	float lifeFrac;
	// random generator with seed for this particle
	rand_c rand;
	// viewer axis
	axis_c viewAxis;
};

class particleStageAPI_i {
public:
	virtual u32 getParticleCount() const = 0;
	virtual const char *getMatName() const = 0;
	virtual u32 getCycleMSec() const = 0;
	virtual float getTime() const = 0;
	virtual float getSpawnBunching() const = 0;
	// returns the number of created vertices
	virtual u32 instanceParticle(particleInstanceData_s &in, struct simpleVert_s *verts) const = 0;
};

class particleDeclAPI_i {
public:
	virtual const char *getName() const = 0;
	virtual u32 getNumStages() const = 0;
	virtual const class particleStageAPI_i *getStage(u32 i) const = 0;
};

#endif // __PARTICLEDECLAPI_H__
