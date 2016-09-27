/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2012-2014 V.

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
//
// cg_snapshot.c -- things that happen on snapshot transition,
// not necessarily every single rendered frame

#include "cg_local.h"
#include "cg_entities.h"
#include "cg_emitter.h"
#include <api/clientAPI.h>
#include <api/rAPI.h>
#include <api/rEntityAPI.h>
#include <api/rLightAPI.h>
#include <api/afDeclAPI.h>
#include <api/coreAPI.h>
#include <shared/autoCvar.h>
#include <shared/boneOrQP.h>
#include <shared/quake3Anims.h>
#include <shared/colorTable.h>
#include <shared/entityType.h>
#include <protocol/snapFlags.h>

static aCvar_c cg_printSnapEntities("cg_printSnapEntities","0");
static aCvar_c cg_printInitialSnapshots("cg_printInitialSnapshots","0");

/*
==================
CG_SetInitialSnapshot

This will only happen on the very first snapshot, or
on tourney restarts.  All other times will use 
CG_TransitionSnapshot instead.

FIXME: Also called by map_restart?
==================
*/
void CG_SetInitialSnapshot( snapshot_t *snap ) {
	int				i;
	cgEntity_c		*cent;
	entityState_s	*state;

	if(cg_printInitialSnapshots.getInt()) {
		g_core->Print("CG_SetInitialSnapshot: snap sequence %i, serverTime %i.\n",snap->serverCommandSequence,snap->serverTime);
	}

	cg.snap = snap;

	entityState_s tmp;
	snap->ps.toEntityState(&tmp, false);
	cg_entities[snap->ps.clientNum].setCurState(&tmp);
	cg_entities[snap->ps.clientNum].setupNewEntity(snap->ps.clientNum);

	CG_ExecuteNewServerCommands( snap->serverCommandSequence );

	// set our local weapon selection pointer to
	// what the server has indicated the current weapon is
	CG_Respawn();

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		state = &cg.snap->entities[ i ];
		cent = &cg_entities[ state->number ];
	
		cent->setCurState(state);
		cent->resetEntity();
		cent->setupNewEntity(state->number);
	}
}


/*
===================
CG_TransitionSnapshot

The transition point from snap to nextSnap has passed
===================
*/
static void CG_TransitionSnapshot() {
	cgEntity_c			*cent;
	snapshot_t			*oldFrame;

	if ( !cg.snap ) {
		CG_Error( "CG_TransitionSnapshot: NULL cg.snap" );
	}
	if ( !cg.nextSnap ) {
		CG_Error( "CG_TransitionSnapshot: NULL cg.nextSnap" );
	}

	// execute any server string commands before transitioning entities
	CG_ExecuteNewServerCommands( cg.nextSnap->serverCommandSequence );

	// if we had a map_restart, set everthing with initial
	if ( !cg.snap ) {
	}
#if 0
	// clear the currentValid flag for all entities in the existing snapshot
	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cent = &cg_entities[ cg.snap->entities[ i ].number ];
		cent->currentValid = false;
	}

	// move nextSnap to snap and do the transitions
	oldFrame = cg.snap;
	cg.snap = cg.nextSnap;

	BG_PlayerStateToEntityState( &cg.snap->ps, &cg_entities[ cg.snap->ps.clientNum ].currentState, false );
	cg_entities[ cg.snap->ps.clientNum ].interpolate = false;

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cent = &cg_entities[ cg.snap->entities[ i ].number ];
		CG_TransitionEntity( cent );

		// remember time of snapshot this entity was last updated in
		cent->snapShotTime = cg.snap->serverTime;
	}
#else
	u32 snapEnt = 0;
	for(u32 i = 0; i < MAX_GENTITIES; i++) {
		cent = &cg_entities[i];	
		u32 snapEntNum;
		if(snapEnt == cg.snap->numEntities) {
			snapEntNum = MAX_GENTITIES;
		} else {
			snapEntNum = cg.snap->entities[snapEnt].number;
		}
		if(i == cg.clientNum) {
			if(i == snapEntNum) {
				CG_Error("CG_TransitionSnapshot: found local player entityState in snapshot entities\n");
			}
			continue;
		}
		if(snapEntNum == i) {
			if(cent->isCurrentValid() == false) {
				if(cg_printSnapEntities.getInt()) {
					CG_Printf("CG_TransitionSnapshot: %i: new entity %i\n",cg.snap->serverTime,i);
				}
				cent->setupNewEntity(i);
			}
			if(cg_printSnapEntities.getInt() > 1) {
				CG_Printf("CG_TransitionSnapshot: %i: transition entity %i\n",cg.snap->serverTime,i);
			}
			cent->transitionEntity();

			snapEnt++;
		} else if(cent->isCurrentValid()) {
			if(cg_printSnapEntities.getInt()) {
				CG_Printf("CG_TransitionSnapshot: %i: removed entity %i\n",cg.snap->serverTime,i);
			}
			cent->clearEntity();
		} else if(cent->getRenderEntity()) {
			CG_Error("CG_TransitionSnapshot: found entity with currentValid == false and renderEntity present\n");
		}
	}
	if(snapEnt != cg.snap->numEntities) {
		CG_Error("CG_TransitionSnapshot: snapEnt != cg.snap->numEntities\n");
	}
	// move nextSnap to snap and do the transitions
	oldFrame = cg.snap;
	cg.snap = cg.nextSnap;

	cg_entities[ cg.snap->ps.clientNum ].transitionLocalPlayer(cg.snap->ps,oldFrame == 0);

#endif

	cg.nextSnap = NULL;

	// check for playerstate transition events
	if ( oldFrame ) {
		playerState_s	*ops, *ps;

		ops = &oldFrame->ps;
		ps = &cg.snap->ps;

		// if we are not doing client side movement prediction for any
		// reason, then the client events and view changes will be issued now
		//if ( cg_nopredict.getInt() || cg_synchronousClients.getInt() ) {
			CG_TransitionPlayerState( ps, ops );
		//}
		cg_entities[ps->clientNum].transitionEntity();
	}

}


/*
===================
CG_SetNextSnap

A new snapshot has just been read in from the client system.
===================
*/
static void CG_SetNextSnap( snapshot_t *snap ) {
	int					num;
	entityState_s		*es;
	cgEntity_c			*cent;

	cg.nextSnap = snap;

	entityState_s tmp;
	snap->ps.toEntityState(&tmp, false );
	cg_entities[ cg.snap->ps.clientNum ].setNextState(&tmp);

	// check for extrapolation errors
	for ( num = 0 ; num < snap->numEntities ; num++ ) {
		es = &snap->entities[num];
		cent = &cg_entities[ es->number ];

		cent->setNextState(es);
	}

	// sort out solid entities
//	CG_BuildSolidList();
}


/*
========================
CG_ReadNextSnapshot

This is the only place new snapshots are requested
This may increment cgs.processedSnapshotNum multiple
times if the client system fails to return a
valid snapshot.
========================
*/
static snapshot_t *CG_ReadNextSnapshot() {
	bool	r;
	snapshot_t	*dest;

	if ( cg.latestSnapshotNum > cgs.processedSnapshotNum + 1000 ) {
		CG_Printf( "WARNING: CG_ReadNextSnapshot: way out of range, %i > %i\n", 
			cg.latestSnapshotNum, cgs.processedSnapshotNum );
	}

	while ( cgs.processedSnapshotNum < cg.latestSnapshotNum ) {
		// decide which of the two slots to load it into
		if ( cg.snap == &cg.activeSnapshots[0] ) {
			dest = &cg.activeSnapshots[1];
		} else {
			dest = &cg.activeSnapshots[0];
		}

		// try to read the snapshot from the client system
		cgs.processedSnapshotNum++;
		r = g_client->GetSnapshot( cgs.processedSnapshotNum, dest );

		// FIXME: why would trap_GetSnapshot return a snapshot with the same server time
		if ( cg.snap && r && dest->serverTime == cg.snap->serverTime ) {
			//continue;
		}

		// if it succeeded, return
		if ( r ) {
			CG_AddLagometerSnapshotInfo( dest );
			return dest;
		}

		// a GetSnapshot will return failure if the snapshot
		// never arrived, or  is so old that its entities
		// have been shoved off the end of the circular
		// buffer in the client system.

		// record as a dropped packet
		CG_AddLagometerSnapshotInfo( NULL );

		// If there are additional snapshots, continue trying to
		// read them.
	}

	// nothing left to read
	return NULL;
}


/*
============
CG_ProcessSnapshots

We are trying to set up a renderable view, so determine
what the simulated time is, and try to get snapshots
both before and after that time if available.

If we don't have a valid cg.snap after exiting this function,
then a 3D game view cannot be rendered.  This should only happen
right after the initial connection.  After cg.snap has been valid
once, it will never turn invalid.

Even if cg.snap is valid, cg.nextSnap may not be, if the snapshot
hasn't arrived yet (it becomes an extrapolating situation instead
of an interpolating one)

============
*/
void CG_ProcessSnapshots() {
	snapshot_t		*snap;
	int				n;

	// see what the latest snapshot the client system has is
	g_client->GetCurrentSnapshotNumber( &n, &cg.latestSnapshotTime );
	if ( n != cg.latestSnapshotNum ) {
		if ( n < cg.latestSnapshotNum ) {
			// this should never happen
			CG_Error( "CG_ProcessSnapshots: n < cg.latestSnapshotNum" );
		}
		cg.latestSnapshotNum = n;
	}

	// If we have yet to receive a snapshot, check for it.
	// Once we have gotten the first snapshot, cg.snap will
	// always have valid data for the rest of the game
	while ( !cg.snap ) {
		snap = CG_ReadNextSnapshot();
		if ( !snap ) {
			// we can't continue until we get a snapshot
			return;
		}

		// set our weapon selection to what
		// the playerstate is currently using
		if ( !( snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
			CG_SetInitialSnapshot( snap );
		}
	}

	// loop until we either have a valid nextSnap with a serverTime
	// greater than cg.time to interpolate towards, or we run
	// out of available snapshots
	do {
		// if we don't have a nextframe, try and read a new one in
		if ( !cg.nextSnap ) {
			snap = CG_ReadNextSnapshot();

			// if we still don't have a nextframe, we will just have to
			// extrapolate
			if ( !snap ) {
				break;
			}

			CG_SetNextSnap( snap );


			// if time went backwards, we have a level restart
			if ( cg.nextSnap->serverTime < cg.snap->serverTime ) {
				CG_Error( "CG_ProcessSnapshots: Server time went backwards" );
			}
		}

		// if our time is < nextFrame's, we have a nice interpolating state
		if ( cg.time >= cg.snap->serverTime && cg.time < cg.nextSnap->serverTime ) {
			break;
		}

		// we have passed the transition from nextFrame to frame
		CG_TransitionSnapshot();
	} while ( 1 );

	// assert our valid conditions upon exiting
	if ( cg.snap == NULL ) {
		CG_Error( "CG_ProcessSnapshots: cg.snap == NULL" );
	}
	if ( cg.time < cg.snap->serverTime ) {
		// this can happen right after a vid_restart
		cg.time = cg.snap->serverTime;
	}
	if ( cg.nextSnap != NULL && cg.nextSnap->serverTime <= cg.time ) {
		CG_Error( "CG_ProcessSnapshots: cg.nextSnap->serverTime <= cg.time" );
	}

}

