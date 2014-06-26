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
#include "Projectile.h"
#include "../g_local.h"
#include <api/coreAPI.h>

DEFINE_CLASS(Q3Weapon, "Weapon");

enum quake3WeaponType_e {
	EQ3WPN_BAD,
	EQ3WPN_PLASMAGUN,
	EQ3WPN_ROCKETLAUNCHER,
	EQ3WPN_SHOTGUN,
	EQ3WPN_RAILGUN,
	EQ3WPN_GRENADE_LAUNCHER,

	EQ3WPN_NUM_KNOWN_WEAPONS,
};
Q3Weapon::Q3Weapon() {
	q3WeaponType = EQ3WPN_BAD;
	railMats = 0;
}
Q3Weapon::~Q3Weapon() {
	if(railMats)
		delete railMats;
}
void Q3Weapon::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"giTag")) {
		// "giTag" field of Quake3 gitem_t structure stores weapon type
		g_core->Print("Q3Weapon::setKeyValue: giTag: %s\n",value);
		if(!stricmp(value,"WP_PLASMAGUN")) {
			q3WeaponType = EQ3WPN_PLASMAGUN;
			this->setDelayBetweenShots(100);
		} else if(!stricmp(value,"WP_ROCKET_LAUNCHER")) {
			q3WeaponType = EQ3WPN_ROCKETLAUNCHER;
			this->setDelayBetweenShots(1000);
		} else if(!stricmp(value,"WP_SHOTGUN")) {
			q3WeaponType = EQ3WPN_SHOTGUN;
			this->setDelayBetweenShots(1000);
		} else if(!stricmp(value,"WP_RAILGUN")) {
			q3WeaponType = EQ3WPN_RAILGUN;
			this->setDelayBetweenShots(2000);
			// use Quake3 railgun materials
			//if(railMats == 0) {
			//	railMats = new railgunAttackMaterials_s;
			//}
			//railMats->setupQuake3();
		} else if(!stricmp(value,"WP_GRENADE_LAUNCHER")) {
			q3WeaponType = EQ3WPN_GRENADE_LAUNCHER;
			this->setDelayBetweenShots(1000);
		}	
	} else {
		Weapon::setKeyValue(key,value);
	}
}

void Q3Weapon::doWeaponAttack() {
	if(q3WeaponType == EQ3WPN_PLASMAGUN) {
		Projectile *plasma = new Projectile;
		plasma->setSpriteModel("sprites/plasma1",32.f);
		vec3_c forward = owner->getViewAngles().getForward();
		plasma->setEntityLightRadius(128.f);
		plasma->setEntityLightColor(vec3_c(0.1f,0.1f,1.f));
		plasma->setExplosionDelay(500);
		plasma->setOrigin(owner->getEyePos()+forward*32);
		plasma->setLinearVelocity(forward*500.f);
		plasma->setExplosionMarkMaterial("gfx/damage/plasma_mrk");
		plasma->setExplosionMarkRadius(16.f);
	} else if(q3WeaponType == EQ3WPN_ROCKETLAUNCHER) {
		Projectile *rocket = new Projectile;
		rocket->setRenderModel("models/ammo/rocket/rocket.md3");
		vec3_c forward = owner->getViewAngles().getForward();
		rocket->setEntityLightRadius(128.f);
		rocket->setProjectileSyncAngles(true);
		rocket->setExplosionDelay(0);
		rocket->setExplosionRadius(128);
		rocket->setExplosionSpriteRadius(64);
		rocket->setExplosionSpriteMaterial("rocketExplosion");
		rocket->setExplosionForce(2000);
		rocket->setExplosionMarkMaterial("gfx/damage/burn_med_mrk");
		rocket->setExplosionMarkRadius(32.f);
		rocket->setOrigin(owner->getEyePos()+forward*32);
		rocket->setLinearVelocity(forward*500.f);
	} else if(q3WeaponType == EQ3WPN_SHOTGUN) {
		G_MultiBulletAttack(owner->getEyePos(), owner->getViewAngles().getForward(), owner, 12, 5, 100);
	} else if(q3WeaponType == EQ3WPN_RAILGUN) {
		G_RailGunAttack(owner->getEyePos(), owner->getViewAngles().getForward(), owner, railMats);
	} else if(q3WeaponType == EQ3WPN_GRENADE_LAUNCHER) {
		Projectile *rocket = new Projectile;
		rocket->setRenderModel("models/ammo/grenade1.md3");
		rocket->setColModel("models/ammo/grenade1.md3");
		rocket->setPhysBounciness(0.5f);
		rocket->initRigidBodyPhysics();
		vec3_c forward = owner->getViewAngles().getForward();
		rocket->setEntityLightRadius(128.f);
		rocket->setExplosionDelay(0);
		rocket->setExplosionRadius(128);
		rocket->setExplosionSpriteRadius(64);
		rocket->setExplosionSpriteMaterial("rocketExplosion");
		rocket->setExplosionForce(2000);
		rocket->setExplosionMarkMaterial("gfx/damage/burn_med_mrk");
		rocket->setExplosionMarkRadius(32.f);
		rocket->setLifeTime(3000);
		rocket->setTrailEmitterMaterial("smokePuff");
		rocket->setTrailEmitterSpriteRadius(8.f);
		rocket->setTrailEmitterInterval(50);

		rocket->setOrigin(owner->getEyePos()+forward*32);
		rocket->setLinearVelocity(forward*1000.f);
	}
}