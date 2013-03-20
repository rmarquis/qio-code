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
// bspFileFormat_hl2.h - SourceEngine (Half Life2, Portal, etc) .bsp file structures

#ifndef __BSPFILEFORMAT_HL2__
#define __BSPFILEFORMAT_HL2__

#define BSP_IDENT_VBSP	(('P'<<24)+('S'<<16)+('B'<<8)+'V')
// older hl2 bsps
#define BSP_VERSION_HL2_18	18
// HalfLife2
#define BSP_VERSION_HL2_19	19
// Portal1
#define BSP_VERSION_HL2_20	20

#define SRC_ENTITIES 0
#define SRC_PLANES 1
#define SRC_TEXDATA 2
#define SRC_VERTEXES 3
#define SRC_NODES 5
#define SRC_TEXINFO 6
#define SRC_FACES 7
#define SRC_LIGHTING 8
#define SRC_LEAFS 10
#define SRC_EDGES 12
#define SRC_SURFEDGES 13
#define SRC_MODELS 14
#define SRC_LEAFFACES 16
#define SRC_LEAFBRUSHES 17
#define SRC_BRUSHES 18
#define SRC_BRUSHSIDES 19
#define SRC_TEXDATA_STRING_DATA 43
#define SRC_TEXDATA_STRING_TABLE 44
#define SRC_LUMPS 64

#pragma pack(push, 1)

struct srcLump_s {
	int	fileOfs;	// offset into file (bytes)
	int	fileLen;	// length of lump (bytes)
	int	version;	// lump format version
	char fourCC[4];	// lump ident code
};

struct srcHeader_s {
	int	ident;                // BSP file identifier
	int	version;              // BSP file version
	srcLump_s lumps[SRC_LUMPS];  // lump directory array
	int	mapRevision;          // the map's revision (iteration, version) number'

	const srcLump_s *getLumps() const {
		return &lumps[0];
	}
	const byte *getLumpData(u32 lumpNum) const {
		return (const byte*)(((const byte*)this)+lumps[lumpNum].fileOfs);
	}
};

struct srcNode_s {
	int		planeNum;	// index into plane array
	int		children[2];	// negative numbers are -(leafs 1), not nodes
	short		mins[3];	// for frustom culling
	short		maxs[3];
	unsigned short	firstFace;	// index into face array
	unsigned short	numFaces;	// counting both sides
	short		area;		// If all leaves below this node are in the same area, then
					// this is the area index. If not, this is -1.
	short		paddding;	// pad to 32 bytes length
};

struct srcCompressedLightCube_s {
	byte dummy[24];
};

// for bsp version != 19
struct srcLeaf_noLightCube_s
{
	int			contents;		// OR of all brushes (not needed?)
	short			cluster;		// cluster this leaf is in
	short			area:9;			// area this leaf is in
	short			flags:7;		// flags
	short			mins[3];		// for frustum culling
	short			maxs[3];
	unsigned short		firstLeafSurface;		// index into leaffaces
	unsigned short		numLeafSurfaces;
	unsigned short		firstLeafBrush;		// index into leafbrushes
	unsigned short		numLeafBrushes;
	short			leafWaterDataID;	// -1 for not in water
	short padding; // to 32 boundary
};

// for bsp version == 19
struct srcLeaf_s
{
	int			contents;		// OR of all brushes (not needed?)
	short			cluster;		// cluster this leaf is in
	short			area:9;			// area this leaf is in
	short			flags:7;		// flags
	short			mins[3];		// for frustum culling
	short			maxs[3];
	unsigned short		firstLeafSurface;		// index into leaffaces
	unsigned short		numLeafSurfaces;
	unsigned short		firstLeafBrush;		// index into leafbrushes
	unsigned short		numLeafBrushes;
	short			leafWaterDataID;	// -1 for not in water
	srcCompressedLightCube_s	ambientLighting;	// Precaculated light info for entities.
	short			padding;		// padding to 4-byte boundary
};

struct srcBrushSide_s {
	unsigned short	planeNum;	// facing out of the leaf
	short		texInfo;	// texture info
	short		dispInfo;	// displacement info
	short		bevel;		// is the side a bevel plane?
};


struct srcEdge_s {
	unsigned short	v[2];	// vertex indices
};

struct srcSurface_s {
	unsigned short	planeNum;		// the plane number
	byte		side;			// faces opposite to the node's plane direction
	byte		onNode;			// 1 of on node, 0 if in leaf
	int		firstEdge;		// index into surfedges
	short		numEdges;		// number of surfedges
	unsigned short		texInfo;		// texture info
	short		dispInfo;		// displacement info
	short		surfaceFogVolumeID;	// ?
	byte		styles[4];		// switchable lighting info
	int		lightOfs;		// offset into lightmap lump
	float		area;			// face area in units^2
	int		lightmapTextureMinsInLuxels[2];	// texture lighting info
	int		lightmapTextureSizeInLuxels[2];	// texture lighting info
	int		origFace;		// original face this was split from
	unsigned short	numPrims;		// primitives
	unsigned short	firstPrimID;
	unsigned int	smoothingGroups;	// lightmap smoothing group
};
struct srcSurfaceV18_s {
	byte dummy[4][4]; // looks like color (for 4 lightmaps)
	srcSurface_s s;
};

struct srcTexInfo_s
{
	float	textureVecs[2][4];	// [s/t][xyz offset]
	float	lightmapVecs[2][4];	// [s/t][xyz offset] - length is in units of texels/area
	int	flags;			// miptex flags	overrides
	int	texData;		// Pointer to texture name, size, etc.
};

struct srcTexData_s
{
	vec3_t	reflectivity;		// RGB reflectivity
	int	nameStringTableID;	// index into TexdataStringTable
	int	width, height;		// source image
	int	viewWidth, viewHeight;
};

struct srcVert_s {
	float	point[3];
};

#pragma pack(pop)

#endif // __BSPFILEFORMAT_HL2__

