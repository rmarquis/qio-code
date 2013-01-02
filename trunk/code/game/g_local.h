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
#include <shared/array.h>
#include <math/vec3.h>

//==================================================================

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	BASEGAME

#define	FRAMETIME			100					// msec

//============================================================================

typedef struct {
	struct edict_s	*gentities;
	int			gentitySize;
	int			num_entities;		// MAX_CLIENTS <= num_entities <= ENTITYNUM_MAX_NORMAL

	int			framenum;
	int			time;					// in msec
	float frameTime; // in sec
	int			previousTime;			// so movers can back up when blocked

	int			startTime;				// level.time the map was started
} level_locals_t;

//
// g_utils.cpp
//
int G_RenderModelIndex( const char *name );
int G_CollisionModelIndex( const char *name );
int	G_SoundIndex( const char *name );
int G_AnimationIndex( const char *name );
int G_RagdollDefIndex( const char *name );
int G_RenderSkinIndex( const char *name );

edict_s *G_Find (edict_s *from, int fieldofs, const char *match);
edict_s	*G_Spawn (void);
void G_FreeEntity( edict_s *e );
qboolean G_EntitiesFree( void );
u32 G_GetEntitiesOfClass(const char *classNameOrig, arraySTD_c<class BaseEntity*> &out);
class BaseEntity *G_GetRandomEntityOfClass(const char *classNameOrig);
class BaseEntity *G_FindFirstEntityWithTargetName(const char *targetName);

//
// g_client.c
//
void ClientRespawn(edict_s *ent);
void ClientSpawn( edict_s *ent );

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
struct playerState_s *ClientGetPlayerState(u32 clientNum);

//
// g_active.c
//
void ClientThink( int clientNum );
void ClientEndFrame( edict_s *ent );
void G_RunClient( edict_s *ent );


//
// g_bullet.cpp
//
void G_InitBullet();
void G_ShudownBullet();
void G_RunPhysics();
void G_LoadMap(const char *mapName);
void G_RunCharacterController(vec3_t dir, class btKinematicCharacterController *ch, vec3_t newPos);
class btKinematicCharacterController* BT_CreateCharacter(float stepHeight, vec3_t pos, float characterHeight,  float characterWidth);
bool G_TryToJump(btKinematicCharacterController *ch);
bool BT_IsCharacterOnGround(btKinematicCharacterController *ch);
void BT_FreeCharacter(class btKinematicCharacterController *c);
void BT_SetCharacterPos(class btKinematicCharacterController *c, const vec3_c &p);
void BT_SetCharacterVelocity(class btKinematicCharacterController *c, const vec3_c &newVel);
void G_UpdatePhysicsObject(edict_s *ent);
class physVehicleAPI_i *BT_CreateVehicle(const vec3_c &pos, const vec3_c &angles, class cMod_i *cmodel);
void BT_RemoveVehicle(class physVehicleAPI_i *pv);
void BT_RunVehicles();
void BT_ShutdownVehicles();
const class aabb &G_GetInlineModelBounds(u32 inlineModelNum);
class cMod_i *BT_GetSubModelCModel(u32 inlineModelNum);
bool BT_IsInSolid(const class matrix_c &mat, const class aabb &bb);
bool BT_TraceRay(class trace_c &tr);
void BT_TestSpawnInternalRagDoll(const vec3_c &at);

//
// g_debugDraw.cpp
//
void G_DebugDrawFrame(class rAPI_i *pRFAPI);

//
// g_spawn.cpp
//
void G_SpawnMapEntities(const char *mapName);
// spawns a new entity with classname and key values
// loaded from .entDef file
BaseEntity *G_SpawnFirstEntDefFromFile(const char *fileName);
// spawn entity defined in .def file (Doom3 decls)
BaseEntity *G_SpawnEntityFromEntDecl(const char *declName);
BaseEntity *G_SpawnClass(const char *className);

//
// g_bullet_debugDraw.cpp
//
void G_DoBulletDebugDrawing(class rDebugDrawer_i *dd);

//
// g_collision.cpp
//
bool G_TraceRay(class trace_c &tr, BaseEntity *baseSkip);

//
// g_weapons.cpp
//
void G_BulletAttack(const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip);

//
// g_ragdoll.cpp
//
class ragdollAPI_i *G_SpawnTestRagdollFromAF(const char *afName, const vec3_c &pos, const vec3_c &angles);

//
// g_saveMapFile.cpp
//
bool G_SaveCurrentSceneToMapFile(const char *outFName);

//
// g_scriptedClasses.cpp
//
void G_InitScriptedClasses();
void G_ShutdownScriptedClasses();
const class scriptedClass_c *G_FindScriptedClassDef(const char *className);
u32 G_GetNumKnownScriptedClassDefs();

extern	level_locals_t	level;
extern	edict_s		g_entities[MAX_GENTITIES];
