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
#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__

#include "bg_public.h"
#include "edict.h"
#include <shared/array.h>
#include <math/vec3.h>

//==================================================================

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	BASEGAME

#define	FRAMETIME			100					// msec

//============================================================================

struct level_locals_t {
	struct edict_s	*gentities;
	int			gentitySize;
	int			num_entities;		// MAX_CLIENTS <= num_entities <= ENTITYNUM_MAX_NORMAL

	int			framenum;
	int			time;					// in msec
	float		frameTime; // in sec
	int			frameTimeMs;
	int			previousTime;			// so movers can back up when blocked

	int			startTime;				// level.time the map was started
};

#include <shared/str.h>
struct railgunAttackMaterials_s {
	str railCore;
	str railExplosion;
	str railDisc;
	str markMaterial;

	railgunAttackMaterials_s() {
		railCore = "xrealRailCore";
		railExplosion = "xrealRailRing";
		railDisc = "xrealRailDisc";
		markMaterial = "xrealPlasmaMark";
	}
	void setupQuake3() {
		railCore = "railCore";
		railExplosion = "railExplosion";
		railDisc = "railDisc";
		markMaterial = "gfx/damage/plasma_mrk";
	}
};

//
// g_utils.cpp
//
int G_RenderModelIndex( const char *name );
int G_CollisionModelIndex( const char *name );
int	G_SoundIndex( const char *name );
int G_AnimationIndex( const char *name );
int G_RagdollDefIndex( const char *name );
int G_RenderSkinIndex( const char *name );
int G_RenderMaterialIndex( const char *name );

edict_s *G_Find (edict_s *from, int fieldofs, const char *match);
edict_s	*G_Spawn (void);
bool G_EntitiesFree( void );
u32 G_GetEntitiesOfClass(const char *classNameOrig, arraySTD_c<class BaseEntity*> &out);
u32 G_GetEntitiesWithTargetName(const char *targetName, arraySTD_c<class BaseEntity*> &out);
u32 G_GetEntitiesWithScriptName(const char *targetName, arraySTD_c<class BaseEntity*> &out);
class BaseEntity *G_GetRandomEntityOfClass(const char *classNameOrig);
class BaseEntity *G_FindFirstEntityWithTargetName(const char *targetName);
void G_HideEntitiesWithTargetName(const char *targetName);
void G_ShowEntitiesWithTargetName(const char *targetName);
void G_PostEvent(const char *targetName, int execTime, const char *eventName, const char *arg0 = 0, const char *arg1 = 0, const char *arg2 = 0, const char *arg3 = 0);
u32 G_RemoveEntitiesOfClass(const char *className);
const vec3_c &G_GetPlayerOrigin(u32 playerNum);
class Player *G_GetPlayer(u32 playerNum);
u32 G_FindMoversWithTeam(class arraySTD_c<class Mover*> &out, const char *team);

//
// g_client.c
//
void ClientSpawn( edict_s *ent );

//
// g_main.c
//
void G_InitGame( int levelTime, int randomSeed, int restart );
void G_RunFrame( int levelTime );
void G_ProcessEntityEvents();
void G_ShutdownGame( int restart );
void G_Printf( const char *fmt, ... );
void G_Error( const char *fmt, ... );

//
// g_client.c
//
const char *ClientConnect( int clientNum, bool firstTime, bool isBot );
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
//void G_InitBullet();
//void G_ShudownBullet();
//void G_RunPhysics();
//void G_LoadMap(const char *mapName);
//void G_RunCharacterController(vec3_t dir, class btKinematicCharacterController *ch, vec3_t newPos);
//class btKinematicCharacterController* BT_CreateCharacter(float stepHeight, vec3_t pos, float characterHeight,  float characterWidth);
//void BT_SetCharacterEntity(class btKinematicCharacterController *ch, class ModelEntity *e);
//bool G_TryToJump(btKinematicCharacterController *ch);
//bool BT_IsCharacterOnGround(btKinematicCharacterController *ch);
//void BT_FreeCharacter(class btKinematicCharacterController *c);
//void BT_SetCharacterPos(class btKinematicCharacterController *c, const vec3_c &p);
//void BT_SetCharacterVelocity(class btKinematicCharacterController *c, const vec3_c &newVel);
//void G_UpdatePhysicsObject(edict_s *ent);
//class physVehicleAPI_i *BT_CreateVehicle(const vec3_c &pos, const vec3_c &angles, class cMod_i *cmodel);
//void BT_RemoveVehicle(class physVehicleAPI_i *pv);
//void BT_RunVehicles();
//void BT_ShutdownVehicles();
//const class aabb &G_GetInlineModelBounds(u32 inlineModelNum);
//class cMod_i *BT_GetSubModelCModel(u32 inlineModelNum);
//bool BT_IsInSolid(const class matrix_c &mat, const class aabb &bb);
//bool BT_TraceRay(class trace_c &tr);

//
// g_physDLL.cpp
//
void G_InitPhysicsEngine();
void G_LoadMap(const char *mapName);
void G_ShutdownPhysicsEngine();
void G_RunPhysics();

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
BaseEntity *G_SpawnGeneric(const char *classOrModelName);

//
// g_bullet_debugDraw.cpp
//
void G_DoBulletDebugDrawing(class rDebugDrawer_i *dd);

//
// g_collision.cpp
//
bool G_TraceRay(class trace_c &tr, BaseEntity *baseSkip);
u32 G_BoxEntities(const class aabb &bb, arraySTD_c<class BaseEntity*> &out);

//
// g_weapons.cpp
//
void G_BulletAttack(const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip, const char *markMaterial = 0);
void G_MultiBulletAttack(const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip, u32 numBullets, float maxSpread, float spreadDist, const char *markMaterial = 0);
void G_Explosion(const vec3_c &pos, const struct explosionInfo_s &explosionInfo, const char *extraDamageDefName = 0);
void G_RailGunAttack(const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip, const struct railgunAttackMaterials_s *mats);
// projectileDefName is the name of Doom3 projectile entity def
// (for example: "projectile_bfg")
void G_FireProjectile(const char *projectileDefName, const vec3_c &muzzle, const vec3_c &dir, BaseEntity *baseSkip);

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

//
// g_lua.cpp
//
void            G_InitLua();
void            G_ShutdownLua();
void            G_LoadLuaScript(const char *filename);
void            G_DumpLuaStack();

//
// g_ammo.cpp
//
void G_InitAmmoTypes();

//
// g_wolfScript.cpp
//

void G_InitWolfScripts(const char *mapName);
void G_WolfScript_StartScript(const char *scriptName, const char *labelName);
void G_WolfScript_StartScript_Spawn(const char *scriptName);

extern	level_locals_t	level;
extern	edict_s		g_entities[MAX_GENTITIES];

#endif // __G_LOCAL_H__
