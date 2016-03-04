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
#include "../fileFormats/md2_file_format.h"
#include "../fileFormats/mdc_file_format.h"
#include "../fileFormats/tan_file_format.h"
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
u32 kfSurf_c::getNumTriangles() const {
	return indices.getNumTriangles();
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
		memcpy(p+12,in->normal,sizeof(vec3_c));
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
	if(!_stricmp(ext,"md3")) {
		return loadMD3(fname);
	} else if(!_stricmp(ext,"mdc")) {
		return loadMDC(fname);
	} else if(!_stricmp(ext,"md2")) {
		return loadMD2(fname);
	} else if(!_stricmp(ext,"tan")) {
		return loadTAN(fname);
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
				// avoid using image directly - always prefer material
				sf->matName.stripExtension();
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
bool kfModelImpl_c::loadMD2(const char *fname) {
	byte *buf;
	u32 len = g_vfs->FS_ReadFile(fname,(void**)&buf);
	if(buf == 0)
		return true;
	bool res = loadMD2(buf,len,fname);
	g_vfs->FS_FreeFile(buf);
	return res;
}
bool kfModelImpl_c::loadMD2(const byte *buf, const u32 fileLen, const char *fname) {
	const md2Header_s *h = (const md2Header_s *) buf;
	if(h->ident != MD2_IDENT) {
		g_core->RedWarning("kfModel_c::loadMD2: %s has bad ident %i, should be \"IDP2\"\n",fname,h->ident);
		return true; // error
	}
	if(h->version != MD2_VERSION) {
		g_core->RedWarning("kfModel_c::loadMD2: %s has bad version %i, should be 8\n",fname,h->version);
		return true; // error
	}
	kfSurf_c &sf = surfs.pushBack();
	if(h->numSkins) {
		const md2Skin_s *s = h->pSkin(0);
		sf.matName = s->name;
	} else {
		sf.matName = fname;
		sf.matName.stripExtension();
	}
	// decode texcoords
	sf.texCoords.resize(h->numTris*3);
	for(u32 j = 0; j < h->numTris; j++) {
		const md2Triangle_s *t = h->pTri(j);
		for(u32 k = 0; k < 3; k++) {
			const md2TexCoord_s *tc = h->pTC(t->st[k]);
			vec2_c v(tc->s,tc->t);
			v.x /= h->skinWidth;
			v.y /= h->skinHeight;
			sf.texCoords[j*3+k] = v;
		}
		sf.indices.addIndex(j*3+0);
		sf.indices.addIndex(j*3+1);
		sf.indices.addIndex(j*3+2);
	}
	// decode frames and vertices
	this->frames.resize(h->numFrames);
	sf.xyzFrames.resize(h->numFrames);
	for(u32 i = 0; i < h->numFrames; i++) {
		kfFrame_c &of = frames[i];
		const md2Frame_s *f = h->pFrame(i);
		//of.scale = f->scale;
		//of.offset = f->translate;
		vec3_c frameScale = f->scale;
		vec3_c frameTranslate = f->translate;
		of.name = f->name;

		//for(u32 j = 0; j < h->numVertices; j++) {
		//	const md2Vertex_s *v = f->pVertex(j);
		//	printf("Vert %i of %i - normalIndex %i\n",i,h->numVertices,v->normalIndex);
		//	vec3_c p(v->v[0],v->v[1],v->v[2]);
		//	

		//}
		kfSurfFrame_c &xyzFrame = sf.xyzFrames[i];
		xyzFrame.verts.resize(h->numTris*3);
		kfVert_c *ov = xyzFrame.verts.getArray();
		for(u32 j = 0; j < h->numTris; j++) {
			const md2Triangle_s *t = h->pTri(j);
			const md2Vertex_s *v[3];
			//const md2TexCoord_s *tc[3];
			for(u32 k = 0; k < 3; k++) {
				v[k] = f->pVertex(t->vertex[k]);
				//tc[k] = h->pTC(t->st[k]);
			}
			for(u32 k = 0; k < 3; k++, ov++) {
				ov->xyz.set(v[k]->v[0],v[k]->v[1],v[k]->v[2]);
				ov->xyz.scaleXYZ(frameScale);
				ov->xyz += frameTranslate;
			}
		}
	}

	return false; // no error
}
bool kfModelImpl_c::loadMDC(const char *fname) {
	byte *buf;
	u32 len = g_vfs->FS_ReadFile(fname,(void**)&buf);
	if(buf == 0)
		return true;
	bool res = loadMDC(buf,len,fname);
	g_vfs->FS_FreeFile(buf);
	return res;
}
bool kfModelImpl_c::loadMDC(const byte *buf, const u32 fileLen, const char *fname) {
	const mdcHeader_s *h = (const mdcHeader_s *) buf;
	if(h->ident != MDC_IDENT) {
		g_core->RedWarning("kfModel_c::loadMDC: %s has bad ident %i, should be \"IDPC\"\n",fname,h->ident);
		return true; // error
	}
	if(h->version != MDC_VERSION) {
		g_core->RedWarning("kfModel_c::loadMDC: %s has bad version %i, should be %i\n",fname,h->version,MDC_VERSION);
		return true; // error
	}
	surfs.resize(h->numSurfaces);
	kfSurf_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		const mdcSurface_s *is = h->pSurf(i);
		//if(is->ident != MD3_SURF_IDENT)
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
		// read baseframe verts
		arraySTD_c<kfSurfFrame_c> baseFrames;
		baseFrames.resize(h->numFrames);
		kfSurfFrame_c *f = baseFrames.getArray();
		const md3XyzNormal_s *xyzNormal = is->getXYZNormal(0);
		for(u32 j = 0; j < is->numBaseFrames; j++, f++) {
			f->verts.resize(is->numVerts);
			kfVert_c *v = f->verts.getArray();
			for(u32 k = 0; k < is->numVerts; k++, v++) {
				v->xyz = xyzNormal->getPos();
				v->normal = xyzNormal->getNormal();
				xyzNormal++;
			}
		}
		// read compressed verts
		//arraySTD_c<kfSurfFrame_c> compFrames;
		//baseFrames.resize(h->numFrames);
		//kfSurfFrame_c *f = baseFrames.getArray();
		//const mdcXyzCompressed_s *xyzComp = is->getXYZCompressed(0);
		//for(u32 j = 0; j < is->numBaseFrames; j++, f++) {
		//	f->verts.resize(is->numVerts);
		//	kfVert_c *v = f->verts.getArray();
		//	for(u32 k = 0; k < is->numVerts; k++, v++) {
		//		v->xyz = xyzNormal->getPos();
		//		xyzNormal++;
		//	}
		//}
		// read frames (finally)
		const short *frameBaseFrames = is->getFrameBaseFrames();
		const short *frameCompFrames = is->getFrameCompFrames();
		sf->xyzFrames.resize(h->numFrames);
		u32 compVertOfs = 0;
		for(u32 j = 0; j < h->numFrames; j++) {
			short base = frameBaseFrames[j];
			short comp = frameCompFrames[j];
			sf->xyzFrames[j] = baseFrames[base];
			// check for no compression
			if(comp == -1) {
				continue;
			}
			// add compressed vertex offsets
			for(u32 l = 0; l < is->numVerts; l++) {
				const mdcXyzCompressed_s *cv = is->getXYZCompressed(compVertOfs);
				vec3_c ofs;
				R_MDC_DecodeXyzCompressed(cv->ofsVec,ofs);
				sf->xyzFrames[j].verts[l].xyz += ofs;
				compVertOfs++;
			}

		}
		for(u32 j = 0; j < is->numShaders; j++) {
			const md3Shader_s *shi = is->getShader(j);
			if(j == 0) {
				sf->matName = shi->name;
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
	if(h->numTags) {
		tagFrames.resize(h->numFrames);
		kfTagFrame_c *ktf = tagFrames.getArray();
		for(u32 i = 0; i < h->numFrames; i++, ktf++) {
			ktf->tags.resize(h->numTags);
			const mdcTag_s *t = h->getFrameTags(i);
			for(u32 j = 0; j < h->numTags; j++, t++) {
				tagOr_c &o = ktf->tags[j];
				vec3_c a(t->angles[0]*MDC_TAG_ANGLE_SCALE,t->angles[1]*MDC_TAG_ANGLE_SCALE,t->angles[2]*MDC_TAG_ANGLE_SCALE);
				o.pos = vec3_c(t->xyz[0]*MD3_XYZ_SCALE,t->xyz[1]*MD3_XYZ_SCALE,t->xyz[2]*MD3_XYZ_SCALE);
				o.axis.fromAngles(a);
				
///				o.pos = t->xyz;

			}
		}
	}
	return false; // no error
}
bool kfModelImpl_c::loadTAN(const char *fname) {
	byte *buf;
	u32 len = g_vfs->FS_ReadFile(fname,(void**)&buf);
	if(buf == 0)
		return true;
	bool res = loadTAN(buf,len,fname);
	g_vfs->FS_FreeFile(buf);
	return res;
}
bool kfModelImpl_c::loadTAN(const byte *buf, const u32 fileLen, const char *fname) {
	const tanHeader_s *h = (const tanHeader_s *) buf;
	if(h->ident != TAN_IDENT) {
		g_core->RedWarning("kfModel_c::loadTAN: %s has bad ident %i, should be \"TAN \"\n",fname,h->ident);
		return true; // error
	}
	frames.resize(h->numFrames);
	kfFrame_c *of = frames.getArray();
	for(u32 i = 0; i < h->numFrames; i++, of++) {
		const tanFrame_s *f = h->pFrame(i);
		of->name = va("tan_frame_%i",i); // f->name;
		of->bounds.fromTwoPoints(f->bounds[0],f->bounds[1]);
		of->localOrigin = f->offset;
		of->scale = f->scale;
		of->radius = f->radius;
		//of->bounds.translate(of->localOrigin);
	//	of->bounds.scaleBB(f->scale);
	}
	surfs.resize(h->numSurfaces);
	kfSurf_c *sf = surfs.getArray();
	for(u32 i = 0; i < surfs.size(); i++, sf++) {
		const tanSurface_s *is = h->pSurf(i);
		//if(is->ident != MD3_SURF_IDENT)
		u32 numIndices = is->numTriangles*3;
		u16 *indices = sf->indices.initU16(numIndices);
		const u32 *firstIndex = is->getFirstIndex();
		for(u32 j = 0; j < numIndices; j++) {
			indices[j] = firstIndex[j];
		}
		sf->texCoords.resize(is->numVerts);
		vec2_c *tc = sf->texCoords.getArray();
		for(u32 j = 0; j < is->numVerts; j++, tc++) {
			const tanSt_s *st = is->getSt(j);
			tc->x = st->st[0];
			tc->y = st->st[1];
		}
		sf->xyzFrames.resize(h->numFrames);
		kfSurfFrame_c *f = sf->xyzFrames.getArray();
		const tanXyzNormal_s *xyzNormal = is->getXYZNormal(0);
		for(u32 j = 0; j < h->numFrames; j++, f++) {
			f->verts.resize(is->numVerts);
			const kfFrame_c &kf = frames[j];
			kfVert_c *v = f->verts.getArray();
			for(u32 k = 0; k < is->numVerts; k++, v++) {
				v->xyz = xyzNormal->getPos();
				v->xyz += kf.localOrigin;
				v->xyz.scaleXYZ(kf.scale*64.f);
				xyzNormal++;
			}
		}
		sf->name = is->name;
	}
	// load tags
	for(u32 i = 0; i < h->numFrames; i++) {
		for(u32 j = 0; j < h->numTags; j++) {

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

//bool KF_HasKeyFramedModelExt(const char *fname) {
//	const char *ext = G_strgetExt(fname);
//	if(ext == 0)
//		return false;
//	if(!stricmp(ext,"md3")) {
//		return true;
//	}
//	if(!stricmp(ext,"md2")) {
//		return true;
//	}
//	return false;
//}

