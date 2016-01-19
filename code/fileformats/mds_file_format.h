/*
============================================================================
Copyright (C) 2010 V.

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

#ifndef __MDS_FILE_FORMAT_H__
#define __MDS_FILE_FORMAT_H__

/*
==============================================================================

MDS file format (Wolfenstein Skeletal Format)

==============================================================================
*/

#define MDS_IDENT           ( ( 'W' << 24 ) + ( 'S' << 16 ) + ( 'D' << 8 ) + 'M' )
#define MDS_VERSION         4
// These are internal RTCW engine limits.
//#define MDS_MAX_VERTS       6000
//#define MDS_MAX_TRIANGLES   8192
//#define MDS_MAX_BONES       128
//#define MDS_MAX_SURFACES    32
//#define MDS_MAX_TAGS        128

#define MDS_TRANSLATION_SCALE   ( 1.0 / 64 )
#define MDS_ROTATION_SCALE   ( 360.0 / 65536.0 )

struct mdsWeight_t {
	int boneIndex;              // these are indexes into the boneReferences,
	float boneWeight;           // not the global per-frame bone list
	vec3_t offset;
};

struct mdsVertex_t {
	vec3_t normal;
	vec2_t texCoords;
	int numWeights;
	int fixedParent;            // stay equi-distant from this parent
	float fixedDist;

	const mdsWeight_t *pWeight(u32 weightNum) const {
		return (const mdsWeight_t *)(((byte*)this)+sizeof(*this)+weightNum*sizeof(mdsWeight_t));
	}
	const mdsVertex_t *pNextVert() const {
		return (const mdsVertex_t *)(((byte*)this)+sizeof(*this)+this->numWeights*sizeof(mdsWeight_t));
	}
};

struct mdsTriangle_t {
	int indexes[3];
};

struct mdsSurface_t {
	int ident;

	char name[64];           // polyset name
	char shader[64];
	int shaderIndex;                // for in-game use

	int minLod;

	int ofsHeader;                  // this will be a negative number

	int numVerts;
	int ofsVerts;

	int numTriangles;
	int ofsTriangles;

	int ofsCollapseMap;           // numVerts * int

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int numBoneReferences;
	int ofsBoneReferences;

	int ofsEnd;                     // next surface follows

	const mdsVertex_t *pFirstVert() const {
		return (const mdsVertex_t *)(((byte*)this)+ofsVerts);
	}
	const mdsTriangle_t *pTri(u32 triNum) const {
		return (const mdsTriangle_t *)(((byte*)this)+ofsTriangles+triNum*sizeof(mdsTriangle_t));
	}
};

struct mdsBoneFrameCompressed_t {
	short angles[4];            // to be converted to axis at run-time (this is also better for lerping)
	short ofsAngles[2];         // PITCH/YAW, head in this direction from parent to go to the offset position

	vec3_c getAngles() const {
		vec3_c ret;
		ret.x = double(angles[0]) * MDS_ROTATION_SCALE;
		ret.y = double(angles[1]) * MDS_ROTATION_SCALE;
		ret.z = double(angles[2]) * MDS_ROTATION_SCALE;
		return ret;
	}
	vec3_c getOfsAngles() const {
		vec3_c ret;
		ret.x = double(ofsAngles[0]) * MDS_ROTATION_SCALE;
		ret.y = double(ofsAngles[1]) * MDS_ROTATION_SCALE;
		ret.z = 0;
		return ret;
	}
};

struct mdsFrame_t {
	vec3_t bounds[2];               // bounds of all surfaces of all LOD's for this frame
	vec3_t localOrigin;             // midpoint of bounds, used for sphere cull
	float radius;                   // dist from localOrigin to corner
	vec3_t parentOffset;            // one bone is an ascendant of all other bones, it starts the hierachy at this position

	const mdsBoneFrameCompressed_t *pBone(u32 boneNum) const {
		return (mdsBoneFrameCompressed_t*)(((byte*)this)+sizeof(*this)+sizeof(mdsBoneFrameCompressed_t)*boneNum);
	}
};

struct mdsTag_t {
	char name[64];           // name of tag
	float torsoWeight;
	int boneIndex;                  // our index in the bones
};

#define BONEFLAG_TAG        1       // this bone is actually a tag

struct mdsBoneInfo_t {
	char name[64];           // name of bone
	int parent;                     // not sure if this is required, no harm throwing it in
	float torsoWeight;              // scale torso rotation about torsoParent by this
	float parentDist;
	int flags;
};

struct mdsHeader_t {
	int ident;
	int version;

	char name[64];           // model name

	float lodScale;
	float lodBias;

	// frames and bones are shared by all levels of detail
	int numFrames;
	int numBones;
	int ofsFrames;                  // mdsFrame_t[numFrames]
	int ofsBones;                   // mdsBoneInfo_t[numBones]
	int torsoParent;                // index of bone that is the parent of the torso

	int numSurfaces;
	int ofsSurfaces;

	// tag data
	int numTags;
	int ofsTags;                    // mdsTag_t[numTags]

	int ofsEnd;                     // end of file

	

	const mdsSurface_t *pSurface(u32 surfNum) const {
		const mdsSurface_t *sf = (const mdsSurface_t *)(((byte*)this)+this->ofsSurfaces);
		while(surfNum) {
			sf = (const mdsSurface_t *)(((byte*)sf)+sf->ofsEnd);
			surfNum--;
		}
		return sf;
	}
	const mdsBoneInfo_t *pBone(u32 boneNum) const {
		return (const mdsBoneInfo_t *)(((byte*)this)+this->ofsBones+boneNum*sizeof(mdsBoneInfo_t));
	}
	const mdsFrame_t *pFrame(u32 frameNum) const {
		return (const mdsFrame_t *)(((byte*)this)+this->ofsFrames+frameNum*(sizeof(mdsFrame_t)+this->numBones*sizeof(mdsBoneFrameCompressed_t)));
	}
};

#endif // __MDS_FILE_FORMAT_H__
