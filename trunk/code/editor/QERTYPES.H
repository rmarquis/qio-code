/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// qertypes.h
//
// common types
// merged from brush.h, etc. for plugin support
//
#ifndef _QERTYPE_H
#define _QERTYPE_H

typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/textureAPI.h>

class texdef_t
{
public:
  texdef_t()
  {
    name = new char[1];
    name[0] = '\0';
  }
  ~texdef_t()
  {
    delete []name;
    name = NULL;
  }

  const char *Name( void )
  {
	  if ( name ) {
		  return name;
	  }

	  return "";
  }

  void SetName(const char *p)
  {
    if (name)
    {
      delete []name;
    }
    if (p)
    {
      name = strcpy(new char[strlen(p)+1], p);
    }
    else
    {
      name = new char[1];
      name[0] = '\0';
    }
  }

  texdef_t& operator =(const texdef_t& rhs)
  {
    if (&rhs != this)
    {
      SetName(rhs.name);
      shift[0] = rhs.shift[0];
      shift[1] = rhs.shift[1];
      rotate = rhs.rotate;
      scale[0] = rhs.scale[0];
      scale[1] = rhs.scale[1];
      contents = rhs.contents;
      flags = rhs.flags;
      value = rhs.value;
    }
    return *this;
  }
	//char	name[128];
	char	*name;
	float	shift[2];
	float	rotate;
	float	scale[2];
	int		contents;
	int		flags;
	int		value;
};

// Timo
// new brush primitive texdef
struct brushprimit_texdef_s
{
	vec_t	coords[2][3];
};

class texturewin_t
{
public:
  texturewin_t()
  {
  }
  ~texturewin_t()
  {
  }
	int			width, height;
	int			originy;
	// add brushprimit_texdef_s for brush primitive coordinates storage
	brushprimit_texdef_s	brushprimit_texdef;
	int m_nTotalHeight;
	texdef_t	texdef;
};

#define QER_TRANS     0x00000001
#define QER_NOCARVE   0x00000002

#include "mathlib.h"
class texturedVertex_c {
public:
	edVec3_c xyz;
	float st[2];


	void setXYZ(const float *newXYZ) {
		xyz = newXYZ;
	}
	const edVec3_c &getXYZ() const {
		return xyz;
	}
	float dotProduct(const float *pVec3) const {
		return xyz.dotProduct(pVec3);
	}
	bool vectorCompare(const edVec3_c &other, float epsilon = EQUAL_EPSILON) const {
		return xyz.vectorCompare(other,epsilon);
	}
	const float operator [] (int index) const {
		return (&xyz.x)[index];
	}
	float &operator [] (int index) {
		return (&xyz.x)[index];
	}
	operator float *() {
		return &xyz.x;
	}
};
// NOTE: don't trust this definition!
// you should read float points[..][5]
// see NewWinding definition
#define MAX_POINTS_ON_WINDING 64
struct winding_t
{
	int		numpoints;
	int		maxpoints;
	texturedVertex_c 	points[8];			// variable sized
};

#define	NORMAL_EPSILON	0.0001
#define	DIST_EPSILON	0.02

class edPlane_c {
public:
    edVec3_c normal;
    double dist;
public:
	int isPlaneEqual(const edPlane_c &b, int flip) const {
		edVec3_c tempNormal;
		double tempDist;

		if (flip) {
			tempNormal = -b.normal;
			tempDist = -b.dist;
		} else {
			tempNormal = b.normal;
			tempDist = b.dist;
		}
		if (
		   fabs(this->normal[0] - tempNormal[0]) < NORMAL_EPSILON
		&& fabs(this->normal[1] - tempNormal[1]) < NORMAL_EPSILON
		&& fabs(this->normal[2] - tempNormal[2]) < NORMAL_EPSILON
		&& fabs(this->dist - tempDist) < DIST_EPSILON )
			return true;
		return false;
	}
	int fromPoints(const edVec3_c &p1, const edVec3_c &p2, const edVec3_c &p3)
	{
		edVec3_c v1 = p2 - p1;
		edVec3_c v2 = p3 - p1;
		this->normal.crossProduct(v1,v2);
		if (this->normal.normalize() < 0.1) 
			return false;
		this->dist = this->normal.dotProduct(p1);
		return true;
	}
	// project on normal plane
	// along ez 
	// assumes plane normal is normalized
	void projectOnPlane(const edVec3_c &ez, edVec3_c &p) {
		if (fabs(ez[0]) == 1)
			p[0] = (dist - normal[1] * p[1] - normal[2] * p[2]) / normal[0];
		else if (fabs(ez[1]) == 1)
			p[1] = (dist - normal[0] * p[0] - normal[2] * p[2]) / normal[1];
		else
			p[2] = (dist - normal[0] * p[0] - normal[1] * p[1]) / normal[2];
	}

};

//++timo texdef and brushprimit_texdef are static
// TODO : do dynamic ?
struct face_s
{
	struct face_s			*next;
	struct face_s			*original;		//used for vertex movement
	edVec3_c				planepts[3];
	texdef_t				texdef;
	edPlane_c				plane;

	winding_t				*face_winding;

	vec3_t					d_color;
	mtrAPI_i				*d_texture;

	// Timo new brush primit texdef
	brushprimit_texdef_s	brushprimit_texdef;
};

struct curveVertex_t {
	edVec3_c xyz;
	float	sideST[2];
	float	capST[2];
};


#define	MIN_PATCH_WIDTH		3
#define	MIN_PATCH_HEIGHT 	3

#define	MAX_PATCH_WIDTH		16
#define	MAX_PATCH_HEIGHT	16

// patch type info
// type in lower 16 bits, flags in upper
// endcaps directly follow this patch in the list

// types
#define PATCH_GENERIC     0x00000000    // generic flat patch
#define PATCH_CYLINDER    0x00000001    // cylinder
#define PATCH_BEVEL       0x00000002    // bevel
#define PATCH_ENDCAP      0x00000004    // endcap
#define PATCH_HEMISPHERE  0x00000008    // hemisphere
#define PATCH_CONE        0x00000010    // cone
#define PATCH_TRIANGLE    0x00000020    // simple tri, assumes 3x3 patch

// behaviour styles
#define PATCH_CAP         0x00001000    // flat patch applied as a cap
#define PATCH_SEAM        0x00002000    // flat patch applied as a seam
#define PATCH_THICK       0x00004000    // patch applied as a thick portion

// styles
#define PATCH_BEZIER      0x00000000    // default bezier
#define PATCH_BSPLINE     0x10000000    // bspline

#define PATCH_TYPEMASK     0x00000fff    // 
#define PATCH_BTYPEMASK    0x0000f000    // 
#define PATCH_STYLEMASK    0xffff0000    // 

struct drawVert_t {
	edVec3_c	xyz;
	float		st[2];
	float		lightmap[2];
	edVec3_c	normal;	
	
	texturedVertex_c &asWindingPoint() {
		return *((texturedVertex_c*)(this));
	}
};

// used in brush primitive AND entities
struct epair_s
{
	struct epair_s	*next;
	char	*key;
	char	*value;
};

class patchMesh_c {
	float calcPatchWidth();
	float calcPatchWidthDistanceTo(int j);
	float calcPatchHeight();
	float calcPatchHeightDistanceTo(int j);
public:
	int	width, height;		// in control points, not patches
	int   contents, flags, value, type;
	mtrAPI_i *d_texture;
	drawVert_t ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT];
	struct brush_s *pSymbiot;
	bool bSelected;
	bool bOverlay;
	bool bDirty;
	int  nListID;

	patchMesh_c() {
		width = height = 0;
		contents = flags = value = type = 0;
		d_texture = 0;
		pSymbiot = 0;
		bSelected = false;
		bOverlay = false;
		bDirty = false;
		nListID = 0;
	}
	/*void setType(int nType)
	{
		this->type = (this->type & PATCH_STYLEMASK) | nType;
	}
	void setStyle(int nStyle)
	{
		this->type = (this->type & PATCH_TYPEMASK) | nStyle;
	}*/
	void fillPatch(vec3_t v);
	void naturalizePatch();
	void invertPatch();
	void meshNormals();
	void interpolateInteriorPoints();
	void rebuildPatch();
	void adjustPatchRows(int nRows);
	void adjustPatchColumns(int nCols);
	void movePatch(const vec3_t vMove, bool bRebuild = false);
	void calcPatchBounds(edVec3_c& vMin, edVec3_c& vMax);
	void drawPatchMesh(bool bPoints, bool bShade = false);
	void drawPatchXY();
	void removePatchColumn(bool bFirst);
	void drawPatchCam();

};


typedef struct brush_s
{
	struct brush_s	*prev, *next;	// links in active/selected
	struct brush_s	*oprev, *onext;	// links in entity
	struct entity_s	*owner;
	edAABB_c bounds;
	face_s     *brush_faces;

	bool bModelFailed;
	//
	// curve brush extensions
	// all are derived from brush_faces
	bool	patchBrush;
	bool	hiddenBrush;
	bool	terrainBrush;
  
	patchMesh_c *pPatch;

	struct entity_s *pUndoOwner;

	int undoId;						//undo ID
	int redoId;						//redo ID
	int ownerId;					//entityId of the owner entity for undo

	const edAABB_c &getBounds() const {
		return bounds;
	}
	const edVec3_c &getMins() const {
		return bounds.getMins();
	}
	const edVec3_c &getMaxs() const {
		return bounds.getMaxs();
	}
} brush_s;


//#define	MAX_FLAGS	8
//

// eclass show flags

#define     ECLASS_LIGHT      0x00000001
#define     ECLASS_ANGLE      0x00000002
#define     ECLASS_PATH       0x00000004
#define     ECLASS_MISCMODEL  0x00000008
#define		ECLASS_PLUGINENTITY 0x00000010

struct eclass_s
{
	struct eclass_s *next;
	char	*name;
	bool	fixedsize;
	bool	unknown;		// wasn't found in source
	edVec3_c	mins, maxs;
	vec3_t	color;
	texdef_t	texdef;
	char	*comments;
	char	flagnames[8][32];


	char	*modelpath;
	char	*skinpath;
	int   nFrame;
	unsigned int nShowFlags;
};

extern	eclass_s	*eclass;

/*
** window bits
*/
#define	W_CAMERA		  0x0001
#define	W_XY			    0x0002
#define	W_XY_OVERLAY	0x0004
#define	W_Z				    0x0008
#define	W_TEXTURE		  0x0010
#define	W_Z_OVERLAY		0x0020
#define W_CONSOLE		  0x0040
#define W_ENTITY		  0x0080
#define W_CAMERA_IFON 0x0100
#define W_XZ          0x0200  //--| only used for patch vertex manip stuff
#define W_YZ          0x0400  //--|
#define W_GROUP       0x0800 
#define W_MEDIA       0x1000 
#define	W_ALL			0xFFFFFFFF

// used in some Drawing routines
enum VIEWTYPE {YZ, XZ, XY};

#endif