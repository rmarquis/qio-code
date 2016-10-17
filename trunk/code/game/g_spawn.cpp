/*
============================================================================
Copyright (C) 2012 V.

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
// g_spawn.cpp - game entities spawning
#include "g_local.h"
#include "g_classes.h"
#include "g_scriptedClasses.h"
#include <shared/entDefsList.h>
#include <api/coreAPI.h>
#include <api/loadingScreenMgrAPI.h>
#include <api/declManagerAPI.h>
#include <api/entityDeclAPI.h>
#include <api/vfsAPI.h>
#include <api/tikiAPI.h>
#include "classes/BaseEntity.h"
#include "classes/ModelEntity.h"
#include "classes/World.h"
#include <shared/autoCvar.h>

static aCvar_c g_debugSpawn("g_debugSpawn","0");
static aCvar_c g_printSpawnedClassNames("g_printSpawnedClassNames","0");

static entDefsList_c g_entDefs;


/*
=================
G_AllocEdict

Either finds a free entity, or allocates a new one.

  The slots from 0 to MAX_CLIENTS-1 are always reserved for clients, and will
never be used by anything else.

Try to avoid reusing an entity that was recently freed, because it
can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated
angles and bad trails.
=================
*/
edict_s *G_AllocEdict() {
	edict_s	*e = 0;
	u32 entNum = 0;
	for (u32 force = 0; force < 2; force++) {
		// if we go through all entities and can't find one to free,
		// override the normal minimum times before use
		e = &g_entities[MAX_CLIENTS];
		for ( entNum = MAX_CLIENTS; entNum < g_numEdicts ; entNum++, e++) {
			if ( e->s != 0 ) {
				continue;
			}

			// the first couple seconds of server time can involve a lot of
			// freeing and allocating, so relax the replacement policy
			if ( !force && e->freetime > g_startTime + 2000 && g_time - e->freetime < 1000 ) {
				continue;
			}

			// reuse this slot
			return e;
		}
		if ( entNum != MAX_GENTITIES ) {
			break;
		}
	}
	if ( entNum == ENTITYNUM_MAX_NORMAL ) {
		g_core->RedWarning( "G_AllocEdict: no free edicts\n" );
		return 0;
	}
	
	// open up a new slot
	g_numEdicts++;

	return e;
}

void G_LoadMapEntities(const char *mapName) {
	g_entDefs.clear();
	bool error = g_entDefs.load(mapName);
	if(error) {
		return;
	}
	return;
}
// classnName is name of hardcoded classDef or name of scripted class
// (from .def file)
BaseEntity *G_SpawnClass(const char *className) {
	BaseEntity *ent = (BaseEntity*)G_SpawnClassDef(className);
	if(ent) {
		return ent;
	}
	const scriptedClass_c *scriptedClassDef = G_FindScriptedClassDef(className);
	if(scriptedClassDef) {
		ent = G_SpawnClass(scriptedClassDef->getBaseClassName());
		if(ent == 0) {
			g_core->RedWarning("G_SpawnClass: failed to spawn base class %s of scripted class %s\n",
				scriptedClassDef->getBaseClassName(),className);
			return 0;
		}
		const ePairList_c &epairs = scriptedClassDef->getKeyValues();
		for(u32 i = 0; i < epairs.size(); i++) {		
			const char *key, *value;
			epairs.getKeyValue(i,&key,&value);
			if(g_debugSpawn.getInt()) {
				g_core->Print("G_SpawnClass: kv %i of %i: %s %s\n",i,epairs.size(),key,value);
			}
			ent->setKeyValue(key,value);
		}
		return ent;
	}
	ent = G_SpawnEntityFromEntDecl(className);
	return ent;
}
BaseEntity *G_SpawnGeneric(const char *classOrModelName) {
	BaseEntity *e = G_SpawnClass(classOrModelName);
	if(e)
		return e;
	// try to fix model path
	// this is for .tik models (FAKK/MoHAA)
	str fixed;
	if(g_vfs->FS_FileExists(classOrModelName) == false) {
		fixed = "models/";
		fixed.append(classOrModelName);
		if(g_vfs->FS_FileExists(fixed)) {
			classOrModelName = fixed;
		}
	}
	if(g_vfs->FS_FileExists(classOrModelName)) {
		str tmp = classOrModelName;
		tiki_i *tiki;
		if(tmp.hasExt("tik") && (tiki = g_tikiMgr->registerModel(classOrModelName))) {
			const char *className = tiki->getClassName();
			if(className && className[0]) {
				e = G_SpawnClass(className);
				if(e == 0) {
					g_core->RedWarning("G_SpawnGeneric: TIKI %s has unknown classname %s, using ModelEntty\n",
						classOrModelName,className);
					e = G_SpawnClass("ModelEntity");
				}
			} else {
				e = G_SpawnClass("ModelEntity");
			}
			e->setRenderModel(classOrModelName);
		} else {
			e = G_SpawnClass("ModelEntity");
			e->setRenderModel(classOrModelName);
			tmp.setExtension("map");
			if(g_vfs->FS_FileExists(tmp)) {
				e->setColModel(tmp);
			}
		}
		e->postSpawn();
	}
	return e;
}
BaseEntity *G_SpawnEntDef(const class entDefAPI_i *entDef) {
	const char *className = entDef->getClassName();
	if(className == 0 || className[0] == 0) {
		g_core->Print("G_SpawnEntDef: No classname set\n");
		return 0;
	}
	if(!_stricmp(className,"worldspawn") || !_stricmp(className,"world")) {
		for(u32 j = 0; j < entDef->getNumKeyValues(); j++) {
			const char *key, *value;
			entDef->getKeyValue(j,&key,&value);
			g_world.setKeyValue(key,value);
		}
		return 0;
	}
	BaseEntity *ent = G_SpawnClass(className);
	if(ent == 0) {
		// hack to spawn inline models
#if 0
		if(0 && entDef->hasKey("model") && entDef->getKeyValue("model")[0] == '*') {
#else
		if(entDef->hasKey("model")) {
#endif
			ModelEntity *mEnt;
			if(entDef->keyValueHasExtension("model","tik")) {
				const char *modelName = entDef->getKeyValue("model");
				str tmp;
				if(!g_vfs->FS_FileExists(modelName)) {
					// try to add "models/" to the beginning of the path
					tmp = "models/";
					tmp.append(modelName);
					if(g_vfs->FS_FileExists(tmp)) {
						modelName = tmp;
					}
				}
				tiki_i *tiki = g_tikiMgr->registerModel(modelName);
				if(tiki == 0) {
					mEnt = (ModelEntity *)G_SpawnClassDef("ModelEntity");
				} else {
					mEnt = (ModelEntity *)G_SpawnClassDef(tiki->getClassName());
					if(mEnt == 0) {
						g_core->RedWarning("G_SpawnEntDef: TIKI %s has unknown classname %s, using ModelEntty\n",
							tiki->getName(),tiki->getClassName());
						mEnt = (ModelEntity *)G_SpawnClassDef("ModelEntity");
					}
				}
			} else {
				mEnt = (ModelEntity *)G_SpawnClassDef("ModelEntity");
			}
			// make them immobile
			mEnt->setRigidBodyPhysicsEnabled(false);
			ent = mEnt;
		} else {
			const char *originStr = entDef->getKeyValue("origin");
			if(originStr == 0)
				originStr = "";
			const char *targetNameStr = entDef->getKeyValue("targetName");
			if(targetNameStr == 0)
				targetNameStr = "";
			g_core->Print("G_SpawnEntDef: Failed to spawn class %s (origin='%s',targetname='%s')\n",
				className,originStr,targetNameStr);
			return 0;
		}
	}
	if(g_printSpawnedClassNames.getInt()) {
		g_core->Print("G_SpawnEntDef: Spawning %s\n",className);
	}
	for(u32 j = 0; j < entDef->getNumKeyValues(); j++) {
		const char *key, *value;
		entDef->getKeyValue(j,&key,&value);
		if(g_debugSpawn.getInt()) {
			g_core->Print("G_SpawnEntDef: kv %i of %i: %s %s\n",j,entDef->getNumKeyValues(),key,value);
		}
		ent->setKeyValue(key,value);
	}
	
#if 1
	{
		//ModelEntity *m = dynamic_cast<ModelEntity*>(ent);
		//if(m/* && m->hasCollisionModel()*/) {
			// this will call "initRigidBodyPhysics()" for ModelEntities,
			// and "initVehiclePhysics" for VehicleCars
			ent->postSpawn();
		//}
	}
#endif
	return ent;
}
BaseEntity *G_SpawnFirstEntDefFromFile(const char *fileName) {
	entDef_c entDef;
	if(entDef.readFirstEntDefFromFile(fileName)) {
		g_core->RedWarning("G_SpawnFirstEntDefFromFile: failed to read entDef from %s\n",fileName);
		return 0;
	}
	BaseEntity *ret = G_SpawnEntDef(&entDef);
	return ret;
}
BaseEntity *G_SpawnEntityFromEntDecl(const char *declName) {
	if(g_declMgr == 0)
		return 0; // decl system was not present
	entityDeclAPI_i *decl = g_declMgr->registerEntityDecl(declName);
	if(decl == 0)
		return 0;
	BaseEntity *ret = G_SpawnEntDef(decl->getEntDefAPI());
	return ret;
}
void G_SpawnMapEntities(const char *mapName) {
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString("G_SpawnMapEntities: \"%s\" ",mapName);
	}
	G_LoadMapEntities(mapName);
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString("- %i entdefs.\nSpawning...",g_entDefs.size());
	}
	bool bIsWorldBSP = g_server->isWorldTypeBSP();
	for(u32 i = 0; i < g_entDefs.size(); i++) {
		entDef_c *entDef = g_entDefs[i];
		// misc_model entities are tesselated into bsp surfaces during map compilation,
		// but bsp files still contain misc_model entries,
		// so don't spawn misc_models, but only if worldtype is BSP
		// (we also support direct .map loading)
		if(bIsWorldBSP) {
			if(entDef->hasClassName("misc_model")) {
				continue;
			}
		}
		G_SpawnEntDef(entDef);
	}
	// perform a final fixups on entities
	// (spawn constraints, etc)
	G_ProcessEntityEvents();
	// FIXME: do this other way
	for(u32 i = 0; i < g_numEdicts; i++) {
		if(g_entities[i].ent) {
			g_entities[i].ent->postSpawn2();
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" done.\n");
		g_loadingScreen->addLoadingString("Current game entities count: %i.\n",g_numEdicts);
	}
}

