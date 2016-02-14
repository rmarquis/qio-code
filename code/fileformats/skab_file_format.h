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
#ifndef __SKAB_FILE_FORMAT_H__
#define __SKAB_FILE_FORMAT_H__

/*
==============================================================================

SKB file format

==============================================================================
*/

// applies to both skbHeader_s and skbSurface_s
#define SKB_IDENT	((' '<<24)+('L'<<16)+('K'<<8)+'S')
#define SKB_VERSION	3
// Elite Force 2 skb's
#define SKB_VERSION_EF2 4

#define SKAB_BONEFLAG_LEG     1

struct skbWeight_s {
	int		boneIndex;
	float	boneWeight;
	float	offset[3];
};

struct skbVertex_s {
	float	normal[3];
	float	texCoords[2];
	int		numWeights;

	const skbWeight_s *pWeight(int w) const {
		return (const skbWeight_s*)(((const byte*)this) + sizeof(*this) + sizeof(skbWeight_s) * w);
	}
	const skbVertex_s *getNextVert() const {
		return (const skbVertex_s*)(((const byte*)this) + sizeof(*this) + sizeof(skbWeight_s) * numWeights);
	}
};

struct skbTriangle_s {
	int 	         indexes[ 3 ];
};

struct skbSurface_s {
	int		ident;
	char	name[64];	// polyset name
	int		numTriangles;
	int		numVerts;
	int		minLod;
	int		ofsTriangles;
	int		ofsVerts;
	int		ofsCollapse;
	int		ofsEnd;     // next surface follows

	const u32 *pIndices() const {
		return (const u32*)(((byte*)this) + ofsTriangles);
	}
	skbVertex_s *pVerts() const {
		return (skbVertex_s*)(((byte*)this) + ofsVerts);
	}
};

struct skbBone_s {
	int		parent;
	int		flags;
	char	name[64];	// bone name
};

struct skbHeader_s {	    
	int		ident;
	int		version;
	char	name[64];	// model name

	int	  numSurfaces;
	int	  numBones;

	int	  ofsBones;
	int	  ofsSurfaces;
	int	  ofsEnd;

	const skbSurface_s *pSurf(int s) const {
		const skbSurface_s *out = (const skbSurface_s *)(((const byte*)this) + ofsSurfaces);
		while(s) {
			out = (const skbSurface_s*)(((const byte*)out) + out->ofsEnd);
			s--;
		}
		return out;
	}
	const skbBone_s *pBone(int b) const {
		const skbBone_s *out = (const skbBone_s *)(((const byte*)this) + ofsBones + sizeof(skbBone_s) * b);
		return out;
	}
	// returns the local index of bone with given name. Returns -1 if bone is not found
	s32 boneNumForName(const char *bName) const {
		const skbBone_s *out = (const skbBone_s *)(((const byte*)this) + ofsBones);
		for(int i = 0; i < this->numBones; i++) {
			if(!stricmp(bName,out->name)) {
				return i;
			}
			out++;
		}
		return -1;
	}
};

struct skbHeader4_s {	    
	int		ident;
	int		version;
	char	name[64];	// model name

	int	  numSurfaces;
	int	  numBones;

	int	  ofsBones;
	int	  ofsSurfaces;
	int   ofsBaseFrame; // added for V4 (Elite Force)
	int	  ofsEnd;

	const skbSurface_s *pSurf(int s) const {
		const skbSurface_s *out = (const skbSurface_s *)(((const byte*)this) + ofsSurfaces);
		while(s) {
			out = (const skbSurface_s*)(((const byte*)out) + out->ofsEnd);
			s--;
		}
		return out;
	}
	const skbBone_s *pBone(int b) const {
		const skbBone_s *out = (const skbBone_s *)(((const byte*)this) + ofsBones + sizeof(skbBone_s) * b);
		return out;
	}
	// returns the local index of bone with given name. Returns -1 if bone is not found
	s32 boneNumForName(const char *bName) const {
		const skbBone_s *out = (const skbBone_s *)(((const byte*)this) + ofsBones);
		for(int i = 0; i < this->numBones; i++) {
			if(!stricmp(bName,out->name)) {
				return i;
			}
			out++;
		}
		return -1;
	}
};


/*
==============================================================================

SKA file format

==============================================================================
*/

#define SKA_IDENT	(('N'<<24)+('A'<<16)+('K'<<8)+'S')
#define SKA_VERSION		3
#define SKA_VERSION_EF2	4

#define SKAB_ANIM_NORMAL      0
#define SKAB_ANIM_NO_OFFSETS  1

#define SKA_BONE_OFFSET_MANTISSA_BITS ( 9 )
#define SKA_BONE_OFFSET_MAX_SIGNED_VALUE ( ( 1 << SKA_BONE_OFFSET_MANTISSA_BITS ) - 1 )
#define SKA_BONE_OFFSET_SIGNED_SHIFT ( 15 - ( SKA_BONE_OFFSET_MANTISSA_BITS ) )
#define SKA_BONE_OFFSET_MULTIPLIER ( ( 1 << ( SKA_BONE_OFFSET_SIGNED_SHIFT ) ) - 1 )
#define SKA_BONE_OFFSET_MULTIPLIER_RECIPROCAL ( ( 1.0f ) / ( SKA_BONE_OFFSET_MULTIPLIER ) )

#define SKA_BONE_QUAT_FRACTIONAL_BITS ( 15 )
#define SKA_BONE_QUAT_MULTIPLIER ( ( 1 << ( SKA_BONE_QUAT_FRACTIONAL_BITS ) ) - 1 )
#define SKA_BONE_QUAT_MULTIPLIER_RECIPROCAL ( ( 1.0f ) / ( SKA_BONE_QUAT_MULTIPLIER ) )


struct skaBone_s {
	short shortQuat[4];
	short shortOffset[3];
	short padding_do_not_use;
};

struct skaFrame_s {
	float	bounds[2][3];
	float	radius;  // dist to corner
	float	delta[3];

	const skaBone_s *pBone(u32 b) const {
		return (const skaBone_s *)(((byte*)this) + sizeof(*this) + sizeof(skaBone_s) * b);
	}
};

struct skaHeader_s {
	int		ident;
	int		version; // 3
	char	name[64];   // anim name

	int		type;
	int		numFrames;
	int		numBones;
	float	totaltime;
	float	frametime;
	float	totaldelta[3];
	int		ofsFrames;

	const skaFrame_s *pFrame(int f) const {
		return (const skaFrame_s*)(((const byte*)this) + ofsFrames + f * (sizeof(skaFrame_s) + sizeof(skaBone_s)*numBones));
	}
};

struct skaHeader4_s {
	int		ident;
	int		version; // 4
	char	name[64];   // anim name

	int		type;
	int		numFrames;
	int		numBones;
	float	totaltime;
	float	frametime;
	float	totaldelta[3];
	int		ofsBoneRefs; // added for SKA version 4 (Elite Force)
	int		ofsFrames;

	const skaFrame_s *pFrame(int f) const {
		return (const skaFrame_s*)(((const byte*)this) + ofsFrames + f * (sizeof(skaFrame_s) + sizeof(skaBone_s)*numBones));
	}
};
#endif // __SKAB_FILE_FORMAT_H__
