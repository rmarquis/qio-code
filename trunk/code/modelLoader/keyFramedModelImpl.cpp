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
// keyFramedModelImpl.cpp - per vertex model animation (Quake3 md3 style)
#include "keyFramedModelImpl.h"
#include "../fileFormats/md3_file_format.h"
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include <api/materialSystemAPI.h>

const char *kfSurf_c::getSurfName() const {
	return name;
}
const char *kfSurf_c::getMatName() const {
	return matName;
}
//class mtrAPI_i *kfSurf_c::getMaterial() const {
//	return mat;
//}
const rIndexBuffer_c *kfSurf_c::getIBO() const {
	return &indices;
}
u32 kfSurf_c::getNumVertices() const {
	return texCoords.size();
}
void kfSurf_c::copyTexCoords(void *outTC, u32 outStride) const {
	byte *p = (byte*)outTC;
	u32 numTexCoords = texCoords.size();
	const vec2_c *tc = texCoords.getArray();
	for(u32 i = 0; i < numTexCoords; i++, tc++) {
		memcpy(p,tc,sizeof(vec2_c));

		p += outStride;
	}
}
void kfSurf_c::instanceSingleFrame(void *outXYZ, u32 outStride, u32 frameNum) const {
	const kfSurfFrame_c &f = this->xyzFrames[frameNum];
	byte *p = (byte*)outXYZ;
	u32 numVerts = texCoords.size();
	const kfVert_c *in = f.verts.getArray();
	for(u32 i = 0; i < numVerts; i++, in++) {
		memcpy(p,in->xyz,sizeof(vec3_c));
		p += outStride;
	}
}
void kfSurf_c::instance(void *outXYZ, u32 outStride, u32 from, u32 to, float lerp) const {
	if(lerp == 0.f) {
		instanceSingleFrame(outXYZ,outStride,from);
		return;
	} else if(lerp == 1.f) {
		instanceSingleFrame(outXYZ,outStride,to);
		return;
	}
	const kfSurfFrame_c &fromFrame = this->xyzFrames[from];	
	const kfSurfFrame_c &toFrame = this->xyzFrames[to];
	byte *p = (byte*)outXYZ;
	u32 numVerts = texCoords.size();
	const kfVert_c *fromVert = fromFrame.verts.getArray();
	const kfVert_c *toVert = toFrame.verts.getArray();
	for(u32 i = 0; i < numVerts; i++,toVert++, fromVert++) {
		((vec3_c*)p)->lerpResult(fromVert->xyz,toVert->xyz, lerp);
		p += outStride;
	}
}

bool kfModelImpl_c::load(const char *fname) {
	const char *ext = G_strgetExt(fname);
	if(!stricmp(ext,"md3")) {
		return loadMD3(fname);
	} else {
		g_core->RedWarning("kfModelImpl_c::load: %s has unknown extension\n",fname);
	}
	return true; // error
}
bool kfModelImpl_c::loadMD3(const char *fname) {
	byte *buf;
	u32 len = g_vfs->FS_ReadFile(fname,(void**)&buf);
	if(buf == 0)
		return true;
	bool res = loadMD3(buf,len,fname);
	g_vfs->FS_FreeFile(buf);
	return res;
}
bool kfModelImpl_c::loadMD3(const byte *buf, const u32 fileLen, const char *fname) {
	const md3Header_s *h = (const md3Header_s *) buf;
	if(h->ident != MD3_IDENT) {
		g_core->RedWarning("kfModelImpl_c::loadMD3: %s has bad ident %i, should be \"IDP3\"\n",fname,h->ident);
		return true; // error
	}
	if(h->version != MD3_VERSION) {
		g_core->RedWarning("kfModelImpl_c::loadMD3: %s has bad version %i, should be %i\n",fname,h->ident,MD3_VERSION);
		return true; // error
	}
	surfs.resize(h->numSurfaces);
	kfSurf_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		const md3Surface_s *is = h->pSurf(i);
		//if(is->ident != MD3_SURF_IDENT)
		sf->name = is->name;
		u32 numIndices = is->numTriangles*3;
		u16 *indices = sf->indices.initU16(numIndices);
		const u32 *firstIndex = is->getFirstIndex();
		for(u32 j = 0; j < numIndices; j++) {
			indices[j] = firstIndex[j];
		}
		sf->texCoords.resize(is->numVerts);
		vec2_c *tc = sf->texCoords.getArray();
		for(u32 j = 0; j < is->numVerts; j++, tc++) {
			const md3St_s *st = is->getSt(j);
			tc->x = st->st[0];
			tc->y = st->st[1];
		}
		sf->xyzFrames.resize(h->numFrames);
		kfSurfFrame_c *f = sf->xyzFrames.getArray();
		const md3XyzNormal_s *xyzNormal = is->getXYZNormal(0);
		for(u32 j = 0; j < h->numFrames; j++, f++) {
			f->verts.resize(is->numVerts);
			kfVert_c *v = f->verts.getArray();
			for(u32 k = 0; k < is->numVerts; k++, v++) {
				v->xyz = xyzNormal->getPos();
				xyzNormal++;
			}
		}
		for(u32 j = 0; j < is->numShaders; j++) {
			const md3Shader_s *shi = is->getShader(j);
			if(j == 0) {
				sf->matName = shi->name;
//				sf->mat = g_ms->registerMaterial(sf->matName);
			}
		}
	}
	frames.resize(h->numFrames);
	kfFrame_c *of = frames.getArray();
	for(u32 i = 0; i < h->numFrames; i++, of++) {
		const md3Frame_s *f = h->pFrame(i);
		of->name = f->name;
		of->bounds.fromTwoPoints(f->bounds[0],f->bounds[1]);
		of->localOrigin = f->localOrigin;
		of->radius = f->radius;
	}
	// load tags
	const md3Tag_s *tag = h->getTags();
	tagNames.resize(h->numTags);
	for(u32 j = 0; j < h->numTags; j++, tag++) {
		tagNames[j] = tag->name;
	}
	tag = h->getTags();
	tagFrames.resize(h->numFrames);
	for(u32 i = 0; i < h->numFrames; i++) {
		kfTagFrame_c &f = tagFrames[i];
		f.tags.resize(h->numTags);
		for(u32 j = 0; j < h->numTags; j++, tag++) {
			f.tags[j].axis = tag->axis;
			f.tags[j].pos = tag->origin;
		}
	}
	return false; // no error
}
kfModelImpl_c *KF_LoadKeyFramedModel(const char *fname) {
	if(g_vfs->FS_FileExists(fname)==false) {
		g_core->RedWarning("KF_LoadKeyFramedModel: file %s does not exist\n",fname);
		return 0;
	}
	kfModelImpl_c *ret = new kfModelImpl_c;
	if(ret->load(fname)) {
		delete ret;
		return 0;
	}
	return ret;
}
kfModelAPI_i *KF_LoadKeyFramedModelAPI(const char *fname) {
	return KF_LoadKeyFramedModel(fname);
}

bool KF_HasKeyFramedModelExt(const char *fname) {
	const char *ext = G_strgetExt(fname);
	if(ext == 0)
		return false;
	if(!stricmp(ext,"md3")) {
		return true;
	}
	return false;
}
