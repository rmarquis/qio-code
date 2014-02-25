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
#include <shared/autoCmd.h>
#include <api/imgAPI.h>
#include <api/coreAPI.h>

void RF_TakeCubeMapScreenShot(const char *baseName) {
	vec3_c pos = rf_camera.getOrigin();

	vec3_c lookAngles[] = { 
		vec3_c(0,0,0), vec3_c(0,90,0),
		vec3_c(0,180,0), vec3_c(0,270,0),
		vec3_c(-90,0,0), vec3_c(90,0,0)};
	const char *sufixes [] = { "rt", "bk", "lf", "ft", "up", "dn" };
	for(u32 side = 0; side < 6; side++) {
		axis_c ax;
		ax.fromAngles(lookAngles[side]);
		
		projDef_s pd;
		pd.zFar = 10000.f;
		pd.zNear = 1.f;
		pd.fovX = pd.fovY = 90.f;
		rf_camera.setup(pos,ax,pd);
		rb->beginFrame();
		rb->setColor4(0);
		RF_Draw3DView();

		u32 w, h;
		byte *pic = rb->getScreenShotRGB(&w,&h);
		if(pic) {
			str imageName = baseName;
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
	RF_TakeCubeMapScreenShot(baseName);
}

static aCmd_c rf_cubeMapScreenShot("cubeMapScreenShot",RF_CubeMapScreenShot_f);
