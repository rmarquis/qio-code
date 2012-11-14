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
// cg_collision.cpp - clientside-only collision detection system.
// Usefull for 3rd person camera clipping, local entities and particles.
#include "cg_local.h"
#include <api/rAPI.h>
#include <api/rEntityAPI.h>
#include <shared/trace.h>

bool CG_RayTrace(class trace_c &tr) {
	if(rf->rayTraceWorldMap(tr)) {
		tr.setHitCGEntity(&cg_entities[ENTITYNUM_WORLD]);	
	}
	for(u32 i = 0; i < MAX_GENTITIES; i++) {
		centity_s *cent = &cg_entities[i];
		if(cent->currentValid == false) {
			continue;
		}
		if(cent->rEnt == 0) {
			continue;
		}
		const aabb &bb = cent->rEnt->getBoundsABS();
		if(tr.getTraceBounds().intersect(bb) == false) {
			continue;
		}
		trace_c transformedTrace;
		tr.getTransformed(transformedTrace,cent->rEnt->getMatrix());
		if(cent->rEnt->rayTrace(transformedTrace)) {
			tr.updateResultsFromTransformedTrace(transformedTrace);
			tr.setHitCGEntity(cent);
		}
	}
	return tr.hasHit();
}





