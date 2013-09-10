/*
============================================================================
Copyright (C) 2012 V.

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
// Light.cpp
#include "Light.h"
#include "../g_local.h"
#include <shared/eventBaseAPI.h>

DEFINE_CLASS(Light, "BaseEntity");
DEFINE_CLASS_ALIAS(Light, light_dynamic);
// Doom3 light classname
DEFINE_CLASS_ALIAS(Light, idLight);
// "HumanHead" light for Prey
DEFINE_CLASS_ALIAS(Light, hhLight);

Light::Light() {
	this->myEdict->s->eType = ET_LIGHT;
	setRadius(512.f);
}
void Light::setRadius(float newRadius) {
	this->radius = newRadius;
	this->myEdict->s->lightRadius = newRadius;
}
void Light::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"light")) {
		// Q3 light value
		float lightKeyValue = atof(value);
		this->setRadius(lightKeyValue);
	} else if(!stricmp(key,"light_radius")) {
		// Doom3 light value? 3 values
		vec3_c sizes(value);
		// FIXME: 
		this->setRadius((sizes.x+sizes.y+sizes.z)/3.f);
	} else if(!stricmp(key,"noshadows")) {
		int bNoShadows = atoi(value);
		if(bNoShadows) {
			this->myEdict->s->lightFlags |= LF_NOSHADOWS;
		} else {
			this->myEdict->s->lightFlags &= ~LF_NOSHADOWS;
		}
	} else {
		BaseEntity::setKeyValue(key,value);
	}
}
// NOTE: we cant use just point pvs for lights,
// because renderer might need them
// even if light center (origin) 
// is outside player PVS
void Light::getLocalBounds(aabb &out) const {
	out.fromHalfSize(radius);
}
void Light::processEvent(class eventBaseAPI_i *ev) {
	if(!stricmp(ev->getEventName(),"onMoverReachPos1")
		||
		!stricmp(ev->getEventName(),"onMoverReachPos2")) {
		toggleEntityVisibility();
	} else {
		BaseEntity::processEvent(ev);
	}
}


