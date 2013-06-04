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
// cg_emitter.h
#ifndef __CG_EMITTER_H__
#define __CG_EMITTER_H__

#include <api/customRenderObjectAPI.h>
#include <shared/array.h>
#include <math/vec3.h>

struct emitterParticle_s {
	vec3_c origin;
	int spawnTime;
};

class emitter_c : public customRenderObjectAPI_i {
	arraySTD_c<emitterParticle_s> particles;

	class mtrAPI_i *mat;
	float spriteRadius;
	int spawnInterval;
	int particleLife;

	vec3_c origin;
	int lastSpawnTime;
	int lastUpdateTime;

	void spawnSingleParticle(int curTime);

	virtual void instanceModel(class staticModelCreatorAPI_i *out, const class axis_c &viewerAxis);
public:
	emitter_c(int timeNow);
	~emitter_c();

	void updateEmitter(int newTime);

	void setOrigin(const vec3_c &newOrigin);
	void setRadius(float newSpriteRadius);
	void setInterval(int newSpawnInterval);
	void setMaterial(class mtrAPI_i *newMat);
};

#endif // __CG_EMITTER_H__
