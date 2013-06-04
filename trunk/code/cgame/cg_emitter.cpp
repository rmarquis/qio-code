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
// cg_emitter.cpp
#include "cg_emitter.h"
#include <api/rAPI.h>
#include <api/coreAPI.h>
#include <api/staticModelCreatorAPI.h>

emitter_c::emitter_c(int timeNow) {
	lastSpawnTime = timeNow;

	particleLife = 500;
}
emitter_c::~emitter_c() {
	rf->removeCustomRenderObject(this);
}

void emitter_c::instanceModel(class staticModelCreatorAPI_i *out, const class axis_c &viewerAxis) {
	g_core->Print("emitter_c::instanceModel: at %f %f %f\n",origin.x,origin.y,origin.z);

	out->clear();
	for(int i = particles.size()-1; i >= 0; i--) {
		const emitterParticle_s &p = particles[i];
		int elapsed = lastUpdateTime - p.spawnTime;
		float lifeFracInv = float(elapsed)/float(particleLife);
		float lifeFrac = 1.f - lifeFracInv;
		float radius = spriteRadius + 30.f * lifeFracInv;
		byte alpha = 50 * lifeFrac;
		out->addSprite(p.origin,radius,this->mat,viewerAxis,alpha);
	}
}
void emitter_c::spawnSingleParticle(int curTime) {
	emitterParticle_s np;
	np.spawnTime = curTime;
	np.origin = origin;
	particles.push_back(np);
}
void emitter_c::updateEmitter(int newTime) {
	lastUpdateTime = newTime;
	// spawn new particles
	while(newTime - lastSpawnTime > spawnInterval) {
		spawnSingleParticle(lastSpawnTime+1);
		lastSpawnTime += spawnInterval;
	}
	// remove old particles
	for(int i = 0; i < particles.size(); i++) {
		if(newTime - particles[i].spawnTime > particleLife) {
			particles.erase(i);
			i--;
		}
	}
}
void emitter_c::setOrigin(const vec3_c &newOrigin) {
	origin = newOrigin;
}
void emitter_c::setRadius(float newSpriteRadius) {
	spriteRadius = newSpriteRadius;
}
void emitter_c::setInterval(int newSpawnInterval) {
	spawnInterval = newSpawnInterval;
}
void emitter_c::setMaterial(class mtrAPI_i *newMat) {
	mat = newMat;
}



