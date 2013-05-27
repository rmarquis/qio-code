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
// Projectile.h
#ifndef __PROJECTILE_H__
#define __PROJECTILE_H__

#include "ModelEntity.h"
#include "../explosionInfo.h"

class Projectile : public ModelEntity {
	// time to wait between collision and explosion
	u32 explosionDelay;
	// time of last collision
	u32 collisionTime;
	// true if object angles should be synced with projectile direction
	// (used for projectiles with rocket models)
	bool bSyncModelAngles;
	// explosion parameters
	explosionInfo_s explosionInfo;
	// time when projectile was spawned
	int projLaunchTime;
	// time until automatic projectile explesion
	int lifeTime; // in msec
public:
	Projectile();

	DECLARE_CLASS( Projectile );

	void setProjectileSyncAngles(bool newBSyncModelAngles) {
		this->bSyncModelAngles = newBSyncModelAngles;
	}
	void setExplosionDelay(u32 newExplosionDelay) {
		this->explosionDelay= newExplosionDelay;
	}
	void setExplosionRadius(float newExplosionRadius) {
		this->explosionInfo.radius = newExplosionRadius;
	}
	void setExplosionSpriteRadius(float newExplosionSpriteRadius) {
		this->explosionInfo.spriteRadius = newExplosionSpriteRadius;
	}
	void setExplosionSpriteMaterial(const char *matName) {
		this->explosionInfo.materialName = matName;
	}
	void setExplosionForce(float newExplosionForce) {
		this->explosionInfo.force = newExplosionForce;
	}
	void setExplosionMarkMaterial(const char *matName) {
		this->explosionInfo.explosionMark = matName;
	}
	void setExplosionMarkRadius(float newExplosionMarkRadius) {
		this->explosionInfo.markRadius = newExplosionMarkRadius;
	}
	// projectile with automatically explode when a lifetime expire
	void setLifeTime(int newLifeTimeMsec) {
		this->lifeTime = newLifeTimeMsec;
	}

	void explodeProjectile();
	virtual void runFrame();
};

#endif // __PROJECTILE_H__


