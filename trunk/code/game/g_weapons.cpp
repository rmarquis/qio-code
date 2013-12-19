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
// g_weapons.cpp - helper functions for Weapon class
#include "g_local.h"
#include "explosionInfo.h"
#include "classes/BaseEntity.h"
#include "classes/Projectile.h"
#include <shared/trace.h>
#include <shared/autoCvar.h>
#include <api/rApi.h>
#include <api/serverApi.h>
#include <api/physAPI.h>
#include <api/declManagerAPI.h>
#include <api/coreAPI.h>

static aCvar_c g_showBulletTraces("g_showBulletTraces","0");

void G_BulletAttack(const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip) {
	trace_c tr;
	tr.setupRay(muzzle,muzzle + dir * 10000.f);
	g_physWorld->traceRay(tr);
	G_Printf("G_BulletAttack: hit %f %f %f\n",tr.getHitPos().x,tr.getHitPos().y,tr.getHitPos().z);
	if(rf && g_showBulletTraces.getInt()) {
		rf->addDebugLine(tr.getStartPos(),tr.getHitPos(),vec3_c(1,0,0),5.f);
	}
	g_server->SendServerCommand(-1,va("test_bulletAttack gfx/damage/bullet_mrk %f %f %f %f %f %f %i",muzzle.x,muzzle.y,muzzle.z,
		dir.x,dir.y,dir.z,baseSkip->getEntNum()));
	if(tr.hasHit()) {
		BaseEntity *h = tr.getHitEntity();
		if(h) {
			h->onBulletHit(tr.getHitPos(), dir, 10);		
		}
	}
} 
void G_RailGunAttack(const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip) {
	trace_c tr;
	tr.setupRay(muzzle,muzzle + dir * 10000.f);
	g_physWorld->traceRay(tr);
	G_Printf("G_RailGunAttack: hit %f %f %f\n",tr.getHitPos().x,tr.getHitPos().y,tr.getHitPos().z);

	// TODO: clientside railgun effect?
	g_server->SendServerCommand(-1,va("doRailgunEffect railCore railDisc railExplosion %f %f %f %f %f %f %i",muzzle.x,muzzle.y,muzzle.z,
		dir.x,dir.y,dir.z,baseSkip->getEntNum()));
	if(tr.hasHit()) {
		BaseEntity *h = tr.getHitEntity();
		if(h) {
			h->onBulletHit(tr.getHitPos(), dir, 100);
			// apply extra impulse
			h->applyPointImpulse(dir*500.f,tr.getHitPos());
		}
	}
}
void G_FireProjectile(const char *projectileDefName, const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip) {
	BaseEntity *e = G_SpawnClass(projectileDefName);
	Projectile *p = dynamic_cast<Projectile*>(e);
	if(p == 0) {
		g_core->RedWarning("G_FireProjectile: spawned entity was not a subclass of projectile\n");
		delete e;
		return;
	}
	p->setOrigin(muzzle+dir*32);
	p->setAngles(dir.toAngles());
	if(p->hasStartVelocitySet()) {
		vec3_c left, down;
		dir.makeNormalVectors(left,down);
		vec3_c startVel = p->getStartVelocity();
		p->setLinearVelocity(dir * startVel.x - left * startVel.y - down * startVel.z);
	} else {
		p->setLinearVelocity(dir*1000.f);
	}
}
float G_randomFloat(float min, float max) {
    // this  function assumes max > min, you may want 
    // more robust error checking for a non-debug build
    float random = ((float) rand()) / (float) RAND_MAX;

    // generate (in your case) a float between 0 and (4.5-.78)
    // then add .78, giving you a float between .78 and 4.5
    float range = max - min;  
    return (random*range) + min;
}
void G_MultiBulletAttack(const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip, u32 numBullets, float maxSpread, float spreadDist) {
	vec3_c u, r;
	u = dir.getPerpendicular();
	r.crossProduct(u,dir);
	u.normalize();
	r.normalize();
	for(u32 i = 0; i < numBullets; i++) {
		vec3_c end = muzzle + dir * spreadDist + u * G_randomFloat(-maxSpread,maxSpread) + r * G_randomFloat(-maxSpread,maxSpread);
		vec3_c newDir = (end - muzzle);
		newDir.normalize();
		G_BulletAttack(muzzle,newDir,baseSkip);
	}
}

void G_Explosion(const vec3_c &pos, const struct explosionInfo_s &explosionInfo, const char *extraDamageDefName) {
	if(explosionInfo.radius <= 0.f) {
		g_core->RedWarning("G_Explosion: radius is %f\n",explosionInfo.radius);
		return;
	}
	aabb bb;
	bb.fromPointAndRadius(pos,explosionInfo.radius);
	arraySTD_c<BaseEntity*> ents;
	G_BoxEntities(bb,ents);
	int damage = 200;
	for(u32 i = 0; i < ents.size(); i++) {
		BaseEntity *be = ents[i];
		vec3_c dir = be->getOrigin() - pos;
		float dist = dir.len();
		if(dist > explosionInfo.radius)
			continue;
		float frac = 1.f - dist / explosionInfo.radius;
		int damageScaled = int(float(damage)*frac);
		be->damage(damageScaled);
		if(be->hasPhysicsObject() == false)
			continue;
		dir.normalize();
		dir *= (explosionInfo.force * frac);
		be->applyCentralForce(dir);
	}
	// add clientside effect
	if(explosionInfo.materialName.length()) {
		g_server->SendServerCommand(-1,va("doExplosionEffect %f %f %f %f %s",pos.x,pos.y,pos.z,
			explosionInfo.spriteRadius,explosionInfo.materialName.c_str()));
	}
}


