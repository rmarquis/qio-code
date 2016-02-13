/*
============================================================================
Copyright (C) 2016 V.

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
// QioSpawner.cpp
#include "QioSpawner.h"
#include "../g_local.h"
#include <api/coreAPI.h>

DEFINE_CLASS(QioSpawner, "BaseEntity");
DEFINE_CLASS_ALIAS(QioSpawner, qio_spawner);

QioSpawner::QioSpawner() {
	bEnabled = true;
	interval = 1000;
	elapsed = 0;
	random = 0;
	spawnLimit = 10;
	spawnedCount = 0;
	intervalToWait = interval;
	modelToSpawn = "models/props/barrel_c/Barrel_C.obj";
}
void QioSpawner::runFrame() {
	if(spawnedCount >= spawnLimit)
		return;
	elapsed += level.frameTime*1000;
	if(elapsed > intervalToWait) {
		BaseEntity *be = G_SpawnGeneric(modelToSpawn);
		if(be) {
			be->setOrigin(this->getOrigin());
		}
		spawnedCount++;
		elapsed = 0;
		intervalToWait = interval;
		if(random) {
			intervalToWait += rand()%random;
			g_core->Print("Interval to wait: %i\n",intervalToWait);
		}
	}
}
void QioSpawner::setKeyValue(const char *key, const char *value) {
	if(!_stricmp(key,"interval")) {
		interval = atoi(value);
	} else if(!_stricmp(key,"bEnabled")) {
		bEnabled = atoi(value);
	} else if(!_stricmp(key,"random")) {
		random = atoi(value);
	} else {
		BaseEntity::setKeyValue(key,value);
	}
}
