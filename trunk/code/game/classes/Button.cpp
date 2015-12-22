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
// Button.cpp
#include "Button.h"
#include "../g_local.h"
#include <shared/autoCvar.h>

static aCvar_c g_mover_warp("g_mover_warp","0");

DEFINE_CLASS(Button, "ModelEntity");
DEFINE_CLASS_ALIAS(Button, func_button);

Button::Button() {
	///bRigidBodyPhysicsEnabled = false;
	lip = 4.f;
	state = MOVER_POS1;
	speed = 40.f;
}

void Button::setKeyValue(const char *key, const char *value) {
	if(!_stricmp(key,"lip")) {
		lip = atof(value);
	} else if(!_stricmp(key,"angle")) {
		moverAngles.set(0,atof(value),0);
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
void Button::postSpawn() {
	vec3_c size = getAbsBounds().getSizes();
	moveDir = moverAngles.getForward();
	vec3_c absMoveDir(abs(moveDir.x),abs(moveDir.y),abs(moveDir.z));
	float dist = absMoveDir.x * size.x + absMoveDir.y * size.y + absMoveDir.z * size.z - lip;
	pos1 = this->getOrigin();
	pos2.vectorMA(pos1,moveDir,dist);
}
bool Button::doUse(class Player *activator) {
	if(state == MOVER_POS1) {
		if(g_mover_warp.getInt()) {
			this->setOrigin(pos2);
			state = MOVER_POS2;
			// fire reach events
			onMoverReachPos2();
		} else {
			state = MOVER_1TO2;
		}
	} else if(state == MOVER_POS2) {
		if(g_mover_warp.getInt()) {
			this->setOrigin(pos1);
			state = MOVER_POS1;
			// fire reach events
			onMoverReachPos1();
		} else {
			state = MOVER_2TO1;
		}
	}
	return true;
}
void Button::runFrame() {
	if(state != MOVER_1TO2 && state != MOVER_2TO1)
		return;
	float delta = level.frameTime * speed;
	vec3_c dest;
	if(state == MOVER_1TO2) {
		dest = pos2;
	} else if(state == MOVER_2TO1) {
		dest = pos1;
		delta *= -1.f;
	}
	float remaining = dest.dist(this->getOrigin());
	if(abs(delta) > remaining) {
		this->setOrigin(dest);
		if(state == MOVER_1TO2) {
			state = MOVER_POS2;
			// fire reach events
			onMoverReachPos2();
		} else {
			state = MOVER_POS1;
			// fire reach events
			onMoverReachPos1();
		}
		return;
	}
	vec3_c p = this->getOrigin();
	p += moveDir * delta;
	this->setOrigin(p);
}
void Button::onMoverReachPos1() {
	G_PostEvent(getTarget(),0,"onMoverReachPos1");
}
void Button::onMoverReachPos2() {
	G_PostEvent(getTarget(),0,"onMoverReachPos2");
}


