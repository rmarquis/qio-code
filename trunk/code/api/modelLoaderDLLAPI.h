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
// modelLoaderDLLAPI.h - interface of model loading module
#ifndef __MODELLOADERDLLAPI_H__
#define __MODELLOADERDLLAPI_H__

#include <shared/typedefs.h>

#define MODELLOADERDLL_API_IDENTSTR "ModelLoaderDLLAPI0001"

class modelLoaderDLLAPI_i {
public:
	// for console commands and cvars
	virtual void initModelLoader() = 0;
	virtual void shutdownModelLoader() = 0;

	/// static (non animated) models.
	virtual bool isStaticModelFile(const char *fname) = 0;
	virtual bool loadStaticModelFile(const char *fname, class staticModelCreatorAPI_i *out) = 0;
	/// models with per-vertex animation (Quake3 .md3, etc..)
	virtual bool isKeyFramedModelFile(const char *fname) = 0;
	virtual class kfModelAPI_i *loadKeyFramedModelFile(const char *fname) = 0;
	/// skeletal animated models
	virtual bool isSkelModelFile(const char *fname) = 0;
	virtual class skelModelAPI_i *loadSkelModelFile(const char *fname) = 0;
	/// skeletal animations
	virtual bool isSkelAnimFile(const char *fname) = 0;
	virtual class skelAnimAPI_i *loadSkelAnimFile(const char *fname) = 0;
	/// helpers
	// read the number of animation frames in .md3 file (1 for non-animated models, 0 if model file does not exist)
	virtual u32 readMD3FrameCount(const char *fname) = 0;
	// development tools
	virtual bool convertToMD5Anim(const char *fname, const char *out = 0) = 0;
	virtual bool convertToMD5Mesh(const char *fname, const char *out = 0) = 0;
};

extern class modelLoaderDLLAPI_i *g_modelLoader;

#endif // __MODELLOADERDLLAPI_H__
