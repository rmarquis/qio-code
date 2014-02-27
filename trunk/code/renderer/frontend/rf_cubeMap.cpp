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
// rf_cubeMap.cpp
#include "rf_local.h"
#include "rf_world.h"
#include <shared/autoCmd.h>
#include <shared/entDefsList.h>
#include <api/imgAPI.h>
#include <api/coreAPI.h>
#include <api/materialSystemAPI.h>

class rCubeMap_c {
	vec3_c origin;
	int mapEntityNumber;
	class cubeMapAPI_i *texture;
public:
	rCubeMap_c(const vec3_c &newPos, int newMapEntityNumber, class cubeMapAPI_i *newTexture) {
		origin = newPos;
		mapEntityNumber = newMapEntityNumber;
		texture = newTexture;
	}
	const vec3_c &getOrigin() const {
		return origin;
	}
	cubeMapAPI_i *getCubeMapTexture() const {
		return texture;
	}
};

static arraySTD_c<rCubeMap_c*> r_staticCubeMaps;

void RF_ShutdownStaticCubeMaps() {
	for(u32 i = 0; i < r_staticCubeMaps.size(); i++) {
		delete r_staticCubeMaps[i];
	}
	r_staticCubeMaps.clear();
}

rCubeMap_c *RF_FindNearestEnvCubeMap(const vec3_c &p) {
	if(r_staticCubeMaps.size() == 0)
		return 0;
	float best = r_staticCubeMaps[0]->getOrigin().distSQ(p);
	u32 ret = 0;
	for(u32 i = 1; i < r_staticCubeMaps.size(); i++) {
		float next = r_staticCubeMaps[i]->getOrigin().distSQ(p);
		if(next < best) {
			best = next;
			ret = i;
		}
	}
	return r_staticCubeMaps[ret];
}
cubeMapAPI_i *RF_FindNearestEnvCubeMap_Image(const vec3_c &p) {
	rCubeMap_c *c = RF_FindNearestEnvCubeMap(p);
	if(c == 0)
		return 0;
	return c->getCubeMapTexture();
}

void RF_FormatStaticCubeMapName(const vec3_c &p, str &out) {
	out = RF_GetWorldMapName();
	out.stripExtension();
	out.append("/cubemaps/");
	char buff[64];
	sprintf(buff,"%i_%i_%i",int(p.x),int(p.y),int(p.z));
	out.append(buff);
}
bool RF_LoadWorldMapCubeMaps(const char *mapName) {
	RF_ShutdownStaticCubeMaps();

	entDefsList_c entities;
	if(entities.load(mapName)) {
		g_core->Print("RF_LoadMapCubeMaps: Failed to load map entities list.\n");
		return true;
	}
	g_core->Print("Loaded %i entdefs, looking for 'env_cubemap' instances...\n",entities.size());
	for(u32 i = 0; i < entities.size(); i++) {
		const entDef_c *e = entities[i];
		// skip non-cubemap entities
		if(!e->hasClassName("env_cubemap"))
			continue;
		vec3_c p;
		if(e->getKeyValue("origin",p)) {
			g_core->RedWarning("Entity %i is missing origin key\n",i);
			continue;
		}
		str cubeMapName;
		RF_FormatStaticCubeMapName(p,cubeMapName);
		cubeMapAPI_i *cubeMapTexture = g_ms->registerCubeMap(cubeMapName,true);
		if(cubeMapTexture == 0) {
			g_core->RedWarning("Entity %i is missing cubemap texture (%s)\n",i,cubeMapName.c_str());
			continue;
		}
		r_staticCubeMaps.push_back(new rCubeMap_c(p,i,cubeMapTexture));
	}
	g_core->Print("%i static cubemaps ('env_cubemap') loaded.\n",r_staticCubeMaps.size());
	return false;
}

void RF_TakeCubeMapScreenShot(const vec3_c &pos, const char *baseName) {
	vec3_c lookAngles[] = { 
		vec3_c(0,0,0), vec3_c(0,90,0),
		vec3_c(0,180,0), vec3_c(0,270,0),
		vec3_c(-90,0,0), vec3_c(90,0,0)};
	//const char *sufixes [] = { "rt", "bk", "lf", "ft", "up", "dn" };
	const char *sufixes [] = { "forward", "left", "back", "right", "up", "down" };
	for(u32 side = 0; side < 6; side++) {
		axis_c ax;
		ax.fromAngles(lookAngles[side]);
		
		projDef_s pd;
		pd.zFar = 10000.f;
		pd.zNear = 1.f;
		pd.fovX = pd.fovY = 90.f;
		rf_camera.setup(pos,ax,pd);
		rb->beginFrame();
		rb->setViewPort(512,512);
		rb->setColor4(0);
		RF_Draw3DView();

		u32 w, h;
		byte *pic = rb->getScreenShotRGB(&w,&h);
		if(pic) {
			str imageName = baseName;
			imageName.append("_");
			imageName.append(sufixes[side]);
			imageName.setExtension("tga");
			//g_img->writeImageRGB(imageName.c_str(),pic,w,h);
			g_img->writeTGA(imageName.c_str(),pic,w,h,3);
			rb->freeScreenShotData(pic);
		} else {
			g_core->RedWarning("RF_TakeCubeMapScreenShot: Couldn't get screen surface data\n");
		}
		rb->endFrame();
	}
}
void RF_CubeMapScreenShot_f() {
	str baseName = "screenshot_cubemaps/test_";
	vec3_c pos = rf_camera.getOrigin();
	RF_TakeCubeMapScreenShot(pos,baseName);
}

void RF_BuildCubeMaps_f() {
	if(!RF_IsAnyMapLoaded()) {
		g_core->Print("You must load world map first\n");
		return;
	}
	const char *name = RF_GetWorldMapName();
	entDefsList_c entities;
	if(entities.load(name)) {
		g_core->Print("Failed to load map entities list.\n");
		return;
	}
	g_core->Print("Loaded %i entdefs, looking for 'env_cubemap' instances...\n",entities.size());
	u32 c_envCubeMaps = 0;
	rb->setBSkipStaticEnvCubeMapStages(true);
	for(u32 i = 0; i < entities.size(); i++) {
		const entDef_c *e = entities[i];
		// skip non-cubemap entities
		if(!e->hasClassName("env_cubemap"))
			continue;
		vec3_c p;
		if(e->getKeyValue("origin",p)) {
			g_core->RedWarning("Entity %i is missing origin key\n",i);
			continue;
		}
		str cubeMapName;
		RF_FormatStaticCubeMapName(p,cubeMapName);
		RF_TakeCubeMapScreenShot(p,cubeMapName);
		c_envCubeMaps++;
	}
	rb->setBSkipStaticEnvCubeMapStages(false);
	g_core->Print("%i env_cubemaps build.\n",c_envCubeMaps);

	RF_LoadWorldMapCubeMaps(name);
}

static aCmd_c rf_cubeMapScreenShot("cubeMapScreenShot",RF_CubeMapScreenShot_f);
static aCmd_c rf_buildCubeMaps("buildCubeMaps",RF_BuildCubeMaps_f);
