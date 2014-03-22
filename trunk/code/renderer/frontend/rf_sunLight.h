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
// rf_sunLight.h
#ifndef __RF_SUNLIGHT_H__
#define __RF_SUNLIGHT_H__

class rSunLight_c {
	class rIndexedShadowVolume_c *mainVolume;

	arraySTD_c<class sunLightEntityInteraction_c*> entityInteractions;

	int findInteractionForEntity(class rEntityImpl_c *ent);
public:
	rSunLight_c();
	~rSunLight_c();
	void addSunLightShadowVolumes();
	void freeSunLightShadowVolumes();
	vec3_c getFakePointLightPosition();
	float getFakePointLightRadius();

	void addEntityInteraction(class rEntityImpl_c *ent);
	void removeEntityInteraction(class rEntityImpl_c *ent);
};

rSunLight_c *RF_GetSunLight();
void RF_ShutdownSunLight();
void RF_AddSunLightInteractionEntity(class rEntityImpl_c *ent);
void RF_RemoveSunLightInteractionEntity(class rEntityImpl_c *ent);

#endif // __RF_SUNLIGHT_H__
