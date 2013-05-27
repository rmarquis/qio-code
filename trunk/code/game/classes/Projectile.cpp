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
// Projectile.cpp
#include "Projectile.h"
#include "../g_local.h"
#include <api/serverAPI.h>
#include <api/physAPI.h>
#include <shared/trace.h>

DEFINE_CLASS(Projectile, "ModelEntity");

Projectile::Projectile() {
	explosionDelay = 500;
	collisionTime = 0;
	bSyncModelAngles = false;
	lifeTime = -1;
	projLaunchTime = level.time;
}

void Projectile::explodeProjectile() {
	G_Explosion(this->getOrigin(), this->explosionInfo);
	destroyPhysicsObject();
	delete this;
}
void Projectile::runFrame() {
	if(this->lifeTime > 0) {
		int passedTime = level.time - projLaunchTime;
		if(passedTime > this->lifeTime) {
			explodeProjectile();
			return;
		}
	}
	// if projectile has physics body
	if(this->body) {
		// update rigid body physics and we're done
		ModelEntity::runPhysicsObject();
		return;
	}
	if(collisionTime) {
		if(collisionTime + explosionDelay < level.time) {
			delete this;
		}
		return;
	}
	// run the physics manually
	vec3_c newPos = this->getOrigin() + linearVelocity * level.frameTime;
	trace_c tr;
	tr.setupRay(this->getOrigin(),newPos);
	if(g_physWorld->traceRay(tr)) {
		if(tr.getHitEntity()) {
			tr.getHitEntity()->applyPointImpulse(linearVelocity,tr.getHitPos());
		}
		G_Explosion(this->getOrigin(), this->explosionInfo);
		// add clientside mark (decal)
		if(explosionInfo.explosionMark.length()) {
			vec3_c pos = tr.getHitPos();
			vec3_c dir = linearVelocity.getNormalized();
			g_server->SendServerCommand(-1,va("createDecal %f %f %f %f %f %f %f %s",pos.x,pos.y,pos.z,dir.x,dir.y,dir.z,
				explosionInfo.markRadius,explosionInfo.explosionMark.c_str()));
		}
		collisionTime = level.time;
		this->linearVelocity.clear();
		return;
	}
	this->setOrigin(newPos);
	if(bSyncModelAngles) {
		vec3_c dir = this->linearVelocity.getNormalized();
		vec3_c na(dir.getPitch(),dir.getYaw(),0);
		this->setAngles(na);
	}
}



