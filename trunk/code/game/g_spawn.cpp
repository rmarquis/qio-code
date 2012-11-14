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
#include <shared/entDefsList.h>
#include <api/coreAPI.h>
#include <api/loadingScreenMgrAPI.h>
#include "classes/BaseEntity.h"
#include "classes/ModelEntity.h"

static entDefsList_c g_entDefs;

void G_LoadMapEntities(const char *mapName) {
	g_entDefs.clear();
	bool error = g_entDefs.load(mapName);
	if(error) {
		return;
	}
	return;
}
BaseEntity *G_SpawnEntDef(const class entDef_c *entDef) {
	const char *className = entDef->getClassName();
	if(className == 0 || className[0] == 0) {
		return 0;
	}
	BaseEntity *ent = (BaseEntity*)G_SpawnClass(className);
	if(ent == 0) {
		// hack to spawn inline models
		if(entDef->hasKey("model") && entDef->getKeyValue("model")[0] == '*') {
			ent = (BaseEntity*)G_SpawnClass("ModelEntity");
		} else {
			g_core->Print("G_SpawnEntDef: Failed to spawn class %s\n",className);
			return 0;
		}
	}
	g_core->Print("G_SpawnEntDef: Spawning %s\n",className);
	for(u32 j = 0; j < entDef->getNumKeyValues(); j++) {
		const char *key, *value;
		entDef->getKeyValue(j,&key,&value);
		ent->setKeyValue(key,value);
	}
	
#if 1
	{
		ModelEntity *m = dynamic_cast<ModelEntity*>(ent);
		if(m && m->hasCollisionModel()) {
			//m->initStaticBodyPhysics();
			m->initRigidBodyPhysics();
		}
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
	if(g_loadingScreen) { // update loading screen (if its present)
		g_loadingScreen->addLoadingString(" done.\n");
		g_loadingScreen->addLoadingString("Current game entities count: %i.\n",level.num_entities);
	}
}

