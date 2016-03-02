/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2012-2014 V.

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
#include "cg_emitter.h" // default emitter
#include "cg_emitter_d3.h" // Doom3 emitter
#include <api/rEntityAPI.h>
#include <api/rLightAPI.h>
#include <api/coreAPI.h>
#include <api/declManagerAPI.h>
#include <math/matrix.h>
#include <shared/autoCvar.h>
//#include <shared/entityType.h>

static aCvar_c cg_printLightFlags("cg_printLightFlags","0");
static aCvar_c cg_printAttachedEntities("cg_printAttachedEntities","0");

/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

//=====================================================================

static void CG_OnEntityOrientationChange(centity_s *cent) {
	// NOTE: some centities might have both rEnt and rLight present
	if(cent->rEnt) {
		cent->rEnt->setOrigin(cent->lerpOrigin);
		cent->rEnt->setAngles(cent->lerpAngles);
	} 
	if(cent->rLight) {
		if(cg_printLightFlags.getInt()) {
			g_core->Print("Light entity %i lightFlags %i\n",cent->currentState.number,cent->currentState.lightFlags);
		}
		cent->rLight->setOrigin(cent->lerpOrigin);
		// TODO: lerp light radius?
		cent->rLight->setRadius(cent->currentState.lightRadius);
		cent->rLight->setBNoShadows(cent->currentState.lightFlags & LF_NOSHADOWS);
		if(cent->currentState.lightFlags & LF_SPOTLIGHT) {
			// see if the spotlight can find it's target
			const centity_s *target = &cg_entities[cent->currentState.lightTarget];
			cent->rLight->setSpotRadius(cent->currentState.spotLightRadius);
			cent->rLight->setLightType(LT_SPOTLIGHT);
			if(target->currentValid == false) {
				vec3_c targetPos = cent->lerpOrigin + cent->lerpAngles.getForward() * 64.f;
				cent->rLight->setSpotLightTarget(targetPos);
			} else {
				cent->rLight->setSpotLightTarget(target->lerpOrigin);
			}
		} else {
			cent->rLight->setLightType(LT_POINT);
		}
		if(cent->currentState.lightFlags & LF_COLOURED) {
			cent->rLight->setBColoured(true);
			cent->rLight->setColor(cent->currentState.lightColor);
		} else {
			cent->rLight->setBColoured(false);
		}
	}
}
/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_s *cent ) {
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

	// update render entity and/or render light
	CG_OnEntityOrientationChange(cent);
}


static void CG_RunCEntity( centity_s *cent );
/*
===============
CG_CalcEntityLerpPositions

===============
*/
#include <api/rAPI.h>
#include <api/mtrAPI.h>
static void CG_CalcEntityLerpPositions( centity_s *cent ) {
	// fast test
	if(cent->rEnt) {
		cent->rEnt->setScale(cent->currentState.scale);
	}
	if(cent->currentState.parentNum != ENTITYNUM_NONE) {
		centity_s *parent = &cg_entities[cent->currentState.parentNum];
		if(parent->rEnt == 0)
			return;
		if(cg_printAttachedEntities.getInt()) {
			g_core->Print("Entity %i is attached to %i\n",cent->currentState.number,cent->currentState.parentNum);
		}
		// first we have to update parent orientation (pos + rot),
		// then we can attach current entity to it
		CG_RunCEntity(parent);
		matrix_c mat;
		parent->rEnt->getBoneWorldOrientation(cent->currentState.parentTagNum,mat);
		cent->lerpAngles = mat.getAngles();
		cent->lerpOrigin = mat.getOrigin();
		if(cent->currentState.parentOffset.isAlmostZero() == false) {
			matrix_c matAngles = mat;
			matAngles.setOrigin(vec3_c(0,0,0));
			vec3_c ofs;
			matAngles.transformPoint(cent->currentState.parentOffset,ofs);
			cent->lerpOrigin += ofs;
		}
		if(cent->currentState.localAttachmentAngles.isAlmostZero() == false) {
			cent->lerpAngles += cent->currentState.localAttachmentAngles;
		}
		// NOTE: some centities might have both rEnt and rLight present
		// update render entity and/or render light
		CG_OnEntityOrientationChange(cent);
		return;
	}

	if ( cent->interpolate) {
		CG_InterpolateEntityPosition( cent );
		return;
	}
}

static void CG_UpdateEntityEmitter( centity_s *cent ) {
	if(cent->currentState.isEmitterActive()) {
		// get emitter name (it might be a material name or Doom3 particleDecl name)
		const char *emitterName = CG_ConfigString(CS_MATERIALS+cent->currentState.trailEmitterMaterial);
		if(emitterName[0] == 0) {
			g_core->RedWarning("CG_UpdateEntityEmitter: NULL entity emitter name\n");
		} else {
			// see if we have a Doom3 .prt decl for it
			class particleDeclAPI_i *prtDecl = g_declMgr->registerParticleDecl(emitterName);
			// if particle decl was not present, fall back to default simple emitter
			if(prtDecl == 0) {
				class mtrAPI_i *mat = cgs.gameMaterials[cent->currentState.trailEmitterMaterial];
				// if we have a valid material
				if(mat) {
					// create a simple material-based emitter
					if(cent->emitter == 0) {
						cent->emitter = new emitterDefault_c(cg.time);
						rf->addCustomRenderObject(cent->emitter);
					}
					cent->emitter->setMaterial(mat);
				}
			} else {
				// create a Doom3 particle system emitter
				if(cent->emitter == 0) {
					cent->emitter = new emitterD3_c;;
					rf->addCustomRenderObject(cent->emitter);
				}	
				cent->emitter->setParticleDecl(prtDecl);
			}
			cent->emitter->setOrigin(cent->lerpOrigin);
			cent->emitter->setRadius(cent->currentState.trailEmitterSpriteRadius);
			cent->emitter->setInterval(cent->currentState.trailEmitterInterval);

			cent->emitter->updateEmitter(cg.time);
		}
	} else {
		if(cent->emitter) {
			delete cent->emitter;
			cent->emitter = 0;
		}
	}
}
/*
===============
CG_RunCEntity

===============
*/
static void CG_RunCEntity( centity_s *cent ) {
	if(cent->lastUpdateFrame == cg.clientFrame)
		return; // it was already updated (this may happen for attachment parents)
	cent->lastUpdateFrame = cg.clientFrame;

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );
	// update entity emitter
	CG_UpdateEntityEmitter( cent );
}

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
	int					num;
	centity_s			*cent;

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

	CG_RunCEntity( &cg_entities[ cg.snap->ps.clientNum ] );

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
		CG_RunCEntity( cent );
	}
}

