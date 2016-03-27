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
// MiscGameModel.cpp
#include "MiscGameModel.h"
#include "../edict.h"
#include <api/coreAPI.h>

DEFINE_CLASS(MiscGameModel, "ModelEntity");
DEFINE_CLASS_ALIAS(MiscGameModel, misc_gamemodel);

MiscGameModel::MiscGameModel() {
}

void MiscGameModel::setKeyValue(const char *key, const char *value) {
	g_core->Print("MiscGameModel::setKeyValue: key %s, value %s\n",key,value);
	if(!stricmp(key,"frames")) {
		numFrames = atoi(value);
	} else if(!stricmp(key,"physics")) {
		if(atoi(value)) {
			bUseRModelToCreateDynamicCVXShape = true;
			bUseDynamicConvexForTrimeshCMod = true;
		} else {
			bUseRModelToCreateDynamicCVXShape = false;
			bUseDynamicConvexForTrimeshCMod = false;
		}
	} else if(!stricmp(key,"fps")) {
		atof(value);
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}

void MiscGameModel::runFrame() {
	this->myEdict->s->animIndex++;
	this->myEdict->s->animIndex %= numFrames;

	ModelEntity::runFrame();
}
