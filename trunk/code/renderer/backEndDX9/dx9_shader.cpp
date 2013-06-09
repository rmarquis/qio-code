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
// dx9_shader.cpp - HLSL shaders loading

#include "dx9_local.h"
#include "dx9_shader.h"
#include <shared/autoCvar.h>


arraySTD_c<hlslShader_c*> dx9_shaders;
static hlslShader_c *DX9_FindShader(const char *baseName, const hlslPermutationFlags_s &p) {
	for(u32 i = 0; i < dx9_shaders.size(); i++) {
		hlslShader_c *s = dx9_shaders[i];
		if(!stricmp(baseName,s->getName())
			&& !memcmp(&s->getPermutations(),&p,sizeof(hlslPermutationFlags_s))) {
			return s;
		}
	}
	return 0;
}
static hlslPermutationFlags_s dx9_defaultPermutations;
static aCvar_c dx9_verboseRegisterShader("dx9_verboseRegisterShader",0);
hlslShader_c *DX9_RegisterShader(const char *baseName, const hlslPermutationFlags_s *p) {
	if(p == 0) {
		p = &dx9_defaultPermutations;
	}
	if(dx9_verboseRegisterShader.getInt()) {
		g_core->Print("DX9_RegisterShader: name %s, bTexGenEnvironment %i\n",baseName,p->hasTexGenEnvironment);
	}
	// see if the shader is already loaded
	hlslShader_c *ret = DX9_FindShader(baseName,*p);
	if(ret) {
		if(ret->isValid())
			return ret;
		return 0;
	}
	ret = new hlslShader_c;
	ret->name = baseName;
	ret->permutations = *p;
	dx9_shaders.push_back(ret);
	str fname = "hlsl/dx9/";
	fname.append(baseName);
	fname.append(".fx");
	char *fileData;
	int len = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return 0; // cannot open
	}
    ID3DXBuffer *pCompilationErrors = 0;
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_NO_PRESHADER;
	// build-in #defines + effect file text
	str finalEffectDef;
	if(p->hasTexGenEnvironment) {
		finalEffectDef.append("#define HAS_TEXGEN_ENVIROMENT\n");
	}
	if(p->hasLightmap) {
		finalEffectDef.append("#define HAS_LIGHTMAP\n");
	}
	if(p->onlyLightmap) {
		finalEffectDef.append("#define ONLY_LIGHTMAP\n");
	}
	if(p->hasVertexColors) {
		finalEffectDef.append("#define HAS_VERTEXCOLORS\n");
	}
	if(p->hasMaterialColor) {
		finalEffectDef.append("#define HAS_MATERIAL_COLOR\n");
	}

	finalEffectDef.append(fileData);
	HRESULT hr = D3DXCreateEffect(pDev,finalEffectDef.c_str(),finalEffectDef.length(),0,0,dwShaderFlags,0,&ret->effect,&pCompilationErrors);
	g_vfs->FS_FreeFile(fileData);
    if (FAILED(hr)) {
		g_core->RedWarning("D3DXCreateEffect: failed for %s\n",fname.c_str());
    }

	if (pCompilationErrors) {
		str errorStr = (const char*)pCompilationErrors->GetBufferPointer();
        pCompilationErrors->Release();
		g_core->RedWarning("DX9_RegisterShader: error: %s\n",errorStr.c_str());
	}
	if(ret->isValid() == false)
		return 0;
	return ret;
}
void DX9_ShutdownHLSLShaders() {
	for(u32 i = 0; i < dx9_shaders.size(); i++) {
		delete dx9_shaders[i];
	}
	dx9_shaders.clear();
}