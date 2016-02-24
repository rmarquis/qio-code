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
// Mover.cpp
#include "Mover.h"
#include <api/serverAPI.h>
#include <math/aabb.h>
#include <shared/autoCvar.h>
#include "../g_local.h"

DEFINE_CLASS(Mover, "ModelEntity");
DEFINE_CLASS_ALIAS(Mover, func_mover);

static aCvar_c g_mover_warp("g_mover_warp","0");
bool g_mover_callingMoverTeam = false;

Mover::Mover() {
	bPhysicsBodyKinematic = true;
	bRigidBodyPhysicsEnabled = true;
	lip = 4.f;
	moverState = MOVER_POS1;
	speed = 40.f;
	mass = 0.f;
	direction.set(1,0,0);
}
void Mover::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"angle")) {
		float f = atof(value);
		if(f == -1.f) {
			direction.set(0,0,1);
		} else if(f == -2.f) {
			direction.set(0,0,-1);
		} else {
			direction = vec3_c(0,f,0).getForward();
		}
		//g_logger->printMsg("Door %i angle %f\n",getEntNum(),f);
	} else if(!stricmp(key,"angles")) {
		direction = vec3_c(value).getForward();
	} else if(!stricmp(key,"lip")) {
		lip = atof(value);
	} else if(!stricmp(key,"team")) {
		team = value;
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
void Mover::postSpawn() {
	distance = abs(direction.dotProduct(getAbsBounds().getSizes())) - lip;
	pos1 = getOrigin();
	pos2 = pos1 + distance * direction;
	if(distance < 0)
		direction *= -1;
	ModelEntity::postSpawn();
}
void Mover::runFrame() {
	if(moverState != MOVER_1TO2 && moverState != MOVER_2TO1)
		return;
	float delta = level.frameTime * speed;
	vec3_c dest;
	if(moverState == MOVER_1TO2) {
		dest = pos2;
	} else if(moverState == MOVER_2TO1) {
		dest = pos1;
		delta *= -1.f;
	}
	float remaining = dest.dist(this->getOrigin());
	if(abs(delta) > remaining) {
		this->setOrigin(dest);
		if(moverState == MOVER_1TO2) {
			moverState = MOVER_POS2;
			// fire reach events
			onMoverReachPos2();
		} else {
			moverState = MOVER_POS1;
			// fire reach events
			onMoverReachPos1();
			// close areaportal
			closeAreaPortalIfPossible();
		}
		return;
	}
	vec3_c p = this->getOrigin();
	p += direction * delta;
	this->setOrigin(p);
}
bool Mover::doUse(class Player *activator) {
	// also activate all other objects with the same team field value,
	// but avoid stack overflow
	if(g_mover_callingMoverTeam == false) {
		g_mover_callingMoverTeam = true;
		if(team.length()) {
			arraySTD_c<Mover*> movers;
			G_FindMoversWithTeam(movers,team);
			for(u32 i = 0; i < movers.size(); i++) {
				if(movers[i] != this)
					movers[i]->doUse(activator);
			}
		}
		g_mover_callingMoverTeam = false;
	}
	if(moverState == MOVER_POS1) {
		if(g_mover_warp.getInt()) {
			this->setOrigin(pos2);
			moverState = MOVER_POS2;
			// fire reach events
			onMoverReachPos2();
		} else {
			openAreaPortalIfPossible();
			moverState = MOVER_1TO2;
		}
	} else if(moverState == MOVER_POS2) {
		if(g_mover_warp.getInt()) {
			this->setOrigin(pos1);
			moverState = MOVER_POS1;
			// fire reach events
			onMoverReachPos1();
		} else {
			moverState = MOVER_2TO1;
		}
	}
	return true;
}


