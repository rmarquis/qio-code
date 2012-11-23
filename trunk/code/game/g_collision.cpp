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
// g_collision.cpp - simple serverside collision detection system.
// Totaly independent from physics engine.
#include "g_local.h"
#include <shared/trace.h>
#include "classes/BaseEntity.h"
 
// TODO: improve it with octree/kd tree ?
u32 G_BoxEntities(const aabb &bb, arraySTD_c<BaseEntity*> &out) {
	for(u32 i = 0; i < level.num_entities; i++) {
		edict_s *ed = &g_entities[i];
		if(ed->ent == 0)
			continue;
		if(bb.intersect(ed->absBounds)) {
			out.push_back(ed->ent);
		}
	}
	return out.size();
}

bool G_TraceRay(class trace_c &tr, BaseEntity *baseSkip) {
	arraySTD_c<BaseEntity*> ents;
	G_BoxEntities(tr.getTraceBounds(),ents);
	bool hasHit = false;
	for(u32 i = 0; i < ents.size(); i++) {
		BaseEntity *e = ents[i];
		if(baseSkip) {
			if(e == baseSkip || e->getOwner() == baseSkip) {
				continue;
			}
		}
		if(e->traceWorldRay(tr)) {
			hasHit = true;
		}
	}
	return hasHit;
}



