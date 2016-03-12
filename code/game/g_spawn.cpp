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
	if(g_vfs->FS_FileExists(classOrModelName)) {
		str tmp = classOrModelName;
		if(tmp.hasExt("tik")) {
			tiki_i *tiki = g_tikiMgr->registerModel(classOrModelName);
			const char *className = tiki->getClassName();
			if(className && className[0]) {
				e = G_SpawnClass(className);
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
			ModelEntity *mEnt = (ModelEntity *)G_SpawnClassDef("ModelEntity");
			// make them immobile
			mEnt->setRigidBodyPhysicsEnabled(false);
			ent = mEnt;
		} else {
			g_core->Print("G_SpawnEntDef: Failed to spawn class %s\n",className);
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
	for(u32 i = 0; i < g_entDefs.size(); i++) {
		entDef_c *entDef = g_entDefs[i];
		G_SpawnEntDef(entDef);
	}
	// perform a final fixups on entities
	// (spawn constraints, etc)
	G_ProcessEntityEvents();
	// FIXME: do this other way
	for(u32 i = 0; i < level.num_entities; i++) {
		if(g_entities[i].ent) {
			g_entities[i].ent->postSpawn2();
		}
	}
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" done.\n");
		g_loadingScreen->addLoadingString("Current game entities count: %i.\n",level.num_entities);
	}
}

