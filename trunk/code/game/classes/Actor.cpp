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
#include "../g_local.h"
#include <api/serverAPI.h>
#include <api/coreAPI.h>

DEFINE_CLASS(Actor, "ModelEntity");
// Doom3 AI
//DEFINE_CLASS_ALIAS(Actor, idAI);
 
Actor::Actor() {
	health = 100;
	bTakeDamage = true;
	st = 0;
}
void Actor::loadAIStateMachine(const char *fname) {
	st = G_LoadStateMachine(fname);
	if(st == 0) {
		g_core->RedWarning("Actor::loadAIStateMachine: failed to load %s\n",fname);
	}
}
void Actor::setKeyValue(const char *key, const char *value) {
	if(!stricmp("statemap",key)) {
		loadAIStateMachine(value);
	}
	ModelEntity::setKeyValue(key,value);
}
void Actor::postSpawn() {
	ModelEntity::postSpawn();
}


