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
#define SRC_DISPINFO 26
#define SRC_DISP_VERTS 33
#define SRC_GAME_LUMP 35
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

struct srcDispSubNeigbour_s {
	unsigned short neigbourIndex;	// -1 if none
	unsigned char neighbourOrientation;
	unsigned char span;
	unsigned char neighbourSpan;
};

#define MAX_DISP_CORNER_NEIGHBORS	4

class srcDispNeigbours_s {
	srcDispSubNeigbour_s subNeighbors[2];
};
struct srcDispCornerNeigbours_s {
	unsigned short neighbours[MAX_DISP_CORNER_NEIGHBORS];	// indices of neighbors.
	unsigned char count;
};
struct srcDisplacement_s {
	float startPosition[3];		// start position used for orientation
	int dispVertStart;		// Index into LUMP_DISP_VERTS.
	int dispTriStart;		// Index into LUMP_DISP_TRIS.
	int power;			// power - indicates size of surface (2^power	1)
	int minTess;		// minimum tesselation allowed
	float smoothingAngle;		// lighting smoothing angle
	int	contents;		// surface contents
	unsigned short mapFace;		// Which map face this displacement comes from.
	int	lightmapAlphaStart;	// Index into ddisplightmapalpha.
	int	lightmapSamplePositionStart;	// Index into LUMP_DISP_LIGHTMAP_SAMPLE_POSITIONS.
	srcDispNeigbours_s edgeNeighbors[4];	// Indexed by NEIGHBOREDGE_ defines.
	srcDispCornerNeigbours_s cornerNeighbors[4];	// Indexed by CORNER_ defines.
	unsigned int allowedVerts[10];	// active verticies
};

struct srcDispVert_s {
	float vec[3];
	float dist;
	float alpha;
};

struct srcStaticProp_s {
	// v4
	float origin[3];
	float angles[3];
	unsigned short propType;	 // index into model name dictionary
	unsigned short firstLeaf;	 // index into leaf array
	unsigned short numLeafs;
	unsigned char solid;		 // solidity type
	unsigned char flags;
	int skin;		 // model skin numbers
	float fadeMinDist;
	float fadeMaxDist;
	float lightingOrigin[3];  // for lighting
	// since v5
	float forcedFadeScale; // fade distance scale
	//// v6 and v7 only
	//unsigned short minDXLevel;      // minimum DirectX version to be visible
	//unsigned short maxDXLevel;      // maximum DirectX version to be visible
 //       // since v8
	//unsigned char minCPULevel;
	//unsigned char maxCPULevel;
	//unsigned char minGPULevel;
	//unsigned char maxGPULevel;
 //       // since v7
 //       int diffuseModulation; // per instance color and alpha modulation
 //       // since v10
 //       float unknown; 
 //       // since v9
 //       bool disableX360;     // if true, don't show on XBox 360
};
struct srcStaticPropName_s {
	char text[128];
};
struct srcStaticPropNames_s {
	u32 count;
	srcStaticPropName_s names[64]; // count times

	const byte *getEnd() const {
		return (((const byte*)this)+sizeof(srcStaticPropName_s)*count+4);
	}
};
struct srcStaticPropLeafs_s {
	u32 count;
	short data[64];

	const byte *getEnd() const {
		return (((const byte*)this)+sizeof(short)*count+4);
	}
};
struct srcStaticPropsList_s {
	u32 count;
	srcStaticProp_s props[64];

	const byte *getFirstOffset() const {
		return ((const byte*)this)+4;
	}
};
struct srcGameLump_s {
	union { int iIdent; char ident[4]; };
	unsigned short flags;
	unsigned short version;
	int fileOfs;
	int fileLen;
};
struct srcGameLumpsHeader_s {
	int numSubLumps;	// number of game lumps
	srcGameLump_s subLumps[64];
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

	const srcDisplacement_s *getDisplacements() const {
		const srcLump_s &dl = this->getLumps()[SRC_DISPINFO];
		if(dl.fileLen == 0)
			return 0;
		//int dispInfoStructSize = sizeof(srcDisplacement_s);
		//if(dl.fileLen % dispInfoStructSize) {
		//	g_core->RedWarning("srcHeader_s::getDisplacements: invalid dispinfo lump size\n");
		//	return 0; // error
		//}
		const srcDisplacement_s *d = (const srcDisplacement_s *)this->getLumpData(SRC_DISPINFO);
		return d;
	}
	const srcDisplacement_s *getDisplacement(u32 index) const {
		const srcDisplacement_s *d = getDisplacements();
		if(d == 0)
			return 0;
		return d + index;
	}
	const srcDispVert_s *getDispVerts() const {
		const srcLump_s &dl = this->getLumps()[SRC_DISP_VERTS];
		if(dl.fileLen == 0)
			return 0;
		//int dispInfoStructSize = sizeof(srcDisplacement_s);
		//if(dl.fileLen % dispInfoStructSize) {
		//	g_core->RedWarning("srcHeader_s::getDisplacements: invalid dispinfo lump size\n");
		//	return 0; // error
		//}
		const srcDispVert_s *d = (const srcDispVert_s *)this->getLumpData(SRC_DISP_VERTS);
		return d;
	}
};



#endif // __BSPFILEFORMAT_HL2__

