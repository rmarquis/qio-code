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
DEFINE_CLASS_ALIAS(Weapon, idItem);

Weapon::Weapon() {
	owner = 0;
	autoFire = true;
	delayBetweenShots = 250;
	lastShotTime = 0;
}
Weapon::~Weapon() {

}
void Weapon::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"model_view")) {
		model_view = value;
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
bool Weapon::doUse(class Player *activator) {
	if(owner) {
		g_core->RedWarning("Weapon::doUse: weapon is already in use\n");
		return true; // this item cannot be carried
	}
	owner = activator;
	activator->addWeapon(this);
	this->destroyPhysicsObject();
	this->unlink();
	return true; // this item cannot be carried
}

BaseEntity *Weapon::getOwner() const {
	return owner.getPtr();
}
void Weapon::onFireKeyHeld() {
	if(autoFire) {
		if(canFireAgain()) {
			doWeaponAttack();
		}
	}
}
void Weapon::onFireKeyDown() {
	if(canFireAgain()) {
		doWeaponAttack();
	}
}

void Weapon::doWeaponAttack() {
	this->lastShotTime = level.time;
	if(owner) {
		G_BulletAttack(owner->getEyePos(),owner->getViewAngles().getForward(),owner);
	} else {
		G_BulletAttack(this->getOrigin(),this->getForward(),this);
	}
}

bool Weapon::canFireAgain() const {
	u32 timeElapsed = level.time - this->lastShotTime;
	if(timeElapsed < this->delayBetweenShots) {
		g_core->Print("Weapon::canFireAgain: cant fire because elapsed time is %i and delay is %i\n",timeElapsed,this->delayBetweenShots);
		return false;
	}
	g_core->Print("Weapon::canFireAgain: CAN FIRE, because elapsed time is %i and delay is %i\n",timeElapsed,this->delayBetweenShots);
	return true;
}