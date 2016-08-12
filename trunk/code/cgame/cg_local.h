/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

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
#include "../game/bg_public.h"
#include <protocol/gameState.h> 
#include "cg_public.h"
#include <math/vec3.h>
#include <math/axis.h>
#include <shared/autoCvar.h>


// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

#define	LAND_DEFLECT_TIME	150
#define	LAND_RETURN_TIME	300
#define	STEP_TIME			200
#define	DUCK_TIME			100
#define	ZOOM_TIME			150

#define	MAX_STEP_CHANGE		32

#define	CHAR_WIDTH			32
#define	CHAR_HEIGHT			48

//=================================================


// centity_s have a direct corespondence with edict_s in the game, but
// only the entityState_s is directly communicated to the cgame
struct centity_s {
	entityState_s	currentState;	// from cg.frame
	entityState_s	nextState;		// from cg.nextFrame, if available
	bool		interpolate;	// true if next is valid to interpolate to
	bool		currentValid;	// true if cg.frame holds this entity

	int				snapShotTime;	// last time this entity was found in a snapshot
	int				lastUpdateFrame; // cg.frameNum when entity was updated

	bool		extrapolated;	// false if origin / angles is an interpolation

	// exact interpolated position of entity on this frame
	vec3_c			lerpOrigin;
	vec3_c			lerpAngles;

	class rEntityAPI_i *rEnt; // for all entity types except ET_LIGHT
	class rLightAPI_i *rLight; // for ET_LIGHT and for all entities with entityState_t::lightRadius != 0.f
	class emitterBase_c *emitter; // for all entities with entity emitter enabled
};


//======================================================================


//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

struct cg_t {
	int			clientFrame;		// incremented each frame

	int			clientNum;

	bool	demoPlayback;

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	int			frametime;		// cg.time - cg.oldTime

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	// prediction state
	playerState_s	predictedPlayerState;
	centity_s		predictedPlayerEntity;
	bool	validPPS;				// clear until the first call to CG_PredictPlayerState

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

	// input state sent to server



	// view rendering
	vec3_c		refdefViewOrigin;
	vec3_c		refdefViewAngles;		// will be converted to refdef.viewaxis
	axis_c		refdefViewAxis;

	// zoom key
	bool	zoomed;
	int			zoomTime;
	float		zoomSensitivity;

	// zFar value
	float farPlane;


};


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
struct cgMedia_t {
	class mtrAPI_i *charsetShader;
	class mtrAPI_i *whiteShader;


};


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
struct cgs_t {
	gameState_s		gameState;			// gamestate from server
	float			screenXScale;		// derived from renderer
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	bool		localServer;		// detected on startup by checking sv_running

	int				maxclients;
	char			mapname[1024];


	int				levelStartTime;


	//
	// locally derived information from gamestate
	//
	class rModelAPI_i	*gameModels[MAX_MODELS];
	class cMod_i		*gameCollModels[MAX_MODELS];
	//sfxHandle_t		gameSounds[MAX_SOUNDS];
	const class skelAnimAPI_i	*gameAnims[MAX_ANIMATIONS];
//	str gameAnimNames[MAX_ANIMATIONS];
	const class afDeclAPI_i	*gameAFs[MAX_RAGDOLLDEFS];
	class mtrAPI_i *gameMaterials[MAX_MATERIALS];
	// media
	cgMedia_t		media;

};

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_s		cg_entities[MAX_GENTITIES];

extern	aCvar_c		cg_lagometer;
extern	aCvar_c		cg_drawFPS;
extern	aCvar_c		cg_draw2D;
extern	aCvar_c		cg_fov;
extern	aCvar_c		cg_thirdPersonRange;
extern	aCvar_c		cg_thirdPersonAngle;
extern	aCvar_c		cg_thirdPerson;
extern	aCvar_c		cg_timescale;
extern	aCvar_c		cg_timescaleFadeEnd;
extern	aCvar_c		cg_timescaleFadeSpeed;

//
// cg_main.c
//
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void CG_Printf( const char *msg, ... );
void CG_Error( const char *msg, ... );

void CG_UpdateCvars( void );

//
// cg_view.c
//
void CG_DrawActiveFrame( int serverTime, bool demoPlayback );


//
// cg_player.c
//
void CG_Player( centity_s *cent );

//
// cg_ents.c
//
void CG_AddPacketEntities( void );


//
// cg_snapshot.c
//
void CG_ProcessSnapshots( void );

//
// cg_consolecmds.c
//
bool CG_ConsoleCommand( void );
void CG_InitConsoleCommands( void );

//
// cg_servercmds.c
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo( void );
void CG_SetConfigValues( void );


//
// cg_playerstate.c
//
void CG_Respawn( void );
void CG_TransitionPlayerState( playerState_s *ps, playerState_s *ops );
void CG_CheckChangedPredictableEvents( playerState_s *ps );
void CG_PredictPlayerState();

//
// cg_draw.c
//
void CG_DrawActive();
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_AddLagometerFrameInfo( void ) ;

//
// cg_collision.cpp
//
bool CG_RayTrace(class trace_c &tr, u32 skipEntNum = ENTITYNUM_NONE);

//
// cg_testModel.cpp
//
void CG_RunTestModel();

//
// cg_viewModel.cpp
//
void CG_RunViewModel();
bool CG_GetViewModelBonePos(const char *boneName, class vec3_c &out);

//
// cg_tracer.cpp
//
void CG_AddBulletTracer(const vec3_c &from, const vec3_c &to, float width, class mtrAPI_i *mat, int life);
void CG_UpdateBulletTracers();

//
// cg_testEmitter.cpp
//
void CG_RunTestEmitter();

//
// cg_testMaterial.cpp
//
void CG_RunTestMaterial();
void CG_FreeTestMaterialClass();

//
// cg_tempLighs.cpp
//
void CG_CreateTempLight(const class vec3_c &pos, float radius, int totalLife);
void CG_RunTempLights();

//
// cg_chat.cpp
//
void CG_DrawChat();
void CG_AddChatMessage(const char *msg);

