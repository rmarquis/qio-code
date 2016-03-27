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
// FuncSpawn.cpp - FAKK func_spawn
// Can be tested on funcspawn.bsp from FAKK sdk.
// "modelname" (NOT "model") key defines what will be spawned.
#include "FuncSpawn.h"
#include "../g_local.h"

DEFINE_CLASS(FuncSpawn, "BaseEntity");
DEFINE_CLASS_ALIAS(FuncSpawn, func_spawn);
 
FuncSpawn::FuncSpawn() {

}
void FuncSpawn::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"modelname")) {
		// eg. "modelname creeper.tik" - see funcspawn.bsp from FAKK sdk
		modelToSpawn = value;
	} else {
		BaseEntity::setKeyValue(key,value);
	}
}
void FuncSpawn::triggerBy(class BaseEntity *from, class BaseEntity *activator) {
	if(modelToSpawn.size()==0)
		return;
	BaseEntity *be = G_SpawnGeneric(modelToSpawn);	
	if(be == 0) {
		return;
	}
	be->setOrigin(this->getOrigin());
	be->postSpawn();
}


