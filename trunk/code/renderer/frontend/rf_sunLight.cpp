/*
============================================================================
Copyright (C) 2014 V.

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
// rf_sunLight.cpp
#include "rf_local.h"
#include "rf_sunLight.h"
#include "rf_world.h"
#include "rf_shadowVolume.h"
#include "rf_surface.h"
#include "rf_drawCall.h"
#include "rf_entities.h"
#include <shared/autoCvar.h>

static aCvar_c rf_usePointLightForSun("rf_usePointLightForSun","0");
static aCvar_c rf_sunLight_printUnchangedInteractions("rf_sunLight_printUnchangedInteractions","0");

static rSunLight_c *rf_sunLight = 0;

class sunLightEntityInteraction_c {
	class rEntityImpl_c *ent;
	class rIndexedShadowVolume_c *shadowVolume;
	u32 absSilChangeCount;
public:
	sunLightEntityInteraction_c(class rEntityImpl_c *newEnt) {
		ent = newEnt;
		shadowVolume = 0;
		absSilChangeCount = 0;
	}
	~sunLightEntityInteraction_c() {
		delete shadowVolume;
	}
	void updateSunLightInteraction() {
		if(shadowVolume == 0) {
			shadowVolume = new rIndexedShadowVolume_c;
		} else {
			if(ent->isAnimated() == false) {
				if(absSilChangeCount == ent->getSilChangeCount()) {
					if(rf_sunLight_printUnchangedInteractions.getInt()) {
						g_core->Print("sunLightEntityInteraction_c::updateSunLightInteraction: %s didnt change\n",
							ent->getModelName());
					}
					addDrawCall();
					return;
				}
			}
		}
		ent->updateAnimatedEntity();
		absSilChangeCount = ent->getSilChangeCount();
		if(rf_usePointLightForSun.getInt()) {
			vec3_c pos = rf_sunLight->getFakePointLightPosition();
			ent->getMatrix().getInversed().transformPoint(pos);
			shadowVolume->createShadowVolumeForEntity(ent,pos,rf_sunLight->getFakePointLightRadius());
		} else {
			vec3_c dir = RF_GetSunDirection();
			ent->getMatrix().getInversed().transformNormal(dir);
			shadowVolume->createDirectionalShadowVolumeForEntity(ent,dir,5000);
		}
		addDrawCall();
	}
	void addDrawCall() {
		rf_currentEntity = ent;
		shadowVolume->addDrawCall();
		rf_currentEntity = 0;
	}
	void freeShadowVolume() {
		delete shadowVolume;
		shadowVolume = 0;
	}
	class rEntityImpl_c *getEntity() {
		return ent;
	}
};

rSunLight_c::rSunLight_c() {
	mainVolume = 0;
}
rSunLight_c::~rSunLight_c() {
	if(mainVolume) {
		delete mainVolume;
	}
	for(u32 i = 0; i < entityInteractions.size(); i++) {
		delete entityInteractions[i];
	}
	entityInteractions.clear();
}
void rSunLight_c::addEntityInteraction(class rEntityImpl_c *ent) {
	sunLightEntityInteraction_c *i = new sunLightEntityInteraction_c(ent);
	entityInteractions.push_back(i);
}
int rSunLight_c::findInteractionForEntity(class rEntityImpl_c *ent) {
	for(u32 i = 0; i < entityInteractions.size(); i++) {
		if(entityInteractions[i]->getEntity() == ent)
			return i;
	}
	return -1;
}
void rSunLight_c::removeEntityInteraction(class rEntityImpl_c *ent) {
	int index = findInteractionForEntity(ent);
	if(index < 0)
		return;
	delete entityInteractions[index];
	entityInteractions.erase(index);
}
vec3_c rSunLight_c::getFakePointLightPosition() {
	vec3_c sunDir = RF_GetSunDirection();
	return sunDir*10000.f;
}
float rSunLight_c::getFakePointLightRadius() {
	return 50000.f;
}
void rSunLight_c::addSunLightShadowVolumes() {
	if(mainVolume == 0) {
		mainVolume = new rIndexedShadowVolume_c;
		const r_model_c *m = RF_GetWorldModel();
		if(m) {
			for(u32 i = 0; i < m->getNumSurfs(); i++) {
				if(m->getSurf(i)->findSunMaterial())
					continue;
				if(rf_usePointLightForSun.getInt()) {
					mainVolume->addRSurface(m->getSurf(i),getFakePointLightPosition(),0,getFakePointLightRadius());
				} else {
					mainVolume->addDirectionalRSurface(m->getSurf(i),RF_GetSunDirection(),5000);
				}
			}
		}
	}
	for(u32 i = 0; i < entityInteractions.size(); i++) {
		entityInteractions[i]->updateSunLightInteraction();
	}
	mainVolume->addDrawCall();
}
void rSunLight_c::freeSunLightShadowVolumes() {
	if(mainVolume) {
		delete mainVolume;
		mainVolume = 0;
	}
	for(u32 i = 0; i < entityInteractions.size(); i++) {
		entityInteractions[i]->freeShadowVolume();
	}
}


rSunLight_c *RF_GetSunLight() {
	if(rf_sunLight == 0) {
		rf_sunLight = new rSunLight_c;
		RFE_IterateEntities(RF_AddSunLightInteractionEntity);
	}
	return rf_sunLight;
}

void RF_ShutdownSunLight() {
	if(rf_sunLight) {
		delete rf_sunLight;
		rf_sunLight = 0;
	}
}
void RF_AddSunLightInteractionEntity(class rEntityImpl_c *ent) {
	if(rf_sunLight) {
		rf_sunLight->addEntityInteraction(ent);
	}
}
void RF_RemoveSunLightInteractionEntity(class rEntityImpl_c *ent) {
	if(rf_sunLight) {
		rf_sunLight->removeEntityInteraction(ent);
	}
}