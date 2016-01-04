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
#include "qe3.h"
#include <api/entityDeclAPI.h>



void CSG_MakeHollow ()
{
	edBrush_c		*b, *front, *back, *next;
	face_s		*f;
	vec3_c		move;
	int			i;

	for (b = selected_brushes.next ; b != &selected_brushes ; b=next)
	{
		next = b->next;

    if (b->owner->getEntityClass()->isFixedSize() || b->patchBrush || b->terrainBrush || b->hiddenBrush)
		  continue;

		for (f = b->getFirstFace() ; f ; f=f->next)
		{
			face_s *split = f->cloneFace();
			move = f->plane.norm * g_qeglobals.d_gridsize;
			for (i=0 ; i<3 ; i++) {
				split->planepts[i] -= move;
			}

			b->splitBrushByFace(split, &front, &back);
			if (back)
				Brush_Free (back);
			if (front)
				Brush_AddToList (front, &selected_brushes);
			delete split;
		}
		Brush_Free (b);
	}
	Sys_UpdateWindows (W_ALL);
}

/*
 Returns a new brush that is created by merging brush1 and brush2.
 May return NULL if brush1 and brush2 do not create a convex brush when merged.
 The input brushes brush1 and brush2 stay intact.

 if onlyshape is true then the merge is allowed based on the shape only
 otherwise the texture/shader references of faces in the same plane have to
 be the same as well.
*/
edBrush_c *edBrush_c::tryMergeWith(edBrush_c *brush2, int onlyshape)
{
	int i, shared;
	edBrush_c *newbrush;
	face_s *face1, *face2, *newface, *f;

	// check for bounding box overlapp
	for (i = 0; i < 3; i++)
	{
		if (this->getMins()[i] > brush2->getMaxs()[i] + ON_EPSILON
				|| this->getMaxs()[i] < brush2->getMins()[i] - ON_EPSILON)
		{
			// never merge if the brushes overlap
			return NULL;
		}
	}
	//
	shared = 0;
	// check if the new brush would be convex... flipped planes make a brush non-convex
	for (face1 = this->getFirstFace(); face1; face1 = face1->next)
	{
		// don't check the faces of brush 1 and 2 touching each other
		for (face2 = brush2->getFirstFace(); face2; face2 = face2->next)
		{
			if (face1->plane.isPlaneEqual(face2->plane, true))
			{
				shared++;
				// there may only be ONE shared side
				if (shared > 1)
					return NULL;
				break;
			}
		}
		// if this face plane is shared
		if (face2) continue;
		//
		for (face2 = brush2->getFirstFace(); face2; face2 = face2->next)
		{
			// don't check the faces of brush 1 and 2 touching each other
			for (f = this->getFirstFace(); f; f = f->next)
			{
				if (face2->plane.isPlaneEqual(f->plane, true)) break;
			}
			if (f)
				continue;
			//
			if (face1->plane.isPlaneEqual(face2->plane, false))
			{
				//if the texture/shader references should be the same but are not
				if (!onlyshape && _stricmp(face1->getMatName(), face2->getMatName()) != 0) return NULL;
				continue;
			}
			//
			if (texturedWinding_c::planesConcave(face1->face_winding, face2->face_winding,
									face1->plane.norm, face2->plane.norm,
									face1->plane.dist, face2->plane.dist))
			{
				return NULL;
			} //end if
		} //end for
	} //end for
	//
	newbrush = new edBrush_c(false);
	//
	for (face1 = this->getFirstFace(); face1; face1 = face1->next)
	{
		// don't add the faces of brush 1 and 2 touching each other
		for (face2 = brush2->getFirstFace(); face2; face2 = face2->next)
		{
			if (face1->plane.isPlaneEqual(face2->plane, true))
				break;
		}
		if (face2)
			continue;
		// don't add faces with the same plane twice
		for (f = newbrush->getFirstFace(); f; f = f->next)
		{
			if (face1->plane.isPlaneEqual(f->plane, false))
				break;
			if (face1->plane.isPlaneEqual(f->plane, true))
				break;
		}
		if (f)
			continue;
		//
		newface = new face_s();
		newface->texdef = face1->texdef;
		newface->planepts[0] = face1->planepts[0];
		newface->planepts[1] = face1->planepts[1];
		newface->planepts[2] = face1->planepts[2];
		newface->plane = face1->plane;
		newface->next = newbrush->getFirstFace();
		newbrush->brush_faces = newface;
	}
	//
	for (face2 = brush2->getFirstFace(); face2; face2 = face2->next)
	{
		// don't add the faces of brush 1 and 2 touching each other
		for (face1 = this->getFirstFace(); face1; face1 = face1->next)
		{
			if (face2->plane.isPlaneEqual(face1->plane, true))
				break;
		}
		if (face1)
			continue;
		// don't add faces with the same plane twice
		for (f = newbrush->getFirstFace(); f; f = f->next)
		{
			if (face2->plane.isPlaneEqual(f->plane, false))
				break;
			if (face2->plane.isPlaneEqual(f->plane, true))
				break;
		}
		if (f)
			continue;
		//
		newface = new face_s();
		newface->texdef = face2->texdef;
		newface->planepts[0] = face2->planepts[0];
		newface->planepts[1] = face2->planepts[1];
		newface->planepts[2] = face2->planepts[2];
		newface->plane = face2->plane;
		newface->next = newbrush->brush_faces;
		newbrush->brush_faces = newface;
	}
	// link the new brush to an entity
	this->owner->linkBrush(newbrush);
	// build windings for the faces
	newbrush->buildWindings(false);
	return newbrush;
}

/*
  Returns a list with merged brushes.
  Tries to merge brushes pair wise.
  The input list is destroyed.
  Input and output should be a single linked list using .next
*/
edBrush_c *Brush_MergeListPairs(edBrush_c *brushlist, int onlyshape)
{
	int nummerges, merged;
	edBrush_c *b1, *b2, *tail, *newbrush, *newbrushlist;
	edBrush_c *lastb2;

	if (!brushlist) return NULL;

	nummerges = 0;
	do
	{
		for (tail = brushlist; tail; tail = tail->next)
		{
			if (!tail->next) break;
		}
		merged = 0;
		newbrushlist = NULL;
		for (b1 = brushlist; b1; b1 = brushlist)
		{
			lastb2 = b1;
			for (b2 = b1->next; b2; b2 = b2->next)
			{
				newbrush = b1->tryMergeWith(b2, onlyshape);
				if (newbrush)
				{
					tail->next = newbrush;
					lastb2->next = b2->next;
					brushlist = brushlist->next;
					b1->next = b1->prev = NULL;
					b2->next = b2->prev = NULL;
					Brush_Free(b1);
					Brush_Free(b2);
					for (tail = brushlist; tail; tail = tail->next)
					{
						if (!tail->next) break;
					} //end for
					merged++;
					nummerges++;
					break;
				}
				lastb2 = b2;
			}
			//if b1 can't be merged with any of the other brushes
			if (!b2)
			{
				brushlist = brushlist->next;
				//keep b1
				b1->next = newbrushlist;
				newbrushlist = b1;
			}
		}
		brushlist = newbrushlist;
	} while(merged);
	return newbrushlist;
}

/*
 Tries to merge all brushes in the list into one new brush.
 The input brush list stays intact.
 Returns NULL if no merged brush can be created.
 To create a new brush the brushes in the list may not overlap and
 the outer faces of the brushes together should make a new convex brush.

 if onlyshape is true then the merge is allowed based on the shape only
 otherwise the texture/shader references of faces in the same plane have to
 be the same as well.
*/
edBrush_c *Brush_MergeList(edBrush_c *brushlist, int onlyshape)
{
	edBrush_c *brush1, *brush2, *brush3, *newbrush;
	face_s *face1, *face2, *face3, *newface, *f;

	if (!brushlist) return NULL;
	for (brush1 = brushlist; brush1; brush1 = brush1->next)
	{
		// check if the new brush would be convex... flipped planes make a brush concave
		for (face1 = brush1->getFirstFace(); face1; face1 = face1->next)
		{
			// don't check face1 if it touches another brush
			for (brush2 = brushlist; brush2; brush2 = brush2->next)
			{
				if (brush2 == brush1) continue;
				for (face2 = brush2->getFirstFace(); face2; face2 = face2->next)
				{
					if (face1->plane.isPlaneEqual(face2->plane, true))
					{
						break;
					}
				}
				if (face2) break;
			}
			// if face1 touches another brush
			if (brush2) continue;
			//
			for (brush2 = brush1->next; brush2; brush2 = brush2->next)
			{
				// don't check the faces of brush 2 touching another brush
				for (face2 = brush2->getFirstFace(); face2; face2 = face2->next)
				{
					for (brush3 = brushlist; brush3; brush3 = brush3->next)
					{
						if (brush3 == brush2) continue;
						for (face3 = brush3->getFirstFace(); face3; face3 = face3->next)
						{
							if (face2->plane.isPlaneEqual(face3->plane, true)) break;
						}
						if (face3) break;
					}
					// if face2 touches another brush
					if (brush3) continue;
					//
					if (face1->plane.isPlaneEqual(face2->plane, false))
					{
						//if the texture/shader references should be the same but are not
						if (!onlyshape && _stricmp(face1->getMatName(), face2->getMatName()) != 0) return NULL;
						continue;
					}
					//
					if (texturedWinding_c::planesConcave(face1->face_winding, face2->face_winding,
											face1->plane.norm, face2->plane.norm,
											face1->plane.dist, face2->plane.dist))
					{
						return NULL;
					}
				}
			}
		}
	}
	//
	newbrush = new edBrush_c(false);
	//
	for (brush1 = brushlist; brush1; brush1 = brush1->next)
	{
		for (face1 = brush1->getFirstFace(); face1; face1 = face1->next)
		{
			// don't add face1 to the new brush if it touches another brush
			for (brush2 = brushlist; brush2; brush2 = brush2->next)
			{
				if (brush2 == brush1) continue;
				for (face2 = brush2->getFirstFace(); face2; face2 = face2->next)
				{
					if (face1->plane.isPlaneEqual(face2->plane, true))
					{
						break;
					}
				}
				if (face2) break;
			}
			if (brush2) continue;
			// don't add faces with the same plane twice
			for (f = newbrush->getFirstFace(); f; f = f->next)
			{
				if (face1->plane.isPlaneEqual(f->plane, false))
					break;
				if (face1->plane.isPlaneEqual(f->plane, true))
					break;
			}
			if (f)
				continue;
			//
			newface = new face_s();
			newface->texdef = face1->texdef;
			newface->planepts[0] = face1->planepts[0];
			newface->planepts[1] = face1->planepts[1];
			newface->planepts[2] = face1->planepts[2];
			newface->plane = face1->plane;
			newface->next = newbrush->getFirstFace();
			newbrush->setFaces(newface);
		}
	}
	// link the new brush to an entity
	brushlist->owner->linkBrush(newbrush);
	// build windings for the faces
	newbrush->buildWindings(false);
	return newbrush;
}

/*
 Returns a list of brushes that remain after B is subtracted from A.
 May by empty if A is contained inside B.
 The originals are undisturbed.
*/
edBrush_c *Brush_Subtract(edBrush_c *a, edBrush_c *b)
{
	// a - b = out (list)
	edBrush_c *front, *back;
	edBrush_c *in, *out, *next;
	face_s *f;

	in = a;
	out = NULL;
	for (f = b->getFirstFace(); f && in; f = f->next)
	{
		in->splitBrushByFace(f, &front, &back);
		if (in != a) Brush_Free(in);
		if (front)
		{	// add to list
			front->next = out;
			out = front;
		}
		in = back;
	}
	//NOTE: in != a just in case brush b has no faces
	if (in && in != a)
	{
		Brush_Free(in);
	}
	else
	{	//didn't really intersect
		for (b = out; b; b = next)
		{
			next = b->next;
			b->next = b->prev = NULL;
			Brush_Free(b);
		}
		return a;
	}
	return out;
}


void CSG_Subtract ()
{
	edBrush_c		*b, *s, *fragments, *nextfragment, *frag, *next, *snext;
	edBrush_c		fragmentlist(true);
	int			i, numfragments, numbrushes;

	Sys_Printf ("Subtracting...\n");

	if (selected_brushes.next == &selected_brushes)
	{
		Sys_Printf("No brushes selected.\n");
		return;
	}

	fragmentlist.next = &fragmentlist;
	fragmentlist.prev = &fragmentlist;

	numfragments = 0;
	numbrushes = 0;
	for (b = selected_brushes.next ; b != &selected_brushes ; b=next)
	{
		next = b->next;

		if (b->owner->getEntityClass()->isFixedSize())
			continue;	// can't use texture from a fixed entity, so don't subtract

		// chop all fragments further up
		for (s = fragmentlist.next; s != &fragmentlist; s = snext)
		{
			snext = s->next;

			for (i=0 ; i<3 ; i++)
				if (b->getMins()[i] >= s->getMaxs()[i] - ON_EPSILON 
				|| b->getMaxs()[i] <= s->getMins()[i] + ON_EPSILON)
					break;
			if (i != 3)
				continue;	// definately don't touch
			fragments = Brush_Subtract(s, b);
			// if the brushes did not really intersect
			if (fragments == s)
				continue;
			// try to merge fragments
			fragments = Brush_MergeListPairs(fragments, true);
			// add the fragments to the list
			for (frag = fragments; frag; frag = nextfragment)
			{
				nextfragment = frag->next;
				frag->next = NULL;
				frag->owner = s->owner;
				Brush_AddToList(frag, &fragmentlist);
			}
			// free the original brush
			Brush_Free(s);
		}

		// chop any active brushes up
		for (s = active_brushes.next; s != &active_brushes; s = snext)
		{
			snext = s->next;

			if (s->owner->getEntityClass()->isFixedSize() || s->patchBrush || s->terrainBrush || s->hiddenBrush)
				continue;

			//face_s *pFace = s->brush_faces;
			if (s->getFirstFace()->d_texture->hasNoCarveFlag())
			{
				continue;
			}

			for (i=0 ; i<3 ; i++)
				if (b->getMins()[i] >= s->getMaxs()[i] - ON_EPSILON 
				|| b->getMaxs()[i] <= s->getMins()[i] + ON_EPSILON)
					break;
			if (i != 3)
				continue;	// definately don't touch

			fragments = Brush_Subtract(s, b);
			// if the brushes did not really intersect
			if (fragments == s)
				continue;
			//
			Undo_AddBrush(s);
			// one extra brush chopped up
			numbrushes++;
			// try to merge fragments
			fragments = Brush_MergeListPairs(fragments, true);
			// add the fragments to the list
			for (frag = fragments; frag; frag = nextfragment)
			{
				nextfragment = frag->next;
				frag->next = NULL;
				frag->owner = s->owner;
				Brush_AddToList(frag, &fragmentlist);
			}
			// free the original brush
			Brush_Free(s);
		}
	}

	// move all fragments to the active brush list
	for (frag = fragmentlist.next; frag != &fragmentlist; frag = nextfragment)
	{
		nextfragment = frag->next;
		numfragments++;
		Brush_RemoveFromList(frag);
		Brush_AddToList(frag, &active_brushes);
		Undo_EndBrush(frag);
	}

	if (numfragments == 0)
	{
		Sys_Printf("Selected brush%s did not intersect with any other brushes.\n",
					(selected_brushes.next->next == &selected_brushes) ? "":"es");
		return;
	}
	Sys_Printf("done. (created %d fragment%s out of %d brush%s)\n", numfragments, (numfragments == 1)?"":"s",
							numbrushes, (numbrushes == 1)?"":"es");
	Sys_UpdateWindows(W_ALL);
}

void CSG_Merge()
{
	edBrush_c *b, *next, *newlist, *newbrush;
	class entity_s	*owner;

	Sys_Printf ("Merging...\n");

	if (selected_brushes.next == &selected_brushes)
	{
		Sys_Printf("No brushes selected.\n");
		return;
	}

	if (selected_brushes.next->next == &selected_brushes)
	{
		Sys_Printf("At least two brushes have to be selected.\n");
		return;
	}

	owner = selected_brushes.next->owner;

	for (b = selected_brushes.next; b != &selected_brushes; b = next)
	{
		next = b->next;

		if (b->owner->getEntityClass()->isFixedSize())
		{
			// can't use texture from a fixed entity, so don't subtract
			Sys_Printf("Cannot add fixed size entities.\n");
			return;
		}

		if (b->patchBrush)
		{
			Sys_Printf("Cannot add patches.\n");
			return;
		}
		if (b->terrainBrush)
		{
			Sys_Printf("Cannot add terrains.\n");
			return;
		}

		if (b->getFirstFace()->d_texture->hasNoCarveFlag())
		{
			Sys_Printf("Cannot add brushes using shaders that don't allows CSG operations.\n");
			return;
		}

		if (b->owner != owner)
		{
			Sys_Printf("Cannot add brushes from different entities.\n");
			return;
		}

	}

	newlist = NULL;
	for (b = selected_brushes.next; b != &selected_brushes; b = next)
	{
		next = b->next;

		Brush_RemoveFromList(b);
		b->next = newlist;
		b->prev = NULL;
		newlist = b;
	}

	newbrush = Brush_MergeList(newlist, true);
	// if the new brush would not be convex
	if (!newbrush)
	{
		// add the brushes back into the selection
		for (b = newlist; b; b = next)
		{
			next = b->next;
			b->next = NULL;
			b->prev = NULL;
			Brush_AddToList(b, &selected_brushes);
		}
		Sys_Printf("Cannot add a set of brushes with a concave hull.\n");
		return;
	}
	// free the original brushes
	for (b = newlist; b; b = next)
	{
		next = b->next;
		b->next = NULL;
		b->prev = NULL;
		Brush_Free(b);
	}
	Brush_AddToList(newbrush, &selected_brushes);

	Sys_Printf ("done.\n");
	Sys_UpdateWindows (W_ALL);
}
