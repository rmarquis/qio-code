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
// Weapon_PhysGun.h - HalfLife2 style physgun
#include "Weapon_PhysGun.h"
#include "Player.h"
#include "../g_local.h"
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include <shared/trace.h>
#include <shared/autoCvar.h>

DEFINE_CLASS(Weapon_PhysGun, "Weapon");

aCvar_c physgun_impulseForce_mult("physgun_impulseForce_mult","1.0");

Weapon_PhysGun::Weapon_PhysGun() {
	phMaxDist = 300.f;
	phPickupDist = 96.f;
	phDragForce = 250.f;
	phImpulseForce = 15000.f;
	phHoldDist = 60.f;
}
void Weapon_PhysGun::postSpawn() {
	// old hardcoded fix
	if(this->getRenderModelName()[0] == 0) {
		// see if we can use Half Life 2 physgun model
		if(g_vfs->FS_FileExists("models/weapons/w_physics.mdl")) {
			this->setRenderModel("models/weapons/w_physics.mdl");
			bUseDynamicConvexForTrimeshCMod = true;
			this->setColModel("models/weapons/w_physics.mdl");
			// "v_*" is a viewmodel, we should use it instead of worldmodel,
			// but it has a lot of dangling edges and breaks stencil shadows
			//this->setViewModel("models/weapons/v_physcannon.mdl");
		} else {
			this->setRenderModel("models/testweapons/smallGun.obj");
			bUseDynamicConvexForTrimeshCMod = true;
			this->setColModel("models/testweapons/smallGun.map");
		}
	}
	ModelEntity::postSpawn();
}
void Weapon_PhysGun::onWeaponPickedUp() {
	// we need to rotate w_physics.mdl 90 degrees around Z axis
	if(this->hasRenderModel("models/weapons/w_physics.mdl")) {
		this->setLocalAttachmentAngles(vec3_c(0,90,0));
	}
}
void Weapon_PhysGun::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"physGunForce")) {

	} else {
		Weapon::setKeyValue(key,value);
	}
}
// update picked up entity orientation
void Weapon_PhysGun::runFrame() {
	if(holdingEntity) {
		vec3_c pos = holdingEntity->getPhysicsOrigin();
		vec3_c neededPos = owner->getEyePos() + owner->getViewAngles().getForward() * phHoldDist;
		vec3_c delta = neededPos - pos;

		holdingEntity->setLinearVelocity(holdingEntity->getLinearVelocity()*0.5f);
		holdingEntity->setAngularVelocity(holdingEntity->getAngularVelocity()*0.5f);
		holdingEntity->applyCentralImpulse(delta*50.f);
		//carryingEntity->setOrigin(neededPos);
	}
	ModelEntity::runFrame();
}
void Weapon_PhysGun::tryToPickUpEntity() {
	if(this->holdingEntity) {
		g_core->RedWarning("Weapon_PhysGun::tryToPickUpEntity(): already holding an entity\n");
		return;
	}

	vec3_c muzzle = owner->getEyePos();
	trace_c tr;
	tr.setupRay(muzzle,muzzle + owner->getViewAngles().getForward() * phPickupDist);
	if(G_TraceRay(tr,owner)) {
		BaseEntity *hit = tr.getHitEntity();
		if(hit == 0) {
			G_Printf("Weapon_PhysGun::tryToPickUpEntity: WARNING: null hit entity\n");
			return;
		}
		G_Printf("Weapon_PhysGun::tryToPickUpEntity: Use trace hit\n");
		if(hit->isDynamic()) {
			ModelEntity *me = dynamic_cast<ModelEntity*>(hit);
			this->holdingEntity = me;
		}
	} else {
		tr.setupRay(muzzle,muzzle + owner->getViewAngles().getForward() * phMaxDist);
		if(G_TraceRay(tr,owner)) {
			BaseEntity *hit = tr.getHitEntity();
			if(hit == 0) {
				G_Printf("Weapon_PhysGun::tryToPickUpEntity: WARNING: null hit entity\n");
				return;
			}
			float dist = tr.getHitPos().dist(muzzle);
			G_Printf("Weapon_PhysGun::tryToPickUpEntity: Use trace hit\n");
			if(hit->isDynamic()) {
				ModelEntity *me = dynamic_cast<ModelEntity*>(hit);
				float frac;
				if(dist > phMaxDist) {
					frac = 0.f;
				} else if(dist < phPickupDist) {
					frac = 1.f;
				} else {
					frac = 1.f-((phMaxDist-dist)/(phMaxDist-phPickupDist));
				}
				vec3_c dir = (muzzle-tr.getHitPos());
				dir.normalize();
				vec3_c force = dir * frac * phDragForce;
				me->applyPointImpulse(force,tr.getHitPos());
			}
		}
	}
}
void Weapon_PhysGun::addGravityImpulse() {
	vec3_c muzzle = owner->getEyePos();
	trace_c tr;
	tr.setupRay(muzzle,muzzle + owner->getViewAngles().getForward() * phPickupDist);
	if(G_TraceRay(tr,owner)) {
		BaseEntity *hit = tr.getHitEntity();
		if(hit == 0) {
			G_Printf("Weapon_PhysGun::addGravityImpulse: WARNING: null hit entity\n");
			return;
		}
		G_Printf("Weapon_PhysGun::addGravityImpulse: Use trace hit\n");
		vec3_c dir = (tr.getHitPos()-muzzle);
		dir.normalize();
		ModelEntity *me = dynamic_cast<ModelEntity*>(hit);
		vec3_c force = dir * phImpulseForce;
		me->applyPointImpulse(force,tr.getHitPos());
	}
}
// shooting entity - left mouse button
void Weapon_PhysGun::shootPickedUpEntity() {
	if(this->holdingEntity == 0) {
		g_core->RedWarning("Weapon_PhysGun::shootPickedUpEntity(): no entity picked up\n");
		return;
	}
	vec3_c dir = owner->getViewAngles().getForward();
	this->holdingEntity->applyCentralImpulse(dir*phImpulseForce*physgun_impulseForce_mult.getFloat());
	this->holdingEntity = 0;
}
// dropping picked up entity - right mouse button
void Weapon_PhysGun::dropPickedUpEntity() {
	this->holdingEntity = 0;
}

void Weapon_PhysGun::doWeaponAttack() {
	if(holdingEntity) {
		shootPickedUpEntity();
	} else {
		addGravityImpulse();
	}
}
void Weapon_PhysGun::onSecondaryFireKeyHeld() {
	if(bDropped)
		return;
	if(holdingEntity) {
		return;
	} else {
		tryToPickUpEntity();
	}
}
void Weapon_PhysGun::onSecondaryFireKeyDown() {
	if(holdingEntity) {
		dropPickedUpEntity();
		bDropped = true;
	} else {
		tryToPickUpEntity();
	}
}
void Weapon_PhysGun::onSecondaryFireKeyUp() {
	bDropped = false;
}
