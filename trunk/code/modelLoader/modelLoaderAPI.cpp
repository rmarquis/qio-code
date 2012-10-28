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
// modelLoaderAPI.cpp - model loader interface

#include "modelLoaderLocal.h"
#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/modelLoaderDLLAPI.h>
#include <api/staticModelCreatorAPI.h>
#include <shared/autoCvar.h>

class modelLoaderDLLIMPL_c : public modelLoaderDLLAPI_i {
public:
	virtual bool isStaticModelFile(const char *fname) {
		return true;
	}
	virtual bool loadStaticModelFile(const char *fname, class staticModelCreatorAPI_i *out)  {
		const char *ext = strchr(fname,'.');
		if(ext == 0) {
			return true;
		}
		ext++; // skip '.'
		bool error;
		if(!stricmp(ext,"obj")) {
			error = MOD_LoadOBJ(fname,out);
		} else {
			error = true;
		}
		if(error == false) {
			// apply model postprocess steps (scaling, rotating, etc)
			// defined in optional .mdlpp file
			MOD_ApplyPostProcess(fname,out);
			return false; // no error
		}
		return true;
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
// exports
static modelLoaderDLLIMPL_c g_staticModelLoaderDLLAPI;
modelLoaderDLLAPI_i *g_modelLoader = &g_staticModelLoaderDLLAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)g_modelLoader,MODELLOADERDLL_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_MODELLOADER;
}

