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

#ifndef __MDM_FILE_FORMAT_H__
#define __MDM_FILE_FORMAT_H__

/*
==============================================================================

MDM file format (Wolfenstein Skeletal Mesh)

version history:
	2 - initial version
	3 - removed all frame data, this format is pure mesh and bone references now

==============================================================================
*/

#define MDM_IDENT           ( ( 'W' << 24 ) + ( 'M' << 16 ) + ( 'D' << 8 ) + 'M' )
#define MDM_VERSION			3

#define MDM_TRANSLATION_SCALE   ( 1.0 / 64 )

struct mdmWeight_t {
	int             boneIndex;	// these are indexes into the boneReferences,
	float           boneWeight;	// not the global per-frame bone list
	vec3_t          offset;
};

struct mdmVertex_t {
	vec3_t          normal;
	vec2_t          texCoords;
	int             numWeights;

	const mdmWeight_t *pWeight(u32 weightNum) const {
		return (const mdmWeight_t *)(((byte*)this)+sizeof(*this)+weightNum*sizeof(mdmWeight_t));
	}
	const mdmVertex_t *pNextVert() const {
		return (const mdmVertex_t *)(((byte*)this)+sizeof(*this)+this->numWeights*sizeof(mdmWeight_t));
	}
};

struct mdmTriangle_t {
	int             indexes[3];
};

struct mdmSurface_t {
	int             ident;

	char            name[64];	// polyset name
	char            shader[64];
	int             shaderIndex;	// for in-game use

	int             minLod;

	int             ofsHeader;	// this will be a negative number

	int             numVerts;
	int             ofsVerts;

	int             numTriangles;
	int             ofsTriangles;

	int             ofsCollapseMap;	// numVerts * int

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int             numBoneReferences;
	int             ofsBoneReferences;

	int             ofsEnd;		// next surface follows

	const mdmVertex_t *pFirstVert() const {
		return (const mdmVertex_t *)(((byte*)this)+ofsVerts);
	}
	const mdmTriangle_t *pTri(u32 triNum) const {
		return (const mdmTriangle_t *)(((byte*)this)+ofsTriangles+triNum*sizeof(mdmTriangle_t));
	}
};

// Tags always only have one parent bone
struct mdmTag_t {
	char            name[64];	// name of tag
	vec3_t          axis[3];

	int             boneIndex;
	vec3_t          offset;

	int             numBoneReferences;
	int             ofsBoneReferences;

	int             ofsEnd;		// next tag follows
};

struct mdmHeader_t {
	int             ident;
	int             version;

	char            name[64];	// model name

	float           lodScale;
	float           lodBias;

	int             numSurfaces;
	int             ofsSurfaces;

	// tag data
	int             numTags;
	int             ofsTags;

	int             ofsEnd;		// end of file


	const mdmSurface_t *pSurface(u32 surfNum) const {
		const mdmSurface_t *sf = (const mdmSurface_t *)(((byte*)this)+this->ofsSurfaces);
		while(surfNum) {
			sf = (const mdmSurface_t *)(((byte*)sf)+sf->ofsEnd);
			surfNum--;
		}
		return sf;
	}
};

#endif // __MDM_FILE_FORMAT_H__
