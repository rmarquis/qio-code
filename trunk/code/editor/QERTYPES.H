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

typedef float vec3_t[3];

#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/textureAPI.h>
#include <shared/str.h>

class texdef_t
{
	str name;
public:
	texdef_t()
	{
	}
	~texdef_t()
	{
	}

  const char *getName()
  {
	  return name.c_str();
  }

  void setName(const char *p)
  {
    if (p)
    {
      name = p;
    }
    else
    {
      name = "";
    }
  }

  texdef_t& operator =(const texdef_t& rhs)
  {
    if (&rhs != this)
    {
     setName(rhs.name);
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
	float	coords[2][3];
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
	vec3_c xyz;
	vec2_c st;


	void setXYZ(const float *newXYZ) {
		xyz = newXYZ;
	}
	const vec3_c &getXYZ() const {
		return xyz;
	}
	float dotProduct(const float *pVec3) const {
		return xyz.dotProduct(pVec3);
	}
	bool vectorCompare(const vec3_c &other, float epsilon = EQUAL_EPSILON) const {
		return xyz.vectorCompare(other,epsilon);
	}
	void setTC(float s, float t) {
		st[0] = s;
		st[1] = t;
	}
	const float operator [] (int index) const {
		return (&xyz.x)[index];
	}
	float &operator [] (int index) {
		return (&xyz.x)[index];
	}
	operator vec3_c &() {
		return xyz;
	}
	operator float *() {
		return &xyz.x;
	}
};
// NOTE: don't trust this definition!
// you should read float points[..][5]
// see NewWinding definition
#define MAX_POINTS_ON_WINDING 128
class edWinding_t
{
	arraySTD_c<texturedVertex_c> points;
public:
	edWinding_t();
	edWinding_t(const class edPlane_c &pl);
	edWinding_t(u32 numPoints);
	edWinding_t *cloneWinding();
	edWinding_t *clip(const class edPlane_c &split, bool keepon);
	void splitEpsilon(vec3_t normal, double dist, float epsilon, edWinding_t **front, edWinding_t **back);
	edWinding_t *tryMerge(const edWinding_t *f2, vec3_t planenormal, int keep);
	bool isTiny() const;
	bool removePoint(int point);
	edWinding_t *insertPoint(vec3_t point, int spot);
	static int planesConcave(edWinding_t *w1, edWinding_t *w2, const vec3_c &normal1, const vec3_c &normal2, float dist1, float dist2);


	void removeLastPoint() {
		points.resize(points.size()-1);
	}
	u32 size() const {
		return points.size();
	}
	void addPointXYZ(const vec3_c &xyz) {
		texturedVertex_c tv;
		tv.xyz = xyz;
		points.push_back(tv);
	}
	void addPoint(const texturedVertex_c &tv) {
		points.push_back(tv);
	}
	const vec3_c &getXYZ(u32 i) const {
		return points[i].xyz;
	}
	texturedVertex_c &getPoint(u32 i) {
		return points[i];
	}
	const texturedVertex_c &getPoint(u32 i) const {
		return points[i];
	}
	const float *getTC(u32 i) const {
		return points[i].st;
	}
	void setTC(u32 i, float s, float t) {
		points[i].setTC(s,t);
	}
	void setXYZ(u32 i, const vec3_c &v) {
		points[i].xyz = v;
	}
	void setPoint(u32 i, const texturedVertex_c &v) {
		points[i] = v;
	}

};



#define	NORMAL_EPSILON	0.0001
#define	DIST_EPSILON	0.02

class edPlane_c {
public:
    vec3_c normal;
    double dist;
public:
	int isPlaneEqual(const edPlane_c &b, int flip) const {
		vec3_c tempNormal;
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
	int fromPoints(const vec3_c &p1, const vec3_c &p2, const vec3_c &p3)
	{
		vec3_c v1 = p2 - p1;
		vec3_c v2 = p3 - p1;
		this->normal.crossProduct(v1,v2);
		if (this->normal.normalize2() < 0.1) 
			return false;
		this->dist = this->normal.dotProduct(p1);
		return true;
	}
	// project on normal plane
	// along ez 
	// assumes plane normal is normalized
	void projectOnPlane(const vec3_c &ez, vec3_c &p) {
		if (fabs(ez[0]) == 1)
			p[0] = (dist - normal[1] * p[1] - normal[2] * p[2]) / normal[0];
		else if (fabs(ez[1]) == 1)
			p[1] = (dist - normal[0] * p[0] - normal[2] * p[2]) / normal[1];
		else
			p[2] = (dist - normal[0] * p[0] - normal[1] * p[1]) / normal[2];
	}

};

extern int g_allocatedCounter_face;

//++timo texdef and brushprimit_texdef are static
// TODO : do dynamic ?
struct face_s
{
	struct face_s			*next;
	struct face_s			*original;		//used for vertex movement
	vec3_c				planepts[3];
	texdef_t				texdef;
	edPlane_c				plane;

	edWinding_t				*face_winding;

	vec3_t					d_color;
	mtrAPI_i				*d_texture;

	// Timo new brush primit texdef
	brushprimit_texdef_s	brushprimit_texdef;

	face_s() {
		next = 0;
		original = 0;
		face_winding = 0;
		d_texture = 0;
		g_allocatedCounter_face++;
	}
	~face_s() {
		if ( this->face_winding )
		{
			delete this->face_winding;
			this->face_winding = 0;
		}

		// allocated counter is only used for debugging
		g_allocatedCounter_face--;
	}
	void calculatePlaneFromPoints()
	{
		int		j;
		vec3_c	t1, t2, t3;

		// convert to a vector / dist plane
		for (j=0 ; j<3 ; j++)
		{
			t1[j] = this->planepts[0][j] - this->planepts[1][j];
			t2[j] = this->planepts[2][j] - this->planepts[1][j];
			t3[j] = this->planepts[1][j];
		}
		
		this->plane.normal.crossProduct(t1,t2);
		if (this->plane.normal.vectorCompare(vec3_c(0,0,0)))
			printf ("WARNING: brush plane with no normal\n");
		this->plane.normal.normalize();
		this->plane.dist = t3.dotProduct(this->plane.normal);
	}	
	face_s *cloneFace()
	{
		face_s	*n;

		n = new face_s();
		n->texdef = this->texdef;

		memcpy (n->planepts, this->planepts, sizeof(n->planepts));
		n->plane = this->plane;

		// all other fields are derived, and will be set by Brush_Build
		return n;
	}
	void fitTexture(int nHeight, int nWidth );
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

#include <renderer/rVertex.h>

class patchMesh_c {
	float calcPatchWidth();
	float calcPatchWidthDistanceTo(int j);
	float calcPatchHeight();
	float calcPatchHeightDistanceTo(int j);
public:
	int	width, height;		// in control points, not patches
	int   contents, flags, value, type;
	mtrAPI_i *d_texture;
	rVert_c ctrl[MAX_PATCH_WIDTH][MAX_PATCH_HEIGHT];
	class brush_s *pSymbiot;
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
	void fillPatch(vec3_t v);
	void naturalizePatch();
	void invertPatch();
	void meshNormals();
	void interpolateInteriorPoints();
	void rebuildPatch();
	void adjustPatchRows(int nRows);
	void adjustPatchColumns(int nCols);
	void movePatch(const vec3_t vMove, bool bRebuild = false);
	void calcPatchBounds(vec3_c& vMin, vec3_c& vMax);
	void drawPatchMesh(bool bPoints, bool bShade = false);
	void drawPatchXY();
	void removePatchColumn(bool bFirst);
	void drawPatchCam();

};

// allocated counter is only used for debugging
extern int g_allocatedCounter_brush;

class brush_s
{
	bool bIsLinkedListHeader;
public:
	class brush_s	*prev, *next;	// links in active/selected
	class brush_s	*oprev, *onext;	// links in entity
	class entity_s	*owner;
	aabb bounds;
	face_s     *brush_faces;

	//
	// curve brush extensions
	// all are derived from brush_faces
	bool	patchBrush;
	bool	hiddenBrush;
	bool	terrainBrush;
  
	patchMesh_c *pPatch;

	class entity_s *pUndoOwner;

	int undoId;						//undo ID
	int redoId;						//redo ID
	int ownerId;					//entityId of the owner entity for undo

	brush_s(bool bIsLinkedListHeader = true) {
		undoId = 0;
		redoId = 0;
		ownerId = 0;
		pUndoOwner = 0;
		pPatch = 0;
		prev = 0;
		next = 0;
		oprev = 0;
		onext = 0;
		owner = 0;
		brush_faces = 0;
		patchBrush = 0;
		hiddenBrush = 0;
		terrainBrush = 0;
		this->bIsLinkedListHeader = bIsLinkedListHeader;
		// allocated counter is only used for debugging
		if(bIsLinkedListHeader==false) 
			g_allocatedCounter_brush++;
	}
	~brush_s() {
		// allocated counter is only used for debugging
		if(bIsLinkedListHeader==false) 
			g_allocatedCounter_brush--;
	}
	void makeFacePlanes();
	void fitTexture(int nHeight, int nWidth);
	void removeEmptyFaces();
	void rotateBrush(vec3_t vAngle, vec3_t vOrigin, bool bBuild = true);
	edWinding_t *makeFaceWinding (face_s *face);

	const aabb &getBounds() const {
		return bounds;
	}
	const vec3_c &getMins() const {
		return bounds.getMins();
	}
	const vec3_c &getMaxs() const {
		return bounds.getMaxs();
	}
};


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