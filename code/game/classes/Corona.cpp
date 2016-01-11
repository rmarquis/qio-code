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
// Corona.cpp - Corona class for ET support.
// In ET coronas are handled separately in renderer,
// see RB_AddCoronaFlares,
// trap_R_AddCoronaToScene,
// RB_RenderFlare
// They are always drawn with tr.flareShader
// which is: "flareShader"
#include "Corona.h"
#include <shared/entityType.h>

DEFINE_CLASS(Corona, "ModelEntity");

Corona::Corona() {
	//this->setEntityType(ET_CORONA);
	//this->setRenderModel("*qioCoronaTest");
	scale = 1.f;
	updateCoronaModel();
}

void Corona::updateCoronaModel() {
	// let's use Qio model-sprite system now (altough dedicated system would be faster)
	float radius = scale * 32.f;
	char tmp[512];
	sprintf(tmp,"flareShader|sprite|radius,%f",radius);
	this->setRenderModel(tmp);
}
void Corona::setKeyValue(const char *key, const char *value){ 
	if(!stricmp(key,"angle")) {

	} else if(!stricmp(key,"scale")) {
		scale = atof(value);
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
