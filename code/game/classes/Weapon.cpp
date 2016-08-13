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
#include "Light.h"
#include <api/coreAPI.h>
#include <api/declManagerAPI.h>
#include <api/entityDeclAPI.h>
#include <api/modelDeclAPI.h>
#include <api/entDefAPI.h>
#include <api/serverAPI.h>

DEFINE_CLASS(Weapon, "ModelEntity");
DEFINE_CLASS_ALIAS(Weapon, idItem);
// for Prey weapons (HumanHead items)
DEFINE_CLASS_ALIAS(Weapon, hhItem);
DEFINE_CLASS_ALIAS(Weapon, hhWeaponRifle);

Weapon::Weapon() {
	owner = 0;
	autoFire = true;
	delayBetweenShots = 250;
	lastShotTime = 0;
	invWeaponDecl = 0;
	clipSize = 1;
	curClipSize = 1;
	raiseTime = 0;
	lowerTime = 0;
	reloadTime = 0;
	flashColor.set(1,1,1);
	flashRadius = 100.f;
	shotBulletCount = 1;
	spreadDist = 1000.f;
	maxSpread = 1.f;
	weaponHand = WH_DUALHAND;
	bLoopFire = false;
	dualAttachToTag = "tag_weapon_right";
}
Weapon::~Weapon() {

}
void Weapon::setViewModel(const char *newViewModelName) {
	model_view = newViewModelName;

	modelDeclAPI_i *decl = g_declMgr->registerModelDecl(model_view);
	if(decl) {
		raiseTime = decl->getAnimationTimeMSec("raise");
		lowerTime = decl->getAnimationTimeMSec("putaway");
		delayBetweenShots = decl->getAnimationTimeMSec("fire");
		if(delayBetweenShots == 0) {
			delayBetweenShots = 10;
		}
		reloadTime = decl->getAnimationTimeMSec("reload");
		if(reloadTime == 0) {
			reloadTime = 10;
		}
	}
}
void Weapon::postSpawn() {
	ModelEntity::postSpawn();
	// make sure that player can pickup weapons
	// They must have a cmod
	if(cmod == 0) {
		setKeyValue("size", "64 64 64");
	}
}
void Weapon::setKeyValue(const char *key, const char *value) {
	if(!_stricmp(key,"model_view")) {
		// use this model for first person view
		this->setViewModel(value);
	} else if(invWeaponDecl && !_stricmp(key,"model") && model_view.length()==0) {
		// "model" keyword inside a "inv_weapon" entdefs sets the weapons viewModel
		this->setViewModel(value);
#if 0
	} else if(!stricmp(key,"model")) {
		this->setRenderModel(value);
		this->setColModel(value);
#endif
	} else if(!_stricmp(key,"inv_weapon")) {	
		if(invWeaponDecl)
			return;
		invWeaponDecl = g_declMgr->registerEntityDecl(value);
		if(invWeaponDecl) {
			applyKeyValues(invWeaponDecl->getEntDefAPI());
			invWeaponDecl = 0;
		}
	} else if(!_stricmp(key,"clipSize")) {
		clipSize = atoi(value);
		curClipSize = clipSize;
	} else if(!_stricmp(key,"ddaName")) {
		ddaName = value;
	} else if(!_stricmp(key,"weaponName")) {
		weaponName = value;
	} else if(!_stricmp(key,"continuousFire")) {
	} else if(!_stricmp(key,"ammoRequired")) {
	} else if(!_stricmp(key,"ammoType")) {

	} else if(!_stricmp(key,"fireRate")) {

	} else if(!_stricmp(key,"def_viewStyle")) {
		// Quake4 (???) viewStyle dict
		entityDeclAPI_i *viewStyleDef = g_declMgr->registerEntityDecl(value);
		if(viewStyleDef) {
			applyKeyValues(viewStyleDef->getEntDefAPI());
		}
	} else if(!_stricmp(key,"viewoffset")) {
		// set from "def_viewStyle" entityDef
		viewOffset.fromString(value);
	} else if(!_stricmp(key,"viewangles")) {
		// set from "def_viewStyle" entityDef
		viewAngles.fromString(value);
	} else if(!_stricmp(key,"def_projectile")) {
		def_projectile = value;
	} else if(!_stricmp(key,"smoke_muzzle")) {
		smoke_muzzle = value;
	} else if(!_stricmp(key,"flashColor")) {
		// example usage: "flashColor"	"1 0.8 0.4"
		flashColor.fromString(value);
	} else if(!_stricmp(key,"flashRadius")) {
		// example usage: "flashRadius"	"120"
		flashRadius = atof(value);
	} else if(!_stricmp(key,"shotBulletCount")) {
		shotBulletCount = atoi(value);
	} else if(!_stricmp(key,"maxSpread")) {
		maxSpread = atof(value);
	} else if(!_stricmp(key,"spreadDist")) {
		spreadDist = atof(value);
	} else if(!_stricmp(key,"name") && tiki) {
		// TIKI - "name" is a weapon name event used in .tik files
		// "name" is also a "targetname" in Doom3
		// Avoid conflicts and use "name" here only if TIKI is set.
		weaponName = value;
		weaponName.removeCharacter('"');
	} else if(!_stricmp(key,"hand")) {
		// TIKI - used in FAKK2
		setWeaponHand(value);
	} else if(!_stricmp(key,"fireType")) {
		// TIKI - FAKK/MoHAA fireType (bullet, projectile)
	} else if(!_stricmp(key,"projectile")) {
		// TIKI - FAKK/MoHAA projectile tiki
		def_projectile = value; // PLACEHOLDER
	} else if(!_stricmp(key,"autoaim")) {
		// TIKI FAKK auto aim
	} else if(!_stricmp(key,"alternate")) {
		// TIKI firemodes
		// for primary fire: "fireType projectile"
		// for alternate fire: "alternate fireType projectile"
		// Just remember that we're setting alternate fire and pass the event again
		str tmp = value;
		str newKey, newValue;
		const char *p = tmp.getToken(newKey);
		newValue = p;
		this->setKeyValue(newKey,newValue);
	} else if(!_stricmp(key,"startammo")) {
		// TIKI
	} else if(!_stricmp(key,"loopFire")) {
		// TIKI
		bLoopFire = true;
	} else if(!_stricmp(key,"coolitem")) {
		// FAKK-only, ignore
	} else if(!_stricmp(key,"dualAttachToTag")) {
		// FAKK
		dualAttachToTag = value;
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
bool Weapon::hasAmmoForFireType(const char *fireType) const {

	return true; // TODO
}
void Weapon::setWeaponHand(const char *handName) {
	if(!stricmp(handName,"dual") || !stricmp(handName,"dualhand")) {
		// NOTE: both "dual" and "dualhand" are used in FAKK
		// "dualhand" is used weapon_axe.tik
		weaponHand = WH_DUALHAND;
	} else if(!stricmp(handName,"any")) {
		// weapon_uzi.tik can be wielded in any hand
		weaponHand = WH_ANY;
	} else if(!stricmp(handName,"righthand")) {
		// weapon_sword.tik can be only wielded in rigth hand
		weaponHand = WH_RIGHT;
	} else if(!stricmp(handName,"left")) {
		// weapon_shield.tik can be only wielded in left hand
		weaponHand = WH_LEFT;
	} else {
		g_core->RedWarning("Weapon::setWeaponHand: unknown weaponhand '%s'\n",handName);
	}
}
bool Weapon::doUse(class Player *activator) {
	if(owner) {
		g_core->RedWarning("Weapon::doUse: weapon is already in use\n");
		return true; // this item cannot be carried
	}
	if(activator->canPickUpWeapon(this)==false)
		return true;
	owner = activator;
	activator->addWeapon(this);
	this->destroyPhysicsObject();
	this->unlink();
	// run callback (so Weapon child classes know they were picked up)
	this->onWeaponPickedUp();
	return true; // this item cannot be carried
}

BaseEntity *Weapon::getOwner() const {
	return owner.getPtr();
}
void Weapon::onFireKeyHeld() {
	if(autoFire) {
		if(canFireAgain()) {
			this->lastShotTime = g_time;
			doWeaponAttack();
		}
	}
}
void Weapon::onFireKeyDown() {
	if(canFireAgain()) {
		this->lastShotTime = g_time;
		doWeaponAttack();
	}
}
void Weapon::onSecondaryFireKeyHeld() {

}
void Weapon::onSecondaryFireKeyDown() {

}
void Weapon::onSecondaryFireKeyUp() {

}

void Weapon::fireEvent() {
	doWeaponAttack();
}
void Weapon::doWeaponAttack() {
	vec3_c muzzlePos, muzzleDir;
	BaseEntity *skip;
	if(owner) {
		skip = owner;
		curClipSize--;
		muzzlePos = owner->getEyePos();
		muzzleDir = owner->getViewAngles().getForward();
	} else {
		skip = this;
		muzzlePos = this->getOrigin();
		muzzleDir = this->getForward();
	}
	if(flashRadius > 0.f) {
#if 0
		Light *l = new Light;
		l->setOrigin(muzzlePos+muzzleDir*20.f);
		l->setRadius(flashRadius);
		l->removeAfterDelay(100);
#else
		g_server->SendServerCommand(-1,va("doLocalMuzzleFlash %f",flashRadius));
#endif
	}
	if(def_projectile.size()) {
		G_FireProjectile(def_projectile.c_str(),muzzlePos,muzzleDir,skip);
	} else {
		//G_BulletAttack(muzzlePos,muzzleDir,skip);
		G_MultiBulletAttack(muzzlePos,muzzleDir,skip,shotBulletCount,maxSpread,spreadDist);
	}
}
void Weapon::doWeaponAttackSecondary() {

}
bool Weapon::canFireAgain() const {
	u32 timeElapsed = g_time - this->lastShotTime;
	if(timeElapsed < this->delayBetweenShots) {
		g_core->Print("Weapon::canFireAgain: cant fire because elapsed time is %i and delay is %i\n",timeElapsed,this->delayBetweenShots);
		return false;
	}
	g_core->Print("Weapon::canFireAgain: CAN FIRE, because elapsed time is %i and delay is %i\n",timeElapsed,this->delayBetweenShots);
	return true;
}