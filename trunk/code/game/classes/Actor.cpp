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
// Actor.cpp
#include "Actor.h"
#include <api/serverAPI.h>

DEFINE_CLASS(Actor, "ModelEntity");
// Doom3 AI
//DEFINE_CLASS_ALIAS(Actor, idAI);
 
Actor::Actor() {
	health = 100;
	bTakeDamage = true;
}
void Actor::setKeyValue(const char *key, const char *value) {
	ModelEntity::setKeyValue(key,value);
}
void Actor::postSpawn() {
	if(cmod) {
		// create a rigid body (physics prop)
		initRigidBodyPhysics();
	}
}


