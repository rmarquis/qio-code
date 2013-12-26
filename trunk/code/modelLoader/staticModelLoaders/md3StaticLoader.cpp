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
// md3StaticLoader.cpp - load first frame of Quake3 .md3 file
// (so animations are not support here; .md3 is handled
// as completly static, non-animated mesh)

#include <api/coreAPI.h>
#include <api/staticModelCreatorAPI.h>
#include <api/vfsAPI.h>
#include <fileFormats/md3_file_format.h>

bool MOD_LoadStaticMD3(const char *fname, staticModelCreatorAPI_i *out) {
	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open file
	}
	const md3Header_s *h = 0;
	h = (const md3Header_s*)fileData;
	if(h->ident != MD3_IDENT) {
		g_core->RedWarning("MOD_LoadStaticMD3: %s has bad ident\n",fname);
		// free loaded file data
		g_vfs->FS_FreeFile(fileData);
		return true;
	}
	if(h->version != MD3_VERSION) {
		g_core->RedWarning("MOD_LoadStaticMD3: %s has bad version %i (should be %i)\n",fname,h->version,MD3_VERSION);
		// free loaded file data
		g_vfs->FS_FreeFile(fileData);
		return true;
	}
	u32 loadFrameNum = 0;
	// see if we need to keep surfaces data
	if(out->hasPerSurfaceFunctionsImplemented()) {
		// separate surfaces are necessary for renderer,
		// but they might be not needed for collision detection code
		out->setNumSurfs(h->numSurfaces);
		for(u32 i = 0; i < h->numSurfaces; i++) {
			const md3Surface_s *sf = h->pSurf(i);
			if(sf->ident != MD3_IDENT) {
				g_core->RedWarning("MOD_LoadStaticMD3: %s has bad surface %i ident %i (should be %i)\n",fname,i,sf->ident,MD3_IDENT);
				// free loaded file data
				g_vfs->FS_FreeFile(fileData);
				return true;
			}
			out->resizeSurfaceVerts(i,sf->numVerts);
			const md3Shader_s *sh = sf->getShader(0);
			for(u32 j = 0; j < sf->numVerts; j++) {
				const md3XyzNormal_s *xyz = sf->getXYZNormal(j);
				const md3St_s *st = sf->getSt(j);
				out->setSurfaceVert(i,j,xyz->getPos(),st->st);
			}
			out->setSurfaceIndicesU32(i,sf->numTriangles*3,sf->getFirstIndex());
			out->setSurfaceMaterial(i,sh->name);
		}
	} else {
		for(u32 i = 0; i < h->numSurfaces; i++) {
			const md3Surface_s *sf = h->pSurf(i);
			if(sf->ident != MD3_IDENT) {
				g_core->RedWarning("MOD_LoadStaticMD3: %s has bad surface %i ident %i (should be %i)\n",fname,i,sf->ident,MD3_IDENT);
				// free loaded file data
				g_vfs->FS_FreeFile(fileData);
				return true;
			}
			out->resizeSurfaceVerts(i,sf->numVerts);
			const md3Shader_s *sh = sf->getShader(0);
			const md3XyzNormal_s *xyz = sf->getXYZNormal(0);
			const md3St_s *st = sf->getSt(0);
			const u32 *indices = sf->getFirstIndex();
			for(u32 j = 0; j < sf->numTriangles; j++) {
				u32 i0 = indices[j*3+0];
				u32 i1 = indices[j*3+1];
				u32 i2 = indices[j*3+2];
				simpleVert_s v0;
				v0.xyz = xyz[i0].getPos();
				v0.tc = st[i0].st;
				simpleVert_s v1;
				v1.xyz = xyz[i1].getPos();
				v1.tc = st[i1].st;
				simpleVert_s v2;
				v2.xyz = xyz[i2].getPos();
				v2.tc = st[i2].st;
				out->addTriangle(sh->name,v0,v1,v2);
			}
			out->setSurfaceMaterial(i,sh->name);
		}
	}
	out->recalcBoundingBoxes();
	// free loaded file data
	g_vfs->FS_FreeFile(fileData);
	return false; // no error
}

u32 MOD_ReadMD3FileFrameCount(const char *fname) {
	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return 0; // cannot open file
	}
	const md3Header_s *h = 0;
	h = (const md3Header_s*)fileData;

	u32 numFrames = h->numFrames;

	// free loaded file data
	g_vfs->FS_FreeFile(fileData);
	return numFrames;
}