#ifndef __BSPFILEFORMAT_Q2_H__
#define __BSPFILEFORMAT_Q2_H__

// NOTE: Q2_BSP_IDENT is "IBSP" (the same as in Q3)
#define BSP_VERSION_Q2	38

// NOTE: there are no hardcoded bsp limits in qio,
// but leaffaces, leafbrushes, planes, and verts 
// are still bounded by 16 bit short limits

#define	Q2_ENTITIES		0
#define	Q2_PLANES		1
#define	Q2_VERTEXES		2
#define	Q2_VISIBILITY	3
#define	Q2_NODES			4
#define	Q2_TEXINFO		5
#define	Q2_FACES			6
#define	Q2_LIGHTING		7
#define	Q2_LEAFS			8
#define	Q2_LEAFFACES		9
#define	Q2_LEAFBRUSHES	10
#define	Q2_EDGES			11
#define	Q2_SURFEDGES		12
#define	Q2_MODELS		13
#define	Q2_BRUSHES		14
#define	Q2_BRUSHSIDES	15
#define	Q2_POP			16
#define	Q2_AREAS			17
#define	Q2_AREAPORTALS	18
#define	Q2_LUMPS		19

// NOTE: Quake2 bsp model struct is exacly the same as Source Engine model struct
struct q2Model_s
{
	float		mins[3], maxs[3];
	float		origin[3];		// for sounds or lights
	int			headnode;
	int			firstSurface, numSurfaces;	// submodels just draw faces
										// without walking the bsp tree
};


struct q2Vert_s
{
	float	point[3];
};



// planes (x&~1) and (x&~1)+1 are always opposites
// NOTE: the same plane structure is used in Source Engine
struct q2Plane_s
{
	float	normal[3];
	float	dist;
	int		type;		// PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
};

struct q2Node_s
{
	int			planeNum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	short		mins[3];		// for frustom culling
	short		maxs[3];
	unsigned short	firstFace;
	unsigned short	numFaces;	// counting both sides
};


struct q2TexInfo_s
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			flags;			// miptex flags + overrides
	int			value;			// light emission, etc
	char		texture[32];	// texture name (textures/*.wal)
	int			nexttexinfo;	// for animations, -1 = end of chain
};


// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
struct q2Edge_s
{
	unsigned short	v[2];		// vertex numbers
};

#define	Q2_MAX_LIGHTMAPS	4

struct q2Surface_s
{
	unsigned short	planenum;
	short		side;

	int			firstEdge;		// we must support > 64k edges
	short		numEdges;	
	short		texinfo;

// lighting info
	byte		styles[Q2_MAX_LIGHTMAPS];
	int			lightofs;		// start of [numstyles*surfsize] samples
};

struct q2Leaf_s
{
	int				contents;			// OR of all brushes (not needed?)

	short			cluster;
	short			area;

	short			mins[3];			// for frustum culling
	short			maxs[3];

	unsigned short	firstLeafSurface;
	unsigned short	numLeafSurfaces;

	unsigned short	firstLeafBrush;
	unsigned short	numLeafBrushes;
};

struct q2BrushSide_s
{
	unsigned short	planenum;		// facing out of the leaf
	short	texinfo;
};

// NOTE: Quake2 brush structure is exacly the same as Source Engine brush
struct q2Brush_s
{
	int			firstside;
	int			numsides;
	int			contents;
};

#define	ANGLE_UP	-1
#define	ANGLE_DOWN	-2


// the visibility lump consists of a header with a count, then
// byte offsets for the PVS and PHS of each cluster, then the raw
// compressed bit vectors
#define	DVIS_PVS	0
#define	DVIS_PHS	1
struct q2Vis_s
{
	int			numclusters;
	int			bitofs[8][2];	// bitofs[numclusters][2]
};

// each area has a list of portals that lead into other areas
// when portals are closed, other areas may not be visible or
// hearable even if the vis info says that it should be
typedef struct
{
	int		portalnum;
	int		otherarea;
} dareaportal_t;

typedef struct
{
	int		numareaportals;
	int		firstareaportal;
} darea_t;


#endif // __BSPFILEFORMAT_Q2_H__
