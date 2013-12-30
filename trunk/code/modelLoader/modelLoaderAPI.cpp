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
#include "skelModelImpl.h"
#include "skelAnimImpl.h"
#include "hl2MDLReader.h"
#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/imgAPI.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>
#include <api/modelLoaderDLLAPI.h>
#include <api/staticModelCreatorAPI.h>
#include <api/materialSystemAPI.h>
#include <shared/autoCvar.h>

int		Q_stricmpn (const char *s1, const char *s2, int n);

bool MOD_LoadModelFromHeightmap(const char *fname, staticModelCreatorAPI_i *out);
class modelLoaderDLLIMPL_c : public modelLoaderDLLAPI_i {
public:
	virtual bool isStaticModelFile(const char *fname) {
		// that's a procedural model
		if(fname[0] == '_')
			return true;
		const char *ext = strchr(fname,'.');
		if(ext == 0) {
			return false;
		}
		ext++;
		// Wavefront .obj static triangle model
		if(!stricmp(ext,"obj"))
			return true;
		// raw .map file that will be converted to trimesh
		if(!stricmp(ext,"map"))
			return true;
		// .ASE model (this format is used in Doom3)
		if(!stricmp(ext,"ase"))
			return true;
		// we can load heightmaps here as well
		if(!stricmp(ext,"png") || !stricmp(ext,"jpg") || !stricmp(ext,"tga") || !stricmp(ext,"bmp"))
			return true;
		// Quake3 .md3 models (without animations; only first frame is loaded)
		if(!stricmp(ext,"md3"))
			return true;
		// LWO, used in Doom3 along with ASE
		if(!stricmp(ext,"lwo"))
			return true;
		// HL2 (Source Engine) .mdl (without animations)
		if(!stricmp(ext,"mdl"))
			return true;
		return false;
	}
	virtual bool loadStaticModelFile(const char *fileNameWithExtraCommands, class staticModelCreatorAPI_i *out)  {
		// extra post process commands (like model scaling, rotating, etc)
		// can be applied directly in model name string, after special '|' character, eg:
		// "models/props/truck.obj|scale10|texturespecialTexture"
		const char *inlinePostProcessCommandMarker = strchr(fileNameWithExtraCommands,'|');
		// get raw filename
		str fname = fileNameWithExtraCommands;
		if(inlinePostProcessCommandMarker) {
			fname.capLen(inlinePostProcessCommandMarker-fileNameWithExtraCommands);
		}
		// see if it's a build-in model shape
		if(fname[0] == '_') {
			if(!Q_stricmpn(fname+1,"quadZ",5)) {
				// it's a flat, Z-oriented quad
				// (in Q3 Z axis is up-down)
				float x,y;
				sscanf(fname+1+5,"%fx%f",&x,&y);
				simpleVert_s points[4];
				points[0].xyz.set(x*0.5f,y*0.5,0);
				points[0].tc.set(1,0);
				points[1].xyz.set(x*0.5f,-y*0.5,0);
				points[1].tc.set(1,1);
				points[2].xyz.set(-x*0.5f,-y*0.5,0);
				points[2].tc.set(0,1);
				points[3].xyz.set(-x*0.5f,y*0.5,0);
				points[3].tc.set(0,0);
				out->addTriangle("nomaterial",points[0],points[1],points[2]);
				out->addTriangle("nomaterial",points[2],points[3],points[0]);
				if(inlinePostProcessCommandMarker) {
					MOD_ApplyInlinePostProcess(inlinePostProcessCommandMarker,out);
				}
				return false; // no error
			}
		}
		const char *ext = strchr(fname,'.');
		if(ext == 0) {
			return true;
		}
		ext++; // skip '.'
		bool error;
		if(!stricmp(ext,"obj")) {
			error = MOD_LoadOBJ(fname,out);
		} else if(!stricmp(ext,"ase")) {
			error = MOD_LoadASE(fname,out);
		} else if(!stricmp(ext,"map")) {
			error = MOD_LoadConvertMapFileToStaticTriMesh(fname,out);
		} else if(!stricmp(ext,"png") || !stricmp(ext,"jpg") || !stricmp(ext,"tga") || !stricmp(ext,"bmp")) {
			error = MOD_LoadModelFromHeightmap(fname,out);
		} else if(!stricmp(ext,"md3")) {
			// load md3 as static model (first animation frame)
			error = MOD_LoadStaticMD3(fname,out);
		} else if(!stricmp(ext,"lwo")) {
			error = MOD_LoadLWO(fname,out);
		} else if(!stricmp(ext,"mdl")) {
			hl2MDLReader_c r;
			if(r.beginReading(fname) == false) {
				error = r.getStaticModelData(out);
			} else {
				error = true;
			}

		} else {
			error = true;
		}
		if(error == false) {
			// apply model postprocess steps (scaling, rotating, etc)
			// defined in optional .mdlpp file
			MOD_ApplyPostProcess(fname,out);
			if(inlinePostProcessCommandMarker) {
				MOD_ApplyInlinePostProcess(inlinePostProcessCommandMarker,out);
			}
			return false; // no error
		}
		return true;
	}
	virtual bool isKeyFramedModelFile(const char *fname) {
		const char *ext = strchr(fname,'.');
		if(ext == 0) {
			return false;
		}	
		ext++; // skip '.'
		if(!stricmp(ext,"md3")) {
			u32 numFrames = MOD_ReadMD3FileFrameCount(fname);
			if(numFrames > 1) {
				return true;
			}
		}
		if(!stricmp(ext,"md2")) {
			// TODO: read the number of md2 frame?
			return true;
		}
		if(!stricmp(ext,"mdc")) {
			// TODO: read the number of mdc frame?
			return true;
		}
		return false;
	}
	virtual class kfModelAPI_i *loadKeyFramedModelFile(const char *fname) {
		const char *ext = strchr(fname,'.');
		if(ext == 0) {
			return 0;
		}	
		//if(!stricmp(ext,"md3")) {
		//	return MOD_LoadAnimatedMD3(fname);
		//}
		return KF_LoadKeyFramedModelAPI(fname);
		//return 0;
	}
	virtual bool isSkelModelFile(const char *fname) {
		const char *ext = strchr(fname,'.');
		if(ext == 0) {
			return false;
		}
		ext++;
		if(!stricmp(ext,"md5mesh"))
			return true;
		return false;
	}
	virtual class skelModelAPI_i *loadSkelModelFile(const char *fname) {
		// check for empty file name
		if(fname == 0 || fname[0] == 0)
			return 0;
		skelModelIMPL_c *skelModel = new skelModelIMPL_c;
		if(skelModel->loadMD5Mesh(fname)) {
			delete skelModel;
			return 0;
		}		
		skelModel->recalcEdges();
		//if(skelModel) {
			// apply model postprocess steps (scaling, rotating, etc)
			// defined in optional .mdlpp file
			MOD_ApplyPostProcess(fname,skelModel);
		//}
		return skelModel;
	}
	virtual bool isSkelAnimFile(const char *fname) {
		const char *ext = strchr(fname,'.');
		if(ext == 0) {
			return false;
		}
		ext++;
		if(!stricmp(ext,"md5anim"))
			return true;
		return false;
	}
	virtual class skelAnimAPI_i *loadSkelAnimFile(const char *fname) {
		const char *ext = strchr(fname,'.');
		if(ext == 0) {
			return 0;
		}
		ext++; // skip '.'
		skelAnimAPI_i *ret;
		if(!stricmp(ext,"md5anim")) {
			skelAnimMD5_c *md5Anim = new skelAnimMD5_c;
			if(md5Anim->loadMD5Anim(fname)) {
				delete md5Anim;
				return 0;
			}
			ret = md5Anim;
		} else {
			return 0;
		}
		SK_ApplyAnimPostProcess(fname,ret);
		return ret;
	}
	// read the number of animation frames in .md3 file (1 for non-animated models, 0 if model file does not exist)
	virtual u32 readMD3FrameCount(const char *fname) {
		return MOD_ReadMD3FileFrameCount(fname);
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
materialSystemAPI_i *g_ms = 0;
imgAPI_i *g_img = 0;
rAPI_i *rf = 0;
rbAPI_i *rb = 0;
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
	g_iFaceMan->registerIFaceUser(&g_ms,MATERIALSYSTEM_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_img,IMG_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rf,RENDERER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rb,RENDERER_BACKEND_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_MODELLOADER;
}

