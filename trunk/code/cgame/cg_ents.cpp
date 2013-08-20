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
// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"
#include "cg_emitter.h"
#include <api/rEntityAPI.h>
#include <api/rLightAPI.h>
#include <api/coreAPI.h>
#include <math/matrix.h>

/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/
/*
===============
CG_Player
===============
*/
//void CG_Player( centity_t *cent ) {
//	
//
//}


//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
	cent->extrapolated = qfalse;	

//	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
//	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );


}

/*
==================
CG_General
==================
*/
//static void CG_General( centity_t *cent ) {
//	
//}

/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_t *cent ) {
	float		f;

	// it would be an internal error to find an entity that interpolates without
	// a snapshot ahead of the current one
	if ( cg.nextSnap == NULL ) {
		CG_Error( "CG_InterpoateEntityPosition: cg.nextSnap == NULL" );
	}
	//CG_Printf("CG_InterpolateEntityPosition: TODO\n");
	f = cg.frameInterpolation;

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	//BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
	//BG_EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

	const float *current = cent->currentState.origin;
	const float *next = cent->nextState.origin;

	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

	//BG_EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
	//BG_EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

	current = cent->currentState.angles;
	next = cent->nextState.angles;

	cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );

	
	// NOTE: some centities might have both rEnt and rLight present
	if(cent->rEnt) {
		cent->rEnt->setOrigin(cent->lerpOrigin);
		cent->rEnt->setAngles(cent->lerpAngles);
	} 
	if(cent->rLight) {
		cent->rLight->setOrigin(cent->lerpOrigin);
		// TODO: lerp light radius?
		cent->rLight->setRadius(cent->currentState.lightRadius);
		cent->rLight->setBNoShadows(cent->currentState.lightFlags & LF_NOSHADOWS);
	}
}


static void CG_AddCEntity( centity_t *cent );
/*
===============
CG_CalcEntityLerpPositions

===============
*/
#include <api/rAPI.h>
#include <api/mtrAPI.h>
static void CG_CalcEntityLerpPositions( centity_t *cent ) {
	if(cent->currentState.parentNum != ENTITYNUM_NONE) {
		centity_t *parent = &cg_entities[cent->currentState.parentNum];
		if(parent->rEnt == 0)
			return;
		// first we have to update parent orientation (pos + rot),
		// then we can attach current entity to it
		CG_AddCEntity(parent);
		matrix_c mat;
		parent->rEnt->getBoneWorldOrientation(cent->currentState.parentTagNum,mat);
		cent->lerpAngles = mat.getAngles();
		cent->lerpOrigin = mat.getOrigin();
		if(cent->currentState.parentOffset.isAlmostZero() == false) {
			matrix_c matAngles = mat;
			matAngles.setOrigin(vec3_origin);
			vec3_c ofs;
			matAngles.transformPoint(cent->currentState.parentOffset,ofs);
			cent->lerpOrigin += ofs;
		}
		if(cent->currentState.localAttachmentAngles.isAlmostZero() == false) {
			cent->lerpAngles += cent->currentState.localAttachmentAngles;
		}
		// NOTE: some centities might have both rEnt and rLight present
		if(cent->rEnt) {
			cent->rEnt->setOrigin(cent->lerpOrigin);
			cent->rEnt->setAngles(cent->lerpAngles);
		} 
		if(cent->rLight) {
			cent->rLight->setOrigin(cent->lerpOrigin);
		}
		return;
	}

	if ( cent->interpolate) {
		CG_InterpolateEntityPosition( cent );
		return;
	}


	// just use the current frame and evaluate as best we can
	//BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	//BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	//// adjust for riding a mover if it wasn't rolled into the predicted
	//// player state
	//if ( cent != &cg.predictedPlayerEntity ) {
	//	CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum, 
	//	cg.snap->serverTime, cg.time, cent->lerpOrigin, cent->lerpAngles, cent->lerpAngles);
	//}
}

static void CG_UpdateEntityEmitter( centity_t *cent ) {
	if(cent->currentState.isEmitterActive()) {
		class mtrAPI_i *mat = cgs.gameMaterials[cent->currentState.trailEmitterMaterial];
		if(cent->emitter == 0) {
			cent->emitter = new emitter_c(cg.time);
			rf->addCustomRenderObject(cent->emitter);
		}
		cent->emitter->setOrigin(cent->lerpOrigin);
		cent->emitter->setMaterial(mat);
		cent->emitter->setRadius(cent->currentState.trailEmitterSpriteRadius);
		cent->emitter->setInterval(cent->currentState.trailEmitterInterval);

		cent->emitter->updateEmitter(cg.time);
	} else {
		if(cent->emitter) {
			delete cent->emitter;
			cent->emitter = 0;
		}
	}
}
/*
===============
CG_AddCEntity

===============
*/
static void CG_AddCEntity( centity_t *cent ) {
	if(cent->lastUpdateFrame == cg.clientFrame)
		return; // it was already updated (this may happen for attachment parents)
	cent->lastUpdateFrame = cg.clientFrame;

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );
	// update entity emitter
	CG_UpdateEntityEmitter( cent );

	switch ( cent->currentState.eType ) {
	default:
		g_core->RedWarning( "Bad entity type: %i\n", cent->currentState.eType );
		break;
	case ET_TRIGGER:
		// they should be never sent to client...
		// TODO: print warning?
		break;

	case ET_PORTAL:

		break;
	case ET_GENERAL:
		//CG_General( cent );
		break;
	case ET_PLAYER:
		//CG_Player( cent );
		break;
	case ET_LIGHT:
		//CG_Light( cent );
		break;
	}
}

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
	int					num;
	centity_t			*cent;
//	playerState_s		*ps;

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


	// generate and add the entity from the playerstate
	//ps = &cg.predictedPlayerState;
	//BG_PlayerStateToEntityState( ps, &cg.predictedPlayerEntity.currentState, qfalse );
	//CG_AddCEntity( &cg.predictedPlayerEntity );

	// lerp the non-predicted value for lightning gun origins
	//CG_CalcEntityLerpPositions( &cg_entities[ cg.snap->ps.clientNum ] );

	CG_AddCEntity( &cg_entities[ cg.snap->ps.clientNum ] );

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
		CG_AddCEntity( cent );
	}
}

