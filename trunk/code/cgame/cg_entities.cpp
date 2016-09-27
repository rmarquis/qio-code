/*
===========================================================================
Copyright (C) 2012-2016 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Qio source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Qio source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// cg_entities.cpp - cgame entities managment
#include "cg_local.h"
#include "cg_entity.h"
#include <api/rAPI.h>

cgEntity_c cg_entities[MAX_GENTITIES];

void CG_ShutdownEntities() {
	for(u32 i = 0; i < MAX_GENTITIES; i++) {
		cgEntity_c *cent = &cg_entities[i];
		cent->clearEntity();
	}
}

void CG_RunEntities() {
	int					num;

	// set cg.frameInterpolation
	if ( cg.nextSnap ) {
		int		delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if ( delta == 0 ) {
			cg.frameInterpolation = 0;
		} else {
			cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
		}
	} else {
		cg.frameInterpolation = 0;	// actually, it should never be used, because 
									// no entities should be marked as interpolating
	}

	//CG_Printf("frameInterpolation: %f\n",cg.frameInterpolation);

	cg_entities[ cg.snap->ps.clientNum ].updateCGEntity();

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cgEntity_c *cent = &cg_entities[ cg.snap->entities[ num ].number ];
		cent->updateCGEntity();
	}
}

