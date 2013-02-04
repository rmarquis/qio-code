/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// cg_snapshot.c -- things that happen on snapshot transition,
// not necessarily every single rendered frame

#include "cg_local.h"
#include <api/clientAPI.h>
#include <api/rAPI.h>
#include <api/rEntityAPI.h>
#include <api/rLightAPI.h>
#include <api/afDeclAPI.h>
#include <api/coreAPI.h>
#include <shared/autoCvar.h>
#include <shared/boneOrQP.h>
#include <shared/quake3Anims.h>

static aCvar_c cg_printSnapEntities("cg_printSnapEntities","0");
static aCvar_c cg_printNewSnapEntities("cg_printNewSnapEntities","0");

/*
==================
CG_ResetEntity
==================
*/
static void CG_ResetEntity( centity_t *cent ) {
	VectorCopy (cent->currentState.origin, cent->lerpOrigin);
	VectorCopy (cent->currentState.angles, cent->lerpAngles);
	if ( cent->currentState.eType == ET_PLAYER ) {
//		CG_ResetPlayerEntity( cent );
	}
}

static void CG_TransitionLight(centity_t *cent) {
	if(cent->rLight == 0) {
		g_core->RedWarning("CG_TransitionLight: found ET_LIGHT without rLight (entity %i)\n",cent->currentState.number);
		return;
	}
	cent->rLight->setOrigin(cent->currentState.origin);
	cent->rLight->setRadius(cent->currentState.lightRadius);
}
static void CG_TransitionModel(centity_t *cent) {
	if(cent->rEnt == 0) {
		g_core->RedWarning("CG_TransitionModel: found entity without rEntity (entity %i)\n",cent->currentState.number);
		return;
	}
	cent->rEnt->setOrigin(cent->currentState.origin);
	cent->rEnt->setAngles(cent->currentState.angles);
	cent->rEnt->setModel(cgs.gameModels[cent->currentState.rModelIndex]);
	if(cent->rEnt->hasDeclModel()) {
		cent->rEnt->setDeclModelAnimLocalIndex(cent->currentState.animIndex);
	} else if(cent->rEnt->isQ3PlayerModel()) {
		cent->rEnt->setQ3LegsAnimLocalIndex(cent->currentState.animIndex);
		cent->rEnt->setQ3TorsoAnimLocalIndex(TORSO_STAND);
	} else {
		cent->rEnt->setAnim(cgs.gameAnims[cent->currentState.animIndex]);
	}
	if(cent->currentState.rSkinIndex) {
		// if we have a custom skin...
		const char *skinName = CG_ConfigString(CS_SKINS + cent->currentState.rSkinIndex);
		cent->rEnt->setSkin(skinName);
	}
	if(cent->currentState.number == cg.clientNum) {
		cent->rEnt->setThirdPersonOnly(true);
	} else {
		cent->rEnt->setThirdPersonOnly(false);
	}
	if(cent->currentState.activeRagdollDefNameIndex) {
		const afDeclAPI_i *af = cgs.gameAFs[cent->currentState.activeRagdollDefNameIndex];
		cent->rEnt->setRagdoll(af);
		if(af) {
			for(u32 i = 0; i < af->getNumBodies(); i++) {
				const netBoneOr_s &in = cent->currentState.boneOrs[i];
				boneOrQP_c or;
				or.setPos(in.xyz);
				or.setQuatXYZ(in.quatXYZ);
				cent->rEnt->setRagdollBodyOr(i,or);
			}
		}
	} else {
		cent->rEnt->setRagdoll(0);
	}
}
/*
===============
CG_TransitionEntity

cent->nextState is moved to cent->currentState and events are fired
===============
*/
static void CG_TransitionEntity( centity_t *cent ) {
	entityState_s previousState = cent->currentState;
	cent->currentState = cent->nextState;
	cent->currentValid = qtrue;

	// reset if the entity wasn't in the last frame or was teleported
	if ( !cent->interpolate ) {
		CG_ResetEntity( cent );
	}

	// set all the values here in case that entity cant be interpolated between two snapshots
	
	// NOTE: some centities might have both rEnt and rLight present
	if(cent->currentState.eType == ET_LIGHT) {
		CG_TransitionLight(cent);
	} else {
		// handle extra entity-light effect as well
		if(cent->currentState.lightRadius > 0.f) {
			if(cent->rLight == 0) {
				// entity light has been enabled
				cent->rLight = rf->allocLight();
			}
			CG_TransitionLight(cent);
		} else {
			if(cent->rLight) {	
				// entity light has been disabled
				rf->removeLight(cent->rLight);
				cent->rLight = 0;
			}
		}
		CG_TransitionModel(cent);
	}

	// clear the next state.  if will be set by the next CG_SetNextSnap
	cent->interpolate = qfalse;
}

static void CG_RemoveEntity(u32 entNum) {
	centity_t *cent = &cg_entities[entNum];
//	if(cent->currentState.eType == ET_LIGHT) {
		if(cent->rLight) {
			rf->removeLight(cent->rLight);
			cent->rLight = 0;
		}
///	} else {
		if(cent->rEnt) {
			rf->removeEntity(cent->rEnt);
			cent->rEnt = 0;
		}
//	}
}
static void CG_NewEntity(u32 entNum) {
	centity_t *cent = &cg_entities[entNum];
#if 1
	if(cent->currentState.number != entNum) {
		cent->currentState = cent->nextState;
	}
#endif
	if(cg_printNewSnapEntities.getInt()) {
		CG_Printf("CG_NewEntity: entNum %i, eType %i, ptr %i\n",entNum,cent->currentState.eType,cent);
	}
	if(cent->currentState.eType == ET_LIGHT) {
		// new render light
		if(cent->rLight) {
			CG_Printf(S_COLOR_RED"CG_NewEntity: centity_t %i already have rLight assigned\n",entNum);
			rf->removeLight(cent->rLight);
		}
		cent->rLight = rf->allocLight();
	} else {
		// new render entity
		if(cent->rEnt) {
			CG_Printf(S_COLOR_RED"CG_NewEntity: centity_t %i already have rEntity assigned\n",entNum);
			rf->removeEntity(cent->rEnt);
		}
		cent->rEnt = rf->allocEntity();
		if(cent->currentState.lightRadius >= 0) {
			// new render light
			if(cent->rLight) {
				CG_Printf(S_COLOR_RED"CG_NewEntity: centity_t %i already have rLight assigned\n",entNum);
				rf->removeLight(cent->rLight);
			}
			cent->rLight = rf->allocLight();
		}
	}
}

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
	centity_t		*cent;
	entityState_s	*state;

	cg.snap = snap;

	BG_PlayerStateToEntityState( &snap->ps, &cg_entities[ snap->ps.clientNum ].currentState, qfalse );
	CG_NewEntity(snap->ps.clientNum);

	// sort out solid entities
//	CG_BuildSolidList();

	CG_ExecuteNewServerCommands( snap->serverCommandSequence );

	// set our local weapon selection pointer to
	// what the server has indicated the current weapon is
	CG_Respawn();

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		state = &cg.snap->entities[ i ];
		cent = &cg_entities[ state->number ];

		memcpy(&cent->currentState, state, sizeof(entityState_s));
		//cent->currentState = *state;
		cent->interpolate = qfalse;
		cent->currentValid = qtrue;

		CG_ResetEntity( cent );
		CG_NewEntity( state->number );
	}
}


/*
===================
CG_TransitionSnapshot

The transition point from snap to nextSnap has passed
===================
*/
static void CG_TransitionSnapshot( void ) {
	centity_t			*cent;
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
		cent->currentValid = qfalse;
	}

	// move nextSnap to snap and do the transitions
	oldFrame = cg.snap;
	cg.snap = cg.nextSnap;

	BG_PlayerStateToEntityState( &cg.snap->ps, &cg_entities[ cg.snap->ps.clientNum ].currentState, qfalse );
	cg_entities[ cg.snap->ps.clientNum ].interpolate = qfalse;

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
			if(cent->currentValid == false) {
				if(cg_printSnapEntities.getInt()) {
					CG_Printf("CG_TransitionSnapshot: %i: new entity %i\n",cg.snap->serverTime,i);
				}
				CG_NewEntity(i);
			}
			if(cg_printSnapEntities.getInt() > 1) {
				CG_Printf("CG_TransitionSnapshot: %i: transition entity %i\n",cg.snap->serverTime,i);
			}
			CG_TransitionEntity( cent );

			// remember time of snapshot this entity was last updated in
			cent->snapShotTime = cg.snap->serverTime;

			snapEnt++;
		} else if(cent->currentValid) {
			if(cg_printSnapEntities.getInt()) {
				CG_Printf("CG_TransitionSnapshot: %i: removed entity %i\n",cg.snap->serverTime,i);
			}
			CG_RemoveEntity(i);
			cent->currentValid = false;
		} else if(cent->rEnt) {
			CG_Error("CG_TransitionSnapshot: found entity with currentValid == qfalse and renderEntity present\n");
		}
	}
	if(snapEnt != cg.snap->numEntities) {
		CG_Error("CG_TransitionSnapshot: snapEnt != cg.snap->numEntities\n");
	}
	// move nextSnap to snap and do the transitions
	oldFrame = cg.snap;
	cg.snap = cg.nextSnap;

	if(oldFrame == 0) {
		BG_PlayerStateToEntityState( &cg.snap->ps, &cg_entities[ cg.snap->ps.clientNum ].currentState, qfalse );
	} else {
		BG_PlayerStateToEntityState( &cg.snap->ps, &cg_entities[ cg.snap->ps.clientNum ].nextState, qfalse );
	}
	cg_entities[ cg.snap->ps.clientNum ].interpolate = qfalse;
	if(cg_entities[cg.snap->ps.clientNum].rEnt) {
		// set the playerModel flag as soon as its possible
		cg_entities[cg.snap->ps.clientNum].rEnt->setIsPlayerModel(true);
	}
#endif

	cg.nextSnap = NULL;

	// check for playerstate transition events
	if ( oldFrame ) {
		playerState_s	*ops, *ps;

		ops = &oldFrame->ps;
		ps = &cg.snap->ps;

		// if we are not doing client side movement prediction for any
		// reason, then the client events and view changes will be issued now
		//if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
			CG_TransitionPlayerState( ps, ops );
		//}
		CG_TransitionEntity(&cg_entities[ps->clientNum]);
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
	centity_t			*cent;

	cg.nextSnap = snap;

	BG_PlayerStateToEntityState( &snap->ps, &cg_entities[ snap->ps.clientNum ].nextState, qfalse );
	cg_entities[ cg.snap->ps.clientNum ].interpolate = qtrue;

	// check for extrapolation errors
	for ( num = 0 ; num < snap->numEntities ; num++ ) {
		es = &snap->entities[num];
		cent = &cg_entities[ es->number ];

		memcpy(&cent->nextState, es, sizeof(entityState_s));
		//cent->nextState = *es;

		// if this frame is a teleport, or the entity wasn't in the
		// previous frame, don't interpolate
		if ( !cent->currentValid ) {//|| ( ( cent->currentState.eFlags ^ es->eFlags ) & EF_TELEPORT_BIT )  ) {
			cent->interpolate = qfalse;
		} else {
			cent->interpolate = qtrue;
		}
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
static snapshot_t *CG_ReadNextSnapshot( void ) {
	qboolean	r;
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
void CG_ProcessSnapshots( void ) {
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

