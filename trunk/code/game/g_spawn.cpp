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
#include "classes/BaseEntity.h"

static entDefsList_c g_entDefs;

void G_LoadMapEntities(const char *mapName) {
	g_entDefs.clear();
	bool error = g_entDefs.load(mapName);
	if(error) {
		return;
	}
	return;
}
void G_SpawnMapEntities(const char *mapName) {
	G_LoadMapEntities(mapName);
	for(u32 i = 0; i < g_entDefs.size(); i++) {
		entDef_c *e = g_entDefs[i];
		const char *className = e->getClassName();
		if(className == 0 || className[0] == 0) {
			continue;
		}
		BaseEntity *ent = (BaseEntity*)G_SpawnClass(className);
		if(ent == 0) {
			// hack to spawn inline models
			if(e->hasKey("model") && e->getKeyValue("model")[0] == '*') {
				ent = (BaseEntity*)G_SpawnClass("ModelEntity");
			} else {
				g_core->Print("Failed to spawn class %s\n",className);
				continue;
			}
		}
		g_core->Print("Spawning %s\n",className);
		for(u32 j = 0; j < e->getNumKeyValues(); j++) {
			const char *key, *value;
			e->getKeyValue(j,&key,&value);
			ent->setKeyValue(key,value);
		}
	}
}

