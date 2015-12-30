/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2012-2015 V.

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

#include "stdafx.h"
#include <assert.h>
#include "qe3.h"
#include "winding.h"
#include <shared/typedefs.h>
#include <api/materialSystemAPI.h>
#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/textureAPI.h>
#include <api/entityDeclAPI.h>
#include <math/math.h>

// globals

int g_nBrushId = 0;



void PrintWinding (winding_t *w)
{
	int		i;
	
	printf ("-------------\n");
	for (i=0 ; i<w->numpoints ; i++)
		printf ("(%5.2f, %5.2f, %5.2f)\n", w->points[i][0]
		, w->points[i][1], w->points[i][2]);
}

void PrintPlane (const class edPlane_c &p)
{
  printf ("(%5.2f, %5.2f, %5.2f) : %5.2f\n",  p.normal[0],  p.normal[1], 
  p.normal[2],  p.dist);
}

void PrintVector (vec3_t v)
{
  printf ("(%5.2f, %5.2f, %5.2f)\n",  v[0],  v[1], v[2]);
}


/*
=============================================================================

			TEXTURE COORDINATES

=============================================================================
*/


/*
==================
textureAxisFromPlane
==================
*/
vec3_t	baseaxis[18] =
{
{0,0,1}, {1,0,0}, {0,-1,0},			// floor
{0,0,-1}, {1,0,0}, {0,-1,0},		// ceiling
{1,0,0}, {0,1,0}, {0,0,-1},			// west wall
{-1,0,0}, {0,1,0}, {0,0,-1},		// east wall
{0,1,0}, {1,0,0}, {0,0,-1},			// south wall
{0,-1,0}, {1,0,0}, {0,0,-1}			// north wall
};

void TextureAxisFromPlane(const edPlane_c &pln, vec3_c &xv, vec3_c &yv)
{
	int		bestaxis;
	float	dot,best;
	int		i;
	
	best = 0;
	bestaxis = 0;
	
	for (i=0 ; i<6 ; i++)
	{
		dot = pln.normal.dotProduct(baseaxis[i*3]);
		if (dot > best)
		{
			best = dot;
			bestaxis = i;
		}
	}
	
	xv = (baseaxis[bestaxis*3+1]);
	yv = (baseaxis[bestaxis*3+2]);
}



float	lightaxis[3] = {0.6, 0.8, 1.0};
/*
================
SetShadeForPlane

Light different planes differently to
improve recognition
================
*/
float SetShadeForPlane (const class edPlane_c &p)
{
	int		i;
	float	f;

	// axial plane
	for (i=0 ; i<3 ; i++)
		if (fabs(p.normal[i]) > 0.9)
		{
			f = lightaxis[i];
			return f;
		}

	// between two axial planes
	for (i=0 ; i<3 ; i++)
		if (fabs(p.normal[i]) < 0.1)
		{
			f = (lightaxis[(i+1)%3] + lightaxis[(i+2)%3])/2;
			return f;
		}

	// other
	f= (lightaxis[0] + lightaxis[1] + lightaxis[2]) / 3;
	return f;
}

vec3_t  vecs[2];
float	shift[2];

void Brush_Print(brush_s* b)
{
	int nFace = 0;
	for (face_s* f = b->brush_faces ; f ; f=f->next)
	{
		Sys_Printf("Face %i\n", nFace++);
		Sys_Printf("%f %f %f\n", f->planepts[0][0], f->planepts[0][1], f->planepts[0][2]);
		Sys_Printf("%f %f %f\n", f->planepts[1][0], f->planepts[1][1], f->planepts[1][2]);
		Sys_Printf("%f %f %f\n", f->planepts[2][0], f->planepts[2][1], f->planepts[2][2]);
	}
}


/*
================
Face_Free
================
*/
void Face_Free( face_s *f )
{
	assert( f != 0 );

	if ( f->face_winding )
	{
		free( f->face_winding );
		f->face_winding = 0;
	}


	delete f;
}

/*
================
Face_Clone
================
*/
face_s	*Face_Clone (face_s *f)
{
	face_s	*n;

	n = new face_s();
	n->texdef = f->texdef;

	memcpy (n->planepts, f->planepts, sizeof(n->planepts));

	// all other fields are derived, and will be set by Brush_Build
	return n;
}

/*
================
Face_FullClone

makes an exact copy of the face
================
*/
face_s	*Face_FullClone (face_s *f)
{
	face_s	*n;

	n = new face_s();
	n->texdef = f->texdef;
	memcpy(n->planepts, f->planepts, sizeof(n->planepts));
	n->plane = f->plane;
	if (f->face_winding)
		n->face_winding = Winding_Clone(f->face_winding);
	else
		n->face_winding = NULL;
	n->d_texture = QERApp_TryTextureForName( n->texdef.getName() );
	return n;
}

/*
================
Clamp
================
*/
void Clamp(float& f, int nClamp)
{
  float fFrac = f - static_cast<int>(f);
  f = static_cast<int>(f) % nClamp;
  f += fFrac;
}

/*
================
Face_MoveTexture
================
*/
void Face_MoveTexture(face_s *f, const vec3_c &delta)
{
	vec3_c vX, vY;
/*
#ifdef _DEBUG
	if (g_PrefsDlg.m_bBrushPrimitMode)
		Sys_Printf("Warning : Face_MoveTexture not done in brush primitive mode\n");
#endif
*/
	if (g_qeglobals.m_bBrushPrimitMode)
		Face_MoveTexture_BrushPrimit( f, delta );
	else
	{
		TextureAxisFromPlane(f->plane, vX, vY);

		vec3_t vDP, vShift;
		vDP[0] = delta.dotProduct(vX);
		vDP[1] = delta.dotProduct(vY);

		double fAngle = DEG2RAD(f->texdef.rotate);
		double c = cos(fAngle);
		double s = sin(fAngle);

		vShift[0] = vDP[0] * c - vDP[1] * s;
		vShift[1] = vDP[0] * s + vDP[1] * c;

		if (!f->texdef.scale[0])
			f->texdef.scale[0] = 1;
		if (!f->texdef.scale[1])
			f->texdef.scale[1] = 1;

		f->texdef.shift[0] -= vShift[0] / f->texdef.scale[0];
		f->texdef.shift[1] -= vShift[1] / f->texdef.scale[1];
  
		// clamp the shifts
		Clamp(f->texdef.shift[0], f->d_texture->getImageWidth());
		Clamp(f->texdef.shift[1], f->d_texture->getImageHeight());
	}
}

/*
================
Face_SetColor
================
*/
void Face_SetColor (brush_s *b, face_s *f, float fCurveColor) 
{
	float	shade;
	mtrAPI_i *q;

	q = f->d_texture;

	// set shading for face
	shade = SetShadeForPlane (f->plane);
	if (g_pParentWnd->GetCamera()->Camera().draw_mode == cd_texture && !b->owner->eclass->isFixedSize())
	{
		//if (b->curveBrush)
		//  shade = fCurveColor;
		f->d_color[0] = 
		f->d_color[1] = 
		f->d_color[2] = shade;
	}
	else
	{
		f->d_color[0] = shade*q->getEditorColor()[0];
		f->d_color[1] = shade*q->getEditorColor()[1];
		f->d_color[2] = shade*q->getEditorColor()[2];
	}
}

/*
================
Face_TextureVectors
TTimo: NOTE: this is never to get called while in brush primitives mode
================
*/
void Face_TextureVectors (face_s *f, float STfromXYZ[2][4])
{
	vec3_c		pvecs[2];
	int			sv, tv;
	float		ang, sinv, cosv;
	float		ns, nt;
	int			i,j;
	mtrAPI_i *q;
	texdef_t	*td;

#ifdef _DEBUG
	//++timo when playing with patches, this sometimes get called and the Warning is displayed
	// find some way out ..
	if (g_qeglobals.m_bBrushPrimitMode && !g_qeglobals.bNeedConvert)
		Sys_Printf("Warning : illegal call of Face_TextureVectors in brush primitive mode\n");
#endif

	td = &f->texdef;
	q = f->d_texture;

	memset (STfromXYZ, 0, 8*sizeof(float));

	if (!td->scale[0])
		td->scale[0] =  0.5 ;
	if (!td->scale[1])
		td->scale[1] = 0.5 ;

	// get natural texture axis
	TextureAxisFromPlane(f->plane, pvecs[0], pvecs[1]);

	// rotate axis
	if (td->rotate == 0)
		{ sinv = 0 ; cosv = 1; }
	else if (td->rotate == 90)
		{ sinv = 1 ; cosv = 0; }
	else if (td->rotate == 180)
		{ sinv = 0 ; cosv = -1; }
	else if (td->rotate == 270)
		{ sinv = -1 ; cosv = 0; }
	else
	{	
		ang = DEG2RAD(td->rotate);
		sinv = sin(ang);
		cosv = cos(ang);
	}

	if (pvecs[0][0])
		sv = 0;
	else if (pvecs[0][1])
		sv = 1;
	else
		sv = 2;
				
	if (pvecs[1][0])
		tv = 0;
	else if (pvecs[1][1])
		tv = 1;
	else
		tv = 2;
					
	for (i=0 ; i<2 ; i++) {
		ns = cosv * pvecs[i][sv] - sinv * pvecs[i][tv];
		nt = sinv * pvecs[i][sv] +  cosv * pvecs[i][tv];
		STfromXYZ[i][sv] = ns;
		STfromXYZ[i][tv] = nt;
	}

	// scale
	for (i=0 ; i<2 ; i++)
		for (j=0 ; j<3 ; j++)
			STfromXYZ[i][j] = STfromXYZ[i][j] / td->scale[i];

	// shift
	STfromXYZ[0][3] = td->shift[0];
	STfromXYZ[1][3] = td->shift[1];

	for (j=0 ; j<4 ; j++) {
		STfromXYZ[0][j] /= q->getImageWidth();
		STfromXYZ[1][j] /= q->getImageHeight();
	}
}

/*
================
Face_MakePlane
================
*/
void Face_MakePlane (face_s *f)
{
	int		j;
	vec3_c	t1, t2, t3;

	// convert to a vector / dist plane
	for (j=0 ; j<3 ; j++)
	{
		t1[j] = f->planepts[0][j] - f->planepts[1][j];
		t2[j] = f->planepts[2][j] - f->planepts[1][j];
		t3[j] = f->planepts[1][j];
	}
	
	f->plane.normal.crossProduct(t1,t2);
	if (f->plane.normal.vectorCompare(vec3_c(0,0,0)))
		printf ("WARNING: brush plane with no normal\n");
	f->plane.normal.normalize();
	f->plane.dist = t3.dotProduct(f->plane.normal);
}

/*
================
EmitTextureCoordinates
================
*/
void EmitTextureCoordinates (texturedVertex_c &out, mtrAPI_i *q, face_s *f)
{
	float	STfromXYZ[2][4];

	// out layout is: xyz st
	Face_TextureVectors (f,  STfromXYZ);
	out[3] = out.dotProduct(STfromXYZ[0]) + STfromXYZ[0][3];
	out[4] = out.dotProduct(STfromXYZ[1]) + STfromXYZ[1][3];
}

//==========================================================================

/*
================
Brush_MakeFacePlanes
================
*/
void Brush_MakeFacePlanes (brush_s *b)
{
	face_s	*f;

	for (f=b->brush_faces ; f ; f=f->next)
	{
		Face_MakePlane (f);
	}
}

/*
================
DrawBrushEntityName
================
*/
void DrawBrushEntityName (brush_s *b)
{
	const char	*name;
	//float	a, s, c;
	//vec3_t	mid;
	//int		i;

	if (!b->owner)
		return;		// during contruction

	if (b->owner == world_entity)
		return;

	if (b != b->owner->brushes.onext)
		return;	// not key brush

// MERGEME
#if 0
	if (!(g_qeglobals.d_savedinfo.exclude & EXCLUDE_ANGLES))
	{
		// draw the angle pointer
		a = FloatForKey (b->owner, "angle");
		if (a)
		{
			s = sin (a/180*Q_PI);
			c = cos (a/180*Q_PI);
			for (i=0 ; i<3 ; i++)
				mid[i] = (b->mins[i] + b->maxs[i])*0.5; 

			glBegin (GL_LINE_STRIP);
			glVertex3fv (mid);
			mid[0] += c*8;
			mid[1] += s*8;
			mid[2] += s*8;
			glVertex3fv (mid);
			mid[0] -= c*4;
			mid[1] -= s*4;
			mid[2] -= s*4;
			mid[0] -= s*4;
			mid[1] += c*4;
			mid[2] += c*4;
			glVertex3fv (mid);
			mid[0] += c*4;
			mid[1] += s*4;
			mid[2] += s*4;
			mid[0] += s*4;
			mid[1] -= c*4;
			mid[2] -= c*4;
			glVertex3fv (mid);
			mid[0] -= c*4;
			mid[1] -= s*4;
			mid[2] -= s*4;
			mid[0] += s*4;
			mid[1] -= c*4;
			mid[2] -= c*4;
			glVertex3fv (mid);
			glEnd ();
		}
	}
#endif

	if (g_qeglobals.d_savedinfo.show_names)
	{
		name = b->owner->getKeyValue("classname");
		glRasterPos3f (b->getMins()[0]+4, b->getMins()[1]+4, b->getMins()[2]+4);
		glCallLists (strlen(name), GL_UNSIGNED_BYTE, name);
	}
}

/*
=================
Brush_MakeFaceWinding

returns the visible polygon on a face
=================
*/
winding_t *Brush_MakeFaceWinding (brush_s *b, face_s *face)
{
	winding_t	*w;
	face_s		*clip;
	edPlane_c			plane;
	bool		past;

	// get a poly that covers an effectively infinite area
	w = Winding_BaseForPlane (face->plane);

	// chop the poly by all of the other faces
	past = false;
	for (clip = b->brush_faces ; clip && w ; clip=clip->next)
	{
		if (clip == face)
		{
			past = true;
			continue;
		}
		if (face->plane.normal.dotProduct(clip->plane.normal) > 0.999
			&& fabs(face->plane.dist - clip->plane.dist) < 0.01 )
		{	// identical plane, use the later one
			if (past)
			{
				free (w);
				return NULL;
			}
			continue;
		}

		// flip the plane, because we want to keep the back side
		plane.normal = -clip->plane.normal;
		plane.dist = -clip->plane.dist;
		
		w = Winding_Clip (w, plane, false);
		if (!w)
			return w;
	}
	
	if (w->numpoints < 3)
	{
		free(w);
		w = NULL;
	}

	if (!w)
		printf ("unused plane\n");

	return w;
}

/*
=================
Brush_SnapPlanepts
=================
*/
void Brush_SnapPlanepts (brush_s *b)
{
	int		i, j;
	face_s	*f;

  if (g_PrefsDlg.m_bNoClamp)
    return;

	for (f=b->brush_faces ; f; f=f->next)
		for (i=0 ; i<3 ; i++)
			for (j=0 ; j<3 ; j++)
				f->planepts[i][j] = floor (f->planepts[i][j] + 0.5);
}
	
/*
** Brush_Build
**
** Builds a brush rendering data and also sets the min/max bounds
*/
// TTimo
// added a bConvert flag to convert between old and new brush texture formats
// TTimo
// brush grouping: update the group treeview if necessary
void Brush_Build( brush_s *b, bool bSnap, bool bMarkMap, bool bConvert )
{
	bool		bLocalConvert = false;

#ifdef _DEBUG
	if (!g_qeglobals.m_bBrushPrimitMode && bConvert)
		Sys_Printf("Warning : conversion from brush primitive to old brush format not implemented\n");
#endif

	// if bConvert is set and g_qeglobals.bNeedConvert is not, that just means we need convert for this brush only
	if (bConvert && !g_qeglobals.bNeedConvert)
	{
		bLocalConvert = true;
		g_qeglobals.bNeedConvert = true;
	}

	/*
	** build the windings and generate the bounding box
	*/
	Brush_BuildWindings(b, bSnap);

	Patch_BuildPoints (b);

	/*
	** move the points and edges if in select mode
	*/
	if (g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge)
		SetupVertexSelection ();

	if (bMarkMap)
	{
		Sys_MarkMapModified();
	}

	if (bLocalConvert)
		g_qeglobals.bNeedConvert = false;
}

template<typename TYPE> 
void T_Swap(TYPE &a, TYPE &b) {
	TYPE temp;
	temp = a;
	a = b;
	b = temp;
}
/*
==============
Brush_SplitBrushByFace

The incoming brush is NOT freed.
The incoming face is NOT left referenced.
==============
*/
void Brush_SplitBrushByFace (brush_s *in, face_s *f, brush_s **front, brush_s **back)
{
	brush_s	*b;
	face_s	*nf;
//	vec3_t	temp;

	b = Brush_Clone (in);
	nf = Face_Clone (f);

	nf->texdef = b->brush_faces->texdef;
	nf->next = b->brush_faces;
	b->brush_faces = nf;

	Brush_Build( b );
	Brush_RemoveEmptyFaces ( b );
	if ( !b->brush_faces )
	{	// completely clipped away
		Brush_Free (b);
		*back = NULL;
	}
	else
	{
		Entity_LinkBrush (in->owner, b);
		*back = b;
	}

	b = Brush_Clone (in);
	nf = Face_Clone (f);
	// swap the plane winding
	T_Swap(nf->planepts[0],nf->planepts[1]);

	nf->texdef = b->brush_faces->texdef;
	nf->next = b->brush_faces;
	b->brush_faces = nf;

	Brush_Build( b );
	Brush_RemoveEmptyFaces ( b );
	if ( !b->brush_faces )
	{	// completely clipped away
		Brush_Free (b);
		*front = NULL;
	}
	else
	{
		Entity_LinkBrush (in->owner, b);
		*front = b;
	}
}

/*
=================
Brush_BestSplitFace

returns the best face to split the brush with.
return NULL if the brush is convex
=================
*/
face_s *Brush_BestSplitFace(brush_s *b)
{
	face_s *face, *f, *bestface;
	winding_t *front, *back;
	int splits, tinywindings, value, bestvalue;

	bestvalue = 999999;
	bestface = NULL;
	for (face = b->brush_faces; face; face = face->next)
	{
		splits = 0;
		tinywindings = 0;
		for (f = b->brush_faces; f; f = f->next)
		{
			if (f == face) continue;
			//
			Winding_SplitEpsilon(f->face_winding, face->plane.normal, face->plane.dist, 0.1, &front, &back);

			if (!front)
			{
				Winding_Free(back);
			}
			else if (!back)
			{
				Winding_Free(front);
			}
			else
			{
				splits++;
				if (Winding_IsTiny(front)) tinywindings++;
				if (Winding_IsTiny(back)) tinywindings++;
			}
		}
		if (splits)
		{
			value = splits + 50 * tinywindings;
			if (value < bestvalue)
			{
				bestvalue = value;
				bestface = face;
			}
		}
	}
	return bestface;
}

/*
=================
Brush_MakeConvexBrushes

MrE FIXME: this doesn't work because the old
		   Brush_SplitBrushByFace is used
Turns the brush into a minimal number of convex brushes.
If the input brush is convex then it will be returned.
Otherwise the input brush will be freed.
NOTE: the input brush should have windings for the faces.
=================
*/
brush_s *Brush_MakeConvexBrushes(brush_s *b)
{
	brush_s *front, *back, *end;
	face_s *face;

	b->next = NULL;
	face = Brush_BestSplitFace(b);
	if (!face) return b;
	Brush_SplitBrushByFace(b, face, &front, &back);
	//this should never happen
	if (!front && !back) return b;
	Brush_Free(b);
	if (!front)
		return Brush_MakeConvexBrushes(back);
	b = Brush_MakeConvexBrushes(front);
	if (back)
	{
		for (end = b; end->next; end = end->next);
		end->next = Brush_MakeConvexBrushes(back);
	}
	return b;
}

/*
=================
Brush_Convex
=================
*/
int Brush_Convex(brush_s *b)
{
	face_s *face1, *face2;

	for (face1 = b->brush_faces; face1; face1 = face1->next)
	{
		if (!face1->face_winding) continue;
		for (face2 = b->brush_faces; face2; face2 = face2->next)
		{
			if (face1 == face2) continue;
			if (!face2->face_winding) continue;
			if (Winding_PlanesConcave(face1->face_winding, face2->face_winding,
										face1->plane.normal, face2->plane.normal,
										face1->plane.dist, face2->plane.dist))
			{
				return false;
			}
		}
	}
	return true;
}

/*
=================
Brush_MoveVertexes_old1

- The input brush must have face windings.
- The input brush must be a brush with faces that do not intersect.
- The input brush does not have to be convex.
- The vertex will not be moved if the movement either causes the
  brush to have faces that intersect or causes the brush to be
  flipped inside out.
  (For instance a tetrahedron can easily be flipped inside out
  without having faces that intersect.)
- The created brush does not have to be convex.
- Returns true if the vertex movement is performed.
=================
*/

#define MAX_MOVE_FACES		64
#define INTERSECT_EPSILON	0.1
#define POINT_EPSILON		0.3
//
//int Brush_MoveVertex_old1(brush_s *b, vec3_t vertex, vec3_t delta, vec3_t end, bool bSnap)
//{
//	face_s *f, *face, *newface, *lastface, *nextface;
//	face_s *movefaces[MAX_MOVE_FACES];
//	int movefacepoints[MAX_MOVE_FACES];
//	winding_t *w, tmpw;
//	int i, j, k, nummovefaces, result;
//	float dot;
//
//	result = false;
//	//
//	tmpw.numpoints = 3;
//	tmpw.maxpoints = 3;
//	VectorAdd(vertex, delta, end);
//	//snap or not?
//	if (bSnap)
//		for (i = 0; i < 3; i++)
//			end[i] = floor(end[i] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
//	//chop off triangles from all brush faces that use the to be moved vertex
//	//store pointers to these chopped off triangles in movefaces[]
//	nummovefaces = 0;
//	for (face = b->brush_faces; face; face = face->next)
//	{
//		w = face->face_winding;
//		if (!w) continue;
//		for (i = 0; i < w->numpoints; i++)
//		{
//			if (Point_Equal(w->points[i], vertex, POINT_EPSILON))
//			{
//				if (face->face_winding->numpoints <= 3)
//				{
//					movefacepoints[nummovefaces] = i;
//					movefaces[nummovefaces++] = face;
//					break;
//				}
//				dot = end.dotProduct(face->plane.normal) - face->plane.dist;
//				//if the end point is in front of the face plane
//				if (dot > 0.1)
//				{
//					//fanout triangle subdivision
//					for (k = i; k < i + w->numpoints-3; k++)
//					{
//						VectorCopy(w->points[i], tmpw.points[0]);
//						VectorCopy(w->points[(k+1) % w->numpoints], tmpw.points[1]);
//						VectorCopy(w->points[(k+2) % w->numpoints], tmpw.points[2]);
//						//
//						newface = Face_Clone(face);
//						//get the original
//						for (f = face; f->original; f = f->original) ;
//						newface->original = f;
//						//store the new winding
//						if (newface->face_winding) Winding_Free(newface->face_winding);
//						newface->face_winding = Winding_Clone(&tmpw);
//						//get the texture
//						newface->d_texture = QERApp_TryTextureForName( newface->texdef.getName() );
//						//add the face to the brush
//						newface->next = b->brush_faces;
//						b->brush_faces = newface;
//						//add this new triangle to the move faces
//						movefacepoints[nummovefaces] = 0;
//						movefaces[nummovefaces++] = newface;
//					}
//					//give the original face a new winding
//					VectorCopy(w->points[(i-2+w->numpoints) % w->numpoints], tmpw.points[0]);
//					VectorCopy(w->points[(i-1+w->numpoints) % w->numpoints], tmpw.points[1]);
//					VectorCopy(w->points[i], tmpw.points[2]);
//					Winding_Free(face->face_winding);
//					face->face_winding = Winding_Clone(&tmpw);
//					//add the original face to the move faces
//					movefacepoints[nummovefaces] = 2;
//					movefaces[nummovefaces++] = face;
//				}
//				else
//				{
//					//chop a triangle off the face
//					VectorCopy(w->points[(i-1+w->numpoints) % w->numpoints], tmpw.points[0]);
//					VectorCopy(w->points[i], tmpw.points[1]);
//					VectorCopy(w->points[(i+1) % w->numpoints], tmpw.points[2]);
//					//remove the point from the face winding
//					Winding_RemovePoint(w, i);
//					//get texture crap right
//					Face_SetColor(b, face, 1.0);
//					for (j = 0; j < w->numpoints; j++)
//						EmitTextureCoordinates(w->points[j], face->d_texture, face);
//					//make a triangle face
//					newface = Face_Clone(face);
//					//get the original
//					for (f = face; f->original; f = f->original) ;
//					newface->original = f;
//					//store the new winding
//					if (newface->face_winding) Winding_Free(newface->face_winding);
//					newface->face_winding = Winding_Clone(&tmpw);
//					//get the texture
//					newface->d_texture = QERApp_TryTextureForName( newface->texdef.getName() );
//					//add the face to the brush
//					newface->next = b->brush_faces;
//					b->brush_faces = newface;
//					//
//					movefacepoints[nummovefaces] = 1;
//					movefaces[nummovefaces++] = newface;
//				}
//				break;
//			}
//		}
//	}
//	//now movefaces contains pointers to triangle faces that
//	//contain the to be moved vertex
//
//	//check if the move is valid
//	int l;
//	vec3_t p1, p2;
//	winding_t *w2;
//	edPlane_c plane;
//
//	face = NULL;
//	VectorCopy(vertex, tmpw.points[1]);
//	VectorCopy(end, tmpw.points[2]);
//	for (face = b->brush_faces; face; face = face->next)
//	{
//		for (i = 0; i < nummovefaces; i++)
//		{
//			if (face == movefaces[i])
//				break;
//		}
//		if (i < nummovefaces)
//			continue;
//		//the delta vector may not intersect with any of the not move faces
//		if (Winding_VectorIntersect(face->face_winding, face->plane, vertex, end, INTERSECT_EPSILON))
//			break;
//		//if the end point of the to be moved vertex is near this not move face
//		if (abs(face->plane.normal.dotProduct(end) - face->plane.dist) < 0.5)
//		{
//			//the end point may not be inside or very close to the not move face winding
//			if (Winding_PointInside(face->face_winding, face->plane, end, 0.5))
//				break;
//		}
//		for (i = 0; i < nummovefaces; i++)
//		{
//			w = movefaces[i]->face_winding;
//			j = movefacepoints[i];
//			for (k = -1; k <= 1; k += 2)
//			{
//				//check if the new edge will not intersect with the not move face
//				VectorCopy(w->points[(j + k + w->numpoints) % w->numpoints], tmpw.points[0]);
//				if (Winding_VectorIntersect(face->face_winding, face->plane, tmpw.points[0].getXYZ(), end, INTERSECT_EPSILON))
//				{
//					//ok the new edge instersects with the not move face
//					//we can't perform the vertex movement
//					//break;
//				}
//				//check if the not move face intersects the "movement winding"
//				Winding_Plane(&tmpw, plane.normal, &plane.dist);
//				w2 = face->face_winding;
//				for (l = 0; l < w2->numpoints; l++)
//				{
//					VectorCopy(w2->points[l], p1);
//					if (Point_Equal(p1, tmpw.points[0], POINT_EPSILON)) continue;
//					VectorCopy(w2->points[(l+1) % w2->numpoints], p2);
//					if (Point_Equal(p2, tmpw.points[0], POINT_EPSILON)) continue;
//					if (Winding_VectorIntersect(&tmpw, plane, p1, p2, INTERSECT_EPSILON))
//						break;
//				}
//				if (l < w2->numpoints)
//				{
//					//ok this not move face intersects the "movement winding"
//					//we can't perform the vertex movement
//					break;
//				}
//			}
//			if (k <= 1) break;
//		}
//		if (i < nummovefaces)
//			break;
//	}
//	if (!face)
//	{
//		//ok the move was valid
//		//now move all the vertexes of the movefaces
//		for (i = 0; i < nummovefaces; i++)
//		{
//			VectorCopy(end, movefaces[i]->face_winding->points[movefacepoints[i]]);
//			//create new face plane
//			for (j = 0; j < 3; j++)
//			{
//				VectorCopy(movefaces[i]->face_winding->points[j], movefaces[i]->planepts[j]);
//			}
//			Face_MakePlane(movefaces[i]);
//		}
//		result = true;
//	}
//	//get texture crap right
//	for (i = 0; i < nummovefaces; i++)
//	{
//		Face_SetColor(b, movefaces[i], 1.0);
//		for (j = 0; j < movefaces[i]->face_winding->numpoints; j++)
//			EmitTextureCoordinates(movefaces[i]->face_winding->points[j], movefaces[i]->d_texture, movefaces[i]);
//	}
//
//	//now try to merge faces with their original faces
//	lastface = NULL;
//	for (face = b->brush_faces; face; face = nextface)
//	{
//		nextface = face->next;
//		if (!face->original)
//		{
//			lastface = face;
//			continue;
//		}
//		if (!face->plane.isPlaneEqual(face->original->plane, false))
//		{
//			lastface = face;
//			continue;
//		}
//		w = Winding_TryMerge(face->face_winding, face->original->face_winding, face->plane.normal, true);
//		if (!w)
//		{
//			lastface = face;
//			continue;
//		}
//		Winding_Free(face->original->face_winding);
//		face->original->face_winding = w;
//		//get texture crap right
//		Face_SetColor(b, face->original, 1.0);
//		for (j = 0; j < face->original->face_winding->numpoints; j++)
//			EmitTextureCoordinates(face->original->face_winding->points[j], face->original->d_texture, face->original);
//		//remove the face that was merged with the original
//		if (lastface) lastface->next = face->next;
//		else b->brush_faces = face->next;
//		Face_Free(face);
//	}
//	return result;
//}

/*
=================
Brush_MoveVertexes_old2

- The input brush must be convex
- The input brush must have face windings.
- The output brush will be convex.
- Returns true if the vertex movement is performed.
=================
*/

#define MAX_MOVE_FACES		64
#define INTERSECT_EPSILON	0.1
#define POINT_EPSILON		0.3

//int Brush_MoveVertex_old2(brush_s *b, vec3_t vertex, vec3_t delta, vec3_t end, bool bSnap)
//{
//	face_s *f, *face, *newface, *lastface, *nextface;
//	face_s *movefaces[MAX_MOVE_FACES];
//	int movefacepoints[MAX_MOVE_FACES];
//	winding_t *w, tmpw;
//	int i, j, k, nummovefaces, result;
//	float dot;
//
//	result = true;
//	//
//	tmpw.numpoints = 3;
//	tmpw.maxpoints = 3;
//	VectorAdd(vertex, delta, end);
//	//snap or not?
//	if (bSnap)
//		for (i = 0; i < 3; i++)
//			end[i] = floor(end[i] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
//	//chop off triangles from all brush faces that use the to be moved vertex
//	//store pointers to these chopped off triangles in movefaces[]
//	nummovefaces = 0;
//	for (face = b->brush_faces; face; face = face->next)
//	{
//		w = face->face_winding;
//		if (!w) continue;
//		for (i = 0; i < w->numpoints; i++)
//		{
//			if (Point_Equal(w->points[i], vertex, POINT_EPSILON))
//			{
//				if (face->face_winding->numpoints <= 3)
//				{
//					movefacepoints[nummovefaces] = i;
//					movefaces[nummovefaces++] = face;
//					break;
//				}
//				dot = DotProduct(end, face->plane.normal) - face->plane.dist;
//				//if the end point is in front of the face plane
//				if (dot > 0.1)
//				{
//					//fanout triangle subdivision
//					for (k = i; k < i + w->numpoints-3; k++)
//					{
//						VectorCopy(w->points[i], tmpw.points[0]);
//						VectorCopy(w->points[(k+1) % w->numpoints], tmpw.points[1]);
//						VectorCopy(w->points[(k+2) % w->numpoints], tmpw.points[2]);
//						//
//						newface = Face_Clone(face);
//						//get the original
//						for (f = face; f->original; f = f->original) ;
//						newface->original = f;
//						//store the new winding
//						if (newface->face_winding) Winding_Free(newface->face_winding);
//						newface->face_winding = Winding_Clone(&tmpw);
//						//get the texture
//						newface->d_texture = QERApp_TryTextureForName( newface->texdef.getName() );
//						//add the face to the brush
//						newface->next = b->brush_faces;
//						b->brush_faces = newface;
//						//add this new triangle to the move faces
//						movefacepoints[nummovefaces] = 0;
//						movefaces[nummovefaces++] = newface;
//					}
//					//give the original face a new winding
//					VectorCopy(w->points[(i-2+w->numpoints) % w->numpoints], tmpw.points[0]);
//					VectorCopy(w->points[(i-1+w->numpoints) % w->numpoints], tmpw.points[1]);
//					VectorCopy(w->points[i], tmpw.points[2]);
//					Winding_Free(face->face_winding);
//					face->face_winding = Winding_Clone(&tmpw);
//					//add the original face to the move faces
//					movefacepoints[nummovefaces] = 2;
//					movefaces[nummovefaces++] = face;
//				}
//				else
//				{
//					//chop a triangle off the face
//					VectorCopy(w->points[(i-1+w->numpoints) % w->numpoints], tmpw.points[0]);
//					VectorCopy(w->points[i], tmpw.points[1]);
//					VectorCopy(w->points[(i+1) % w->numpoints], tmpw.points[2]);
//					//remove the point from the face winding
//					Winding_RemovePoint(w, i);
//					//get texture crap right
//					Face_SetColor(b, face, 1.0);
//					for (j = 0; j < w->numpoints; j++)
//						EmitTextureCoordinates(w->points[j], face->d_texture, face);
//					//make a triangle face
//					newface = Face_Clone(face);
//					//get the original
//					for (f = face; f->original; f = f->original) ;
//					newface->original = f;
//					//store the new winding
//					if (newface->face_winding) Winding_Free(newface->face_winding);
//					newface->face_winding = Winding_Clone(&tmpw);
//					//get the texture
//					newface->d_texture = QERApp_TryTextureForName( newface->texdef.getName() );
//					//add the face to the brush
//					newface->next = b->brush_faces;
//					b->brush_faces = newface;
//					//
//					movefacepoints[nummovefaces] = 1;
//					movefaces[nummovefaces++] = newface;
//				}
//				break;
//			}
//		}
//	}
//	//now movefaces contains pointers to triangle faces that
//	//contain the to be moved vertex
//
//	//move the vertex
//	for (i = 0; i < nummovefaces; i++)
//	{
//		//move vertex to end position
//		VectorCopy(end, movefaces[i]->face_winding->points[movefacepoints[i]]);
//		//create new face plane
//		for (j = 0; j < 3; j++)
//		{
//			VectorCopy(movefaces[i]->face_winding->points[j], movefaces[i]->planepts[j]);
//		}
//		Face_MakePlane(movefaces[i]);
//	}
//	//if the brush is no longer convex
//	if (!Brush_Convex(b))
//	{
//		for (i = 0; i < nummovefaces; i++)
//		{
//			//move the vertex back to the initial position
//			VectorCopy(vertex, movefaces[i]->face_winding->points[movefacepoints[i]]);
//			//create new face plane
//			for (j = 0; j < 3; j++)
//			{
//				VectorCopy(movefaces[i]->face_winding->points[j], movefaces[i]->planepts[j]);
//			}
//			Face_MakePlane(movefaces[i]);
//		}
//		result = false;
//	}
//	//get texture crap right
//	for (i = 0; i < nummovefaces; i++)
//	{
//		Face_SetColor(b, movefaces[i], 1.0);
//		for (j = 0; j < movefaces[i]->face_winding->numpoints; j++)
//			EmitTextureCoordinates(movefaces[i]->face_winding->points[j], movefaces[i]->d_texture, movefaces[i]);
//	}
//
//	//now try to merge faces with their original faces
//	lastface = NULL;
//	for (face = b->brush_faces; face; face = nextface)
//	{
//		nextface = face->next;
//		if (!face->original)
//		{
//			lastface = face;
//			continue;
//		}
//		if (!face->plane.isPlaneEqual(face->original->plane, false))
//		{
//			lastface = face;
//			continue;
//		}
//		w = Winding_TryMerge(face->face_winding, face->original->face_winding, face->plane.normal, true);
//		if (!w)
//		{
//			lastface = face;
//			continue;
//		}
//		Winding_Free(face->original->face_winding);
//		face->original->face_winding = w;
//		//get texture crap right
//		Face_SetColor(b, face->original, 1.0);
//		for (j = 0; j < face->original->face_winding->numpoints; j++)
//			EmitTextureCoordinates(face->original->face_winding->points[j], face->original->d_texture, face->original);
//		//remove the face that was merged with the original
//		if (lastface) lastface->next = face->next;
//		else b->brush_faces = face->next;
//		Face_Free(face);
//	}
//	return result;
//}

/*
=================
Brush_MoveVertexes

- The input brush must be convex
- The input brush must have face windings.
- The output brush will be convex.
- Returns true if the WHOLE vertex movement is performed.
=================
*/

#define MAX_MOVE_FACES		64

int Brush_MoveVertex(brush_s *b, const vec3_c &vertex, const vec3_c &delta, vec3_c &end, bool bSnap)
{
	face_s *f, *face, *newface, *lastface, *nextface;
	face_s *movefaces[MAX_MOVE_FACES];
	int movefacepoints[MAX_MOVE_FACES];
	winding_t *w, tmpw;
	vec3_c start, mid;
	edPlane_c plane;
	int i, j, k, nummovefaces, result, done;
	float dot, front, back, frac, smallestfrac;

	result = true;
	//
	tmpw.numpoints = 3;
	tmpw.maxpoints = 3;
	start = vertex;
	end = vertex + delta;
	//snap or not?
	if (bSnap)
		for (i = 0; i < 3; i++)
			end[i] = floor(end[i] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
	//
	mid = end;
	//if the start and end are the same
	if (start.vectorCompare(end, 0.3)) return false;
	//the end point may not be the same as another vertex
	for (face = b->brush_faces; face; face = face->next)
	{
		w = face->face_winding;
		if (!w) continue;
		for (i = 0; i < w->numpoints; i++)
		{
			if (w->points[i].vectorCompare(end, 0.3))
			{
				end = vertex;;
				return false;
			}
		}
	}
	//
	done = false;
	while(!done)
	{
		//chop off triangles from all brush faces that use the to be moved vertex
		//store pointers to these chopped off triangles in movefaces[]
		nummovefaces = 0;
		for (face = b->brush_faces; face; face = face->next)
		{
			w = face->face_winding;
			if (!w) continue;
			for (i = 0; i < w->numpoints; i++)
			{
				if (w->points[i].vectorCompare(start, 0.2))
				{
					if (face->face_winding->numpoints <= 3)
					{
						movefacepoints[nummovefaces] = i;
						movefaces[nummovefaces++] = face;
						break;
					}
					dot = end.dotProduct(face->plane.normal) - face->plane.dist;
					//if the end point is in front of the face plane
					if (dot > 0.1)
					{
						//fanout triangle subdivision
						for (k = i; k < i + w->numpoints-3; k++)
						{
							tmpw.points[0] = w->points[i];
							tmpw.points[1] = w->points[(k+1) % w->numpoints];
							tmpw.points[2] = w->points[(k+2) % w->numpoints];
							//
							newface = Face_Clone(face);
							//get the original
							for (f = face; f->original; f = f->original) ;
							newface->original = f;
							//store the new winding
							if (newface->face_winding) Winding_Free(newface->face_winding);
							newface->face_winding = Winding_Clone(&tmpw);
							//get the texture
							newface->d_texture = QERApp_TryTextureForName( newface->texdef.getName() );
							//add the face to the brush
							newface->next = b->brush_faces;
							b->brush_faces = newface;
							//add this new triangle to the move faces
							movefacepoints[nummovefaces] = 0;
							movefaces[nummovefaces++] = newface;
						}
						//give the original face a new winding
						tmpw.points[0] = w->points[(i-2+w->numpoints) % w->numpoints];
						tmpw.points[1] = w->points[(i-1+w->numpoints) % w->numpoints];
						tmpw.points[2] = w->points[i];
						Winding_Free(face->face_winding);
						face->face_winding = Winding_Clone(&tmpw);
						//add the original face to the move faces
						movefacepoints[nummovefaces] = 2;
						movefaces[nummovefaces++] = face;
					}
					else
					{
						//chop a triangle off the face
						tmpw.points[0] = w->points[(i-1+w->numpoints) % w->numpoints];
						tmpw.points[1] = w->points[i];
						tmpw.points[2] = w->points[(i+1) % w->numpoints];
						//remove the point from the face winding
						Winding_RemovePoint(w, i);
						//get texture crap right
						Face_SetColor(b, face, 1.0);
						for (j = 0; j < w->numpoints; j++)
							EmitTextureCoordinates(w->points[j], face->d_texture, face);
						//make a triangle face
						newface = Face_Clone(face);
						//get the original
						for (f = face; f->original; f = f->original) ;
						newface->original = f;
						//store the new winding
						if (newface->face_winding) Winding_Free(newface->face_winding);
						newface->face_winding = Winding_Clone(&tmpw);
						//get the texture
						newface->d_texture = QERApp_TryTextureForName( newface->texdef.getName() );
						//add the face to the brush
						newface->next = b->brush_faces;
						b->brush_faces = newface;
						//
						movefacepoints[nummovefaces] = 1;
						movefaces[nummovefaces++] = newface;
					}
					break;
				}
			}
		}
		//now movefaces contains pointers to triangle faces that
		//contain the to be moved vertex
		//
		done = true;
		mid = end;
		smallestfrac = 1;
		for (face = b->brush_faces; face; face = face->next)
		{
			//check if there is a move face that has this face as the original
			for (i = 0; i < nummovefaces; i++)
			{
				if (movefaces[i]->original == face) break;
			}
			if (i >= nummovefaces) continue;
			//check if the original is not a move face itself
			for (j = 0; j < nummovefaces; j++)
			{
				if (face == movefaces[j]) break;
			}
			//if the original is not a move face itself
			if (j >= nummovefaces)
			{
				memcpy(&plane, &movefaces[i]->original->plane, sizeof(edPlane_c));
			}
			else
			{
				k = movefacepoints[j];
				w = movefaces[j]->face_winding;
				tmpw.points[0] = w->points[(k+1)%w->numpoints];
				tmpw.points[1] = w->points[(k+2)%w->numpoints];
				//
				k = movefacepoints[i];
				w = movefaces[i]->face_winding;
				tmpw.points[2] = w->points[(k+1)%w->numpoints];
				if (!plane.fromPoints(tmpw.points[0].getXYZ(), tmpw.points[1].getXYZ(), tmpw.points[2].getXYZ()))
				{
					tmpw.points[2] = w->points[(k+2)%w->numpoints];
					if (!plane.fromPoints(tmpw.points[0].getXYZ(), tmpw.points[1].getXYZ(), tmpw.points[2].getXYZ()))
						//this should never happen otherwise the face merge did a crappy job a previous pass
						continue;
				}
			}
			//now we've got the plane to check agains
			front = start.dotProduct(plane.normal) - plane.dist;
			back = end.dotProduct(plane.normal) - plane.dist;
			//if the whole move is at one side of the plane
			if (front < 0.01 && back < 0.01) continue;
			if (front > -0.01 && back > -0.01) continue;
			//if there's no movement orthogonal to this plane at all
			if (fabs(front-back) < 0.001) continue;
			//ok first only move till the plane is hit
			frac = front/(front-back);
			if (frac < smallestfrac)
			{
				mid[0] = start[0] + (end[0] - start[0]) * frac;
				mid[1] = start[1] + (end[1] - start[1]) * frac;
				mid[2] = start[2] + (end[2] - start[2]) * frac;
				smallestfrac = frac;
			}
			//
			done = false;
		}

		//move the vertex
		for (i = 0; i < nummovefaces; i++)
		{
			//move vertex to end position
			movefaces[i]->face_winding->points[movefacepoints[i]].setXYZ(mid);
			//create new face plane
			for (j = 0; j < 3; j++)
			{
				movefaces[i]->planepts[j] = movefaces[i]->face_winding->points[j].getXYZ();
			}
			Face_MakePlane(movefaces[i]);
			if (movefaces[i]->plane.normal.vectorLength() < 0.1)
				result = false;
		}
		//if the brush is no longer convex
		if (!result || !Brush_Convex(b))
		{
			for (i = 0; i < nummovefaces; i++)
			{
				//move the vertex back to the initial position
				movefaces[i]->face_winding->points[movefacepoints[i]].setXYZ(start);
				//create new face plane
				for (j = 0; j < 3; j++)
				{
					movefaces[i]->planepts[j] = movefaces[i]->face_winding->points[j].getXYZ();
				}
				Face_MakePlane(movefaces[i]);
			}
			result = false;
			end = start;
			done = true;
		}
		else
		{
			start = mid;
		}
		//get texture crap right
		for (i = 0; i < nummovefaces; i++)
		{
			Face_SetColor(b, movefaces[i], 1.0);
			for (j = 0; j < movefaces[i]->face_winding->numpoints; j++)
				EmitTextureCoordinates(movefaces[i]->face_winding->points[j], movefaces[i]->d_texture, movefaces[i]);
		}

		//now try to merge faces with their original faces
		lastface = NULL;
		for (face = b->brush_faces; face; face = nextface)
		{
			nextface = face->next;
			if (!face->original)
			{
				lastface = face;
				continue;
			}
			if (!face->plane.isPlaneEqual(face->original->plane, false))
			{
				lastface = face;
				continue;
			}
			w = Winding_TryMerge(face->face_winding, face->original->face_winding, face->plane.normal, true);
			if (!w)
			{
				lastface = face;
				continue;
			}
			Winding_Free(face->original->face_winding);
			face->original->face_winding = w;
			//get texture crap right
			Face_SetColor(b, face->original, 1.0);
			for (j = 0; j < face->original->face_winding->numpoints; j++)
				EmitTextureCoordinates(face->original->face_winding->points[j], face->original->d_texture, face->original);
			//remove the face that was merged with the original
			if (lastface) lastface->next = face->next;
			else b->brush_faces = face->next;
			Face_Free(face);
		}
	}
	return result;
}

/*
=================
Brush_InsertVertexBetween
=================
*/
int Brush_InsertVertexBetween(brush_s *b, const vec3_c &p1, const vec3_c &p2)
{
	face_s *face;
	winding_t *w, *neww;
	vec3_c point;
	int i, insert;

	if (p1.vectorCompare(p2, 0.4))
		return false;
	point = p1 + p2;
	point *= 0.5;
	insert = false;
	//the end point may not be the same as another vertex
	for (face = b->brush_faces; face; face = face->next)
	{
		w = face->face_winding;
		if (!w) continue;
		neww = NULL;
		for (i = 0; i < w->numpoints; i++)
		{
			if (!w->points[i].vectorCompare(p1, 0.1))
				continue;
			if (w->points[(i+1) % w->numpoints].vectorCompare(p2, 0.1))
			{
				neww = Winding_InsertPoint(w, point, (i+1) % w->numpoints);
				break;
			}
			else if (w->points[(i-1+w->numpoints) % w->numpoints].vectorCompare(p2, 0.3))
			{
				neww = Winding_InsertPoint(w, point, i);
				break;
			}
		}
		if (neww)
		{
			Winding_Free(face->face_winding);
			face->face_winding = neww;
			insert = true;
		}
	}
	return insert;
}


/*
=================
Brush_ResetFaceOriginals
=================
*/
void Brush_ResetFaceOriginals(brush_s *b)
{
	if(b == 0)
		return;
	face_s *face;

	for (face = b->brush_faces; face; face = face->next)
	{
		face->original = NULL;
	}
}

/*
=================
Brush_Parse

The brush is NOT linked to any list
=================
*/
//++timo FIXME: when using old brush primitives, the test loop for "Brush" and "patchDef2" "patchDef3" is ran
// before each face parsing. It works, but it's a performance hit
brush_s *Brush_Parse (void)
{
	brush_s		*b;
	face_s		*f;
	int			i,j;
	
	g_qeglobals.d_parsed_brushes++;
	b = new brush_s(false);;

	do
	{
		if (!GetToken (true))
			break;
		if (!strcmp (token, "}") )
			break;
		
		// handle "Brush" primitive
		if (_strcmpi(token, "brushDef") == 0)
		{
			// Timo parsing new brush format
			g_qeglobals.bPrimitBrushes=true;
			// check the map is not mixing the two kinds of brushes
			if (g_qeglobals.m_bBrushPrimitMode)
			{
				if (g_qeglobals.bOldBrushes)
					Sys_Printf("Warning : old brushes and brush primitive in the same file are not allowed ( Brush_Parse )\n");
			}
			//++Timo write new brush primitive -> old conversion code for Q3->Q2 conversions ?
			else
				Sys_Printf("Warning : conversion code from brush primitive not done ( Brush_Parse )\n");
			
			BrushPrimit_Parse(b);
			if (b == NULL)
			{
				Warning ("parsing brush primitive");
				return NULL;
			}
			else
			{
        		continue;
			}
		}
		if (_strcmpi(token, "patchDef2") == 0 || _strcmpi(token, "patchDef3") == 0)
		{
			//free (b);
			Brush_Free(b);
			
			// double string compare but will go away soon
			b = Patch_Parse(_strcmpi(token, "patchDef2") == 0);
			if (b == NULL)
			{
				Warning ("parsing patch/brush");
				return NULL;
			}
			else
			{
				continue;
			}
			// handle inline patch
		}
		else
		{
			// Timo parsing old brush format
			g_qeglobals.bOldBrushes=true;
			if (g_qeglobals.m_bBrushPrimitMode)
			{
				// check the map is not mixing the two kinds of brushes
				if (g_qeglobals.bPrimitBrushes)
					Sys_Printf("Warning : old brushes and brush primitive in the same file are not allowed ( Brush_Parse )\n");
				// set the "need" conversion flag
				g_qeglobals.bNeedConvert=true;
			}
			
			f = new face_s();
			
			// add the brush to the end of the chain, so
			// loading and saving a map doesn't reverse the order
			
			f->next = NULL;
			if (!b->brush_faces)
			{
				b->brush_faces = f;
			}
			else
			{
				face_s *scan;
				for (scan=b->brush_faces ; scan->next ; scan=scan->next)
					;
				scan->next = f;
			}
			
			// read the three point plane definition
			for (i=0 ; i<3 ; i++)
			{
				if (i != 0)
					GetToken (true);
				if (strcmp (token, "(") )
				{
					Warning ("parsing brush");
					return NULL;
				}
				
				for (j=0 ; j<3 ; j++)
				{
					GetToken (false);
					f->planepts[i][j] = atof(token);
				}
				
				GetToken (false);
				if (strcmp (token, ")") )
				{
					Warning ("parsing brush");
					return NULL;
				}
			}
		}


		{
			
			// read the texturedef
			GetToken (false);
			f->texdef.setName(token);
			if (token[0] == '(')
			{
				int i = 32;
			}
			GetToken (false);
			f->texdef.shift[0] = atoi(token);
			GetToken (false);
			f->texdef.shift[1] = atoi(token);
			GetToken (false);
			f->texdef.rotate = atoi(token);	
			GetToken (false);
			f->texdef.scale[0] = atof(token);
			GetToken (false);
			f->texdef.scale[1] = atof(token);
						
			// the flags and value field aren't necessarily present
			f->d_texture = QERApp_TryTextureForName( f->texdef.getName() );
			f->texdef.flags = 0;//f->d_texture->flags;
			// V: what is this used for
			f->texdef.value = 0; //f->d_texture->value;
			f->texdef.contents = f->d_texture->getEditorContentFlags();
			
			if (TokenAvailable ())
			{
				GetToken (false);
				f->texdef.contents = atoi(token);
				GetToken (false);
				f->texdef.flags = atoi(token);
				GetToken (false);
				f->texdef.value = atoi(token);
			}
			
		}
	} while (1);
	
	return b;
}




/*
=================
Brush_Write
save all brushes as Brush primitive format
=================
*/
void Brush_Write (brush_s *b, FILE *f)
{
	face_s	*fa;
	int		i;
	
	if (b->patchBrush)
	{
		Patch_Write(b->pPatch, f);
		return;
	}
	if (g_qeglobals.m_bBrushPrimitMode)
	{
		// save brush primitive format
		fprintf (f, "{\nbrushDef\n{\n");
		for (fa=b->brush_faces ; fa ; fa=fa->next)
		{
			// save planepts
			for (i=0 ; i<3 ; i++)
			{
				fprintf(f, "( ");
				for (int j = 0; j < 3; j++)
					if (fa->planepts[i][j] == static_cast<int>(fa->planepts[i][j]))
						fprintf(f, "%i ", static_cast<int>(fa->planepts[i][j]));
					else
						fprintf(f, "%f ", fa->planepts[i][j]);
				fprintf(f, ") ");
			}
			// save texture coordinates
			fprintf(f,"( ( ");
			for (i=0 ; i<3 ; i++)
				if (fa->brushprimit_texdef.coords[0][i] == static_cast<int>(fa->brushprimit_texdef.coords[0][i]))
					fprintf(f,"%i ",static_cast<int>(fa->brushprimit_texdef.coords[0][i]));
				else
					fprintf(f,"%f ",fa->brushprimit_texdef.coords[0][i]);
			fprintf(f,") ( ");
			for (i=0 ; i<3 ; i++)
				if (fa->brushprimit_texdef.coords[1][i] == static_cast<int>(fa->brushprimit_texdef.coords[1][i]))
					fprintf(f,"%i ",static_cast<int>(fa->brushprimit_texdef.coords[1][i]));
				else
					fprintf(f,"%f ",fa->brushprimit_texdef.coords[1][i]);
			fprintf(f,") ) ");
			// save texture attribs

      char *pName = strlen(fa->texdef.getName()) > 0 ? fa->texdef.getName() : "unnamed";
			fprintf(f, "%s ", pName );
			fprintf(f, "%i %i %i\n", fa->texdef.contents, fa->texdef.flags, fa->texdef.value);
		}
		fprintf (f, "}\n}\n");
	}
	else
	{
		fprintf (f, "{\n");
		for (fa=b->brush_faces ; fa ; fa=fa->next)
		{
			for (i=0 ; i<3 ; i++)
			{
				fprintf(f, "( ");
				for (int j = 0; j < 3; j++)
				{
					if (fa->planepts[i][j] == static_cast<int>(fa->planepts[i][j]))
						fprintf(f, "%i ", static_cast<int>(fa->planepts[i][j]));
					else
						fprintf(f, "%f ", fa->planepts[i][j]);
				}
				fprintf(f, ") ");
			}
			
	
			{
				const char *pname = fa->texdef.getName();
				if (pname[0] == 0)
					pname = "unnamed";
				
				fprintf (f, "%s %i %i %i ", pname,
					(int)fa->texdef.shift[0], (int)fa->texdef.shift[1],
					(int)fa->texdef.rotate);
				
				if (fa->texdef.scale[0] == (int)fa->texdef.scale[0])
					fprintf (f, "%i ", (int)fa->texdef.scale[0]);
				else
					fprintf (f, "%f ", (float)fa->texdef.scale[0]);
				if (fa->texdef.scale[1] == (int)fa->texdef.scale[1])
					fprintf (f, "%i", (int)fa->texdef.scale[1]);
				else
					fprintf (f, "%f", (float)fa->texdef.scale[1]);
				
				fprintf (f, " %i %i %i", fa->texdef.contents, fa->texdef.flags, fa->texdef.value);
			}
			fprintf (f, "\n");
		}
		fprintf (f, "}\n");
	}
}

/*
=================
QERApp_MapPrintf_MEMFILE
callback for surface properties plugin
must fit a PFN_QERAPP_MAPPRINTF ( see isurfaceplugin.h )
=================
*/
// carefully initialize !
CMemFile * g_pMemFile;
void WINAPI QERApp_MapPrintf_MEMFILE( char *text, ... )
{
	va_list argptr;
	char	buf[32768];

	va_start (argptr,text);
	vsprintf (buf, text,argptr);
	va_end (argptr);

	MemFile_fprintf( g_pMemFile, buf );
}

/*
=================
Brush_Write to a CMemFile*
save all brushes as Brush primitive format
=================
*/
void Brush_Write (brush_s *b, CMemFile *pMemFile)
{
	face_s	*fa;
	int		i;
	
	if (b->patchBrush)
	{
		Patch_Write(b->pPatch, pMemFile);
		return;
	}

	if (g_qeglobals.m_bBrushPrimitMode)
	{
		// brush primitive format
		MemFile_fprintf (pMemFile, "{\nBrushDef\n{\n");
		for (fa=b->brush_faces ; fa ; fa=fa->next)
		{
			// save planepts
			for (i=0 ; i<3 ; i++)
			{
				MemFile_fprintf(pMemFile, "( ");
				for (int j = 0; j < 3; j++)
					if (fa->planepts[i][j] == static_cast<int>(fa->planepts[i][j]))
						MemFile_fprintf(pMemFile, "%i ", static_cast<int>(fa->planepts[i][j]));
					else
						MemFile_fprintf(pMemFile, "%f ", fa->planepts[i][j]);
				MemFile_fprintf(pMemFile, ") ");
			}
			// save texture coordinates
			MemFile_fprintf(pMemFile,"( ( ");
			for (i=0 ; i<3 ; i++)
				if (fa->brushprimit_texdef.coords[0][i] == static_cast<int>(fa->brushprimit_texdef.coords[0][i]))
					MemFile_fprintf(pMemFile,"%i ",static_cast<int>(fa->brushprimit_texdef.coords[0][i]));
				else
					MemFile_fprintf(pMemFile,"%f ",fa->brushprimit_texdef.coords[0][i]);
			MemFile_fprintf(pMemFile,") ( ");
			for (i=0 ; i<3 ; i++)
				if (fa->brushprimit_texdef.coords[1][i] == static_cast<int>(fa->brushprimit_texdef.coords[1][i]))
					MemFile_fprintf(pMemFile,"%i ",static_cast<int>(fa->brushprimit_texdef.coords[1][i]));
				else
					MemFile_fprintf(pMemFile,"%f ",fa->brushprimit_texdef.coords[1][i]);
			MemFile_fprintf(pMemFile,") ) ");
			// save texture attribs
      char *pName = strlen(fa->texdef.getName()) > 0 ? fa->texdef.getName() : "unnamed";
			MemFile_fprintf(pMemFile, "%s ", pName);
			MemFile_fprintf(pMemFile, "%i %i %i\n", fa->texdef.contents, fa->texdef.flags, fa->texdef.value);
		}
		MemFile_fprintf (pMemFile, "}\n}\n");
	}
	else
	{
		// old brushes format
		// also handle surface properties plugin
		MemFile_fprintf (pMemFile, "{\n");
		for (fa=b->brush_faces ; fa ; fa=fa->next)
		{
			for (i=0 ; i<3 ; i++)
			{
				MemFile_fprintf(pMemFile, "( ");
				for (int j = 0; j < 3; j++)
				{
					if (fa->planepts[i][j] == static_cast<int>(fa->planepts[i][j]))
						MemFile_fprintf(pMemFile, "%i ", static_cast<int>(fa->planepts[i][j]));
					else
						MemFile_fprintf(pMemFile, "%f ", fa->planepts[i][j]);
				}
				MemFile_fprintf(pMemFile, ") ");
			}
	
			{
				const char *pname = fa->texdef.getName();
				if (pname[0] == 0)
					pname = "unnamed";
				
				MemFile_fprintf (pMemFile, "%s %i %i %i ", pname,
					(int)fa->texdef.shift[0], (int)fa->texdef.shift[1],
					(int)fa->texdef.rotate);
				
				if (fa->texdef.scale[0] == (int)fa->texdef.scale[0])
					MemFile_fprintf (pMemFile, "%i ", (int)fa->texdef.scale[0]);
				else
					MemFile_fprintf (pMemFile, "%f ", (float)fa->texdef.scale[0]);
				if (fa->texdef.scale[1] == (int)fa->texdef.scale[1])
					MemFile_fprintf (pMemFile, "%i", (int)fa->texdef.scale[1]);
				else
					MemFile_fprintf (pMemFile, "%f", (float)fa->texdef.scale[1]);
				
				MemFile_fprintf (pMemFile, " %i %i %i", fa->texdef.contents, fa->texdef.flags, fa->texdef.value);
			}
			MemFile_fprintf (pMemFile, "\n");
		}
		MemFile_fprintf (pMemFile, "}\n");
	}
	
	
}


/*
=============
Brush_Create

Create non-textured blocks for entities
The brush is NOT linked to any list
=============
*/
brush_s	*Brush_Create (vec3_t mins, vec3_t maxs, texdef_t *texdef)
{
	int		i, j;
	vec3_t	pts[4][2];
	face_s	*f;
	brush_s	*b;

	// brush primitive mode : convert texdef to brushprimit_texdef ?
	// most of the time texdef is empty
	if (g_qeglobals.m_bBrushPrimitMode)
	{
		// check texdef is empty .. if there are cases it's not we need to write some conversion code
		if (texdef->shift[0]!=0 || texdef->shift[1]!=0 || texdef->scale[0]!=0 || texdef->scale[1]!=0 || texdef->rotate!=0)
			Sys_Printf("Warning : non-zero texdef detected in Brush_Create .. need brush primitive conversion\n");
	}

	for (i=0 ; i<3 ; i++)
	{
		if (maxs[i] < mins[i])
			Error ("Brush_InitSolid: backwards");
	}

	b = new brush_s(false);;
	
	pts[0][0][0] = mins[0];
	pts[0][0][1] = mins[1];
	
	pts[1][0][0] = mins[0];
	pts[1][0][1] = maxs[1];
	
	pts[2][0][0] = maxs[0];
	pts[2][0][1] = maxs[1];
	
	pts[3][0][0] = maxs[0];
	pts[3][0][1] = mins[1];
	
	for (i=0 ; i<4 ; i++)
	{
		pts[i][0][2] = mins[2];
		pts[i][1][0] = pts[i][0][0];
		pts[i][1][1] = pts[i][0][1];
		pts[i][1][2] = maxs[2];
	}

	for (i=0 ; i<4 ; i++)
	{
		f = new face_s();
		f->texdef = *texdef;
		f->texdef.flags &= ~SURF_KEEP;
		f->texdef.contents &= ~CONTENTS_KEEP;
		f->next = b->brush_faces;
		b->brush_faces = f;
		j = (i+1)%4;

		f->planepts[0] = pts[j][1];
		f->planepts[1] = pts[i][1];
		f->planepts[2] = pts[i][0];
	}
	
	f = new face_s();
	f->texdef = *texdef;
	f->texdef.flags &= ~SURF_KEEP;
	f->texdef.contents &= ~CONTENTS_KEEP;
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[0] = pts[0][1];
	f->planepts[1] = pts[1][1];
	f->planepts[2] = pts[2][1];

	f = new face_s();
	f->texdef = *texdef;
	f->texdef.flags &= ~SURF_KEEP;
	f->texdef.contents &= ~CONTENTS_KEEP;
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[0] = pts[2][0];
	f->planepts[1] = pts[1][0];
	f->planepts[2] = pts[0][0];

	return b;
}

/*
=============
Brush_CreatePyramid

Create non-textured pyramid for light entities
The brush is NOT linked to any list
=============
*/
brush_s	*Brush_CreatePyramid (vec3_t mins, vec3_t maxs, texdef_t *texdef)
{
	int i;
	//++timo handle new brush primitive ? return here ??
	return Brush_Create(mins, maxs, texdef);

	for (i=0 ; i<3 ; i++)
		if (maxs[i] < mins[i])
			Error ("Brush_InitSolid: backwards");

	brush_s* b = new brush_s(false);;

	vec3_t corners[4];

	float fMid = Q_rint(mins[2] + (Q_rint((maxs[2] - mins[2]) / 2)));

	corners[0][0] = mins[0];
	corners[0][1] = mins[1];
	corners[0][2] = fMid;

	corners[1][0] = mins[0];
	corners[1][1] = maxs[1];
	corners[1][2] = fMid;

	corners[2][0] = maxs[0];
	corners[2][1] = maxs[1];
	corners[2][2] = fMid;

	corners[3][0] = maxs[0];
	corners[3][1] = mins[1];
	corners[3][2] = fMid;

	vec3_c top, bottom;

	top[0] = Q_rint(mins[0] + ((maxs[0] - mins[0]) / 2));
	top[1] = Q_rint(mins[1] + ((maxs[1] - mins[1]) / 2));
	top[2] = Q_rint(maxs[2]);

	bottom = top;
	bottom[2] = mins[2];

	// sides
	for (i = 0; i < 4; i++)
	{
		face_s* f = new face_s();
		f->texdef = *texdef;
		f->texdef.flags &= ~SURF_KEEP;
		f->texdef.contents &= ~CONTENTS_KEEP;
		f->next = b->brush_faces;
		b->brush_faces = f;
		int j = (i+1)%4;

		f->planepts[0] = top;
		f->planepts[1] = corners[i];
		f->planepts[2] = corners[j];

		f = new face_s();
		f->texdef = *texdef;
		f->texdef.flags &= ~SURF_KEEP;
		f->texdef.contents &= ~CONTENTS_KEEP;
		f->next = b->brush_faces;
		b->brush_faces = f;

		f->planepts[2] = bottom;
		f->planepts[1] = corners[i];
		f->planepts[0] = corners[j];
	}

	return b;
}




/*
=============
Brush_MakeSided

Makes the current brush have the given number of 2d sides
=============
*/
void Brush_MakeSided (int sides)
{
	int		i, axis;
	vec3_c	mins, maxs;
	brush_s	*b;
	texdef_t	*texdef;
	face_s	*f;
	vec3_t	mid;
	float	width;
	float	sv, cv;

	if (sides < 3)
	{
		Sys_Status ("Bad sides number", 0);
		return;
	}

	if (sides >= MAX_POINTS_ON_WINDING-4)
	{
		Sys_Printf("too many sides.\n");
		return;
	}

	if (!QE_SingleBrush ())
	{
		Sys_Status ("Must have a single brush selected", 0 );
		return;
	}

	b = selected_brushes.next;
	mins = b->getMins();
	maxs = b->getMaxs();
	texdef = &g_qeglobals.d_texturewin.texdef;

	Brush_Free (b);

	if (g_pParentWnd->ActiveXY())
	{
		switch(g_pParentWnd->ActiveXY()->GetViewType())
		{
			case XY: axis = 2; break;
			case XZ: axis = 1; break;
			case YZ: axis = 0; break;
		}
	}
	else
	{
		axis = 2;
	}

	// find center of brush
	width = 8;
	for (i = 0; i < 3; i++)
	{
		mid[i] = (maxs[i] + mins[i]) * 0.5;
		if (i == axis) continue;
		if ((maxs[i] - mins[i]) * 0.5 > width)
			width = (maxs[i] - mins[i]) * 0.5;
	}

	b = new brush_s(false);;
		
	// create top face
	f = new face_s();
	f->texdef = *texdef;
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[2][(axis+1)%3] = mins[(axis+1)%3]; f->planepts[2][(axis+2)%3] = mins[(axis+2)%3]; f->planepts[2][axis] = maxs[axis];
	f->planepts[1][(axis+1)%3] = maxs[(axis+1)%3]; f->planepts[1][(axis+2)%3] = mins[(axis+2)%3]; f->planepts[1][axis] = maxs[axis];
	f->planepts[0][(axis+1)%3] = maxs[(axis+1)%3]; f->planepts[0][(axis+2)%3] = maxs[(axis+2)%3]; f->planepts[0][axis] = maxs[axis];

	// create bottom face
	f = new face_s();
	f->texdef = *texdef;
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[0][(axis+1)%3] = mins[(axis+1)%3]; f->planepts[0][(axis+2)%3] = mins[(axis+2)%3]; f->planepts[0][axis] = mins[axis];
	f->planepts[1][(axis+1)%3] = maxs[(axis+1)%3]; f->planepts[1][(axis+2)%3] = mins[(axis+2)%3]; f->planepts[1][axis] = mins[axis];
	f->planepts[2][(axis+1)%3] = maxs[(axis+1)%3]; f->planepts[2][(axis+2)%3] = maxs[(axis+2)%3]; f->planepts[2][axis] = mins[axis];

	for (i=0 ; i<sides ; i++)
	{
		f = new face_s();
		f->texdef = *texdef;
		f->next = b->brush_faces;
		b->brush_faces = f;

		sv = sin (i*3.14159265*2/sides);
		cv = cos (i*3.14159265*2/sides);

		f->planepts[0][(axis+1)%3] = floor(mid[(axis+1)%3]+width*cv+0.5);
		f->planepts[0][(axis+2)%3] = floor(mid[(axis+2)%3]+width*sv+0.5);
		f->planepts[0][axis] = mins[axis];

		f->planepts[1][(axis+1)%3] = f->planepts[0][(axis+1)%3];
		f->planepts[1][(axis+2)%3] = f->planepts[0][(axis+2)%3];
		f->planepts[1][axis] = maxs[axis];

		f->planepts[2][(axis+1)%3] = floor(f->planepts[0][(axis+1)%3] - width*sv + 0.5);
		f->planepts[2][(axis+2)%3] = floor(f->planepts[0][(axis+2)%3] + width*cv + 0.5);
		f->planepts[2][axis] = maxs[axis];
	}

	Brush_AddToList (b, &selected_brushes);

	Entity_LinkBrush (world_entity, b);

	Brush_Build( b );

	Sys_UpdateWindows (W_ALL);
}



/*
=============
Brush_Free

Frees the brush with all of its faces and display list.
Unlinks the brush from whichever chain it is in.
Decrements the owner entity's brushcount.
Removes owner entity if this was the last brush
unless owner is the world.
Removes from groups
=============
*/
void Brush_Free (brush_s *b, bool bRemoveNode)
{
	face_s	*f, *next;

	// free the patch if it's there
	if (b->patchBrush)
	{
		Patch_Delete(b->pPatch);
	}

	// free faces
	for (f=b->brush_faces ; f ; f=next)
	{
		next = f->next;
		Face_Free( f );
	}

	// unlink from active/selected list
	if (b->next)
		Brush_RemoveFromList (b);

	// unlink from entity list
	if (b->onext)
		Entity_UnlinkBrush (b);

	delete b;
}

/*
=============
Face_MemorySize
=============
*/
int Face_MemorySize(face_s *f )
{
	int size = 0;

	if (f->face_winding)
	{
		size += _msize(f->face_winding);
	}
	//f->texdef.~texdef_t();;
	size += _msize(f);
	return size;
}

/*
=============
Brush_MemorySize
=============
*/
int Brush_MemorySize(brush_s *b)
{
	face_s	*f;
	int size = 0;

	//
	if (b->patchBrush)
	{
		size += Patch_MemorySize(b->pPatch);
	}
	//
	for (f = b->brush_faces; f; f = f->next)
	{
		size += Face_MemorySize(f);
	}
	size += _msize(b);
	return size;
}


/*
============
Brush_Clone

Does NOT add the new brush to any lists
============
*/
brush_s *Brush_Clone (brush_s *b)
{
	brush_s	*n = NULL;
	face_s	*f, *nf;

	if (b->patchBrush)
	{
		patchMesh_c *p = Patch_Duplicate(b->pPatch);
		Brush_RemoveFromList(p->pSymbiot);
		Entity_UnlinkBrush(p->pSymbiot);
		n = p->pSymbiot;
	}
	else
	{
  	n = new brush_s(false);;
		n->owner = b->owner;
		for (f=b->brush_faces ; f ; f=f->next)
		{
			nf = Face_Clone( f );
			nf->next = n->brush_faces;
			n->brush_faces = nf;
		}
	}

	return n;
}



/*
============
Brush_Clone

Does NOT add the new brush to any lists
============
*/
brush_s *Brush_FullClone(brush_s *b)
{
	brush_s	*n = NULL;
	face_s *f, *nf, *f2, *nf2;
	int j;

	if (b->patchBrush)
	{
		patchMesh_c *p = Patch_Duplicate(b->pPatch);
		Brush_RemoveFromList(p->pSymbiot);
		Entity_UnlinkBrush(p->pSymbiot);
		n = p->pSymbiot;
		n->owner = b->owner;
		Brush_Build(n);
	}
	else
	{
  	n = new brush_s(false);;
		n->owner = b->owner;
		n->bounds = b->bounds;
		//
		for (f = b->brush_faces; f; f = f->next)
		{
			if (f->original) continue;
			nf = Face_FullClone(f);
			nf->next = n->brush_faces;
			n->brush_faces = nf;
			//copy all faces that have the original set to this face
			for (f2 = b->brush_faces; f2; f2 = f2->next)
			{
				if (f2->original == f)
				{
					nf2 = Face_FullClone(f2);
					nf2->next = n->brush_faces;
					n->brush_faces = nf2;
					//set original
					nf2->original = nf;
				}
			}
		}
		for (nf = n->brush_faces; nf; nf = nf->next)
		{
			Face_SetColor(n, nf, 1.0);
			if (nf->face_winding)
      {
        if (g_qeglobals.m_bBrushPrimitMode)
    			EmitBrushPrimitTextureCoordinates(nf,nf->face_winding);
        else
        {
				  for (j = 0; j < nf->face_winding->numpoints; j++)
  					EmitTextureCoordinates(nf->face_winding->points[j], nf->d_texture, nf);
        }
      }
		}
  }
	return n;
}

/*
==============
Brush_Ray

Itersects a ray with a brush
Returns the face hit and the distance along the ray the intersection occured at
Returns NULL and 0 if not hit at all
==============
*/
face_s *Brush_Ray (const vec3_c &origin, const vec3_c &dir, brush_s *b, float *dist)
{
	if(b == 0)
		return 0;
	face_s	*f = 0;
	face_s *firstface = 0;
	vec3_c	p1, p2;
	float	frac, d1, d2;
	int		i;

	p1 = origin;
	for (i=0 ; i<3 ; i++)
		p2[i] = p1[i] + dir[i] * 131072*2; // max world coord

	for (f=b->brush_faces ; f ; f=f->next)
	{
		d1 = p1.dotProduct(f->plane.normal) - f->plane.dist;
		d2 = p2.dotProduct(f->plane.normal) - f->plane.dist;
		if (d1 >= 0 && d2 >= 0)
		{
			*dist = 0;
			return NULL;	// ray is on front side of face
		}
		if (d1 <=0 && d2 <= 0)
			continue;
	// clip the ray to the plane
		frac = d1 / (d1 - d2);
		if (d1 > 0)
		{
			firstface = f;
			for (i=0 ; i<3 ; i++)
				p1[i] = p1[i] + frac *(p2[i] - p1[i]);
		}
		else
		{
			for (i=0 ; i<3 ; i++)
				p2[i] = p1[i] + frac *(p2[i] - p1[i]);
		}
	}

	// find distance p1 is along dir
	p1 -= origin;
	d1 = p1.dotProduct(dir);

	*dist = d1;

	return firstface;
}

//PGM
face_s *Brush_Point (const vec3_c &origin, brush_s *b)
{
	face_s	*f;
	float	d1;

	for (f=b->brush_faces ; f ; f=f->next)
	{
		d1 = origin.dotProduct(f->plane.normal) - f->plane.dist;
		if (d1 > 0)
		{
			return NULL;	// point is on front side of face
		}
	}

	return b->brush_faces;
}
//PGM


void	Brush_AddToList (brush_s *b, brush_s *list)
{
	if (b->next || b->prev)
		Error ("Brush_AddToList: allready linked");
	
	if (list == &selected_brushes || list == &active_brushes)
	{
		if (b->patchBrush && list == &selected_brushes)
		{
			Patch_Select(b->pPatch);
		}
	}
	b->next = list->next;
	list->next->prev = b;
	list->next = b;
	b->prev = list;
}

void	Brush_RemoveFromList (brush_s *b)
{
	if (!b->next || !b->prev)
		Error ("Brush_RemoveFromList: not linked");
	
	if (b->patchBrush)
	{
		Patch_Deselect(b->pPatch);
		//Patch_Deselect(b->nPatchID);
	}
	b->next->prev = b->prev;
	b->prev->next = b->next;
	b->next = b->prev = NULL;
}

/*
===============
SetFaceTexdef

Doesn't set the curve flags

NOTE : ( TTimo )
	never trust f->d_texture here, f->texdef and f->d_texture are out of sync when called by Brush_SetTexture
	use QERApp_TryTextureForName() to find the right shader
	FIXME : send the right shader ( mtrAPI_i * ) in the parameters ?

TTimo: surface plugin, added an IPluginTexdef* parameter
		if not NULL, get ->Copy() of it into the face ( and remember to hook )
		if NULL, ask for a default
===============
*/
void SetFaceTexdef (brush_s *b, face_s *f, texdef_t *texdef, brushprimit_texdef_s *brushprimit_texdef, bool bFitScale) {
	int		oldFlags;
	int		oldContents;
	face_s	*tf;

	oldFlags = f->texdef.flags;
	oldContents = f->texdef.contents;
	if (g_qeglobals.m_bBrushPrimitMode)
	{
		f->texdef = *texdef;
		ConvertTexMatWithQTexture( brushprimit_texdef, NULL, &f->brushprimit_texdef, QERApp_TryTextureForName( f->texdef.getName() ) );
	}
	else
		if (bFitScale)
		{
			f->texdef = *texdef;
			// fit the scaling of the texture on the actual plane
			vec3_c p1,p2,p3; // absolute coordinates
			// compute absolute coordinates
			ComputeAbsolute(f,p1,p2,p3);
			// compute the scale
			vec3_c vx = p2 - p1;
			vx.normalize();
			vec3_c vy = p3 - p1;
			vy.normalize();
			// assign scale
			vx *= texdef->scale[0];
			vy *= texdef->scale[1];
			p2 = p1 + vx;
			p3 = p1 + vy; 
			// compute back shift scale rot
			AbsoluteToLocal(f->plane,f,p1,p2,p3);
		}
		else
			f->texdef = *texdef;
	f->texdef.flags = (f->texdef.flags & ~SURF_KEEP) | (oldFlags & SURF_KEEP);
	f->texdef.contents = (f->texdef.contents & ~CONTENTS_KEEP) | (oldContents & CONTENTS_KEEP);
	
	f->d_texture = QERApp_TryTextureForName(f->texdef.getName());

	// if this is a curve face, set all other curve faces to the same texdef
	if (f->texdef.flags & SURF_CURVE) 
	{
		for (tf = b->brush_faces ; tf ; tf = tf->next) 
		{
			if (tf->texdef.flags & SURF_CURVE) 
				tf->texdef = f->texdef;
		}
	}
}


void Brush_SetTexture (brush_s *b, texdef_t *texdef, brushprimit_texdef_s *brushprimit_texdef, bool bFitScale)
{
	for (face_s* f = b->brush_faces ; f ; f = f->next) 
	{
		SetFaceTexdef (b, f, texdef, brushprimit_texdef, bFitScale);
	}
	Brush_Build( b );
	if (b->patchBrush)
	{
		//++timo clean
//		Sys_Printf("WARNING: Brush_SetTexture needs surface plugin code for patches\n");
		Patch_SetTexture(b->pPatch, texdef );
	}
}


bool ClipLineToFace (vec3_c &p1, vec3_c &p2, face_s *f)
{
	float	d1, d2, fr;
	int		i;
	float	*v;

	d1 = p1.dotProduct(f->plane.normal) - f->plane.dist;
	d2 = p2.dotProduct(f->plane.normal) - f->plane.dist;

	if (d1 >= 0 && d2 >= 0)
		return false;		// totally outside
	if (d1 <= 0 && d2 <= 0)
		return true;		// totally inside

	fr = d1 / (d1 - d2);

	if (d1 > 0)
		v = p1;
	else
		v = p2;

	for (i=0 ; i<3 ; i++)
		v[i] = p1[i] + fr*(p2[i] - p1[i]);

	return true;
}


int AddPlanept (float *f)
{
	int		i;

	for (i=0 ; i<g_qeglobals.d_num_move_points ; i++)
		if (g_qeglobals.d_move_points[i] == f)
			return 0;
	g_qeglobals.d_move_points[g_qeglobals.d_num_move_points++] = f;
	return 1;
}

/*
==============
Brush_SelectFaceForDragging

Adds the faces planepts to move_points, and
rotates and adds the planepts of adjacent face if shear is set
==============
*/
void Brush_SelectFaceForDragging (brush_s *b, face_s *f, bool shear)
{
	int		i;
	face_s	*f2;
	winding_t	*w;
	float	d;
	brush_s	*b2;
	int		c;

	if (b->owner->eclass->isFixedSize())
		return;

	c = 0;
	for (i=0 ; i<3 ; i++)
		c += AddPlanept (f->planepts[i]);
	if (c == 0)
		return;		// allready completely added

	// select all points on this plane in all brushes the selection
	for (b2=selected_brushes.next ; b2 != &selected_brushes ; b2 = b2->next)
	{
		if (b2 == b)
			continue;
		for (f2=b2->brush_faces ; f2 ; f2=f2->next)
		{
			for (i=0 ; i<3 ; i++)
				if (fabs(f2->planepts[i].dotProduct(f->plane.normal)
				-f->plane.dist) > ON_EPSILON)
					break;
			if (i==3)
			{	// move this face as well
				Brush_SelectFaceForDragging (b2, f2, shear);
				break;
			}
		}
	}


	// if shearing, take all the planes adjacent to 
	// selected faces and rotate their points so the
	// edge clipped by a selcted face has two of the points
	if (!shear)
		return;

	for (f2=b->brush_faces ; f2 ; f2=f2->next)
	{
		if (f2 == f)
			continue;
		w = Brush_MakeFaceWinding (b, f2);
		if (!w)
			continue;

		// any points on f will become new control points
		for (i=0 ; i<w->numpoints ; i++)
		{
			d = w->points[i].dotProduct(f->plane.normal) 
				- f->plane.dist;
			if (d > -ON_EPSILON && d < ON_EPSILON)
				break;
		}

		//
		// if none of the points were on the plane,
		// leave it alone
		//
		if (i != w->numpoints)
		{
			if (i == 0)
			{	// see if the first clockwise point was the
				// last point on the winding
				d = w->points[w->numpoints-1].dotProduct(f->plane.normal) - f->plane.dist;
				if (d > -ON_EPSILON && d < ON_EPSILON)
					i = w->numpoints - 1;
			}

			AddPlanept (f2->planepts[0]);

			f2->planepts[0] = w->points[i].getXYZ();
			if (++i == w->numpoints)
				i = 0;
			
			// see if the next point is also on the plane
			d = w->points[i].dotProduct(f->plane.normal) - f->plane.dist;
			if (d > -ON_EPSILON && d < ON_EPSILON)
				AddPlanept (f2->planepts[1]);

			f2->planepts[1] = w->points[i].getXYZ();
			if (++i == w->numpoints)
				i = 0;

			// the third point is never on the plane

			f2->planepts[2] = w->points[i].getXYZ();
		}

		free(w);
	}
}

/*
==============
Brush_SideSelect

The mouse click did not hit the brush, so grab one or more side
planes for dragging
==============
*/
void Brush_SideSelect (brush_s *b, vec3_t origin, vec3_t dir
					   , bool shear)
{
	face_s	*f, *f2;
	vec3_c	p1, p2;
	if(b == 0)
		return;
  //if (b->patchBrush)
  //  return;
    //Patch_SideSelect(b->nPatchID, origin, dir);
	for (f=b->brush_faces ; f ; f=f->next)
	{
		p1 = origin;
		p2.vectorMA (origin, 131072*2, dir);

		for (f2=b->brush_faces ; f2 ; f2=f2->next)
		{
			if (f2 == f)
				continue;
			ClipLineToFace (p1, p2, f2);
		}

		if (f2)
			continue;

		if (p1.vectorCompare(origin))
			continue;
		if (ClipLineToFace (p1, p2, f))
			continue;

		Brush_SelectFaceForDragging (b, f, shear);
	}

	
}

void Brush_BuildWindings( brush_s *b, bool bSnap )
{
	winding_t *w;
	face_s    *face;

	if (bSnap)
		Brush_SnapPlanepts( b );

	// clear the mins/maxs bounds
	b->bounds.clear();

	Brush_MakeFacePlanes (b);

	face = b->brush_faces;

	float fCurveColor = 1.0;

	for ( ; face ; face=face->next)
	{
		int i;
		free(face->face_winding);
		w = face->face_winding = Brush_MakeFaceWinding (b, face);
		face->d_texture = QERApp_TryTextureForName( face->texdef.getName() );

		if (!w)
			continue;
	
		for (i=0 ; i<w->numpoints ; i++)
		{
			// add to bounding box
			b->bounds.addPoint(w->points[i].getXYZ());
		}
		// setup s and t vectors, and set color
		//if (!g_PrefsDlg.m_bGLLighting)
    //{
		  Face_SetColor (b, face, fCurveColor);
    //}

		fCurveColor -= .10;
		if (fCurveColor <= 0)
			fCurveColor = 1.0;

		// computing ST coordinates for the windings
		if (g_qeglobals.m_bBrushPrimitMode)
		{
			if (g_qeglobals.bNeedConvert)
			{
				// we have parsed old brushes format and need conversion
				// convert old brush texture representation to new format
				FaceToBrushPrimitFace(face);
#ifdef _DEBUG
				// use old texture coordinates code to check against
			    for (i=0 ; i<w->numpoints ; i++)
					EmitTextureCoordinates( w->points[i], face->d_texture, face);
#endif
			}
			// use new texture representation to compute texture coordinates
			// in debug mode we will check against old code and warn if there are differences
			EmitBrushPrimitTextureCoordinates(face,w);
		}
		else
		{
		    for (i=0 ; i<w->numpoints ; i++)
				EmitTextureCoordinates( w->points[i], face->d_texture, face);
		}
	}
}

/*
==================
Brush_RemoveEmptyFaces

Frees any overconstraining faces
==================
*/
void Brush_RemoveEmptyFaces ( brush_s *b )
{
	face_s	*f, *next;

	f = b->brush_faces;
	b->brush_faces = NULL;

	for ( ; f ; f=next)
	{
		next = f->next;
		if (!f->face_winding)
			Face_Free (f);
		else
		{
			f->next = b->brush_faces;
			b->brush_faces = f;
		}
	}
}

void Brush_SnapToGrid(brush_s *pb)
{
	for (face_s *f = pb->brush_faces ; f; f = f->next)
	{
		for (int i = 0 ;i < 3 ;i++)
		{
			for (int j = 0 ;j < 3 ; j++)
			{
				f->planepts[i][j] = floor (f->planepts[i][j] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
			}
		}
	}
	Brush_Build(pb);
}

void Brush_Rotate(brush_s *b, vec3_t vAngle, vec3_t vOrigin, bool bBuild)
{
	for (face_s* f=b->brush_faces ; f ; f=f->next)
	{
		for (int i=0 ; i<3 ; i++)
		{
			VectorRotate(f->planepts[i], vAngle, vOrigin, f->planepts[i]);
		}
	}
	if (bBuild)
	{
		Brush_Build(b, false, false);
	}
}

void Brush_Center(brush_s *b, const vec3_c &vNewCenter)
{
	vec3_c vMid;
	// get center of the brush
	for (int j = 0; j < 3; j++)
	{
		vMid[j] = b->getMins()[j] + abs((b->getMaxs()[j] - b->getMins()[j]) * 0.5);
	}
	// calc distance between centers
	vMid = vNewCenter - vMid;
	Brush_Move(b, vMid, true);

}

// only designed for fixed size entity brushes
void Brush_Resize(brush_s *b, vec3_t vMin, vec3_t vMax)
{
	brush_s *b2 = Brush_Create(vMin, vMax, &b->brush_faces->texdef);

	face_s *next;
	for (face_s *f=b->brush_faces ; f ; f=next)
	{
		next = f->next;
		Face_Free( f );
	}

	b->brush_faces = b2->brush_faces;

	// unlink from active/selected list
	if (b2->next)
		Brush_RemoveFromList (b2);
	free(b2);
	Brush_Build(b, true);
}


entityDeclAPI_i* HasModel(brush_s *b)
{
	return 0;
}

void FacingVectors (entity_s *e, vec3_t forward, vec3_t right, vec3_t up)
{
	int			angleVal;
	vec3_c		angles;

	angleVal = e->getKeyFloat("angle");
	if (angleVal == -1)				// up
	{
		angles.set(270, 0, 0);
	}
	else if(angleVal == -2)		// down
	{
		angles.set(90, 0, 0);
	}
	else
	{
		angles.set(0, angleVal, 0);
	}

	angles.makeAngleVectors(forward, right, up);
}

void Brush_DrawFacingAngle (brush_s *b, entity_s *e)
{
	vec3_t	forward, right, up;
	vec3_c	endpoint, tip1, tip2;
	vec3_c	start;
	float	dist;

	start = (e->brushes.onext->getMins() + e->brushes.onext->getMaxs()) * 0.5f;
	dist = (b->getMaxs()[0] - start[0]) * 2.5;

	FacingVectors (e, forward, right, up);
	endpoint.vectorMA (start, dist, forward);

	dist = (b->getMaxs()[0] - start[0]) * 0.5;
	tip1.vectorMA (endpoint, -dist, forward);
	tip1.vectorMA (tip1, -dist, up);
	tip2.vectorMA (tip1, 2*dist, up);

	glColor4f (1, 1, 1, 1);
	glLineWidth (4);
	glBegin (GL_LINES);
	glVertex3fv (start);
	glVertex3fv (endpoint);
	glVertex3fv (endpoint);
	glVertex3fv (tip1);
	glVertex3fv (endpoint);
	glVertex3fv (tip2);
	glEnd ();
	glLineWidth (1);
}

void DrawLight(brush_s *b)
{
	vec3_c vTriColor;
	bool bTriPaint = false;

	vTriColor[0] = vTriColor[2] = 1.0;
	vTriColor[1]  = 1.0;
	bTriPaint = true;
	CString strColor = b->owner->getKeyValue("_color");
	if (strColor.GetLength() > 0)
	{
		float fR, fG, fB;
		int n = sscanf(strColor,"%f %f %f", &fR, &fG, &fB);
		if (n == 3)
		{
			vTriColor[0] = fR;
			vTriColor[1] = fG;
			vTriColor[2] = fB;
		}
	}
	glColor3f(vTriColor[0], vTriColor[1], vTriColor[2]);

	vec3_t vCorners[4];
	float fMid = b->getMins()[2] + (b->getMaxs()[2] - b->getMins()[2]) / 2;

	vCorners[0][0] = b->getMins()[0];
	vCorners[0][1] = b->getMins()[1];
	vCorners[0][2] = fMid;

	vCorners[1][0] = b->getMins()[0];
	vCorners[1][1] = b->getMaxs()[1];
	vCorners[1][2] = fMid;

	vCorners[2][0] = b->getMaxs()[0];
	vCorners[2][1] = b->getMaxs()[1];
	vCorners[2][2] = fMid;

	vCorners[3][0] = b->getMaxs()[0];
	vCorners[3][1] = b->getMins()[1];
	vCorners[3][2] = fMid;

	vec3_c vTop, vBottom;

	vTop[0] = b->getMins()[0] + ((b->getMaxs()[0] - b->getMins()[0]) / 2);
	vTop[1] = b->getMins()[1] + ((b->getMaxs()[1] - b->getMins()[1]) / 2);
	vTop[2] = b->getMaxs()[2];

	vBottom = vTop;
	vBottom[2] = b->getMins()[2];

	vec3_c vSave = vTriColor;
	int i;
	glBegin(GL_TRIANGLE_FAN);
	glVertex3fv(vTop);
	for ( i = 0; i <= 3; i++)
	{
		vTriColor[0] *= 0.95;
		vTriColor[1] *= 0.95;
		vTriColor[2] *= 0.95;
		glColor3f(vTriColor[0], vTriColor[1], vTriColor[2]);
		glVertex3fv(vCorners[i]);
	}
	glVertex3fv(vCorners[0]);
	glEnd();

	vTriColor = vSave;
	vTriColor[0] *= 0.95;
	vTriColor[1] *= 0.95;
	vTriColor[2] *= 0.95;

	glBegin(GL_TRIANGLE_FAN);
	glVertex3fv(vBottom);
	glVertex3fv(vCorners[0]);
	for (i = 3; i >= 0; i--)
	{
		vTriColor[0] *= 0.95;
		vTriColor[1] *= 0.95;
		vTriColor[2] *= 0.95;
		glColor3f(vTriColor[0], vTriColor[1], vTriColor[2]);
		glVertex3fv(vCorners[i]);
	}
	glEnd();

	// check for DOOM lights
	CString str = b->owner->getKeyValue("light_right");
	if (str.GetLength() > 0) {
		vec3_c vRight, vUp, vTarget, vTemp;
		b->owner->getKeyVector("light_right", vRight);
		b->owner->getKeyVector("light_up", vUp);
		b->owner->getKeyVector("light_target", vTarget);

		glColor3f(0, 1, 0);
		glBegin(GL_LINE_LOOP);
		vTemp = vTarget + b->owner->origin;
		vTemp += vRight;
		vTemp += vUp;
		glVertex3fv(b->owner->origin);
		glVertex3fv(vTemp);
		vTemp = vTarget + b->owner->origin;
		vTemp += vUp;
		vTemp -= vRight;
		glVertex3fv(b->owner->origin);
		glVertex3fv(vTemp);
		vTemp = vTarget + b->owner->origin;
		vTemp += vRight;
		vTemp -= vUp;
		glVertex3fv(b->owner->origin);
		glVertex3fv(vTemp);
		vTemp = vTarget + b->owner->origin;
		vTemp -= vUp;
		vTemp -= vRight;
		glVertex3fv(b->owner->origin);
		glVertex3fv(vTemp);
		glEnd();
	}

}

void Brush_Draw( brush_s *b )
{
	face_s			*face;
	int				i, order;
	mtrAPI_i		*prev = 0;
	winding_t *w;

	if (b->hiddenBrush)
	{
		return;
	}

	if (b->patchBrush)
	{
		b->pPatch->drawPatchCam();
		return;
	}
	
	int nDrawMode = g_pParentWnd->GetCamera()->Camera().draw_mode;
	
	if (b->owner->eclass->isFixedSize())
	{
		
		if (!(g_qeglobals.d_savedinfo.exclude & EXCLUDE_ANGLES) && (b->owner->eclass->hasEditorFlagAngle()))
		{
			Brush_DrawFacingAngle(b, b->owner);
		}
		
		if (g_PrefsDlg.m_bNewLightDraw && (b->owner->eclass->hasEditorFlagLight()))
		{
			DrawLight(b);
			return;
		}
		if (nDrawMode == cd_texture)
			glDisable (GL_TEXTURE_2D);
		
		// if we are wireframing models
		//bool bp = (b->bModelFailed) ? false : PaintedModel(b, true);
		
		if (nDrawMode == cd_texture)
			glEnable (GL_TEXTURE_2D);
		//
		//if (bp)
		//	return;
	}
	
	// guarantee the texture will be set first
	prev = NULL;
	for (face = b->brush_faces,order = 0 ; face ; face=face->next, order++)
	{
		w = face->face_winding;
		if (!w)
		{
			continue;		// freed face
		}
		
		if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_CAULK)
		{
			if (strstr(face->texdef.getName(), "caulk"))
			{
				continue;
			}
		}
		
#if 0
		if (b->alphaBrush)
		{
			if (!(face->texdef.flags & SURF_ALPHA))
				continue;
			//--glPushAttrib(GL_ALL_ATTRIB_BITS);
			glDisable(GL_CULL_FACE);
			//--glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			//--glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//--glDisable(GL_DEPTH_TEST);
			//--glBlendFunc (GL_SRC_ALPHA, GL_DST_ALPHA);
			//--glEnable (GL_BLEND);
		}
#endif

		mtrAPI_i *temp = face->d_texture; //g_ms->registerMaterial(face->d_texture->getName());
		
		if ((nDrawMode == cd_texture) && face->d_texture != prev)
		{
			for(u32 i = 0; i < temp->getNumStages(); i++) {
				// set the texture for this face
				prev = face->d_texture;

				const mtrStageAPI_i *s = temp->getStage(i);
				u32 h = s->getTexture(0)->getInternalHandleU32();
				glBindTexture( GL_TEXTURE_2D, h );
			}
		}
		
		if (!b->patchBrush)
		{
			glColor3fv( face->d_color );
		}
		else
		{
			glColor4f ( face->d_color[0], face->d_color[1], face->d_color[2], 0.13 );
		}
		
		// shader drawing stuff
		if (face->d_texture && face->d_texture->hasEditorTransparency())
		{
			// setup shader drawing
			glColor4f ( face->d_color[0], face->d_color[1], face->d_color[2], face->d_texture->getEditorTransparency() );
			
		}
		
		// draw the polygon
		
		glBegin(GL_POLYGON);
		
		for (i=0 ; i<w->numpoints ; i++)
		{
			if (nDrawMode == cd_texture)
				glTexCoord2fv( &w->points[i][3] );
			glVertex3fv(w->points[i]);
		}
		glEnd();
	}
	
#if 0
	if (b->alphaBrush)
	{
		//--glPopAttrib();
		glEnable(GL_CULL_FACE);
		//--glDisable (GL_BLEND);
		//--glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}
#endif
	
	if (b->owner->eclass->isFixedSize() && (nDrawMode == cd_texture))
		glEnable (GL_TEXTURE_2D);
	
	glBindTexture( GL_TEXTURE_2D, 0 );
}



void Face_Draw( face_s *f )
{
	int i;

	if ( f->face_winding == 0 )
		return;
	glBegin( GL_POLYGON );
	for ( i = 0 ; i < f->face_winding->numpoints; i++)
		glVertex3fv( f->face_winding->points[i] );
	glEnd();
}

void Brush_DrawXY(brush_s *b, int nViewType)
{
	face_s *face;
	int     order;
	winding_t *w;
	int        i;

	if (b->hiddenBrush)
	{
		return;
	}

	if (b->patchBrush)
	{
		b->pPatch->drawPatchXY();
		if (!g_bPatchShowBounds)
			return;
	}
        

	if (b->owner->eclass->isFixedSize())
	{
		if (g_PrefsDlg.m_bNewLightDraw && (b->owner->eclass->hasEditorFlagLight()))
		{
			vec3_t vCorners[4];
			float fMid = b->getMins()[2] + (b->getMaxs()[2] - b->getMins()[2]) / 2;

			vCorners[0][0] = b->getMins()[0];
			vCorners[0][1] = b->getMins()[1];
			vCorners[0][2] = fMid;

			vCorners[1][0] = b->getMins()[0];
			vCorners[1][1] = b->getMaxs()[1];
			vCorners[1][2] = fMid;

			vCorners[2][0] = b->getMaxs()[0];
			vCorners[2][1] = b->getMaxs()[1];
			vCorners[2][2] = fMid;

			vCorners[3][0] = b->getMaxs()[0];
			vCorners[3][1] = b->getMins()[1];
			vCorners[3][2] = fMid;

			vec3_c vTop, vBottom;

			vTop[0] = b->getMins()[0] + ((b->getMaxs()[0] - b->getMins()[0]) / 2);
			vTop[1] = b->getMins()[1] + ((b->getMaxs()[1] - b->getMins()[1]) / 2);
			vTop[2] = b->getMaxs()[2];

			vBottom = vTop;
			vBottom[2] = b->getMins()[2];

			glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
			glBegin(GL_TRIANGLE_FAN);
			glVertex3fv(vTop);
			glVertex3fv(vCorners[0]);
			glVertex3fv(vCorners[1]);
			glVertex3fv(vCorners[2]);
			glVertex3fv(vCorners[3]);
			glVertex3fv(vCorners[0]);
			glEnd();
			glBegin(GL_TRIANGLE_FAN);
			glVertex3fv(vBottom);
			glVertex3fv(vCorners[0]);
			glVertex3fv(vCorners[3]);
			glVertex3fv(vCorners[2]);
			glVertex3fv(vCorners[1]);
			glVertex3fv(vCorners[0]);
			glEnd();
			DrawBrushEntityName (b);
			return;
		}
		else if (b->owner->eclass->hasEditorFlagMiscModel())
		{
			//if (PaintedModel(b, false))
			//return;
		}
	}

	for (face = b->brush_faces,order = 0 ; face ; face=face->next, order++)
	{
		// only draw polygons facing in a direction we care about
		if(1)
		{
			if (nViewType == XY)
			{
				  if (face->plane.normal[2] <= 0)
					  continue;
			}
			else
			{
				if (nViewType == XZ)
				{
					if (face->plane.normal[1] <= 0)
						continue;
				}
				else 
				{
					if (face->plane.normal[0] <= 0)
						continue;
				}
			}
		}

		w = face->face_winding;
		if (!w)
			continue;

		//if (b->alphaBrush && !(face->texdef.flags & SURF_ALPHA))
		//  continue;

		// draw the polygon
		glBegin(GL_LINE_LOOP);
		for (i=0 ; i<w->numpoints ; i++)
			glVertex3fv(w->points[i]);
		glEnd();
	}

	DrawBrushEntityName (b);

}

/*
============
Brush_Move
============
*/
void Brush_Move (brush_s *b, const vec3_t move, bool bSnap)
{
	int i;
	face_s *f;
	if(b == 0)
		return;

	for (f=b->brush_faces ; f ; f=f->next)
	{
		vec3_c vTemp = move;

		if (g_PrefsDlg.m_bTextureLock)
			Face_MoveTexture(f, vTemp);

		for (i=0 ; i<3 ; i++)
			f->planepts[i] += move;
	}
	Brush_Build( b, bSnap );


	if (b->patchBrush)
	{
		b->pPatch->movePatch(move);
	}

	// PGM - keep the origin vector up to date on fixed size entities.
	if(b->owner->eclass->isFixedSize())
	{
		b->owner->origin += move;
		//VectorAdd(b->maxs, b->mins, b->owner->origin);
		//VectorScale(b->owner->origin, 0.5, b->owner->origin);
	}
}


/*
=============
Brush_MakeSided

Makes the current brushhave the given number of 2d sides and turns it into a cone
=============
*/
void Brush_MakeSidedCone(int sides)
{
	int		i;
	vec3_c	mins, maxs;
	brush_s	*b;
	texdef_t	*texdef;
	face_s	*f;
	vec3_t	mid;
	float	width;
	float	sv, cv;

	if (sides < 3)
	{
		Sys_Status ("Bad sides number", 0);
		return;
	}

	if (!QE_SingleBrush ())
	{
		Sys_Status ("Must have a single brush selected", 0 );
		return;
	}

	b = selected_brushes.next;
	mins = b->getMins();
	maxs = b->getMaxs();
	texdef = &g_qeglobals.d_texturewin.texdef;

	Brush_Free (b);

	// find center of brush
	width = 8;
	for (i=0 ; i<2 ; i++)
	{
		mid[i] = (maxs[i] + mins[i])*0.5;
		if (maxs[i] - mins[i] > width)
			width = maxs[i] - mins[i];
	}
	width /= 2;

	b = new brush_s(false);;

	// create bottom face
	f = new face_s();
	f->texdef = *texdef;
	f->next = b->brush_faces;
	b->brush_faces = f;

	f->planepts[0][0] = mins[0];f->planepts[0][1] = mins[1];f->planepts[0][2] = mins[2];
	f->planepts[1][0] = maxs[0];f->planepts[1][1] = mins[1];f->planepts[1][2] = mins[2];
	f->planepts[2][0] = maxs[0];f->planepts[2][1] = maxs[1];f->planepts[2][2] = mins[2];

	for (i=0 ; i<sides ; i++)
	{
		f = new face_s();
		f->texdef = *texdef;
		f->next = b->brush_faces;
		b->brush_faces = f;

		sv = sin (i*3.14159265*2/sides);
		cv = cos (i*3.14159265*2/sides);


		f->planepts[0][0] = floor(mid[0]+width*cv+0.5);
		f->planepts[0][1] = floor(mid[1]+width*sv+0.5);
		f->planepts[0][2] = mins[2];

		f->planepts[1][0] = mid[0];
		f->planepts[1][1] = mid[1];
		f->planepts[1][2] = maxs[2];

		f->planepts[2][0] = floor(f->planepts[0][0] - width * sv + 0.5);
		f->planepts[2][1] = floor(f->planepts[0][1] + width * cv + 0.5);
		f->planepts[2][2] = maxs[2];

	}

	Brush_AddToList (b, &selected_brushes);

	Entity_LinkBrush (world_entity, b);

	Brush_Build( b );

	Sys_UpdateWindows (W_ALL);
}

/*
=============
Brush_MakeSided

Makes the current brushhave the given number of 2d sides and turns it into a sphere
=============

*/
void Brush_MakeSidedSphere(int sides)
{
	int		i,j;
	vec3_c	mins, maxs;
	brush_s	*b;
	texdef_t	*texdef;
	face_s	*f;
	vec3_t	mid;

	if (sides < 4)
	{
		Sys_Status ("Bad sides number", 0);
		return;
	}

	if (!QE_SingleBrush ())
	{
		Sys_Status ("Must have a single brush selected", 0 );
		return;
	}

	b = selected_brushes.next;
	mins = b->getMins();
	maxs = b->getMaxs();
	texdef = &g_qeglobals.d_texturewin.texdef;

	Brush_Free (b);

	// find center of brush
	float radius = 8;
	for (i=0 ; i<2 ; i++)
	{
		mid[i] = (maxs[i] + mins[i])*0.5;
		if (maxs[i] - mins[i] > radius)
			radius = maxs[i] - mins[i];
	}
	radius /= 2;

	b = new brush_s(false);;

	float dt = float(2 * M_PI / sides);
	float dp = float(M_PI / sides);
	float t,p;
	for(i=0; i <= sides-1; i++)
	{
		for(j=0;j <= sides-2; j++)
		{
			t = i * dt;
			p = float(j * dp - M_PI / 2);

			f = new face_s();
			f->texdef = *texdef;
			f->next = b->brush_faces;
			b->brush_faces = f;

			f->planepts[0].setupPolar(radius, t, p);
			f->planepts[1].setupPolar(radius, t, p + dp);
			f->planepts[2].setupPolar(radius, t + dt, p + dp);

			for (int k = 0; k < 3; k++)
				f->planepts[k] += mid;
		}
	}

	p = float((sides - 1) * dp - M_PI / 2);
	for(i = 0; i <= sides-1; i++)
	{
		t = i * dt;

		f = new face_s();
		f->texdef = *texdef;
		f->next = b->brush_faces;
		b->brush_faces = f;

		f->planepts[0].setupPolar(radius, t, p);
		f->planepts[1].setupPolar(radius, t + dt, p + dp);
		f->planepts[2].setupPolar(radius, t + dt, p);

		for (int k = 0; k < 3; k++)
			f->planepts[k] += mid;
	}

	Brush_AddToList (b, &selected_brushes);

	Entity_LinkBrush (world_entity, b);

	Brush_Build( b );

	Sys_UpdateWindows (W_ALL);
}

void Face_FitTexture( face_s * face, int nHeight, int nWidth )
{
	winding_t *w;
	aabb bounds;
	int i;
	float width, height, temp;
	float rot_width, rot_height;
	float cosv,sinv,ang;
	float min_t, min_s, max_t, max_s;
	float s,t;
	vec3_c	vecs[2];
	vec3_t   coords[4];
	texdef_t	*td;

	if (nHeight < 1)
	{
		nHeight = 1;
	}
	if (nWidth < 1)
	{
		nWidth = 1;
	}

	bounds.clear();

	td = &face->texdef;
	w = face->face_winding;
	if (!w)
	{
		return;
	}
	for (i=0 ; i<w->numpoints ; i++)
	{
		bounds.addPoint(w->points[i]);
	}
	// 
	// get the current angle
	//
	ang = DEG2RAD(td->rotate);
	sinv = sin(ang);
	cosv = cos(ang);

	// get natural texture axis
	TextureAxisFromPlane(face->plane, vecs[0], vecs[1]);

	min_s = bounds.getMins().dotProduct(vecs[0]);
	min_t = bounds.getMins().dotProduct(vecs[1]);
	max_s = bounds.getMaxs().dotProduct(vecs[0]);
	max_t = bounds.getMaxs().dotProduct(vecs[1]);
	width = max_s - min_s;
	height = max_t - min_t;
	coords[0][0] = min_s;
	coords[0][1] = min_t;
	coords[1][0] = max_s;
	coords[1][1] = min_t;
	coords[2][0] = min_s;
	coords[2][1] = max_t;
	coords[3][0] = max_s;
	coords[3][1] = max_t;
	min_s = min_t = 99999;
	max_s = max_t = -99999;
	for (i=0; i<4; i++)
	{
		s = cosv * coords[i][0] - sinv * coords[i][1];
		t = sinv * coords[i][0] + cosv * coords[i][1];
		if (i&1)
		{
			if (s > max_s) 
			{
				max_s = s;
			}
		}
		else
		{
			if (s < min_s) 
			{
				min_s = s;
			}
			if (i<2)
			{
				if (t < min_t) 
				{
					min_t = t;
				}
			}
			else
			{
				if (t > max_t) 
				{
					max_t = t;
				}
			}
		}
	}
	rot_width =  (max_s - min_s);
	rot_height = (max_t - min_t);
	td->scale[0] = -(rot_width/((float)(face->d_texture->getImageWidth()*nWidth)));
	td->scale[1] = -(rot_height/((float)(face->d_texture->getImageHeight()*nHeight)));

	td->shift[0] = min_s/td->scale[0];
	temp = (int)(td->shift[0] / (face->d_texture->getImageWidth()*nWidth));
	temp = (temp+1)*face->d_texture->getImageWidth()*nWidth;
	td->shift[0] = (int)(temp - td->shift[0])%(face->d_texture->getImageWidth()*nWidth);

	td->shift[1] = min_t/td->scale[1];
	temp = (int)(td->shift[1] / (face->d_texture->getImageHeight()*nHeight));
	temp = (temp+1)*(face->d_texture->getImageHeight()*nHeight);
	td->shift[1] = (int)(temp - td->shift[1])%(face->d_texture->getImageHeight()*nHeight);
}

void Brush_FitTexture( brush_s *b, int nHeight, int nWidth )
{
	face_s *face;

	for (face = b->brush_faces ; face ; face=face->next)
	{
		Face_FitTexture( face, nHeight, nWidth );
	}
}


