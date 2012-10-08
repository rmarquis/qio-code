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
// g_local.h -- local definitions for game module

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "g_public.h"

//==================================================================

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	BASEGAME

#define INFINITE			1000000

#define	FRAMETIME			100					// msec

// gentity->flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020

//============================================================================

struct gentity_s {
	entityState_t	s;				// communicated by server to clients
	qboolean	inuse;

	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!
	//================================

	struct gclient_s	*client;			// NULL if not a client



	char		*classname;			// set in QuakeEd

	int			freetime;			// level.time when the object was freed

	// bullet physics object
	class btRigidBody *body;
	// simplified model for collision detection
	class cMod_i *cmod;
};


typedef enum {
	CON_DISCONNECTED,
	CON_CONNECTING,
	CON_CONNECTED
} clientConnected_t;

//
#define MAX_NETNAME			36

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
typedef struct {
	clientConnected_t	connected;	
	usercmd_t	cmd;				// we would lose angles if not persistant
	qboolean	localClient;		// true if "ip" info key is "localhost"
	qboolean	initialSpawn;		// the first spawn should be at a cool location
	qboolean	predictItemPickup;	// based on cg_predictItems userinfo
	qboolean	pmoveFixed;			//
	char		netname[MAX_NETNAME];
	int			maxHealth;			// for handicapping
	int			enterTime;			// level.time the client entered the game
} clientPersistant_t;


// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
struct gclient_s {
	// ps MUST be the first element, because the server expects it
	playerState_t	ps;				// communicated by server to clients

	// the rest of the structure is private to game
	clientPersistant_t	pers;

	class btKinematicCharacterController *characterController;

	int			buttons;
	int			oldbuttons;
};

typedef struct {
	struct gclient_s	*clients;		// [maxclients]

	struct gentity_s	*gentities;
	int			gentitySize;
	int			num_entities;		// MAX_CLIENTS <= num_entities <= ENTITYNUM_MAX_NORMAL

	int			framenum;
	int			time;					// in msec
	float frameTime; // in sec
	int			previousTime;			// so movers can back up when blocked

	int			startTime;				// level.time the map was started
} level_locals_t;

//
// g_utils.c
//
int G_ModelIndex( const char *name );
int G_CollisionModelIndex( const char *name );
int		G_SoundIndex( const char *name );
gentity_s *G_Find (gentity_s *from, int fieldofs, const char *match);
void	G_InitGentity( gentity_s *e );
gentity_s	*G_Spawn (void);
void	G_FreeEntity( gentity_s *e );
qboolean	G_EntitiesFree( void );

//
// g_client.c
//
void SetClientViewAngle( gentity_s *ent, vec3_t angle );
void ClientRespawn(gentity_s *ent);
void ClientSpawn( gentity_s *ent );

//
// g_main.c
//
void G_InitGame( int levelTime, int randomSeed, int restart );
void G_RunFrame( int levelTime );
void G_ShutdownGame( int restart );
void QDECL G_Printf( const char *fmt, ... ) __attribute__ ((format (printf, 1, 2)));
void QDECL G_Error( const char *fmt, ... ) __attribute__ ((noreturn, format (printf, 1, 2)));

//
// g_client.c
//
const char *ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum );
void ClientCommand( int clientNum );

//
// g_active.c
//
void ClientThink( int clientNum );
void ClientEndFrame( gentity_s *ent );
void G_RunClient( gentity_s *ent );


//
// g_bullet.cpp
//
void G_InitBullet();
void G_ShudownBullet();
void G_RunPhysics();
void G_LoadMap(const char *mapName);
void G_RunCharacterController(vec3_t dir, class btKinematicCharacterController *ch, vec3_t newPos);
class btKinematicCharacterController* BT_CreateCharacter(float stepHeight, vec3_t pos, float characterHeight,  float characterWidth);
void G_TryToJump(btKinematicCharacterController *ch);
void BT_FreeCharacter(class btKinematicCharacterController *c);
void G_UpdatePhysicsObject(gentity_s *ent);
void BT_CreateBoxEntity(gentity_s *ent, const float *pos, const float *halfSizes, const float *startVel);
gentity_s *BT_CreateBoxEntity(const float *pos, const float *halfSizes, const float *startVel);

//
// g_debugDraw.cpp
//
void G_DebugDrawFrame(class rAPI_i *pRFAPI);

extern	level_locals_t	level;
extern	gentity_s		g_entities[MAX_GENTITIES];
