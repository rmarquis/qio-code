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
// skelAnimMD5.cpp
#include <shared/parser.h>
#include <math/quat.h>
#include <api/coreAPI.h>
#include "skelAnimMD5.h"
#include <shared/skelUtils.h> // boneOrArray_c
#include <fileFormats/mdx_file_format.h>
#include <fileFormats/mds_file_format.h>
#include <fileFormats/skab_file_format.h>
#include <fileFormats/actorX_file_format.h>
#include <api/vfsAPI.h>
#include <api/skelModelAPI.h>
#include <shared/skc.h>
#include <shared/fileStreamHelper.h> // boneOrArray_c

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
//	Doom3 .md5anim files writing
//
bool skelAnimMD5_c::writeMD5Anim(const char *fname) {
	fileStreamHelper_c f;
	writeStreamAPI_i *w = &f;

	if(frames.size() == 0)
		return true;

	f.beginWriting(fname);
	w->writeText("MD5Version 10\n");
	w->writeText("commandline \"\"\n");
	w->writeText("\n");

	w->writeText("numFrames %i\n",frames.size());
	w->writeText("numJoints %i\n",bones.size());
	w->writeText("frameRate %i\n",int(frameRate));
	w->writeText("numAnimatedComponents %i\n",frames[0].components.size());

	w->writeText("\n");
	
	w->writeText("hierarchy {\n");
	for(u32 i = 0; i < bones.size(); i++) {
		const boneDef_s &b = bones[i];
		const md5AnimBone_s &ab = md5AnimBones[i];
		const char *boneName = SK_GetString(b.nameIndex);
		const char *parentName;
		if(b.parentIndex == -1)
			parentName = "";
		else 
			parentName = SK_GetString(bones[b.parentIndex].nameIndex);

		w->writeText("\t\"%s\"\t%i %i %i // %s\n",
			boneName,b.parentIndex,ab.componentFlags,ab.firstComponent,parentName);
	}
	w->writeText("}\n");

	w->writeText("\n");
	
	w->writeText("bounds {\n");
	for(u32 i = 0; i < frames.size(); i++) {
		const md5Frame_c &f = frames[i];
		const aabb &bb = f.bounds;
		const vec3_c &mi = bb.getMins();
		const vec3_c &ma = bb.getMaxs();
		w->writeText("\t( %f %f %f ) ( %f %f %f )\n",
			mi.getX(),mi.getY(),mi.getZ(),ma.getX(),ma.getY(),ma.getZ());
	}
	w->writeText("}\n");
	w->writeText("\n");
	
	w->writeText("baseframe {\n");
	for(u32 i = 0; i < baseFrame.size(); i++) {
		const md5BoneVal_s &f = baseFrame[i];
		w->writeText("\t( %f %f %f ) ( %f %f %f )\n",
			f.pos[0],f.pos[1],f.pos[2],f.quat[0],f.quat[1],f.quat[2]);
	}
	w->writeText("}\n");
	for(u32 i = 0; i < frames.size(); i++) {
		const md5Frame_c &f = frames[i];
		w->writeText("\n");
		w->writeText("frame %i {\n",i);
		for(u32 j = 0; j < f.components.size(); j++) {
			if(j % 6 == 0) {
				if(j != 0)
					w->writeText("\n");
				w->writeText("\t");
			} else {
				w->writeText(" ");
			}
			w->writeText("%f",f.components[j]);
		}
		w->writeText("\n");
		w->writeText("}\n");
	}
	w->writeText("\n");
	g_core->Print("Wrote md5anim %s\n",fname);
	return false;
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

	if(0) {
		str testName = fname;
		testName.stripExtension();
		testName.append("_rewrite.md5anim");
		writeMD5Anim(testName);
	}

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
void skelAnimMD5_c::buildFrameBonesLocal(u32 frameNum, class boneOrArray_c &out, const class skelModelAPI_i *skelModel) const {
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
bool skelAnimMD5_c::convertToMD5Anim(const char *outPath) {
	// it is actually a md5 anim, so just save
	return writeMD5Anim(outPath);
}
void skelAnimMD5_c::scaleAnimation(float s) {

}
void skelAnimMD5_c::buildLoopAnimLerpFrameBonesLocal(const struct singleAnimLerp_s &lerp, class boneOrArray_c &out, const class skelModelAPI_i *skelModel) const {
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