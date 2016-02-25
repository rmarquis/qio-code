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
// FuncInvisibleUser.cpp
#include "FuncInvisibleUser.h"
#include "Player.h"
#include "../g_local.h"
#include <api/coreAPI.h>

DEFINE_CLASS(FuncInvisibleUser, "ModelEntity");
DEFINE_CLASS_ALIAS(FuncInvisibleUser, func_invisible_user);
 
FuncInvisibleUser::FuncInvisibleUser() {

}
bool FuncInvisibleUser::doUse(class Player *activator) {
	// fire targets, for example, target_script_trigger entities
	arraySTD_c<BaseEntity *> ents;
	G_GetEntitiesWithTargetName(getTarget(),ents);
	for(u32 i = 0; i < ents.size(); i++) {
		BaseEntity *e = ents[i];
		e->triggerBy(this,activator);
	}
	return true;
}
void FuncInvisibleUser::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"target")) {
		target = value;
	} else if(!stricmp(key,"scriptname")) {
		scriptName = value;
	} else if(!stricmp(key,"model")) {
		setColModel(value);
	}
	ModelEntity::setKeyValue(key,value);
}
void FuncInvisibleUser::postSpawn() {

}


