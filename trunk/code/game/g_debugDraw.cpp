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
// g_debugDraw.cpp - debug drawing for game module.
#include "g_local.h"
#include <api/rAPI.h>
#include <api/ddAPI.h>
#include <api/cmAPI.h>
#include <math/vec3.h>

void G_DebugDrawFrame(class rAPI_i *pRFAPI) {
	class rDebugDrawer_i *dd = pRFAPI->getDebugDrawer();
	for(u32 i = 0; i < MAX_GENTITIES; i++) {
		gentity_s *ent = &g_entities[i];
		if(ent->inuse == false)
			continue;
		if(ent->cmod) {
			if(ent->cmod->isCapsule()) {
				cmCapsule_i *c = ent->cmod->getCapsule();
				dd->drawCapsuleZ(ent->client->ps.origin, c->getHeight(), c->getRadius());
			} else if(ent->cmod->isBBExts()) {
				cmBBExts_i *bb = ent->cmod->getBBExts();
				dd->drawBBExts(ent->s.origin,ent->s.angles,bb->getHalfSizes());
			}
		}
	}
}