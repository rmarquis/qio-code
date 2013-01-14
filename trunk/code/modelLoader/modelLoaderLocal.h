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
// modelLoaderLocal.h - local model loader header
#ifndef __MODELLOADERLOCAL_H__
#define __MODELLOADERLOCAL_H__

#include <shared/typedefs.h>

class staticModelCreatorAPI_i;

// staticModelLoaders/wavefrontOBJModelLoader.cpp
bool MOD_LoadOBJ(const char *fname, staticModelCreatorAPI_i *out);
// staticModelLoaders/mapFileConverter.cpp
bool MOD_LoadConvertMapFileToStaticTriMesh(const char *fname, staticModelCreatorAPI_i *out);
// staticModelLoaders/aseLoader.cpp
bool MOD_LoadASE(const char *fname, staticModelCreatorAPI_i *out);
// staticModelLoaders/md3StaticLoader.cpp
// (loading of non-animated md3 models)
bool MOD_LoadStaticMD3(const char *fname, staticModelCreatorAPI_i *out);
u32 MOD_ReadMD3FileFrameCount(const char *fname);
// staticModelLoaders/lwoLoader.cpp
bool MOD_LoadLWO(const char *fname, class staticModelCreatorAPI_i *out);

// mod_postProcess.cpp
// .mdlpp commands parsing and execution
bool MOD_ApplyPostProcess(const char *modName, class modelPostProcessFuncs_i *inout);
// inline postprocess commands parsing and execution
bool MOD_ApplyInlinePostProcess(const char *cmdsText, class modelPostProcessFuncs_i *inout);
// skel_animPostProcess.cpp
bool SK_ApplyAnimPostProcess(const char *modName, class skelAnimPostProcessFuncs_i *inout);

// keyFramedModelIMPL.cpp
class kfModelImpl_c *KF_LoadKeyFramedModel(const char *fname);
class kfModelAPI_i *KF_LoadKeyFramedModelAPI(const char *fname);

#endif // __MODELLOADERLOCAL_H__
