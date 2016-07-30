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
// FuncExplosive.cpp
#include "FuncExplosive.h"

#define RTCW_STARTINVIS 1

DEFINE_CLASS(FuncExplosive, "ModelEntity");
DEFINE_CLASS_ALIAS(FuncExplosive, func_explosive);
 
FuncExplosive::FuncExplosive() {
	health = 100;
	bTakeDamage = true;

}
void FuncExplosive::setKeyValue(const char *key, const char *value) {
	if(!_stricmp(key,"type")) {
		this->type = value;
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
void FuncExplosive::postSpawn() {
	if(!stricmp(type,"glass") || !stricmp(type,"fabric")) {
		bRigidBodyPhysicsEnabled = false;
	}
	if(spawnFlags & RTCW_STARTINVIS) {
		// RTCW start_invis
		this->hideEntity();
	} else {
		ModelEntity::postSpawn();
	}
}
