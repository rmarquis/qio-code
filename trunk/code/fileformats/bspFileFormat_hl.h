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
// bspFileFormat_hl.h - HL-specific .bsp file structures

#ifndef __BSPFILEFORMAT_HL__
#define __BSPFILEFORMAT_HL__

// HL bsps has no ident field,
// version field is at the beginning of file
#define BSP_VERSION_HL 30
// Quake1 bsp is similiar to HL
#define BSP_VERSION_QUAKE1 29

#define	HL_ENTITIES		0
#define	HL_PLANES		1
#define	HL_TEXTURES		2
#define	HL_VERTEXES		3
#define	HL_VISIBILITY	4
#define	HL_NODES		5
#define	HL_TEXINFO		6
#define	HL_FACES		7
#define	HL_LIGHTING		8
#define	HL_CLIPNODES	9
#define	HL_LEAFS		10
#define	HL_MARKSURFACES 11
#define	HL_EDGES		12
#define	HL_SURFEDGES	13
#define	HL_MODELS		14

#define	HL_LUMPS	15

#pragma pack(push, 1)

struct hlModel_s
{
	float		mins[3], maxs[3];
	float		origin[3];
	int			headnode[4]; // it's 8 for Hexen
	int			visLeafs; // not including the solid leaf 0
	int			firstSurface, numSurfaces;
};
#define HL_CONTENTS_TRANSLUCENT	-15

struct hlNode_s
{
	int			planeNum;
	// NOTE: in Q2 those children indexes are 32 bit integers (not 16 bit shorts)
	short		children[2];	// negative numbers are -(leafs+1), not nodes
	short		mins[3];		// for sphere culling
	short		maxs[3];
	// used only for collision detection
	unsigned short	firstFace;
	unsigned short	numFaces;	// counting both sides
};

// leaf 0 is the generic CONTENTS_SOLID leaf, used for all solid areas
// all other leafs need visibility info
struct hlLeaf_s
{
	int			contents;
	int			visOfs;				// -1 = no visibility info

	short		mins[3];			// for frustum culling
	short		maxs[3];

	unsigned short		firstMarkSurface;
	unsigned short		numMarkSurfaces;

	byte		ambientLevel[4]; // automatic ambient sounds
};

struct hlMipTexLump_s
{
	int			numMipTex;
	int			dataofs[4];		// [numMipTex]
};

struct hlTexInfo_s
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			miptex;
	int			flags;
};

struct hlMipTex_s
{
	char		name[16];
	unsigned	width, height;
	unsigned	offsets[4];		// four mip maps stored
};

#define	TEX_SPECIAL		1		// sky or slime, no lightmap or 256 subdivision

// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
struct hlEdge_s
{
	unsigned short	v[2];		// vertex numbers
};

struct hlVert_s
{
	float	point[3];
};

struct hlSurface_s
{
	short		planeNum;
	short		side;

	int			firstEdge;		// we must support > 64k edges
	short		numEdges;	
	short		texInfo;

// lighting info
	byte		styles[4];
	int			lightOfs;		// start of [numstyles*surfsize] samples
};

#pragma pack(pop)

#endif // __BSPFILEFORMAT_HL__

