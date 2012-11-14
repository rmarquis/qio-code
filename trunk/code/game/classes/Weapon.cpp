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
// Weapon.cpp
#include "../g_local.h"
#include "Weapon.h"
#include "Player.h"
#include <api/coreAPI.h>

DEFINE_CLASS(Weapon, "ModelEntity");

Weapon::Weapon() {
	owner = 0;
}
Weapon::~Weapon() {

}
void Weapon::setKeyValue(const char *key, const char *value) {

	ModelEntity::setKeyValue(key,value);
}
void Weapon::doUse(class Player *activator) {
	if(owner) {
		g_core->RedWarning("Weapon::doUse: weapon is already in use\n");
		return;
	}
	activator->addWeapon(this);
	this->destroyPhysicsObject();
	this->unlink();
}
void Weapon::onFireKeyHeld() {
	
}
void Weapon::onFireKeyDown() {
	G_BulletAttack(this->getOrigin(),this->getForward());
}

