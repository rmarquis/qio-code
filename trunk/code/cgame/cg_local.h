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
#include "../qcommon/q_shared.h"
#include "../game/bg_public.h"
#include "cg_public.h"
#include <math/vec3.h>
#include <math/axis.h>


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


// centity_t have a direct corespondence with edict_s in the game, but
// only the entityState_s is directly communicated to the cgame
typedef struct centity_s {
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
} centity_t;


//======================================================================


//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

typedef struct {
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
	centity_t		predictedPlayerEntity;
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


} cg_t;


// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
typedef struct {
	class mtrAPI_i *charsetShader;
	class mtrAPI_i *whiteShader;


} cgMedia_t;


// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
typedef struct {
	gameState_s		gameState;			// gamestate from server
//	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenXBias;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	bool		localServer;		// detected on startup by checking sv_running

	int				maxclients;
	char			mapname[MAX_QPATH];


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

} cgs_t;

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	centity_t		cg_entities[MAX_GENTITIES];

extern	vmCvar_s		cg_lagometer;
extern	vmCvar_s		cg_drawFPS;
extern	vmCvar_s		cg_draw2D;
extern	vmCvar_s		cg_fov;
extern	vmCvar_s		cg_thirdPersonRange;
extern	vmCvar_s		cg_thirdPersonAngle;
extern	vmCvar_s		cg_thirdPerson;
extern	vmCvar_s		cg_timescale;
extern	vmCvar_s		cg_timescaleFadeEnd;
extern	vmCvar_s		cg_timescaleFadeSpeed;

//
// cg_main.c
//
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void QDECL CG_Printf( const char *msg, ... ) __attribute__ ((format (printf, 1, 2)));
void QDECL CG_Error( const char *msg, ... ) __attribute__ ((noreturn, format (printf, 1, 2)));

void CG_UpdateCvars( void );

//
// cg_view.c
//
void CG_DrawActiveFrame( int serverTime, bool demoPlayback );


//
// cg_player.c
//
void CG_Player( centity_t *cent );

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
// cg_drawTools.c
//
void CG_DrawBigString( int x, int y, const char *s, float alpha );
int CG_DrawStrlen( const char *str );
void CG_DrawPic( float x, float y, float width, float height, class mtrAPI_i *hShader );
void CG_DrawSmallString( int x, int y, const char *s, float alpha );
void CG_DrawSmallStringColor( int x, int y, const char *s, float color[4] );

//
// cg_draw.c
//
void CG_DrawActive();
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_AdjustFrom640( float *x, float *y, float *w, float *h );
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

//===============================================

//
// system traps
// These functions are how the cgame communicates with the main game system
//

// add commands to the local console as if they were typed in
// for map changing, etc.  The command is not executed immediately,
// but will be executed in order the next time console commands
// are processed
void		trap_SendConsoleCommand( const char *text );

// register a command name so the console can perform command completion.
// FIXME: replace this with a normal console command "defineCommand"?
void		trap_AddCommand( const char *cmdName );

// send a string to the server over the network
void		trap_SendClientCommand( const char *s );

