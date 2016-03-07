/*
============================================================================
Copyright (C) 2016 V.

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
// MoHAA .skd (skeletal model) file format
#ifndef __SKD_FILE_FORMAT_H__
#define __SKD_FILE_FORMAT_H__

#define SKD_IDENT			(('D'<<24)+('M'<<16)+('K'<<8)+'S')
#define SKD_VERSION			5
#define SKD_VERSION_SH		6
#define	SKD_MAX_BONES		128
#define SKD_SURFACE_IDENT	((' '<<24)+('L'<<16)+('K'<<8)+'S')

struct skdHitbox_s {
	int			boneIndex;
};

struct skdWeight_s {
	int			boneIndex;
	float		boneWeight;
	float		offset[3];
};

struct skdMorph_s {
	int			index;
	float		offset[3];
};

struct skdVertex_s {
	float		normal[3];
	float		texCoords[2];
	int			numWeights;
	int			numMorphs;

	const skdWeight_s *pWeight(int w) const {
		return (const skdWeight_s*)(((const byte*)this) + sizeof(*this) + numMorphs * sizeof(skdMorph_s) + sizeof(skdWeight_s) * w);
	}
	const skdMorph_s *pMorph(int m) const {
		return (const skdMorph_s*)(((const byte*)this) + sizeof(*this) + m * sizeof(skdMorph_s));
	}
	const skdVertex_s *getNextVert() const {
		return (const skdVertex_s*)(((const byte*)this) + sizeof(*this) + numMorphs * sizeof(skdMorph_s) + sizeof(skdWeight_s) * numWeights);
	}
};

struct skdTriangle_s {
	int			indexes[3];
};

struct skdSurface_s {
	int			ident;

	char		name[64];
	
	int			numTriangles;
	int			numVerts;
	int			staticSurfProcessed;
	int			ofsTriangles;
	int			ofsVerts;
	int			ofsCollapse;

	int			ofsEnd;

	int			ofsCollapseIndex;

	skdSurface_s() {
		clear();
	}
	void clear() {
		memset(this,0,sizeof(skdSurface_s));
	}
	const u32 *pIndices() const {
		return (const u32*)(((byte*)this) + ofsTriangles);
	}
	// indices of vertices
	const u32 *pCollapseMap() const {
		return (const u32*)(((byte*)this) + ofsCollapse);
	}
	// indices of indices
	const u32 *pCollapseIndexMap() const {
		return (const u32*)(((byte*)this) + ofsCollapseIndex);
	}
	skdVertex_s *pVerts() const {
		return (skdVertex_s*)(((byte*)this) + ofsVerts);
	}
};

enum skdJointType_e {
	JT_ROT,
	JT_POSROT,
	JT_SHOULDER,
	JT_ELBOW,
	JT_WRIST,
	JT_HOSEROT,
	JT_AVROT,
		JT_ZERO,
		JT_NUMBONETYPES,
		JT_WORLD,
	JT_HOSEROTBOTH,
	JT_HOSEROTPARENT
};

struct skdHoseRot_s {
	float bendRatio;
	float bendMax;
	float spinRatio;
	float constOffset[3];
	float scale[3]; // 1.f 1.f 1.f
	int hoseRotType;
};
struct skdShoulder_s {
	float unk[4];
	float offset[3]; // const offset
	float scale[3]; // 1.f , 1.f , 1.f
};
struct skdBone_s {
	char		name[32];
	char		parent[32];
	int			jointType;
	int			ofsValues;
	int			ofsChannels;
	int			ofsRefs;
	int			ofsEnd;

	skdBone_s() {
		clear();
	}
	void clear() {
		memset(this,0,sizeof(skdBone_s));
	}
	const char *pChannel(int i) const {
		if(ofsChannels >= ofsRefs)
			return 0;
		if(i == 0) {
			return (const char *)(((byte*)this)+ofsChannels);
		} else if(i == 1) {
			const char *prevChannel = pChannel(0);
			if(prevChannel == 0 || prevChannel[0] == 0)
				return 0;
			int pcLen = strlen(prevChannel);
			u32 ofsSecondChannel = ofsChannels + pcLen + 1;
			if(ofsSecondChannel >= ofsRefs) {
				return 0;
			}
			return (const char *)(((byte*)this)+ofsSecondChannel); // +1 to skip terminating zero
		}

		printf("skdBone_s::pChannel: bad index %i\n",i);
		return 0;
	}
	const char *pRef(int i) const {
		if(ofsRefs >= ofsEnd)
			return 0;
		if(i == 0) {
			return (const char *)(((byte*)this)+ofsRefs);
		} else if(i == 1) {
			const char *prevRef= pRef(0);
			if(prevRef == 0 || prevRef[0] == 0)
				return 0;
			int prLen = strlen(prevRef);
			u32 ofsSecondRef = ofsRefs + prLen + 1;
			if(ofsSecondRef >= ofsEnd) {
				return 0;
			}
			return (const char *)(((byte*)this)+ofsSecondRef); // +1 to skip terminating zero
		}

		printf("skdBone_s::pRef: bad index %i\n",i);
		return 0;
	}
	float getBone2Weight() const {
		if(this->jointType != JT_AVROT)
			return 0;
		return *(const float*)(((byte*)this)+ofsValues);
	}
	const float *getConstOffset() const {
		if(this->jointType == JT_POSROT)
			return 0;
		u32 offset = this->ofsValues;
		if(this->jointType == JT_SHOULDER) {
			offset += 16;
		} else if(this->jointType == JT_AVROT) {
			offset += 4; // in AVROT there is a m_bone2weight before const_offset
		} else if(this->jointType == JT_HOSEROT) {
			offset += 12; // skip bendRatio, bendMax and spinRatio
		}
		return (const float*)(((byte*)this)+offset);
	}
	const skdHoseRot_s *getHoseRotExtra() const {
		if(this->jointType != JT_HOSEROT)
			return 0;
		return (const skdHoseRot_s*)(((byte*)this)+ofsValues);
	}
	const skdShoulder_s *getShoulderExtra() const {
		if(this->jointType != JT_SHOULDER)
			return 0;
		return (const skdShoulder_s*)(((byte*)this)+ofsValues);
	}
};

struct skdHeader_s {
	int			ident;
	int			version;

	char		name[64];	// model name

	int			numSurfaces;
	int			numBones;
	int			ofsBones;
	int			ofsSurfaces;

	int			ofsEnd;
	int			lodIndex[10];

	int			numBoxes;
	int			ofsBoxes;
	int			numMorphTargets;
	int			ofsMorphTargets;

	skdHeader_s() {
		clear();
	}
	void clear() {
		memset(this,0,sizeof(skdHeader_s));
	}
	const skdSurface_s *pSurf(int s) const {
		const skdSurface_s *out = (const skdSurface_s *)(((const byte*)this) + ofsSurfaces);
		while(s) {
			out = (const skdSurface_s*)(((const byte*)out) + out->ofsEnd);
			s--;
		}
		return out;
	}
	const skdBone_s *pBone(int b) const {
		const skdBone_s *out = (const skdBone_s *)(((const byte*)this) + ofsBones);
		while(b) {
			out = (const skdBone_s*)(((const byte*)out) + out->ofsEnd);
			b--;
		}
		return out;
	}
	// returns the local index of bone with given name. Returns -1 if bone is not found
	s32 boneNumForName(const char *bName) const {
		const skdBone_s *out = (const skdBone_s *)(((const byte*)this) + ofsBones);
		for(int i = 0; i < this->numBones; i++) {
			if(!stricmp(bName,out->name)) {
				return i;
			}
			out = (const skdBone_s*)(((const byte*)out) + out->ofsEnd);
		}
		return -1;
	}
};

#endif // __SKD_FILE_FORMAT_H__
