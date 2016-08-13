/*
===========================================================================
Copyright (C) 2012-2015 V.

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
// g_ents.cpp
#include "g_local.h"
#include "g_classes.h"
#include "classes/BaseEntity.h"
#include "classes/Player.h"
#include "classes/Mover.h"
#include <api/coreAPI.h>


u32 G_GetEntitiesWithTargetName(const char *targetName, arraySTD_c<class BaseEntity*> &out) {
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < g_numEdicts; i++, e++) {
		if (e->s == 0) {
			continue;
		}
		const char *entTN = e->ent->getTargetName();
		if(!_stricmp(entTN,targetName)) {
			out.push_back(e->ent);
		}
	}	
	return out.size();
}
u32 G_GetEntitiesWithScriptName(const char *scriptName, arraySTD_c<class BaseEntity*> &out) {
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < g_numEdicts; i++, e++) {
		if (e->s == 0) {
			continue;
		}
		const char *entSN = e->ent->getScriptName();
		if(!_stricmp(entSN,scriptName)) {
			out.push_back(e->ent);
		}
	}	
	return out.size();
}
u32 G_GetEntitiesOfClass(const char *classNameOrig, arraySTD_c<BaseEntity*> &out) {
	const char *className = G_TranslateClassAlias(classNameOrig);
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < g_numEdicts; i++, e++) {
		if (e->s == 0) {
			continue;
		}
		const char *entClass = e->ent->getClassName();
		if(!_stricmp(entClass,className)) {
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
	for(u32 i = MAX_CLIENTS; i < g_numEdicts; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		const char *beTargetName = be->getTargetName();
		if(!_stricmp(beTargetName,targetName)) {
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
	for(u32 i = MAX_CLIENTS; i < g_numEdicts; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		const char *beTargetName = be->getTargetName();
		if(!_stricmp(beTargetName,targetName)) {
			be->hideEntity();
		}
	}
}
void G_ShowEntitiesWithTargetName(const char *targetName) {
	if(targetName == 0 || targetName[0] == 0) {
		return;
	}
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < g_numEdicts; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		const char *beTargetName = be->getTargetName();
		if(!_stricmp(beTargetName,targetName)) {
			be->showEntity();
		}
	}
}
void G_PostEvent(const char *targetName, int execTime, const char *eventName, const char *arg0, const char *arg1, const char *arg2, const char *arg3) {
	if(targetName == 0 || targetName[0] == 0) {
		return;
	}
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < g_numEdicts; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		const char *beTargetName = be->getTargetName();
		if(!_stricmp(beTargetName,targetName)) {
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
	for(u32 i = MAX_CLIENTS; i < g_numEdicts; i++, e++) {
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

u32 G_FindMoversWithTeam(class arraySTD_c<class Mover*> &out, const char *team) {
	edict_s	*e = &g_entities[MAX_CLIENTS];
	for(u32 i = MAX_CLIENTS; i < g_numEdicts; i++, e++) {
		BaseEntity *be = e->ent;
		if(be == 0)
			continue;
		if(be->isMover()) {
			if(!stricmp(be->getMoverTeam(),team)) {
				out.push_back(dynamic_cast<Mover*>(be));
			}
		}
	}
	return out.size();
}
