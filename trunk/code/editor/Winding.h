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



//returns true if the planes are equal
//int			Plane_Equal(plane_t *a, plane_t *b, int flip);
////returns false if the points are colinear
//int			Plane_FromPoints(vec3_t p1, vec3_t p2, vec3_t p3, plane_t *plane);
////returns true if the points are equal
int			Point_Equal(vec3_t p1, vec3_t p2, float epsilon);


////create a base winding for the plane
//edWinding_t*	Winding_BaseForPlane (const class edPlane_c &p);
////make a winding clone
//edWinding_t*	Winding_Clone(edWinding_t *w );
////creates the reversed winding
//edWinding_t*	Winding_Reverse(edWinding_t *w);
////remove a point from the winding
//void		Winding_RemovePoint(edWinding_t *w, int point);
////inserts a point to a winding, creating a new winding
//edWinding_t*	Winding_InsertPoint(edWinding_t *w, vec3_t point, int spot);
////returns true if the planes are concave
//int			Winding_PlanesConcave(edWinding_t *w1, edWinding_t *w2,
//									 const vec3_c &normal1, const vec3_c &normal2,
//									 float dist1, float dist2);
////returns true if the winding is tiny
//int			Winding_IsTiny(edWinding_t *w);
////returns true if the winding is huge
//int			Winding_IsHuge(edWinding_t *w);
////clip the winding with the plane
//edWinding_t*	Winding_Clip(edWinding_t *in, const class edPlane_c &split, bool keepon);
////split the winding with the plane
//void		Winding_SplitEpsilon(edWinding_t *in, vec3_t normal, double dist, 
//								vec_t epsilon, edWinding_t **front, edWinding_t **back);
////try to merge the windings, returns the new merged winding or NULL
//edWinding_t *Winding_TryMerge(const edWinding_t *f1, const edWinding_t *f2, vec3_t planenormal, int keep);
////create a plane for the winding
//void		Winding_Plane(edWinding_t *w, class vec3_c &normal, double *dist);
////returns the winding area
//float		Winding_Area(edWinding_t *w);
////returns the bounds of the winding
//void		Winding_Bounds(edWinding_t *w, vec3_t mins, vec3_t maxs);
////returns true if the point is inside the winding
//int			Winding_PointInside(edWinding_t *w, const class edPlane_c &plane, const vec3_c &point, float epsilon);
////returns true if the vector intersects with the winding
//int			Winding_VectorIntersect(edWinding_t *w, const class edPlane_c &plane, const vec3_c &p1, const vec3_c &p2, float epsilon);
