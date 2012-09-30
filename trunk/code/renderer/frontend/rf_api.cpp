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
// rf_api.cpp - renderer DLL entry point

//#include "rf_local.h"
#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>

#include "rf_2d.h"

class rAPIImpl_c : public rAPI_i {
public:
	// functions called every frame
	virtual void beginFrame() {

	}
	virtual void setup3DViewer(const class vec3_c &camPos, const vec3_c &camAngles) {

	}
	//virtual void registerRenderableForCurrentFrame(class iRenderable_c *r) = 0;
	virtual void draw3DView() {

	}
	virtual void setup2DView() {

	}
	virtual void set2DColor(const float *rgba) {
		r_2dCmds.addSetColorCmd(rgba);
	}
	virtual void drawStretchPic(float x, float y, float w, float h,
		float s1, float t1, float s2, float t2, class mtrAPI_i *material) {
		r_2dCmds.addDrawStretchPic(x, y, w, h, s1, t1, s2, t2, material);
	}
	virtual void endFrame() {

	}
	// misc functions
	virtual void loadWorldMap(const char *mapName)  {
		
	}
	virtual class mtrAPI_i *registerMaterial(const char *matName) {
		return 0;
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
// exports
static rAPIImpl_c g_staticRFAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface(&g_staticRFAPI,RENDERER_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_RENDERER;
}

