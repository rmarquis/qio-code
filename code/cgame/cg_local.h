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
#include <protocol/configStrings.h>
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



//======================================================================


//======================================================================

// all cg.stepTime, cg.duckTime, cg.landTime, etc are set to cg.time when the action
// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

struct cg_t {
	int			clientFrame;		// incremented each frame
	int			clientNum;


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

	// prediction state
	playerState_s	predictedPlayerState;
	bool	validPPS;				// clear until the first call to CG_PredictPlayerState

	float		stepChange;				// for stair up smoothing
	int			stepTime;

	float		duckChange;				// for duck viewheight smoothing
	int			duckTime;

	float		landChange;				// for landing hard
	int			landTime;

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



// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
struct cgs_t {
	gameState_s		gameState;			// gamestate from server
	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	char			mapname[1024];


	int				levelStartTime;


	//
	// locally derived information from gamestate
	//
	class rModelAPI_i			*gameModels[MAX_MODELS];
	class cMod_i				*gameCollModels[MAX_MODELS];
	const class skelAnimAPI_i	*gameAnims[MAX_ANIMATIONS];
	const class afDeclAPI_i		*gameAFs[MAX_RAGDOLLDEFS];
	class mtrAPI_i				*gameMaterials[MAX_MATERIALS];
};

//==============================================================================

extern	cgs_t			cgs;
extern	cg_t			cg;

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
// cg_main.cpp
//
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown();
const char *CG_ConfigString( int index );
const char *CG_Argv( int arg );

void CG_Printf( const char *msg, ... );
void CG_Error( const char *msg, ... );

void CG_UpdateCvars();

//
// cg_view.cpp
//
void CG_DrawActiveFrame( int serverTime, bool demoPlayback );



//
// cg_snapshot.cpp
//
void CG_ProcessSnapshots();

//
// cg_consolecmds.cpp
//
bool CG_ConsoleCommand();
void CG_InitConsoleCommands();

//
// cg_servercmds.cpp
//
void CG_ExecuteNewServerCommands( int latestSequence );
void CG_ParseServerinfo();
void CG_SetConfigValues();


//
// cg_playerstate.cpp
//
void CG_Respawn();
void CG_TransitionPlayerState( playerState_s *ps, playerState_s *ops );
void CG_CheckChangedPredictableEvents( playerState_s *ps );
void CG_PredictPlayerState();

//
// cg_draw.cpp
//
void CG_DrawActive();
void CG_AddLagometerSnapshotInfo( snapshot_t *snap );
void CG_AddLagometerFrameInfo() ;

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

