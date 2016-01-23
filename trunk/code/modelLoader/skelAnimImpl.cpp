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
#include <api/vfsAPI.h>

skelAnimGeneric_c::skelAnimGeneric_c() {
	animFlags = 0;
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

void skelAnimGeneric_c::buildSingleBone(int boneNum, const skelFrame_c &f, vec3_c &pos, quat_c &quat) const {
	pos = f.bones[boneNum].pos;
	quat = f.bones[boneNum].quat;
	//if(bones[boneNum].parentIndex != -1)
	//	quat.conjugate();
}
void skelAnimGeneric_c::buildFrameBonesLocal(u32 frameNum, class boneOrArray_c &out) const {

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
void skelAnimGeneric_c::addFrameRelative(const class boneOrArray_c &ors) {
	skelFrame_c f;
	f.setOrs(ors);
	//f.bounds.fromRadius(16.f); // TODO?
	frames.push_back(f);
}
void skelAnimGeneric_c::buildLoopAnimLerpFrameBonesLocal(const struct singleAnimLerp_s &lerp, class boneOrArray_c &out) const {
	if(lerp.to >= this->frames.size()) {
		g_core->RedWarning("skelAnimMD5_c::buildLoopAnimLerpFrameBonesLocal: lerp.to frame index %i out of range <0,%i)\n",lerp.to,frames.size());
		return;
	}
	if(lerp.from >= this->frames.size()) {
		g_core->RedWarning("skelAnimMD5_c::buildLoopAnimLerpFrameBonesLocal: lerp.from frame index %i out of range <0,%i)\n",lerp.from,frames.size());
		return;
	}
	const skelFrame_c &from = this->frames[lerp.from];
	const skelFrame_c &to = this->frames[lerp.to];
	if(out.size() != bones.size()) {
		// reallocating array_c memory in another module might cause
		// some issues (as long we dont have own memory system)
		g_core->DropError("skelAnimMD5_c::buildFrameBonesLocal: out.size() != this->bones.size()\n");
	}
	const boneDef_s *boneDef = bones.getArray();
	boneOr_s *outBone = out.getArray();
	for(u32 i = 0; i < bones.size(); i++, boneDef++, outBone++) {
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
skelAnimMD5_c::skelAnimMD5_c() {
	animFlags = 0;
}
skelAnimMD5_c::~skelAnimMD5_c() {
	frames.clear();
	bones.clear();
	md5AnimBones.clear();
	baseFrame.clear();
}
//
//	Doom3 .md5anim files loading
//
bool skelAnimMD5_c::loadMD5Anim(const char *fname) {
	// md5anim files are a raw text files, so setup the parsing
	parser_c p;
	if(p.openFile(fname)) {
		g_core->RedWarning("skelAnimMD5_c::loadMD5Anim: cannot open %s\n",fname);
		return true;
	}

	this->animFileName = fname;

	if(p.atWord("MD5Version")==false) {
		g_core->RedWarning("skelAnimMD5_c::loadMD5Anim: expected \"MD5Version\" string at the beggining of the file, found %s, in file %s\n",
			p.getToken(),fname);
		return true; // error
	}
	u32 version = p.getInteger();
	if(version != 10) {
		g_core->RedWarning("skelAnimMD5_c::loadMD5Anim: bad MD5Version %i, expected %i, in file %s\n",version,10,fname);
		return true; // error
	}

	u32 numAnimatedComponents = 0;
	u32 numJoints = 0;
	u32 numFramesParsed = 0;

	while(p.atEOF() == false) {
		if(p.atWord("commandline")) {
			const char *commandLine = p.getToken();

		} else if(p.atWord("numFrames")) {
			u32 i = p.getInteger();
			this->frames.resize(i);
		} else if(p.atWord("numJoints")) {
			numJoints = p.getInteger();
			baseFrame.resize(numJoints);
			md5AnimBones.resize(numJoints);
			bones.resize(numJoints);
		} else if(p.atWord("numAnimatedComponents")) {
			numAnimatedComponents = p.getInteger();
		} else if(p.atWord("frameRate")) {
			// read the rate of playback in frames per second
			frameRate = p.getFloat();
		} else if(p.atWord("hierarchy")) {
			if(p.atWord("{") == false) {
				g_core->RedWarning("Expected '{' to follow \"hierarchy\", found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}
			boneDef_s *bone = bones.getArray();
			md5AnimBone_s *b = md5AnimBones.getArray();
			u32 localChannelIndex = 0;
			for(u32 i = 0; i < md5AnimBones.size(); i++, b++, bone++) {
				// 	"origin"	-1 0 0	//
				const char *s = p.getToken();
				bone->nameIndex = SK_RegisterString(s);
				bone->parentIndex = p.getInteger();
				b->componentFlags = p.getInteger();
				b->firstComponent = p.getInteger();
			}
			if(p.atWord("}") == false) {
				g_core->RedWarning("Expected '}' after \"hierarchy\" block, found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}
		} else if(p.atWord("bounds")) {
			if(p.atWord("{") == false) {
				g_core->RedWarning("Expected '{' to follow \"bounds\", found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}

			md5Frame_c *f = this->frames.getArray();

			for(u32 i = 0; i < this->frames.size(); i++, f++) {
				if(p.getFloatMat_braced(f->bounds.mins,3)) {
					g_core->RedWarning("Failed to read bounds mins vector for frame %i in file %s at line %i\n",i,p.getToken(),fname,p.getCurrentLineNumber());
					return true; // error
				}
				if(p.getFloatMat_braced(f->bounds.maxs,3)) {
					g_core->RedWarning("Failed to read bounds maxs vector for frame %i in file %s at line %i\n",i,p.getToken(),fname,p.getCurrentLineNumber());
					return true; // error
				}
			}

			if(p.atWord("}") == false) {
				g_core->RedWarning("Expected '}' after \"bounds\" block, found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}
		} else if(p.atWord("baseframe")) {
			if(p.atWord("{") == false) {
				g_core->RedWarning("Expected '{' to follow \"baseframe\", found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}

			md5BoneVal_s *b = baseFrame.getArray();
			for(u32 i = 0; i < numJoints; i++, b++) {
				if(p.getFloatMat_braced(b->pos,3)) {
					g_core->RedWarning("Failed to read baseframe ofs vector for joint %i in file %s at line %i\n",i,p.getToken(),fname,p.getCurrentLineNumber());
					return true; // error
				}
				if(p.getFloatMat_braced(b->quat,3)) {
					g_core->RedWarning("Failed to read baseframe quat (X Y Z) for joint %i in file %s at line %i\n",i,p.getToken(),fname,p.getCurrentLineNumber());
					return true; // error
				}
			}

			if(p.atWord("}") == false) {
				g_core->RedWarning("Expected '}' after \"bounds\" block, found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}
		} else if(p.atWord("frame")) {
			u32 checkFrameIndex = p.getInteger();
			if(checkFrameIndex != numFramesParsed) {
				g_core->RedWarning("Bad \"frame\" index %i, expected %i in file %s at line %i\n",checkFrameIndex,numFramesParsed,fname,p.getCurrentLineNumber());
				return true; // error
			}

			if(p.atWord("{") == false) {
				g_core->RedWarning("Expected '{' to follow \"frame\", found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}

			md5Frame_c *f = &this->frames[numFramesParsed];

			f->components.resize(numAnimatedComponents);
			for(u32 i = 0; i < numAnimatedComponents; i++) {
				f->components[i] = p.getFloat();
			}

			numFramesParsed++;

			if(p.atWord("}") == false) {
				g_core->RedWarning("Expected '}' after \"frame\" block, found %s in file %s at line %i\n",p.getToken(),fname,p.getCurrentLineNumber());
				return true; // error
			}
		} else {
			g_core->RedWarning("Unknown token %s in md5anim file %s\n",p.getToken(),fname);
		}
	}
	int originBoneIndex = this->getLocalBoneIndexForBoneName("origin");
	if(originBoneIndex >= 0) {
		md5AnimBones[originBoneIndex].componentFlags = 0;
		baseFrame[originBoneIndex].clearPosition();
	}

	frameTime = 1.f / frameRate;
	totalTime = frameTime * float(frames.size());
	return false; // no error
}	
void skelAnimMD5_c::clearMD5BoneComponentFlags(const char *boneName) {
	u32 nameIndex = SK_RegisterString(boneName);
	int localIndex = findBoneForBoneNameIndex(nameIndex);
	if(localIndex == -1) {
		g_core->RedWarning("skelAnimMD5_c::clearMD5BoneComponentFlags: Bone %s not found\n",boneName);
		return;
	}
	md5AnimBones[localIndex].componentFlags = 0;
}
// 
//	md5 animation code
//
void skelAnimMD5_c::buildSingleBone(int boneNum, const md5Frame_c &f, vec3_c &pos, quat_c &quat) const {
	const md5BoneVal_s &base = baseFrame[boneNum];
	const md5AnimBone_s &mb = md5AnimBones[boneNum];
	pos = base.pos;
	quat = base.quat;
	if(mb.componentFlags) {
		int c = 0;
		if(mb.componentFlags & COMPONENT_BIT_TX) {
			pos[0] = f.components[mb.firstComponent + c];
			c++;
		}
		if(mb.componentFlags & COMPONENT_BIT_TY) {
			pos[1] = f.components[mb.firstComponent + c];
			c++;
		}
		if(mb.componentFlags & COMPONENT_BIT_TZ) {
			pos[2] = f.components[mb.firstComponent + c];
			c++;
		}
		if(mb.componentFlags & COMPONENT_BIT_QX) {
			quat[0] = f.components[mb.firstComponent + c];
			c++;
		}
		if(mb.componentFlags & COMPONENT_BIT_QY) {
			quat[1] = f.components[mb.firstComponent + c];
			c++;
		}
		if(mb.componentFlags & COMPONENT_BIT_QZ) {
			quat[2] = f.components[mb.firstComponent + c];
			c++;
		}
	}
	quat.calcW();
}
void skelAnimMD5_c::buildFrameBonesLocal(u32 frameNum, class boneOrArray_c &out) const {
	const md5Frame_c &f = this->frames[frameNum];
	if(out.size() != bones.size()) {
		// reallocating array_c memory in another module might cause
		// some issues (as long we dont have own memory system)
		g_core->DropError("skelAnimMD5_c::buildFrameBonesLocal: out.size() != this->bones.size()\n");
	}
	const boneDef_s *boneDef = bones.getArray();
	boneOr_s *outBone = out.getArray();
	for(u32 i = 0; i < bones.size(); i++, boneDef++, outBone++) {
		quat_c quat;
		vec3_c pos;
		buildSingleBone(i,f,pos,quat);
		outBone->boneName = boneDef->nameIndex;
		outBone->mat.fromQuatAndOrigin(quat,pos);
	}
}
void skelAnimMD5_c::buildFrameBonesABS(u32 frameNum, class boneOrArray_c &out) const {
	buildFrameBonesLocal(frameNum,out);
	out.localBonesToAbsBones(&this->bones);
}
void skelAnimMD5_c::buildLoopAnimLerpFrameBonesLocal(const struct singleAnimLerp_s &lerp, class boneOrArray_c &out) const {
	if(lerp.to >= this->frames.size()) {
		g_core->RedWarning("skelAnimMD5_c::buildLoopAnimLerpFrameBonesLocal: lerp.to frame index %i out of range <0,%i)\n",lerp.to,frames.size());
		return;
	}
	if(lerp.from >= this->frames.size()) {
		g_core->RedWarning("skelAnimMD5_c::buildLoopAnimLerpFrameBonesLocal: lerp.from frame index %i out of range <0,%i)\n",lerp.from,frames.size());
		return;
	}
	const md5Frame_c &from = this->frames[lerp.from];
	const md5Frame_c &to = this->frames[lerp.to];
	if(out.size() != bones.size()) {
		// reallocating array_c memory in another module might cause
		// some issues (as long we dont have own memory system)
		g_core->DropError("skelAnimMD5_c::buildFrameBonesLocal: out.size() != this->bones.size()\n");
	}
	const boneDef_s *boneDef = bones.getArray();
	boneOr_s *outBone = out.getArray();
	for(u32 i = 0; i < bones.size(); i++, boneDef++, outBone++) {
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