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
// skelAnimImpl.cpp
#include <shared/parser.h>
#include <math/quat.h>
#include <api/coreAPI.h>
#include "skelAnimImpl.h"
#include <shared/skelUtils.h> // boneOrArray_c
#include <fileFormats/mdx_file_format.h>
#include <fileFormats/mds_file_format.h>
#include <fileFormats/skab_file_format.h>
#include <fileFormats/actorX_file_format.h>
#include <api/vfsAPI.h>
#include <api/skelModelAPI.h>
#include <shared/skc.h>

skelAnimGeneric_c::skelAnimGeneric_c() {
	animFlags = 0;
	frameTime = 1.f;
	frameRate = 1.f;
	totalTime = 1.f;
}
skelAnimGeneric_c::~skelAnimGeneric_c() {
	frames.clear();
	bones.clear();
	baseFrame.clear();
}
//
//	Wolfenstein: Enemy Territory .mdx files loading
//
bool skelAnimGeneric_c::loadMDXAnim(const char *fname) {

	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open file
	}


	const mdxHeader_t *h = (const mdxHeader_t *)fileData;
	if(h->ident != MDX_IDENT) {
		g_core->RedWarning("MDX file %s header has bad ident\n",fname);
		return true;
	}
	if(h->version != MDX_VERSION) {
		g_core->RedWarning("MDX file %s has bad version %i, should be %i\n",fname,h->version,MDX_VERSION);
		return true;
	}

	this->animFileName = fname;
	this->frameTime = 1.f;
	this->bones.resize(h->numBones);
	boneDef_s *bd = bones.getArray();
	for(u32 i = 0; i < h->numBones; i++, bd++) {
		const mdxBoneInfo_t *bi = h->pBone(i);

		bd->nameIndex = SK_RegisterString(bi->name);
		bd->parentIndex = bi->parent;
	}

	this->frames.resize(h->numFrames);
	for(u32 i = 0; i < h->numFrames; i++) {
		const mdxFrame_t *f = h->pFrame(i);
		skelFrame_c *of = &this->frames[i];
		// step 1 - build ABS frameBones
		boneOrArray_c frameABS;
		frameABS.resize(h->numBones);
		boneOr_s *bor = frameABS.getArray();
		for(u32 j = 0; j < h->numBones; j++, bor++) {
			const mdxBoneFrameCompressed_t *bi = f->pBone(j);
			const mdxBoneInfo_t *bd = h->pBone(j);

			vec3_c offsetAngles = bi->getOfsAngles();

			matrix_c matRot;
			matRot.fromAngles(offsetAngles);
			vec3_c point(bd->parentDist,0,0);

			matRot.transformPoint(point);

			vec3_c parentOrigin;
			if(bd->parent == -1) {
				parentOrigin = f->parentOffset;
			} else {
				parentOrigin = frameABS[bd->parent].mat.getOrigin();
			}
			point += parentOrigin;

			vec3_c angles = bi->getAngles();

			// we cant just use matrix_c::fromAngles
			// because the order of rotations in mds
			// is slightly different.
			// (pitch-yaw-roll vs roll-pitch-yaw)

			matrix_c rollMatrix;
			rollMatrix.setupXRotation(angles[ROLL]);

			matrix_c yawMatrix;
			yawMatrix.setupZRotation(angles[YAW]);

			matrix_c pitchMatrix;
			pitchMatrix.setupYRotation(angles[PITCH]);

			matrix_c rotMatrix = yawMatrix * pitchMatrix * rollMatrix;

			rotMatrix.inverse();

			bor->boneName = bones[j].nameIndex;

			// copy rotation
			bor->mat = rotMatrix;

			// and just set matrix origin fields
			bor->mat.setOrigin(point);			
			
			///g_core->Print("ABS : Vec %i - parent %i - %f %f %f\n",j,bones[j].parentIndex,point[0],point[1],point[2]);

		}
		// step 2 - build relative baseFrame from abs baseFrame
		boneOrArray_c frameRel;
		frameRel.absBonesToLocalBones(&this->bones,&frameABS);
		// step 3 - copy out bone pos/rot channels
		of->bones.resize(h->numBones);
		boneOr_s *bRelOr = frameRel.getArray();
		for(u32 j = 0; j < h->numBones; j++, bRelOr++) {
			of->bones[j].setQuat(bRelOr->mat.getQuat());
			of->bones[j].setVec3(bRelOr->mat.getOrigin());
			//g_core->Print("REL : Vec %i - parent %i - %f %f %f\n",j,bones[j].parentIndex,of->bones[j].pos[0],of->bones[j].pos[1],of->bones[j].pos[2]);
		}
	}
	this->totalTime = this->frameTime * this->frames.size();
	this->frameRate = 1.f / this->frameTime;
	return false;
}

//
//	RTCW .mds files loading
//
bool skelAnimGeneric_c::loadMDSAnim(const char *fname) {

	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open file
	}


	const mdsHeader_t *h = (const mdsHeader_t *)fileData;
	if(h->ident != MDS_IDENT) {
		g_core->RedWarning("MDS file %s header has bad ident\n",fname);
		return true;
	}
	if(h->version != MDS_VERSION) {
		g_core->RedWarning("MDS file %s has bad version %i, should be %i\n",fname,h->version,MDX_VERSION);
		return true;
	}

	this->animFileName = fname;
	this->frameTime = 1.f;
	this->bones.resize(h->numBones);
	boneDef_s *bd = bones.getArray();
	for(u32 i = 0; i < h->numBones; i++, bd++) {
		const mdsBoneInfo_t *bi = h->pBone(i);

		bd->nameIndex = SK_RegisterString(bi->name);
		bd->parentIndex = bi->parent;
	}

	this->frames.resize(h->numFrames);
	for(u32 i = 0; i < h->numFrames; i++) {
		const mdsFrame_t *f = h->pFrame(i);
		skelFrame_c *of = &this->frames[i];
		// step 1 - build ABS frameBones
		boneOrArray_c frameABS;
		frameABS.resize(h->numBones);
		boneOr_s *bor = frameABS.getArray();
		for(u32 j = 0; j < h->numBones; j++, bor++) {
			const mdsBoneFrameCompressed_t *bi = f->pBone(j);
			const mdsBoneInfo_t *bd = h->pBone(j);

			vec3_c offsetAngles = bi->getOfsAngles();

			matrix_c matRot;
			matRot.fromAngles(offsetAngles);
			vec3_c point(bd->parentDist,0,0);

			matRot.transformPoint(point);

			vec3_c parentOrigin;
			if(bd->parent == -1) {
				parentOrigin = f->parentOffset;
			} else {
				parentOrigin = frameABS[bd->parent].mat.getOrigin();
			}
			point += parentOrigin;

			vec3_c angles = bi->getAngles();

			// we cant just use matrix_c::fromAngles
			// because the order of rotations in mds
			// is slightly different.
			// (pitch-yaw-roll vs roll-pitch-yaw)

			matrix_c rollMatrix;
			rollMatrix.setupXRotation(angles[ROLL]);

			matrix_c yawMatrix;
			yawMatrix.setupZRotation(angles[YAW]);

			matrix_c pitchMatrix;
			pitchMatrix.setupYRotation(angles[PITCH]);

			matrix_c rotMatrix = yawMatrix * pitchMatrix * rollMatrix;

			rotMatrix.inverse();

			bor->boneName = bones[j].nameIndex;

			// copy rotation
			bor->mat = rotMatrix;

			// and just set matrix origin fields
			bor->mat.setOrigin(point);			
			
			///g_core->Print("ABS : Vec %i - parent %i - %f %f %f\n",j,bones[j].parentIndex,point[0],point[1],point[2]);

		}
		// step 2 - build relative baseFrame from abs baseFrame
		boneOrArray_c frameRel;
		frameRel.absBonesToLocalBones(&this->bones,&frameABS);
		// step 3 - copy out bone pos/rot channels
		of->bones.resize(h->numBones);
		boneOr_s *bRelOr = frameRel.getArray();
		for(u32 j = 0; j < h->numBones; j++, bRelOr++) {
			of->bones[j].setQuat(bRelOr->mat.getQuat());
			of->bones[j].setVec3(bRelOr->mat.getOrigin());
			//g_core->Print("REL : Vec %i - parent %i - %f %f %f\n",j,bones[j].parentIndex,of->bones[j].pos[0],of->bones[j].pos[1],of->bones[j].pos[2]);
		}
	}
	this->totalTime = this->frameTime * this->frames.size();
	this->frameRate = 1.f / this->frameTime;
	return false;
}

//
//	Source intermediate .smd files loading
//
bool skelAnimGeneric_c::loadSMDAnim(const char *fname) {
	// .smd files are a raw text files, so setup the parsing
	parser_c p;
	p.openFile(fname);

	if(p.atWord("version")==false) {
		g_core->Print("skelModel_c::loadSMD: expected \"version\" string at the beggining of the file, found %s, in file %s\n",
			p.getToken(),fname);
		return true; // error
	}
	u32 version = p.getInteger();
	if(version != 1) {
		g_core->Print("skelModel_c::loadSMD: bad version %i, expected %i, in file %s\n",version,1,fname);
		return true; // error
	}
	u32 numNodesParsed = 0;
	while(!p.atEOF()) {
		if(p.atWord("nodes")) {
			while(p.atWord("end") == false) {
				u32 checkIndex = p.getInteger();
				if(checkIndex != numNodesParsed) {
					g_core->Print("Expected node index %i at line %i of %s, found %s\n",numNodesParsed,p.getCurrentLineNumber(),fname,p.getToken());
				}
				str nodeName = p.getToken();
				int parentIndex = p.getInteger();

				boneDef_s b;
				b.nameIndex = SK_RegisterString(nodeName);
				b.parentIndex = parentIndex;

				numNodesParsed++;

				this->bones.push_back(b);
			}
			g_core->Print("%i smd nodes (bones) loaded\n",this->bones.size());
		} else if(p.atWord("skeleton")) {
			bool stop = false;
			while(stop == false) {
				if(p.atWord("time") == false) {
					g_core->Print("Expected \"time\" to follow \"skeleton\" at line %i of %s, found %s\n",p.getCurrentLineNumber(),fname,p.getToken());
					return true;
				}
				atTimeToken:
				float timeVal = p.getFloat();
				boneOrArray_c frameRelative;
				frameRelative.resize(this->bones.size());
				while(1) {
					// <int|bone ID> <float|PosX PosY PosZ> <float|RotX RotY RotZ>
					u32 id = p.getInteger();
					vec3_c pos;
					p.getFloatMat(pos,3);
					vec3_c rot;
					p.getFloatMat(rot,3);

					vec3_c angles( RAD2DEG(rot.y), RAD2DEG(rot.z), RAD2DEG(rot.x) );
					frameRelative[id].mat.fromAnglesAndOrigin(angles,pos);

					if(p.atWord("time")) {
						this->addFrameRelative(frameRelative);
						goto atTimeToken; 
					} else if(p.atWord("end")) {
						stop = true;
						break;
					}
				}
				this->addFrameRelative(frameRelative);
			}
		} else if(p.atWord("triangles")) {
			while(p.atWord("end") == false && p.atEOF() == false) {
				const char *dummy = p.getToken();
			}
		} else {
			g_core->Print("Unknown token %s in smd file %s\n",p.getToken(),fname);
		}
	}

	this->frameTime = 0.2f;
	this->totalTime = this->frameTime * this->frames.size();
	this->frameRate = 1.f / this->frameTime;
	return false;
}
//
//	ActorX .psk files loading
//
bool skelAnimGeneric_c::loadPSKAnim(const char *fname) {
	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open file
	}

	const axChunkHeader_t *h = (const axChunkHeader_t*)fileData;
	if(h->hasIdent(PSK_IDENT_HEADER)==false) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong ident %s, should be %s\n",fname,h->ident,PSK_IDENT_HEADER);
		return true; // error
	}
	const axChunkHeader_t *s;

	s = h->getNextHeader();
	if(s->hasIdent(PSK_IDENT_POINTS)==false) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_POINTS);
		return true; // error
	}
	if(s->dataSize != sizeof(axPoint_t)) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axPoint_t));
		return true; // error
	}
	const axChunkHeader_t *pSection = s;
	// do nothing for points
	s = s->getNextHeader();
	if(s->hasIdent(PSK_IDENT_VERTS)==false) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_VERTS);
		return true; // error
	}
	if(s->dataSize != sizeof(axVertex_t)) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axVertex_t));
		return true; // error
	}
	const axChunkHeader_t *vertSection = s;
	// do nothing for verts
	s = s->getNextHeader();
	if(s->hasIdent(PSK_IDENT_FACES)==false) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_FACES);
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	if(s->dataSize != sizeof(axTriangle_t)) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axTriangle_t));
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	const axChunkHeader_t *triSection = s;
	// do nothin for tris
	s = s->getNextHeader();
	if(s->hasIdent(PSK_IDENT_MATS)==false) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_MATS);
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	if(s->dataSize != sizeof(axMaterial_t)) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axMaterial_t));
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	const axChunkHeader_t *matSection = s;
	// do nothing for materials
	s = s->getNextHeader();
	if(s->hasIdent(PSK_IDENT_REFS)==false) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section ident %s, should be %s\n",fname,s->ident,PSK_IDENT_REFS);
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	if(s->dataSize != sizeof(axReferenceBone_t)) {
		g_core->RedWarning("skelAnimGeneric_c::loadPSKAnim: psk file %s has wrong internal section dataSize %i, should be %i\n",fname,s->dataSize,sizeof(axReferenceBone_t));
		g_vfs->FS_FreeFile(fileData);
		return true; // error
	}
	u32 nj = s->numData;
	this->bones.resize(nj);
	boneOrArray_c baseFrameABS;
	baseFrameABS.resize(nj);
	boneOrArray_c baseFrameRelative;
	baseFrameRelative.resize(nj);
	for(u32 i = 0; i < nj; i++) {
		const axReferenceBone_t *bi = s->getBone(i);
		if(0) {
			g_core->RedWarning("Bone %i name %s\n",i,bi->name);
		}

		bones[i].nameIndex = SK_RegisterString(bi->name);
		bones[i].parentIndex = bi->parentIndex;
		if(i == 0)
			bones[i].parentIndex--;

		const axBone_t *or = &bi->bone;
		quat_c q;
		q = or->quat;
		if(i == 0) {
			// negate y only
			//q.setY(-q.getY());
		} else {
			// negate x,y,z
			q.conjugate();
		}
		vec3_c pos = or->position;
		baseFrameRelative[i].mat.fromQuatAndOrigin(q,pos);
		baseFrameRelative[i].boneName = bones[i].nameIndex;
	}
	baseFrameABS = baseFrameRelative;
	baseFrameABS.localBonesToAbsBones(&bones);
	boneOrArray_c baseFrameABSInverse = baseFrameABS.getInversed();
	
	// just set a single frame with baseframe
	frames.resize(1);
	frames[0].setOrs(baseFrameRelative);

	this->frameTime = 1.f;
	this->totalTime = this->frameTime * this->frames.size();
	this->frameRate = 1.f / this->frameTime;

	return false;
}
//
//	FAKK .ska files loading
//
bool skelAnimGeneric_c::loadSKAAnim(const char *fname) {
	byte *fileData;
	// load raw file data from disk
	u32 fileLen = g_vfs->FS_ReadFile(fname,(void**)&fileData);
	if(fileData == 0) {
		return true; // cannot open file
	}

	skaHeader_s *h = (skaHeader_s *) fileData;
	
	if(h->ident != SKA_IDENT) {
		g_core->RedWarning("ska file %s header has bad ident\n",fname);
		return true;
	}

	if(h->version != SKA_VERSION && h->version != SKB_VERSION_EF2) {
		g_core->RedWarning("ska file %s header has bad version %i - should be %i or %i\n",fname,h->version,SKA_VERSION, SKB_VERSION_EF2);
		return true;
	}

	this->frameTime = h->frametime;

	//if(skel) {
	//	if(h->numBones != skel->getNumBones())
	//	{
	//		g_core->RedWarning("ska file %s header has bad bone count (%i should be %i to mach skb\n",
	//			fname,h->numBones,skel->getNumBones());
	//		return true;
	//	}
	//}

	this->animFileName = fname;
	if(h->version == SKA_VERSION) {
		this->frames.resize(h->numFrames);
		for(u32 i = 0; i < h->numFrames; i++) {
			const skaFrame_s *f = h->pFrame(i);
			skelFrame_c *of = &this->frames[i];
			of->bones.resize(h->numBones);
			md5BoneVal_s *out = of->bones.getArray();
			for(u32 j = 0; j < h->numBones; j++) {
				const skaBone_s *b = f->pBone(j);
				out->setQuat(-float(b->shortQuat[0])*SKA_BONE_QUAT_MULTIPLIER_RECIPROCAL,
					-float(b->shortQuat[1])*SKA_BONE_QUAT_MULTIPLIER_RECIPROCAL,
					-float(b->shortQuat[2])*SKA_BONE_QUAT_MULTIPLIER_RECIPROCAL,
					float(b->shortQuat[3])*SKA_BONE_QUAT_MULTIPLIER_RECIPROCAL);

				out->setVec3(float(b->shortOffset[0])*SKA_BONE_OFFSET_MULTIPLIER_RECIPROCAL,
					float(b->shortOffset[1])*SKA_BONE_OFFSET_MULTIPLIER_RECIPROCAL,
					float(b->shortOffset[2])*SKA_BONE_OFFSET_MULTIPLIER_RECIPROCAL);
				out++;
			}
		}
	} else if(h->version == SKA_VERSION_EF2) {
		const skaHeader4_s *h4 = (const skaHeader4_s*)h;
		this->frames.resize(h->numFrames);
		for(u32 i = 0; i < h->numFrames; i++) {
			const skaFrame_s *f = h4->pFrame(i);
			skelFrame_c *of = &this->frames[i];
			of->bones.resize(h->numBones);
			md5BoneVal_s *out = of->bones.getArray();
			for(u32 j = 0; j < h->numBones; j++) {
				const skaBone_s *b = f->pBone(j);
				out->setQuat(-float(b->shortQuat[0])*SKA_BONE_QUAT_MULTIPLIER_RECIPROCAL,
					-float(b->shortQuat[1])*SKA_BONE_QUAT_MULTIPLIER_RECIPROCAL,
					-float(b->shortQuat[2])*SKA_BONE_QUAT_MULTIPLIER_RECIPROCAL,
					float(b->shortQuat[3])*SKA_BONE_QUAT_MULTIPLIER_RECIPROCAL);

				out->setVec3(float(b->shortOffset[0])*SKA_BONE_OFFSET_MULTIPLIER_RECIPROCAL,
					float(b->shortOffset[1])*SKA_BONE_OFFSET_MULTIPLIER_RECIPROCAL,
					float(b->shortOffset[2])*SKA_BONE_OFFSET_MULTIPLIER_RECIPROCAL);
				out++;
			}
		}
	}

	this->totalTime = this->frameTime * this->frames.size();
	this->frameRate = 1.f / this->frameTime;
	return false;
}
int skelAnimGeneric_c::registerBone(const char *boneName) {
	int r = getLocalBoneIndexForBoneName(boneName);
	if(r != -1)
		return r;
	boneDef_s nb;
	nb.parentIndex = -1;
	nb.nameIndex = SK_RegisterString(boneName);
	r = bones.size();
	bones.push_back(nb);
	return r;
}
//
//	MoHAA .skc files loading
//
bool skelAnimGeneric_c::loadSKCAnim(const char *fname) {
	skc_c skc;
	if(skc.loadSKC(fname)) {
		return true;
	}

	for(u32 i = 0; i < skc.getNumChannels(); i++) {
		str channelName = skc.getChannelName(i);
		if(channelName.endsWith(" rot")) {
			str boneName = channelName;
			boneName.capLen(boneName.length()-4);
			registerBone(boneName);
		} else if(channelName.endsWith(" pos")) {
			str boneName = channelName;
			boneName.capLen(boneName.length()-4);
			registerBone(boneName);
		}
	}
	frames.resize(skc.getNumFrames());
	for(u32 f = 0; f < frames.size(); f++) {
		frames[f].bones.resize(bones.size());
	}
	for(u32 i = 0; i < skc.getNumChannels(); i++) {
		str channelName = skc.getChannelName(i);
		if(channelName.endsWith(" rot")) {
			str boneName = channelName;
			boneName.capLen(boneName.length()-4);
			int boneIndex = registerBone(boneName);
			for(u32 f = 0; f < frames.size(); f++) {
				skelFrame_c &sf = frames[f];
				const float *val = skc.getFrameChannel(f,i);
				sf.bones[boneIndex].setQuat(val);
				sf.bones[boneIndex].conjugateQuat();
			}
		} else if(channelName.endsWith(" pos")) {
			str boneName = channelName;
			boneName.capLen(boneName.length()-4);
			int boneIndex = registerBone(boneName);
			for(u32 f = 0; f < frames.size(); f++) {
				skelFrame_c &sf = frames[f];
				const float *val = skc.getFrameChannel(f,i);
				sf.bones[boneIndex].setVec3(val);
			}
		}
	}


	bones.clear();

	this->frameTime = skc.getFrameTime();
	this->totalTime = this->frameTime * this->frames.size();
	this->frameRate = 1.f / this->frameTime;
	return false;
}
skelAnimAPI_i *skelAnimGeneric_c::createSubAnim(u32 firstFrame, u32 numFrames) const {
	skelAnimGeneric_c *copy = new skelAnimGeneric_c();
	u32 lastFrame = firstFrame + numFrames;
	for(u32 i = firstFrame; i < lastFrame; i++) {
		copy->frames.push_back(frames[i]);
	}
	copy->animFileName = animFileName;
	copy->frameRate = frameRate;
	copy->frameTime = frameTime;
	copy->totalTime = totalTime;
	copy->bones = bones;
	copy->baseFrame = baseFrame;
	copy->animFlags = animFlags;
	return copy;
}
void skelAnimGeneric_c::buildSingleBone(int boneNum, const skelFrame_c &f, vec3_c &pos, quat_c &quat) const {
	pos = f.bones[boneNum].pos;
	quat = f.bones[boneNum].quat;
	//if(bones[boneNum].parentIndex != -1)
	//	quat.conjugate();
}
void skelAnimGeneric_c::buildFrameBonesLocal(u32 frameNum, class boneOrArray_c &out, const class skelModelAPI_i *skelModel) const {
	const skelFrame_c &f = this->frames[frameNum];
	const boneDef_s *boneDef;
	u32 numBones;
	if(bones.size()) {
		boneDef = bones.getArray();
		numBones = bones.size();
	} else {
		boneDef = skelModel->getBoneDefs()->getArray();
		numBones = skelModel->getBoneDefs()->size();
	}
	boneOr_s *outBone = out.getArray();
	for(u32 i = 0; i < numBones; i++, boneDef++, outBone++) {
		quat_c quat;
		vec3_c pos;
		buildSingleBone(i,f,pos,quat);
		outBone->boneName = boneDef->nameIndex;
		outBone->mat.fromQuatAndOrigin(quat,pos);
	}
}
void skelAnimGeneric_c::buildFrameBonesABS(u32 frameNum, class boneOrArray_c &out) const {

}
void skelFrame_c::setOrs(const class boneOrArray_c &ors) {
	bones.resize(ors.size());
	for(u32 i = 0; i < ors.size(); i++) {
		const matrix_c &m = ors[i].mat;
		md5BoneVal_s &v = bones[i];
		v.setQuat(m.getQuat());
		v.setVec3(m.getOrigin());
	}
}		
void skelAnimGeneric_c::scaleAnimation(float s) {
	for(u32 i = 0; i < frames.size(); i++) {
		skelFrame_c &f = frames[i];
		for(u32 j = 0; j < f.bones.size(); j++) {
			f.bones[j].scalePos(s);
		}
	}
}
void skelAnimGeneric_c::addFrameRelative(const class boneOrArray_c &ors) {
	skelFrame_c f;
	f.setOrs(ors);
	//f.bounds.fromRadius(16.f); // TODO?
	frames.push_back(f);
}
void skelAnimGeneric_c::buildLoopAnimLerpFrameBonesLocal(const struct singleAnimLerp_s &lerp, class boneOrArray_c &out, const class skelModelAPI_i *skelModel) const {
	if(lerp.to >= this->frames.size()) {
		g_core->RedWarning("skelAnimMD5_c::buildLoopAnimLerpFrameBonesLocal: lerp.to frame index %i out of range <0,%i)\n",lerp.to,frames.size());
		return;
	}
	if(lerp.from >= this->frames.size()) {
		g_core->RedWarning("skelAnimMD5_c::buildLoopAnimLerpFrameBonesLocal: lerp.from frame index %i out of range <0,%i)\n",lerp.from,frames.size());
		return;
	}
	u32 numBoneDefs;
	const boneDef_s *boneDef;
	if(bones.size()) {
		boneDef = bones.getArray();
		numBoneDefs = bones.size();
	} else {
		boneDef = skelModel->getBoneDefs()->getArray();
		numBoneDefs = skelModel->getNumBones();
	}

	const skelFrame_c &from = this->frames[lerp.from];
	const skelFrame_c &to = this->frames[lerp.to];
	if(out.size() != numBoneDefs) {
		// reallocating array_c memory in another module might cause
		// some issues (as long we dont have own memory system)
		g_core->DropError("skelAnimMD5_c::buildFrameBonesLocal: out.size() != this->bones.size()\n");
	}
	boneOr_s *outBone = out.getArray();
	for(u32 i = 0; i < numBoneDefs; i++, boneDef++, outBone++) {
		vec3_c posFrom, posTo;
		quat_c quatFrom, quatTo;
		buildSingleBone(i,from,posFrom,quatFrom);
		buildSingleBone(i,to,posTo,quatTo);
		// lerp frame values
		quat_c quat;
		vec3_c pos;
		quat.slerp(quatFrom,quatTo,lerp.frac);
		pos.lerp(posFrom,posTo,lerp.frac);
		outBone->boneName = boneDef->nameIndex;
		outBone->mat.fromQuatAndOrigin(quat,pos);
	}
}
bool skelAnimGeneric_c::getSKCData(class skc_c *out) const {
	out->resizeFrames(frames.size());
	out->resizeChannels(bones.size()*2);
	for(u32 i = 0; i < bones.size(); i++) {
		u32 c0 = i*2;
		u32 c1 = c0 + 1;
		const char *boneName = SK_GetString(bones[i].nameIndex);
		str rotName = boneName;
		rotName.append(" rot");
		str posName = boneName;
		posName.append(" pos");
		out->setChannelName(c0,rotName);
		out->setChannelName(c1,posName);
	}
	for(u32 i = 0; i < frames.size(); i++) {
		const skelFrame_c &f = frames[i];
		for(u32 j = 0; j < bones.size(); j++) {
			const md5BoneVal_s &bv = f.bones[j];
			u32 c0 = j*2;
			u32 c1 = c0 + j;
			out->setFrameChannel(i,c0,bv.quat);
			out->setFrameChannel(i,c1,bv.pos);
		}
	}
	return false;
}