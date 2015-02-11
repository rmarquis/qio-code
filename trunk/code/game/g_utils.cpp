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
// g_utils.c -- misc utility functions for game module

#include "g_local.h"
#include "g_classes.h"
#include "classes/BaseEntity.h"
#include "classes/Player.h"
#include <api/serverAPI.h>
#include <shared/infoString.h>

/*
=========================================================================

model / sound configstring indexes

=========================================================================
*/

/*
================
G_FindConfigstringIndex

================
*/
int G_FindConfigstringIndex( const char *name, int start, int max, bool create ) {
	int		i;
	char	s[MAX_STRING_CHARS];

	if ( !name || !name[0] ) {
		return 0;
	}

	for ( i=1 ; i<max ; i++ ) {
		g_server->GetConfigstring( start + i, s, sizeof( s ) );
		if ( !s[0] ) {
			break;
		}
		if ( !strcmp( s, name ) ) {
			return i;
		}
	}

	if ( !create ) {
		return 0;
	}

	if ( i == max ) {
		G_Error( "G_FindConfigstringIndex: overflow" );
	}

	g_server->SetConfigstring( start + i, name );

	return i;
}


int G_RenderModelIndex( const char *name ) {
	return G_FindConfigstringIndex (name, CS_MODELS, MAX_MODELS, true);
}
int G_CollisionModelIndex( const char *name ) {
	return G_FindConfigstringIndex (name, CS_COLLMODELS, MAX_MODELS, true);
}
int G_SoundIndex( const char *name ) {
	return G_FindConfigstringIndex (name, CS_SOUNDS, MAX_SOUNDS, true);
}
int G_AnimationIndex( const char *name ) {
	return G_FindConfigstringIndex (name, CS_ANIMATIONS, MAX_ANIMATIONS, true);
}
int G_RagdollDefIndex( const char *name ) {
	return G_FindConfigstringIndex (name, CS_RAGDOLLDEFSS, MAX_RAGDOLLDEFS, true);
}
int G_RenderSkinIndex( const char *name ) {
	return G_FindConfigstringIndex (name, CS_SKINS, MAX_SKINS, true);
}
int G_RenderMaterialIndex( const char *name ) {
	return G_FindConfigstringIndex (name, CS_MATERIALS, MAX_MATERIALS, true);
}

//=====================================================================

/*
=================
G_Spawn

Either finds a free entity, or allocates a new one.

  The slots from 0 to MAX_CLIENTS-1 are always reserved for clients, and will
never be used by anything else.

Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_s *G_Spawn( void ) {
	int			i, force;
	edict_s	*e;

	e = NULL;	// shut up warning
	i = 0;		// shut up warning
	for ( force = 0 ; force < 2 ; force++ ) {
		// if we go through all entities and can't find one to free,
		// override the normal minimum times before use
		e = &g_entities[MAX_CLIENTS];
		for ( i = MAX_CLIENTS ; i<level.num_entities ; i++, e++) {
			if ( e->s != 0 ) {
				continue;
			}

			// the first couple seconds of server time can involve a lot of
			// freeing and allocating, so relax the replacement policy
			if ( !force && e->freetime > level.startTime + 2000 && level.time - e->freetime < 1000 ) {
				continue;
			}

			// reuse this slot
			return e;
		}
		if ( i != MAX_GENTITIES ) {
			break;
		}
	}
	if ( i == ENTITYNUM_MAX_NORMAL ) {
		for (i = 0; i < MAX_GENTITIES; i++) {
	//		G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		G_Error( "G_Spawn: no free entities" );
	}
	
	// open up a new slot
	level.num_entities++;

	// let the server system know that there are more entities
	g_server->LocateGameData( level.gentities, level.num_entities );

	return e;
}

/*
=================
G_EntitiesFree
=================
*/
bool G_EntitiesFree( void ) {
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for ( u32 i = MAX_CLIENTS; i < level.num_entities; i++, e++) {
		if ( e->s != 0 ) {
			continue;
		}
		// slot available
		return true;
	}
	return false;
}

u32 G_GetEntitiesOfClass(const char *classNameOrig, arraySTD_c<BaseEntity*> &out) {
	const char *className = G_TranslateClassAlias(classNameOrig);
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < level.num_entities; i++, e++) {
		if (e->s == 0) {
			continue;
		}
		const char *entClass = e->ent->getClassName();
		if(!stricmp(entClass,className)) {
			out.push_back(e->ent);
		}
	}	
	return out.size();
}

BaseEntity *G_GetRandomEntityOfClass(const char *classNameOrig) {
	arraySTD_c<BaseEntity*> ents;
	G_GetEntitiesOfClass(classNameOrig,ents);
	if(ents.size() == 0)
		return 0;
	int i = rand() % ents.size();
	return ents[i];
}
class BaseEntity *G_FindFirstEntityWithTargetName(const char *targetName) {
	if(targetName == 0 || targetName[0] == 0) {
		return 0;
	}
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < level.num_entities; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		const char *beTargetName = be->getTargetName();
		if(!stricmp(beTargetName,targetName)) {
			return be;
		}
	}
	return 0;
}
void G_HideEntitiesWithTargetName(const char *targetName) {
	if(targetName == 0 || targetName[0] == 0) {
		return;
	}
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < level.num_entities; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		const char *beTargetName = be->getTargetName();
		if(!stricmp(beTargetName,targetName)) {
			be->hideEntity();
		}
	}
}
void G_ShowEntitiesWithTargetName(const char *targetName) {
	if(targetName == 0 || targetName[0] == 0) {
		return;
	}
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < level.num_entities; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		const char *beTargetName = be->getTargetName();
		if(!stricmp(beTargetName,targetName)) {
			be->showEntity();
		}
	}
}
void G_PostEvent(const char *targetName, int execTime, const char *eventName, const char *arg0, const char *arg1, const char *arg2, const char *arg3) {
	if(targetName == 0 || targetName[0] == 0) {
		return;
	}
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < level.num_entities; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		const char *beTargetName = be->getTargetName();
		if(!stricmp(beTargetName,targetName)) {
			be->postEvent(execTime,eventName,arg0,arg1,arg2,arg3);
		}
	}
}
u32 G_RemoveEntitiesOfClass(const char *className) {
	if(className == 0 || className[0] == 0) {
		return 0;
	}
	u32 c_removed = 0;
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < level.num_entities; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		if(be->hasClassName(className)) {
			delete be;
			c_removed++;
		}
	}
	return c_removed;
}

const vec3_c &G_GetPlayerOrigin(u32 playerNum) {
	edict_s *ed = &g_entities[playerNum];
	if(ed->s == 0 || ed->ent == 0) {
		static vec3_c dummy(0,0,0);
		return dummy;
	}
	return ed->ent->getOrigin();
}
class Player *G_GetPlayer(u32 playerNum) {
	if(playerNum >= MAX_CLIENTS)
		return 0;
	edict_s *ed = &g_entities[playerNum];
	if(ed->s == 0 || ed->ent == 0) {
		return 0;
	}
	Player *ret = dynamic_cast<Player*>(ed->ent);
	return ret;
}

//==============================================================================
