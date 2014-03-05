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


#include "stdafx.h"
#include <assert.h>
#include "qe3.h"
#include "winding.h"

#define	BOGUS_RANGE	18000

/*
=================
Winding_BaseForPlane
=================
*/
winding_t *Winding_BaseForPlane (const class edPlane_c &p)
{
	int		i, x;
	vec_t	max, v;
	edVec3_c	org, vright, vup;
	winding_t	*w;
	
	// find the major axis

	max = -BOGUS_RANGE;
	x = -1;
	for (i=0 ; i<3; i++)
	{
		v = fabs(p.normal[i]);
		if (v > max)
		{
			x = i;
			max = v;
		}
	}
	if (x==-1)
		Error ("Winding_BaseForPlane: no axis found");
		
	vup.clear();	
	switch (x)
	{
	case 0:
	case 1:
		vup[2] = 1;
		break;		
	case 2:
		vup[0] = 1;
		break;		
	}


	v = vup.dotProduct(p.normal);
	vup.vectorMA (vup, -v, p.normal);
	vup.normalize();
		
	org = p.normal * p.dist;

	vright.crossProduct (vup, p.normal);
	
	vup *= BOGUS_RANGE;
	vright *= BOGUS_RANGE;

	// project a really big	axis aligned box onto the plane
	w = Winding_Alloc (4);
	
	w->points[0].setXYZ(org - vright + vup);
	w->points[1].setXYZ(org + vright + vup);
	w->points[2].setXYZ(org + vright - vup);
	w->points[3].setXYZ(org - vright - vup);
	
	w->numpoints = 4;
	
	return w;	
}

/*
==================
Winding_Alloc
==================
*/
winding_t *Winding_Alloc (int points)
{
	winding_t	*w;
	int			size;
	
	if (points > MAX_POINTS_ON_WINDING)
		Error ("Winding_Alloc: %i points", points);
	
	size = (int)&((winding_t *)0)->points[points];
	w = (winding_t*) malloc (size);
	memset (w, 0, size);
	w->maxpoints = points;
	
	return w;
}


void Winding_Free (winding_t *w)
{
	free(w);
}


/*
==================
Winding_Clone
==================
*/
winding_t *Winding_Clone(winding_t *w)
{
	int			size;
	winding_t	*c;
	
	size = (int)&((winding_t *)0)->points[w->numpoints];
	c = (winding_t*)qmalloc (size);
	memcpy (c, w, size);
	return c;
}

/*
==================
ReverseWinding
==================
*/
winding_t *Winding_Reverse(winding_t *w)
{
	int			i;
	winding_t	*c;

	c = Winding_Alloc(w->numpoints);
	for (i = 0; i < w->numpoints; i++)
	{
		c->points[i].setXYZ(w->points[w->numpoints-1-i].getXYZ());
	}
	c->numpoints = w->numpoints;
	return c;
}


/*
==============
Winding_RemovePoint
==============
*/
void Winding_RemovePoint(winding_t *w, int point)
{
	if (point < 0 || point >= w->numpoints)
		Error("Winding_RemovePoint: point out of range");

	if (point < w->numpoints-1)
	{
		memmove(&w->points[point], &w->points[point+1], (int)&((winding_t *)0)->points[w->numpoints - point - 1]);
	}
	w->numpoints--;
}

/*
=============
Winding_InsertPoint
=============
*/
winding_t *Winding_InsertPoint(winding_t *w, vec3_t point, int spot)
{
	int i, j;
	winding_t *neww;

	if (spot > w->numpoints)
	{
		Error("Winding_InsertPoint: spot > w->numpoints");
	} //end if
	if (spot < 0)
	{
		Error("Winding_InsertPoint: spot < 0");
	} //end if
	neww = Winding_Alloc(w->numpoints + 1);
	neww->numpoints = w->numpoints + 1;
	for (i = 0, j = 0; i < neww->numpoints; i++)
	{
		if (i == spot)
		{
			neww->points[i].setXYZ(point);
		}
		else
		{
			neww->points[i] = w->points[j];
			j++;
		}
	}
	return neww;
}

/*
==============
Winding_IsTiny
==============
*/
#define	EDGE_LENGTH	0.2

int Winding_IsTiny (winding_t *w)
{
	int		i, j;
	vec_t	len;
	int		edges;

	edges = 0;
	for (i=0 ; i<w->numpoints ; i++)
	{
		j = i == w->numpoints - 1 ? 0 : i+1;
		edVec3_c delta = w->points[j].getXYZ() - w->points[i].getXYZ();
		len = delta.vectorLength();
		if (len > EDGE_LENGTH)
		{
			if (++edges == 3)
				return false;
		}
	}
	return true;
}

/*
==============
Winding_IsHuge
==============
*/
int Winding_IsHuge(winding_t *w)
{
	int		i, j;

	for (i=0 ; i<w->numpoints ; i++)
	{
		for (j=0 ; j<3 ; j++)
			if (w->points[i][j] < -BOGUS_RANGE+1 || w->points[i][j] > BOGUS_RANGE-1)
				return true;
	}
	return false;
}

/*
=============
Winding_PlanesConcave
=============
*/
#define WCONVEX_EPSILON		0.2

int Winding_PlanesConcave(winding_t *w1, winding_t *w2,
							 const edVec3_c &normal1, const edVec3_c &normal2,
							 float dist1, float dist2)
{
	int i;

	if (!w1 || !w2) return false;

	// check if one of the points of winding 1 is at the back of the plane of winding 2
	for (i = 0; i < w1->numpoints; i++)
	{
		if (normal2.dotProduct(w1->points[i]) - dist2 > WCONVEX_EPSILON)
			return true;
	}
	// check if one of the points of winding 2 is at the back of the plane of winding 1
	for (i = 0; i < w2->numpoints; i++)
	{
		if (normal1.dotProduct(w2->points[i]) - dist1 > WCONVEX_EPSILON)
			return true;
	}

	return false;
}

/*
==================
Winding_Clip

Clips the winding to the plane, returning the new winding on the positive side
Frees the input winding.
If keepon is true, an exactly on-plane winding will be saved, otherwise
it will be clipped away.
==================
*/
winding_t *Winding_Clip (winding_t *in, const edPlane_c &split, bool keepon)
{
	vec_t	dists[MAX_POINTS_ON_WINDING];
	int		sides[MAX_POINTS_ON_WINDING];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t	*p1, *p2;
	vec3_t	mid;
	winding_t	*neww;
	int		maxpts;
	
	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for (i=0 ; i<in->numpoints ; i++)
	{
		dot = in->points[i].dotProduct(split.normal);
		dot -= split.dist;
		dists[i] = dot;
		if (dot > ON_EPSILON)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];
	
	if (keepon && !counts[0] && !counts[1])
		return in;
		
	if (!counts[0])
	{
		Winding_Free (in);
		return NULL;
	}
	if (!counts[1])
		return in;
	
	maxpts = in->numpoints+4;	// can't use counts[0]+2 because
								// of fp grouping errors
	neww = Winding_Alloc (maxpts);
		
	for (i=0 ; i<in->numpoints ; i++)
	{
		p1 = in->points[i];
		
		if (sides[i] == SIDE_ON)
		{
			neww->points[neww->numpoints].setXYZ(p1);
			neww->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			neww->points[neww->numpoints].setXYZ(p1);
			neww->numpoints++;
		}
		
		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
		// generate a split point
		p2 = in->points[(i+1)%in->numpoints];
		
		dot = dists[i] / (dists[i]-dists[i+1]);
		for (j=0 ; j<3 ; j++)
		{	// avoid round off error when possible
			if (split.normal[j] == 1)
				mid[j] = split.dist;
			else if (split.normal[j] == -1)
				mid[j] = -split.dist;
			else
				mid[j] = p1[j] + dot*(p2[j]-p1[j]);
		}
			
		neww->points[neww->numpoints].setXYZ(mid);
		neww->numpoints++;
	}
	
	if (neww->numpoints > maxpts)
		Error ("Winding_Clip: points exceeded estimate");
		
	// free the original winding
	Winding_Free (in);
	
	return neww;
}

/*
=============
Winding_SplitEpsilon

  split the input winding with the plane
  the input winding stays untouched
=============
*/
void Winding_SplitEpsilon (winding_t *in, vec3_t normal, double dist, 
				vec_t epsilon, winding_t **front, winding_t **back)
{
	vec_t	dists[MAX_POINTS_ON_WINDING+4];
	int		sides[MAX_POINTS_ON_WINDING+4];
	int		counts[3];
	vec_t	dot;
	int		i, j;
	vec_t	*p1, *p2;
	vec3_t	mid;
	winding_t	*f, *b;
	int		maxpts;
	
	counts[0] = counts[1] = counts[2] = 0;

	// determine sides for each point
	for (i = 0; i < in->numpoints; i++)
	{
		dot = in->points[i].dotProduct(normal);
		dot -= dist;
		dists[i] = dot;
		if (dot > epsilon)
			sides[i] = SIDE_FRONT;
		else if (dot < -epsilon)
			sides[i] = SIDE_BACK;
		else
		{
			sides[i] = SIDE_ON;
		}
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];
	
	*front = *back = NULL;

	if (!counts[0])
	{
		*back = Winding_Clone(in);
		return;
	}
	if (!counts[1])
	{
		*front = Winding_Clone(in);
		return;
	}

	maxpts = in->numpoints+4;	// cant use counts[0]+2 because
								// of fp grouping errors

	*front = f = Winding_Alloc (maxpts);
	*back = b = Winding_Alloc (maxpts);
		
	for (i = 0; i < in->numpoints; i++)
	{
		p1 = in->points[i];
		
		if (sides[i] == SIDE_ON)
		{
			f->points[f->numpoints].setXYZ(p1);
			f->numpoints++;
			b->points[b->numpoints].setXYZ(p1);
			b->numpoints++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			f->points[f->numpoints].setXYZ(p1);
			f->numpoints++;
		}
		if (sides[i] == SIDE_BACK)
		{
			b->points[b->numpoints].setXYZ(p1);
			b->numpoints++;
		}

		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;
			
		// generate a split point
		p2 = in->points[(i+1)%in->numpoints];
		
		dot = dists[i] / (dists[i]-dists[i+1]);
		for (j = 0; j < 3; j++)
		{
			// avoid round off error when possible
			if (normal[j] == 1)
				mid[j] = dist;
			else if (normal[j] == -1)
				mid[j] = -dist;
			else
				mid[j] = p1[j] + dot*(p2[j]-p1[j]);
		}
			
		f->points[f->numpoints].setXYZ(mid);
		f->numpoints++;
		b->points[b->numpoints].setXYZ(mid);
		b->numpoints++;
	}
	
	if (f->numpoints > maxpts || b->numpoints > maxpts)
		Error ("Winding_Clip: points exceeded estimate");
	if (f->numpoints > MAX_POINTS_ON_WINDING || b->numpoints > MAX_POINTS_ON_WINDING)
		Error ("Winding_Clip: MAX_POINTS_ON_WINDING");
}

/*
=============
Winding_TryMerge

If two windings share a common edge and the edges that meet at the
common points are both inside the other polygons, merge them

Returns NULL if the windings couldn't be merged, or the new winding.
The originals will NOT be freed.

if keep is true no points are ever removed
=============
*/
#define	CONTINUOUS_EPSILON	0.005

winding_t *Winding_TryMerge(const winding_t *f1, const winding_t *f2, vec3_t planenormal, int keep)
{
	edVec3_c p1, p2, p3, p4, back;
	winding_t	*newf;
	int			i, j, k, l;
	edVec3_c		normal, delta;
	vec_t		dot;
	bool	keep1, keep2;
	

	//
	// find a common edge
	//	
	p1 = p2 = NULL;	// stop compiler warning
	j = 0;			// 
	
	for (i = 0; i < f1->numpoints; i++)
	{
		p1 = f1->points[i].getXYZ();
		p2 = f1->points[(i+1) % f1->numpoints].getXYZ();
		for (j = 0; j < f2->numpoints; j++)
		{
			p3 = f2->points[j].getXYZ();
			p4 = f2->points[(j+1) % f2->numpoints].getXYZ();
			for (k = 0; k < 3; k++)
			{
				if (fabs(p1[k] - p4[k]) > 0.1)//EQUAL_EPSILON) //ME
					break;
				if (fabs(p2[k] - p3[k]) > 0.1)//EQUAL_EPSILON) //ME
					break;
			} //end for
			if (k==3)
				break;
		} //end for
		if (j < f2->numpoints)
			break;
	} //end for
	
	if (i == f1->numpoints)
		return NULL;			// no matching edges

	//
	// check slope of connected lines
	// if the slopes are colinear, the point can be removed
	//
	back = f1->points[(i+f1->numpoints-1)%f1->numpoints].getXYZ();
	delta = p1 - back;
	normal.crossProduct (planenormal, delta);
	normal.normalize();

	back = f2->points[(j+2)%f2->numpoints].getXYZ();
	delta = back - p1;
	dot = delta.dotProduct(normal);
	if (dot > CONTINUOUS_EPSILON)
		return NULL;			// not a convex polygon
	keep1 = (bool)(dot < -CONTINUOUS_EPSILON);
	
	back = f1->points[(i+2)%f1->numpoints].getXYZ();
	delta = back - p2;
	normal.crossProduct (planenormal, delta);
	normal.normalize();

	back = f2->points[(j+f2->numpoints-1)%f2->numpoints].getXYZ();
	delta = back - p2;
	dot = delta.dotProduct(normal);
	if (dot > CONTINUOUS_EPSILON)
		return NULL;			// not a convex polygon
	keep2 = (bool)(dot < -CONTINUOUS_EPSILON);

	//
	// build the new polygon
	//
	newf = Winding_Alloc (f1->numpoints + f2->numpoints);
	
	// copy first polygon
	for (k=(i+1)%f1->numpoints ; k != i ; k=(k+1)%f1->numpoints)
	{
		if (!keep && k==(i+1)%f1->numpoints && !keep2)
			continue;
		
		newf->points[newf->numpoints] = f1->points[k];
		newf->numpoints++;
	}
	
	// copy second polygon
	for (l= (j+1)%f2->numpoints ; l != j ; l=(l+1)%f2->numpoints)
	{
		if (!keep && l==(j+1)%f2->numpoints && !keep1)
			continue;
		newf->points[newf->numpoints] = f2->points[l];
		newf->numpoints++;
	}

	return newf;
}

/*
============
Winding_Plane
============
*/
void Winding_Plane (winding_t *w, class edVec3_c &normal, double *dist)
{
	edVec3_c v1, v2;
	int i;

	//find two vectors each longer than 0.5 units
	for (i = 0; i < w->numpoints; i++)
	{
		v1 = w->points[(i+1) % w->numpoints].xyz - w->points[i].xyz;
		v2 = w->points[(i+2) % w->numpoints].xyz - w->points[i].xyz;
		if (v1.vectorLength() > 0.5 && v2.vectorLength() > 0.5)
			break;
	}
	normal.crossProduct(v2, v1);
	normal.normalize();
	*dist = normal.dotProduct(w->points[0]);
}

/*
=============
Winding_Area
=============
*/
float Winding_Area (winding_t *w)
{
	int		i;
	edVec3_c	d1, d2, cross;
	float	total;

	total = 0;
	for (i=2 ; i<w->numpoints ; i++)
	{
		d1 = w->points[i-1].xyz -w->points[0].xyz;
		d2 = w->points[i].xyz - w->points[0].xyz;
		cross.crossProduct (d1, d2);
		total += 0.5 * cross.vectorLength();
	}
	return total;
}

/*
=============
Winding_Bounds
=============
*/
void Winding_Bounds (winding_t *w, vec3_t mins, vec3_t maxs)
{
	vec_t	v;
	int		i,j;

	mins[0] = mins[1] = mins[2] = 99999;
	maxs[0] = maxs[1] = maxs[2] = -99999;

	for (i=0 ; i<w->numpoints ; i++)
	{
		for (j=0 ; j<3 ; j++)
		{
			v = w->points[i][j];
			if (v < mins[j])
				mins[j] = v;
			if (v > maxs[j])
				maxs[j] = v;
		}
	}
}


/*
=================
Winding_PointInside
=================
*/
int Winding_PointInside(winding_t *w, const class edPlane_c &plane, const edVec3_c &point, float epsilon)
{
	int i;
	edVec3_c dir, normal, pointvec;

	for (i = 0; i < w->numpoints; i++)
	{
		dir = w->points[(i+1) % w->numpoints].getXYZ() - w->points[i].getXYZ();
		pointvec = point - w->points[i].getXYZ();
		//
		normal.crossProduct(dir, plane.normal);
		//
		if (pointvec.dotProduct(normal) < -epsilon)
			return false;
	}
	return true;
}

/*
=================
Winding_VectorIntersect
=================
*/
int Winding_VectorIntersect(winding_t *w, const class edPlane_c &plane, const edVec3_c &p1, const edVec3_c &p2, float epsilon)
{
	float front, back, frac;
	edVec3_c mid;

	front = p1.dotProduct(plane.normal) - plane.dist;
	back = p2.dotProduct(plane.normal) - plane.dist;
	//if both points at the same side of the plane
	if (front < -epsilon && back < -epsilon) return false;
	if (front > epsilon && back > epsilon) return false;
	//get point of intersection with winding plane
	if (fabs(front-back) < 0.001)
	{
		mid = p2;
	}
	else
	{
		frac = front/(front-back);
		mid[0] = p1[0] + (p2[0] - p1[0]) * frac;
		mid[1] = p1[1] + (p2[1] - p1[1]) * frac;
		mid[2] = p1[2] + (p2[2] - p1[2]) * frac;
	}
	return Winding_PointInside(w, plane, mid, epsilon);
}

