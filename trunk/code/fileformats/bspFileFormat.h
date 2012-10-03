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
// bspFileFormat.h - structures used in binary .bsp files

#ifndef __BSP_QUAKE3__
#define __BSP_QUAKE3__

#include <shared/typedefs.h>

// original BSP structures designed by ID Software
// used in their Quake3 game
#define BSP_IDENT_IBSP	(('P'<<24)+('S'<<16)+('B'<<8)+'I')

#define BSP_VERSION_Q3		46
#define BSP_VERSION_RTCW	47
#define BSP_VERSION_ET	47

#define	Q3_ENTITIES		0
#define	Q3_SHADERS		1
#define	Q3_PLANES			2
#define	Q3_NODES			3
#define	Q3_LEAVES			4
#define	Q3_LEAFSURFACES	5
#define	Q3_LEAFBRUSHES	6
#define	Q3_MODELS			7
#define	Q3_BRUSHES		8
#define	Q3_BRUSHSIDES		9
#define	Q3_DRAWVERTS		10
#define	Q3_DRAWINDEXES	11
#define	Q3_FOGS			12
#define	Q3_SURFACES		13
#define	Q3_LIGHTMAPS		14
#define	Q3_LIGHTGRID		15
#define	Q3_VISIBILITY		16
#define	Q3_LUMPS		17

struct q3Model_s {
	float		mins[3], maxs[3];
	int			firstSurface, numSurfaces;
	int			firstBrush, numBrushes;
};

struct q3BSPMaterial_s {
	char		shader[64];
	int			surfaceFlags;
	int			contentFlags;
};

// planes x^1 is allways the opposite of plane x

struct q3Plane_s {
	float		normal[3];
	float		dist;
};

struct q3Node_s {
	int			planeNum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	int			mins[3];		// for frustom culling
	int			maxs[3];
};

struct q3Leaf_s {
	int			cluster;			// -1 = opaque cluster (do I still store these?)
	int			area;

	int			mins[3];			// for frustum culling
	int			maxs[3];

	int			firstLeafSurface;
	int			numLeafSurfaces;

	int			firstLeafBrush;
	int			numLeafBrushes;
};

struct q3BrushSide_s {
	int			planeNum;			// positive plane side faces out of the leaf
	int			materialNum;
};

struct q3Brush_s {
	int			firstSide;
	int			numSides;
	int			materialNum;		// the shader that determines the contents flags
};

struct q3Fog_s {
	char		shader[64];
	int			brushNum;
	int			visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
};

struct q3Vert_s {
	vec3_t		xyz;
	float		st[2];
	float		lightmap[2];
	vec3_t		normal;
	byte		color[4];
};

enum q3mapSurfaceType_e {
	Q3MST_BAD,
	Q3MST_PLANAR,
	Q3MST_PATCH,
	Q3MST_TRIANGLE_SOUP,
	Q3MST_FLARE
};

struct q3Surface_s {
	int			materialNum;
	int			fogNum;
	int			surfaceType;

	int			firstVert;
	int			numVerts;

	int			firstIndex;
	int			numIndexes;

	int			lightmapNum;
	int			lightmapX, lightmapY;
	int			lightmapWidth, lightmapHeight;

	vec3_t		lightmapOrigin;
	vec3_t		lightmapVecs[3];	// for patches, [0] and [1] are lodbounds

	int			patchWidth;
	int			patchHeight;
};

struct lump_s {
	u32 fileOfs;
	u32 fileLen;
};

struct q3Header_s {
	int			ident;
	int			version;
	lump_s		lumps[Q3_LUMPS];

	const q3Model_s *getModels() const {
		return (const q3Model_s*)(((const byte*)this)+lumps[Q3_MODELS].fileOfs);
	}
	const q3BSPMaterial_s *getMaterials() const {
		return (const q3BSPMaterial_s*)(((const byte*)this)+lumps[Q3_SHADERS].fileOfs);
	}
	const q3Plane_s *getPlanes() const {
		return (const q3Plane_s*)(((const byte*)this)+lumps[Q3_PLANES].fileOfs);
	}
	const q3Node_s *getNodes() const {
		return (const q3Node_s*)(((const byte*)this)+lumps[Q3_NODES].fileOfs);
	}
	const q3Leaf_s *getLeaves() const {
		return (const q3Leaf_s*)(((const byte*)this)+lumps[Q3_LEAVES].fileOfs);
	}
	const q3BrushSide_s *getBrushSides() const {
		return (const q3BrushSide_s*)(((const byte*)this)+lumps[Q3_BRUSHSIDES].fileOfs);
	}
	const q3Brush_s *getBrushes() const {
		return (const q3Brush_s*)(((const byte*)this)+lumps[Q3_BRUSHES].fileOfs);
	}
	const q3Vert_s *getVerts() const {
		return (const q3Vert_s*)(((const byte*)this)+lumps[Q3_DRAWVERTS].fileOfs);
	}
	const q3Surface_s *getSurfaces() const {
		return (const q3Surface_s*)(((const byte*)this)+lumps[Q3_SURFACES].fileOfs);
	}
	const byte *getLumpData(u32 lumpNum) const {
		return (const byte*)(((const byte*)this)+lumps[lumpNum].fileOfs);
	}
	u32 getLumpStructCount(u32 lumpNum, u32 elemSize) const {
		return lumps[lumpNum].fileLen / elemSize;
	}
};

#endif // __BSP_QUAKE3__
