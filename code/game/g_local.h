/*
============================================================================
Copyright (C) 2012-2016 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
//
// g_local.h
#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__

#include "bg_public.h"
#include "edict.h"
#include <shared/str.h>
#include <shared/array.h>
#include <math/vec3.h>

// game time stuff
extern u32 g_startTime;
extern u32 g_time; // in msec
extern float g_frameTime; // in sec
extern int	g_frameTimeMs;
extern int g_previousTime;
// edicts
extern edict_s g_entities[MAX_GENTITIES];
extern u32 g_numEdicts;

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
// g_configStrings.cpp
//
u32 G_RenderModelIndex(const char *name);
u32 G_CollisionModelIndex(const char *name);
u32 G_SoundIndex(const char *name);
u32 G_AnimationIndex(const char *name);
u32 G_RagdollDefIndex(const char *name);
u32 G_RenderSkinIndex(const char *name);
u32 G_RenderMaterialIndex(const char *name);

//
// g_ents.cpp
//
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
// g_main.cpp
//
void G_InitGame(int levelTime, int randomSeed, int restart);
void G_RunFrame(int levelTime);
void G_ProcessEntityEvents();
void G_ShutdownGame(int restart);

//
// g_clients.cpp
//
void ClientSpawn( edict_s *ent );
void ClientThink( int clientNum );
void ClientEndFrame( edict_s *ent );
void G_RunClient( edict_s *ent );
const char *ClientConnect( int clientNum, bool firstTime, bool isBot );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientBegin( int clientNum );
void ClientCommand( int clientNum );
struct playerState_s *ClientGetPlayerState(u32 clientNum);

//
// g_physDLL.cpp
// Rigid body physics wrapper.
void G_InitPhysicsEngine();
void G_LoadMap(const char *mapName);
void G_ShutdownPhysicsEngine();
void G_RunPhysics();

//
// g_debugDraw.cpp
// For info_pathnode and other stuff.
void G_DebugDrawFrame(class rAPI_i *pRFAPI);

//
// g_spawn.cpp
// Spawn classes, models, defs and TIKIs.
edict_s	*G_AllocEdict();
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
// Independent from Physics Module.
bool G_TraceRay(class trace_c &tr, const BaseEntity *baseSkip);
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
// g_ragdoll.cpp - wrapper for Physics Module ragdolls
// This is where Doom3 Articulated Figures support is.
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
void G_InitLua();
void G_ShutdownLua();
void G_LoadLuaScript(const char *filename);
void G_DumpLuaStack();

//
// g_ammo.cpp
//
void G_InitAmmoTypes();

//
// g_wolfScript.cpp
// RTCW/ET .script/.ai logic support.
void G_InitWolfScripts(const char *mapName);
void G_WolfScript_StartScript(const char *scriptName, const char *labelName);
void G_WolfScript_StartScript_Spawn(const char *scriptName);

//
// g_stateMachines.cpp
// FAKK/MoHAA/EF2/Alice .st files for player and AI control.
void G_InitStateMachines();
class stateMachineAPI_i *G_LoadStateMachine(const char *fname);

#endif // __G_LOCAL_H__
