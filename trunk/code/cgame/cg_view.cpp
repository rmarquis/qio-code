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
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include "cg_local.h"
#include <api/cvarAPI.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>
#include <math/vec3.h>
#include <shared/trace.h>
#include <shared/autoCvar.h>

//============================================================================

static aCvar_c cg_printCurCamPos("cg_printCurCamPos","0");

/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect (void) {

}

//==============================================================================


/*
===============
CG_OffsetThirdPersonView

===============
*/
#define	FOCUS_DISTANCE	512
static void CG_OffsetThirdPersonView( void ) {
	vec3_t		forward, right, up;
	vec3_c		view;
	vec3_t		focusAngles;
	static vec3_t	mins = { -4, -4, -4 };
	static vec3_t	maxs = { 4, 4, 4 };
	vec3_t		focusPoint;
	float		focusDist;
	float		forwardScale, sideScale;

	cg.refdefViewOrigin[2] += cg.predictedPlayerState.viewheight;

	VectorCopy( cg.refdefViewAngles, focusAngles );

	// if dead, look at killer
	//if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
	//	focusAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
	//	cg.refdefViewAngles[YAW] = cg.predictedPlayerState.stats[STAT_DEAD_YAW];
	//}

	if ( focusAngles[PITCH] > 45 ) {
		focusAngles[PITCH] = 45;		// don't go too far overhead
	}
	AngleVectors( focusAngles, forward, NULL, NULL );

	VectorMA( cg.refdefViewOrigin, FOCUS_DISTANCE, forward, focusPoint );

	VectorCopy( cg.refdefViewOrigin, view );

	view[2] += 8;

	cg.refdefViewAngles[PITCH] *= 0.5;

	AngleVectors( cg.refdefViewAngles, forward, right, up );
float thirdPersonRange = 128.f;
	forwardScale = cos( cg_thirdPersonAngle.value / 180 * M_PI );
	sideScale = sin( cg_thirdPersonAngle.value / 180 * M_PI );
	VectorMA( view, -thirdPersonRange * forwardScale, forward, view );
	VectorMA( view, -thirdPersonRange * sideScale, right, view );

	// trace a ray from the origin to the viewpoint to make sure the view isn't
	// in a solid block.  Use an 8 by 8 block to prevent the view from near clipping anything

//	if (!cg_cameraMode.integer)
	{
		trace_c trace;
		trace.setupRay(cg.refdefViewOrigin,view);

		CG_RayTrace(trace, cg.clientNum);
		//CG_Trace( &trace, cg.refdefViewOrigin, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

		if (trace.getFraction() != 1.f) {
		//	VectorCopy( trace.endpos, view );
		//	view[2] += (1.0 - trace.fraction) * 32;
		//	// try another trace to this position, because a tunnel may have the ceiling
		//	// close enogh that this is poking out

		//	CG_Trace( &trace, cg.refdefViewOrigin, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
		//	VectorCopy( trace.endpos, view );
			view = trace.getHitPos();
			view -= trace.getDir() * 7.f;
		}
	}

	VectorCopy( view, cg.refdefViewOrigin );

	// select pitch to look at focus point from vieword
	VectorSubtract( focusPoint, cg.refdefViewOrigin, focusPoint );
	focusDist = sqrt( focusPoint[0] * focusPoint[0] + focusPoint[1] * focusPoint[1] );
	if ( focusDist < 1 ) {
		focusDist = 1;	// should never happen
	}
	cg.refdefViewAngles[PITCH] = -180 / M_PI * atan2( focusPoint[2], focusDist );
	cg.refdefViewAngles[YAW] -= cg_thirdPersonAngle.value;	
}


// this causes a compiler bug on mac MrC compiler
static void CG_StepOffset( void ) {
	int		timeDelta;
	
	// smooth out stair climbing
	timeDelta = cg.time - cg.stepTime;
	if ( timeDelta < STEP_TIME ) {
		cg.refdefViewOrigin[2] -= cg.stepChange 
			* (STEP_TIME - timeDelta) / STEP_TIME;
	}
}

/*
===============
CG_OffsetFirstPersonView

===============
*/
static void CG_OffsetFirstPersonView( void ) {
	float			*origin;
	float			*angles;
	float			delta;
	float			speed;
	float			f;
	vec3_t			predictedVelocity;
	int				timeDelta;

	origin = cg.refdefViewOrigin;
	angles = cg.refdefViewAngles;

	// add angles based on velocity
	VectorCopy( cg.predictedPlayerState.velocity, predictedVelocity );

	//delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[0]);
	//angles[PITCH] += delta * cg_runpitch.value;
	//
	//delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[1]);
	//angles[ROLL] -= delta * cg_runroll.value;

	// add angles based on bob

	// make sure the bob is visible even at low speeds
	speed = 200;


//===================================

	// add view height
	origin[2] += cg.predictedPlayerState.viewheight;

	// smooth out duck height changes
	timeDelta = cg.time - cg.duckTime;
	if ( timeDelta < DUCK_TIME) {
		cg.refdefViewOrigin[2] -= cg.duckChange 
			* (DUCK_TIME - timeDelta) / DUCK_TIME;
	}




	// add fall height
	delta = cg.time - cg.landTime;
	if ( delta < LAND_DEFLECT_TIME ) {
		f = delta / LAND_DEFLECT_TIME;
		cg.refdefViewOrigin[2] += cg.landChange * f;
	} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
		delta -= LAND_DEFLECT_TIME;
		f = 1.0 - ( delta / LAND_RETURN_TIME );
		cg.refdefViewOrigin[2] += cg.landChange * f;
	}

	// add step offset
	CG_StepOffset();
}

//======================================================================

void CG_ZoomDown_f( void ) { 
	if ( cg.zoomed ) {
		return;
	}
	cg.zoomed = qtrue;
	cg.zoomTime = cg.time;
}

void CG_ZoomUp_f( void ) { 
	if ( !cg.zoomed ) {
		return;
	}
	cg.zoomed = qfalse;
	cg.zoomTime = cg.time;
}


/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4



/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
	playerState_s	*ps;

	// calculate size of 3D view
	CG_CalcVrect();

	ps = &cg.predictedPlayerState;

	cg.refdefViewOrigin = ps->origin;
	cg.refdefViewAngles = ps->viewangles;

	// add first person / third person view offset
	if ( cg_thirdPerson.integer ) {
		// back away from character
		CG_OffsetThirdPersonView();
	} else {
		// offset for local bobbing and kicks
		CG_OffsetFirstPersonView();
	}
	cg.refdefViewAxis.fromAngles(cg.refdefViewAngles);

	if(cg_printCurCamPos.getInt()) {
		CG_Printf("CG_CalcViewValues: camera eye is at %f %f %f\n",cg.refdefViewOrigin[0],cg.refdefViewOrigin[1],cg.refdefViewOrigin[2]);
	}

	projDef_s projDef;
	projDef.setDefaults();
	rf->setupProjection3D(&projDef);
	rf->setup3DView(cg.refdefViewOrigin,cg.refdefViewAngles,cg_thirdPerson.integer);

	return 0;
}



//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame( int serverTime, qboolean demoPlayback ) {
	int		inwater;

	cg.time = serverTime;

	rf->setRenderTimeMsec(cg.time);

	// update cvars
	CG_UpdateCvars();

	// clear all the render lists
//	trap_R_ClearScene();

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots();

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
		//CG_DrawInformation();
		return;
	}

	// this counter will be bumped for every valid scene we generate
	cg.clientFrame++;

	// update cg.predictedPlayerState
	CG_PredictPlayerState();

	// build cg.refdef
	inwater = CG_CalcViewValues();

	// update test model
	CG_RunTestModel();

	// update test emitter
	CG_RunTestEmitter();

	// update test material/shape
	CG_RunTestMaterial();

	// update view model
	CG_RunViewModel();

	// update client-only temporary lights
	CG_RunTempLights();

	// timout bullet tracers
	CG_UpdateBulletTracers();

	// build the render lists
	CG_AddPacketEntities();			// adter calcViewValues, so predicted player state is correct

//	cg.refdef.time = cg.time;
//	memcpy( cg.refdef.areamask, cg.snap->areamask, sizeof( cg.refdef.areamask ) );

	// make sure the lagometerSample and frame timing isn't done twice when in stereo
	//if ( stereoView != STEREO_RIGHT ) {
		cg.frametime = cg.time - cg.oldTime;
		if ( cg.frametime < 0 ) {
			cg.frametime = 0;
		}
		cg.oldTime = cg.time;
		CG_AddLagometerFrameInfo();
	//}
	if (cg_timescale.value != cg_timescaleFadeEnd.value) {
		if (cg_timescale.value < cg_timescaleFadeEnd.value) {
			cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
			if (cg_timescale.value > cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}
		else {
			cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
			if (cg_timescale.value < cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}
		if (cg_timescaleFadeSpeed.value) {
			g_cvars->Cvar_Set("timescale", va("%f", cg_timescale.value));
		}
	}

	// actually issue the rendering calls
	CG_DrawActive();

}

