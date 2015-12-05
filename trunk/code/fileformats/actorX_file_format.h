/*
============================================================================
Copyright (C) 2010-2011 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2015 V.

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
// actorX_file_format.h - Unreal intermediae model format

/*
========================================================================

Actor X - .PSK / .PSA skeletal triangle model file format

========================================================================
*/

// main header ident
#define PSK_IDENT_HEADER	"ACTRHEAD"
// subsection idents
#define PSK_IDENT_POINTS	"PNTS0000"
#define PSK_IDENT_VERTS		"VTXW0000"
#define PSK_IDENT_FACES		"FACE0000"
#define PSK_IDENT_MATS		"MATT0000"
#define PSK_IDENT_REFS		"REFSKELT"
#define PSK_IDENT_WEIGHTS	"RAWWEIGHTS"
// present only in pskx files?
#define PSKX_IDENT_EXTRAUV	"EXTRAUV0"

// main animation header ident
#define PSA_IDENT_HEADER	"ANIMHEAD"
// animation subsections
#define PSA_IDENT_BONENAMES	"BONENAMES"
#define PSA_IDENT_INFO		"ANIMINFO"
#define PSA_IDENT_KEYS		"ANIMKEYS"

typedef struct
{
	float			point[3];
} axPoint_t;

typedef struct
{
	unsigned short	pointIndex;
	unsigned short	unknownA;
	float			st[2];
	byte			materialIndex;
	byte			reserved;		// we don't care about this one
	unsigned short	unknownB;
} axVertex_t;

typedef struct
{
	unsigned short	indexes[3];
	byte			materialIndex;
	byte			materialIndex2;
	unsigned int	smoothingGroups;
} axTriangle_t;

typedef struct
{
	char            name[64];
	int             shaderIndex;	// for in-game use
	unsigned int	polyFlags;
	int				auxMaterial;
	unsigned int	auxFlags;
	int				lodBias;
	int				lodStyle;
} axMaterial_t;

typedef struct
{
	float			quat[4];		// x y z w
	float			position[3];	// x y z

	float			length;
	float			xSize;
	float			ySize;
	float			zSize;
} axBone_t;

typedef struct
{
	char			name[64];
	unsigned int	flags;
	int				numChildren;
	int				parentIndex;
	axBone_t		bone;
} axReferenceBone_t;

typedef struct
{
	float			weight;
	unsigned int	pointIndex;
	unsigned int	boneIndex;
} axBoneWeight_t;


struct axAnimationInfo_s
{
	char			name[64];
	char			group[64];

	int				numBones;		// same as numChannels
	int				rootInclude;

	int				keyCompressionStyle;
	int				keyQuotum;
	float			keyReduction;

	float			trackTime;

	float			frameRate;

	int				startBoneIndex;

	int				firstRawFrame;
	int				numRawFrames;
};

struct axAnimationKey_t
{
	float			position[3];
	float			quat[4];
	float			time;
};

typedef struct axChunkHeader_s
{
	char            ident[20];
	int             flags;

	int				dataSize;	// sizeof(struct)
	int				numData;	// number of structs put into this data chunk

	// general usage
	bool hasIdent(const char *str) const {
		u32 testLen = strlen(str);
		if(memcmp(str,this->ident,testLen)) {
			return false;
		}
		return true;
	}
	u32 getTotalDataLen() const {
		return dataSize * numData;
	}
	// V: for PSK files
	const axChunkHeader_s *getNextHeader() const {
		return (const axChunkHeader_t*)(((byte*)this) + sizeof(*this) + (dataSize*numData) );
	}
	inline const axPoint_t *getPoint(u32 pointNum) const {
		return (const axPoint_t*)(((byte*)this) + sizeof(*this) + sizeof(axPoint_t)*pointNum);
	}
	inline const axVertex_t *getVertex(u32 vertNum) const {
		return (const axVertex_t*)(((byte*)this) + sizeof(*this) + sizeof(axVertex_t)*vertNum);
	}
	inline const axTriangle_t *getTri(u32 triNum) const {
		return (const axTriangle_t*)(((byte*)this) + sizeof(*this) + sizeof(axTriangle_t)*triNum);
	}
	inline const axMaterial_t *getMat(u32 matNum) const {
		return (const axMaterial_t*)(((byte*)this) + sizeof(*this) + sizeof(axMaterial_t)*matNum);
	}
	inline const axReferenceBone_t *getBone(u32 boneNum) const {
		return (const axReferenceBone_t*)(((byte*)this) + sizeof(*this) + sizeof(axReferenceBone_t)*boneNum);
	}
	inline const axBoneWeight_t *getWeight(u32 weightNum) const {
		return (const axBoneWeight_t*)(((byte*)this) + sizeof(*this) + sizeof(axBoneWeight_t)*weightNum);
	}

	// V: returns all the weight referring to given point index
	inline const u32 getPointWeights(u32 pointNum, arraySTD_c<const axBoneWeight_t*> &outList) const {
		for(u32 i = 0; i < this->numData; i++) {
			 const axBoneWeight_t *checkW = this->getWeight(i);
			 if(checkW->pointIndex == pointNum) {
				 outList.push_back(checkW);
			 }
		}
		return outList.size();
	}
	// V: for PSA files
	const struct axAnimationInfo_s *getAnimInfo(u32 animNum) const {
		return (const axAnimationInfo_s*)(((byte*)this) + sizeof(*this) + sizeof(axAnimationInfo_s)*animNum);
	}
	const axAnimationKey_t *getAnimKeyInternal(u32 absKeyIndex) const {
		return (const axAnimationKey_t*)(((byte*)this) + sizeof(*this) + sizeof(axAnimationKey_t)*absKeyIndex);
	}
	const axAnimationKey_t *getAnimKey(u32 absFrameIndex, u32 numBones, u32 keyIndex) const {
		u32 absKeyIndex = absFrameIndex * numBones + keyIndex;
		return this->getAnimKeyInternal(absKeyIndex);
	}
} axChunkHeader_t;


