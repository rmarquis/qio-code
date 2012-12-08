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


arraySTD_c<hlslShader_c*> dx9_shaders;
static hlslShader_c *DX9_FindShader(const char *baseName) {
	for(u32 i = 0; i < dx9_shaders.size(); i++) {
		hlslShader_c *s = dx9_shaders[i];
		if(!stricmp(baseName,s->getName())) {
			return s;
		}
	}
	return 0;
}
hlslShader_c *DX9_RegisterShader(const char *baseName) {
	// see if the shader is already loaded
	hlslShader_c *ret = DX9_FindShader(baseName);
	if(ret) {
		if(ret->isValid())
			return ret;
		return 0;
	}
	str fname = "hlsl/dx9/";
	fname.append(baseName);
	fname.append(".fx");
	char *fileData;
	int len = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return 0; // cannot open
	}
	ret = new hlslShader_c;
	ret->name = baseName;
	dx9_shaders.push_back(ret);
    ID3DXBuffer *pCompilationErrors = 0;
    DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_NO_PRESHADER;
	HRESULT hr = D3DXCreateEffect(pDev,fileData,len,0,0,dwShaderFlags,0,&ret->effect,&pCompilationErrors);
	g_vfs->FS_FreeFile(fileData);
    if (FAILED(hr)) {

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