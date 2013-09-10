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
// cg_emitter_d3.cpp - Doom3 particles (prt) emitter
#include "cg_emitter_d3.h"
#include "cg_local.h"
#include <shared/random.h>
#include <shared/simpleVert.h>
#include <api/particleDeclAPI.h>
#include <api/staticModelCreatorAPI.h>

// customRenderObjectAPI_i impl
void emitterD3_c::instanceModel(class staticModelCreatorAPI_i *out, const class axis_c &viewerAxis) {
	out->clear();
	for(u32 i = 0; i < decl->getNumStages(); i++) {
		const particleStageAPI_i *s = decl->getStage(i);

		int stageAge = cg.time;
		int	stageCycle = stageAge / s->getCycleMSec();
		int	inCycleTime = stageAge - stageCycle * s->getCycleMSec();

		rand_c steppingRandom, steppingRandom2;
		// some particles will be in this cycle, some will be in the previous cycle
		steppingRandom.setSeed(stageCycle << 10);
		steppingRandom2.setSeed((stageCycle-1) << 10);

		particleInstanceData_s in;
		in.viewAxis = viewerAxis;

		simpleVert_s verts[1024];

		u32 numVerts = 0;
		// for each particle in stage
		for(u32 j = 0; j < s->getParticleCount(); j++) {
			// bump the random
			steppingRandom.getRandomInt();
			steppingRandom2.getRandomInt();

			// calculate local age for this index 
			int	bunchOffset = s->getTime() * 1000 * s->getSpawnBunching() * j / s->getParticleCount();

			int particleAge = stageAge - bunchOffset;
			int	particleCycle = particleAge / s->getCycleMSec();
			if ( particleCycle < 0 ) {
				// before the particleSystem spawned
				continue;
			}

			if (particleCycle == stageCycle) {
				in.rand = steppingRandom;
			} else {
				in.rand = steppingRandom2;
			}
			int	inCycleTime = particleAge - particleCycle * s->getCycleMSec();

			// supress particles before or after the age clamp
			in.lifeFrac = float(inCycleTime) / ( s->getTime() * 1000 );
			if (in.lifeFrac < 0.0f) {
				// yet to be spawned
				continue;
			}
			if (in.lifeFrac > 1.0f) {
				// this particle is in the deadTime band
				continue;
			}

			numVerts += s->instanceParticle(in,&verts[numVerts]);
		}
		u16 indices[4096];
		u32 numIndices = 0;
		for(u32 j = 0; j < numVerts; j+=4) {
			indices[numIndices+0] = j;
			indices[numIndices+1] = j+2;
			indices[numIndices+2] = j+3;
			indices[numIndices+3] = j;
			indices[numIndices+4] = j+3;
			indices[numIndices+5] = j+1;
			numIndices += 6;
		}
		out->addSurface(s->getMatName(),verts,numVerts,indices,numIndices);
	}
	out->translateXYZ(this->origin);
}

void emitterD3_c::setOrigin(const vec3_c &newOrigin) {
	origin = newOrigin;
}
void emitterD3_c::setParticleDecl(class particleDeclAPI_i *newDecl) {
	decl = newDecl;
}


