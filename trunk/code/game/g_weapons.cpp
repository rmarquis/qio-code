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
#include "classes/BaseEntity.h"
#include <shared/trace.h>
#include <shared/autoCvar.h>
#include <api/rApi.h>
#include <api/serverApi.h>

static aCvar_c g_showBulletTraces("g_showBulletTraces","0");

void G_BulletAttack(const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip) {
	trace_c tr;
	tr.setupRay(muzzle,muzzle + dir * 10000.f);
	BT_TraceRay(tr);
	G_Printf("G_BulletAttack: hit %f %f %f\n",tr.getHitPos().x,tr.getHitPos().y,tr.getHitPos().z);
	if(rf && g_showBulletTraces.getInt()) {
		rf->addDebugLine(tr.getStartPos(),tr.getHitPos(),vec3_c(1,0,0),5.f);
	}
	g_server->SendServerCommand(-1,va("test_bulletAttack %f %f %f %f %f %f %i",muzzle.x,muzzle.y,muzzle.z,
		dir.x,dir.y,dir.z,baseSkip->getEntNum()));
	if(tr.hasHit()) {
		BaseEntity *h = tr.getHitEntity();
		if(h) {
			h->onBulletHit(dir, 100);
		}
	}
}




