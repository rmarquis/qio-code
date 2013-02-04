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
// Q3Weapon.cpp
#include "Q3Weapon.h"
#include "Player.h"
//#include "Projectile.h"
#include <api/coreAPI.h>

DEFINE_CLASS(Q3Weapon, "Weapon");

enum quake3WeaponType_e {
	EQ3WPN_BAD,
	EQ3WPN_PLASMAGUN,

	EQ3WPN_NUM_KNOWN_WEAPONS,
};
Q3Weapon::Q3Weapon() {
	q3WeaponType = EQ3WPN_BAD;
}
void Q3Weapon::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"giTag")) {
		// "giTag" field of Quake3 gitem_t structure stores weapon type
		g_core->Print("Q3Weapon::setKeyValue: giTag: %s\n",value);
		if(!stricmp(value,"WP_PLASMAGUN")) {
			q3WeaponType = EQ3WPN_PLASMAGUN;
		}
	} else {
		Weapon::setKeyValue(key,value);
	}
}

void Q3Weapon::doWeaponAttack() {
	if(q3WeaponType == EQ3WPN_PLASMAGUN) {
		ModelEntity *plasma = new ModelEntity;
		plasma->setSpriteModel("sprites/plasma1",32.f);
		plasma->setOrigin(owner->getEyePos());
		//plasma->setVelocity(owner->getViewAngles().getForward()*5.f);
	
	}
}