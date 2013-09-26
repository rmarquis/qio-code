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
#include "classes/BaseEntity.h"
#include <api/rAPI.h>
#include <api/physAPI.h>
#include <shared/autoCvar.h>

static aCvar_c g_enableDebugDrawing("g_enableDebugDrawing","1");
static aCvar_c gdd_drawEntityAbsBounds("gdd_drawEntityAbsBounds","0");
static aCvar_c gdd_drawEntityCollisionModels("gdd_drawEntityCollisionModels","0");

// classes/InfoPathNode.cpp
void G_DebugDrawPathNodes(class rDebugDrawer_i *dd);

void G_DebugDrawFrame(class rAPI_i *pRFAPI) {
	if(g_enableDebugDrawing.getInt() == 0)
		return;
	class rDebugDrawer_i *dd = pRFAPI->getDebugDrawer();

	if(gdd_drawEntityAbsBounds.getInt()) {
		for(u32 i = 0; i < MAX_GENTITIES; i++) {
			edict_s *ent = &g_entities[i];
			if(ent->s == 0)
				continue;
			ent->ent->debugDrawAbsBounds(dd);
		}
	}
	if(gdd_drawEntityCollisionModels.getInt()) {
		for(u32 i = 0; i < MAX_GENTITIES; i++) {
			edict_s *ent = &g_entities[i];
			if(ent->s == 0)
				continue;
			ent->ent->debugDrawCollisionModel(dd);
		}
	}
	if(g_physAPI) {
		g_physAPI->doDebugDrawing(dd);
	}
	G_DebugDrawPathNodes(dd);
}