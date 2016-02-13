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
#include <stdafx.h>
#include "qe3.h"
#include <shared/parser.h>
#include <api/entityDeclAPI.h>
#include <api/declManagerAPI.h>
#include <api/rApi.h>



//
int g_entityId = 1;

void entity_s::onBrushSelectedStateChanged(bool newBIsSelected) {
	if(rEnt) {
		if(newBIsSelected) {
			float red[4] = { 1, 0.7, 0.7, 1 };
			rEnt->setColor(red);
		} else {
			rEnt->setColor(0);
		}
	}
}
void entity_s::removeREntity() {
	if(rEnt) {
		rf->removeEntity(rEnt);
		rEnt = 0;
	}
}
void entity_s::moveOrigin(const vec3_c &delta) {
	origin += delta;
	char s[256];
	sprintf(s,"%f %f %f",origin.getX(),origin.getY(),origin.getZ());
	keyValues.set("origin",s);
	if(rEnt) {
		rEnt->setOrigin(origin);
	}
}
void entity_s::trackMD3Angles(const char *key, const char *value)
{
	if (_strcmpi(key, "angle") != 0)
	{
		return;
	}

	if (this->eclass->hasDefinedModel() || this->eclass->hasEditorFlagMiscModel())
	{
		float a = this->getKeyFloat("angle");
		float b = atof(value);
		if (a != b)
		{
			vec3_t vAngle;
			vAngle[0] = vAngle[1] = 0;
			vAngle[2] = -a;
			this->brushes.onext->rotateBrush(vAngle, this->origin, true);
			vAngle[2] = b;
			this->brushes.onext->rotateBrush(vAngle, this->origin, true);
		}
	}
}

void entity_s::setKeyValue(const char *key, const char *value) {
	if (this == NULL)
		return;

	if (!key || !key[0])
		return;

	trackMD3Angles(key, value);
	if(!stricmp(key,"model")) {
		if(eclass->hasEditorFlagMiscModel() || eclass->hasDefinedModel()) {
			if(rEnt == 0) {
				rEnt = rf->allocEntity();
			}
			rEnt->setModel(rf->registerModel(value));
			rEnt->setOrigin(this->origin);
			createGhostBrushForNonBrushEntity();
		}
	}

	this->keyValues.set(key,value);
}
entity_s::entity_s(bool bIsLinkedListHeader) {
	prev = 0;
	next = 0;
	undoId = 0;
	redoId = 0;
	entityId = 0;
	eclass = 0;
	rEnt = 0;

	// mark linked list as empty
	//this->brushes.onext = &this->brushes;
	//this->brushes.oprev = &this->brushes;

	this->bIsLinkedListHeader = bIsLinkedListHeader;
	// allocated counter is only used for debugging
	if(bIsLinkedListHeader==false)
		g_allocatedCounter_entity++;
}
entity_s::~entity_s() {
	if(this->brushes.onext != 0) {
		while (this->brushes.onext != &this->brushes)
			Brush_Free (this->brushes.onext);
	}
	if(rEnt) {
		rf->removeEntity(rEnt);
		rEnt = 0;
	}
	if (this->next)
	{
		this->next->prev = this->prev;
		this->prev->next = this->next;
	}
	// allocated counter is only used for debugging
	if(bIsLinkedListHeader==false) 
		g_allocatedCounter_entity--;
}


void Entity_AddToList(entity_s *e, entity_s *list)
{
	if (e->next || e->prev)
		Error ("Entity_AddToList: allready linked");
	e->next = list->next;
	list->next->prev = e;
	list->next = e;
	e->prev = list;
}

void Entity_RemoveFromList (entity_s *e)
{
	if (!e->next || !e->prev)
		Error ("Entity_RemoveFromList: not linked");
	e->next->prev = e->prev;
	e->prev->next = e->next;
	e->next = e->prev = NULL;
}

void entity_s::createGhostBrushForNonBrushEntity() {
	// create a custom brush
	aabb bb;
	//float a = 0;
	if (eclass->hasEditorFlagMiscModel()) {
		bb = this->getREntity()->getBoundsABS();
	} else if (eclass->hasDefinedModel()) {
		bb = this->getREntity()->getBoundsABS();
	} else {
		bb.fromTwoPoints(eclass->getEditorMins() + this->origin,eclass->getEditorMaxs() + this->origin);
	}
	if(bb.isValid () == false) {
		bb.fromPointAndRadius(this->origin,16.f);
	}
	bool bWasSelected = false;
	// V: NOTE: here is created a ghost brush for fixed-size entity
	// GHOST BRUSH CREATION IS HERE!
	texdef_t td;
	td.setName(eclass->getEditorMaterialName());
	
	edBrush_c *b = 0;
	if(this->brushes.onext != 0) {
		if (this->brushes.onext != &this->brushes) {
			//if(this->brushes.onext->se
			b =this->brushes.onext;
		}
	}
	if(b == 0) {
		b = Brush_Create (bb.getMins(), bb.getMaxs(), &td);
		b->onext = this->brushes.onext;
		b->oprev = &this->brushes;
		this->brushes.onext->oprev = b;
	this->brushes.onext = b;
	} else {
		b->setupBox(bb.getMins(), bb.getMaxs(), &td);
	}
	b->owner = this;
	Brush_Build(b);
	//if (a)
	//{
	//	vec3_t vAngle;
	//	vAngle[0] = vAngle[1] = 0;
	//	vAngle[2] = a;
	//	b->rotateBrush(vAngle, this->origin, false);
	//}
	//
	//bool bOnList = false;
	//for (edBrush_c *brush = active_brushes.next ; brush != &active_brushes ; brush=brush->next)
	//{
	//	if(brush == b)
	//		bOnList = true;
	//}
	//if(bOnList == false) {
	//	Brush_AddToList (b, &selected_brushes);
	//}
	
	
}
entity_s	*Entity_Parse (class parser_c &p, edBrush_c* pList)
{
	entity_s	*ent;
	entityDeclAPI_i	*e;
	edBrush_c		*b;
	vec3_c		mins, maxs;
	bool	has_brushes;
	

	if (p.tryToGetNextToken())
		return NULL;
	
	if (strcmp (p.getLastStoredToken(), "{") )
		Error ("ParseEntity: { not found");
	
	ent = new entity_s();
	ent->entityId = g_entityId++;
	ent->brushes.onext = ent->brushes.oprev = &ent->brushes;

  int n = 0;
	do
	{
		if (p.tryToGetNextToken())
		{
			Warning ("ParseEntity: EOF without closing brace");
			return NULL;
		}
		if (!strcmp (p.getLastStoredToken(), "}") )
			break;
		if (!strcmp (p.getLastStoredToken(), "{") )
		{
			b = Brush_Parse (p);
			if (b != NULL)
			{
				b->owner = ent;
				// add to the end of the entity chain
				b->onext = &ent->brushes;
				b->oprev = ent->brushes.oprev;
				ent->brushes.oprev->onext = b;
				ent->brushes.oprev = b;
			}
			else
			{
				break;
			}
		}
		else
		{
			char key[2048];
			strcpy (key, p.getLastStoredToken());

			p.getToken();

			ent->keyValues.set(key,p.getLastStoredToken());
		}
	} while (1);
	
  // group info entity?
	if (strcmp(ent->getKeyValue("classname"), "group_info") == 0)
		 return ent;

	if (ent->brushes.onext == &ent->brushes)
		has_brushes = false;
	else
		has_brushes = true;
	
	ent->getKeyVector("origin", ent->origin);
	
	e = g_declMgr->findOrCreateEntityDecl (ent->getKeyValue("classname"), has_brushes);
	ent->eclass = e;

	if (e->isFixedSize())
	{	// fixed size entity
		if (ent->brushes.onext != &ent->brushes)
		{
			printf ("Warning: Fixed size entity with brushes\n");
#if 0
			while (ent->brushes.onext != &ent->brushes)
			{	// FIXME: this will free the entity and crash!
				Brush_Free (b);
			}
#endif
			ent->brushes.next = ent->brushes.prev = &ent->brushes;
		}
		if(e->hasDefinedModel()) {
			ent->setKeyValue("model",e->getModelName());
		} else {
			const char *p = ent->getKeyValue( "model");
			ent->setKeyValue("model",p);
		}
		ent->createGhostBrushForNonBrushEntity();
	}
	else
	{	// brush entity
		if (ent->brushes.next == &ent->brushes)
			printf ("Warning: Brush entity with no brushes\n");
	}
	
	// add all the brushes to the main list
	if (pList)
	{
		for (b=ent->brushes.onext ; b != &ent->brushes ; b=b->onext)
		{
			b->next = pList->next;
			pList->next->prev = b;
			b->prev = pList;
			pList->next = b;
		}
	}
	
	return ent;
}

void Entity_Write (entity_s *e, FILE *f, bool use_region)
{
	edBrush_c		*b;
	vec3_c		origin;
	char		text[128];
	int			count;

	// if none of the entities brushes are in the region,
	// don't write the entity at all
	if (use_region)
	{
		// in region mode, save the camera position as playerstart
		if ( !strcmp(e->getKeyValue( "classname"), "info_player_start") )
		{
			fprintf (f, "{\n");
			fprintf (f, "\"classname\" \"info_player_start\"\n");
			fprintf (f, "\"origin\" \"%i %i %i\"\n", (int)g_pParentWnd->GetCamera()->Camera().origin[0],
				(int)g_pParentWnd->GetCamera()->Camera().origin[1], (int)g_pParentWnd->GetCamera()->Camera().origin[2]);
			fprintf (f, "\"angle\" \"%i\"\n", (int)g_pParentWnd->GetCamera()->Camera().angles[YAW]);
			fprintf (f, "}\n");
			return;
		}

		for (b=e->brushes.onext ; b != &e->brushes ; b=b->onext)
			if (!Map_IsBrushFiltered(b))
				break;	// got one

		if (b == &e->brushes)
			return;		// nothing visible
	}

	//if ( e->eclass->nShowFlags & ECLASS_PLUGINENTITY )
	//{
	//	// NOTE: the whole brush placement / origin stuff is a mess
	//	origin = e->origin;
	//	sprintf (text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
	//	e->setKeyValue( "origin", text);
	//}
	// if fixedsize, calculate a new origin based on the current
	// brush position
//	else 
	if (e->eclass->isFixedSize())
	{
		//if (e->eclass->hasEditorFlagMiscModel() && e->md3Class != NULL)
		//{
		//	origin = e->origin;
		//	//VectorSubtract (e->brushes.onext->mins, e->md3Class->mins, origin);
		//}
		//else
		//{
		//	origin = e->brushes.onext->getMins() - e->eclass->getEditorMins();
		//}
		//sprintf (text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
		//e->setKeyValue( "origin", text);
	}

	fprintf (f, "{\n");
	for(u32 i = 0; i < e->keyValues.size(); i++) {
		const char *key = e->keyValues.getKey(i);
		const char *value = e->keyValues.getValue(i);
		fprintf (f, "\"%s\" \"%s\"\n", key, value);
	}

	if (!e->eclass->isFixedSize())
	{
		count = 0;
		for (b=e->brushes.onext ; b != &e->brushes ; b=b->onext)
		{
			if (!use_region || !Map_IsBrushFiltered (b))
      {
				fprintf (f, "// brush %i\n", count);
				count++;
				Brush_Write (b, f);
			}
		}
	}
	fprintf (f, "}\n");
}



bool IsBrushSelected(edBrush_c* bSel)
{
	for (edBrush_c* b = selected_brushes.next ;b != NULL && b != &selected_brushes; b = b->next)
  {
    if (b == bSel)
      return true;
  }
  return false;
}


void Entity_WriteSelected(entity_s *e, FILE *f)
{
	edBrush_c		*b;
	vec3_c		origin;
	char		text[128];
	int			count;

	for (b=e->brushes.onext ; b != &e->brushes ; b=b->onext)
		if (IsBrushSelected(b))
			break;	// got one

	if (b == &e->brushes)
		return;		// nothing selected

	// if fixedsize, calculate a new origin based on the current
	// brush position
	if (e->eclass->isFixedSize())
	{
    //if (e->eclass->hasEditorFlagMiscModel() && e->md3Class != NULL)
    //{
    //  origin = e->origin;
		  ////VectorSubtract (e->brushes.onext->mins, e->md3Class->mins, origin);
    //}
    //else
    {
		origin = e->brushes.onext->getMins() - e->eclass->getEditorMins();
    }
    sprintf (text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
	e->setKeyValue("origin", text);
	}

  fprintf (f, "{\n");
    for(u32 i = 0; i < e->keyValues.size(); i++) 
	{
		const char *key = e->keyValues.getKey(i);
		const char *value = e->keyValues.getValue(i);
	  fprintf (f, "\"%s\" \"%s\"\n", key, value);
	}

  if (!e->eclass->isFixedSize())
  {
	  count = 0;
	  for (b=e->brushes.onext ; b != &e->brushes ; b=b->onext)
	  {
		  if (IsBrushSelected(b))
		  {
			  fprintf (f, "// brush %i\n", count);
			  count++;
			  Brush_Write (b, f);
		  }
	  }
  }
	fprintf (f, "}\n");
}

void Entity_WriteSelected(entity_s *e, CMemFile* pMemFile)
{
	edBrush_c		*b;
	vec3_c		origin;
	char		text[128];
	int			count;

	for (b=e->brushes.onext ; b != &e->brushes ; b=b->onext)
		if (IsBrushSelected(b))
			break;	// got one

	if (b == &e->brushes)
		return;		// nothing selected

	// if fixedsize, calculate a new origin based on the current
	// brush position
	if (e->eclass->isFixedSize())
	{
    //if (e->eclass->hasEditorFlagMiscModel() && e->md3Class != NULL)
    //{
		  ////VectorSubtract (e->brushes.onext->mins, e->md3Class->mins, origin);
    //  origin = e->origin;
    //}
    //else
    {
		origin =  e->brushes.onext->getMins() - e->eclass->getEditorMins();
    }
    sprintf (text, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
	e->setKeyValue("origin", text);
	}

  MemFile_fprintf(pMemFile, "{\n");
	for(u32 i = 0; i < e->keyValues.size(); i++) {
		const char *key = e->keyValues.getKey(i);
		const char *value = e->keyValues.getValue(i);
	  MemFile_fprintf(pMemFile, "\"%s\" \"%s\"\n", key, value);
	}

  if (!e->eclass->isFixedSize())
  {
	  count = 0;
	  for (b=e->brushes.onext ; b != &e->brushes ; b=b->onext)
	  {
		  if (IsBrushSelected(b))
		  {
			  MemFile_fprintf(pMemFile, "// brush %i\n", count);
			  count++;
			  Brush_Write (b, pMemFile);
		  }
	  }
  }
	MemFile_fprintf(pMemFile, "}\n");
}




/*
Creates a new entity out of the selected_brushes list.
If the entity class is fixed size, the brushes are only
used to find a midpoint.  Otherwise, the brushes have
their ownership transfered to the new entity.
*/
entity_s	*Entity_Create (entityDeclAPI_i *c)
{
	entity_s	*e;
	edBrush_c		*b;
	vec3_c	mins, maxs;
	int			i;

	// check to make sure the brushes are ok

	for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
  {
		if (b->owner != world_entity)
		{
			Sys_Printf ("Entity NOT created, brushes not all from world\n");
			Sys_Beep ();
			return NULL;
		}
  }

	// create it

	e = new entity_s();
	e->entityId = g_entityId++;
	e->brushes.onext = e->brushes.oprev = &e->brushes;
	e->eclass = c;
	e->setKeyValue( "classname", c->getDeclName());

	// add the entity to the entity list
  Entity_AddToList(e, &entities);


  if (c->isFixedSize())
	{
		//
		// just use the selection for positioning
		//
		b = selected_brushes.next;
		for (i=0 ; i<3 ; i++)
			e->origin[i] = b->getMins()[i] - c->getEditorMins()[i];

		// create a custom brush
		float a = 0;
		if (c->hasEditorFlagMiscModel()) {
			const char *p = e->getKeyValue( "model");
			e->setKeyValue("model",p);
		} else if (c->hasDefinedModel()) {
			e->setKeyValue("model",c->getModelName());
		}
		e->createGhostBrushForNonBrushEntity();

		// delete the current selection
		Select_Delete ();

		b = e->brushes.onext;
		// select the new brush
		b->next = b->prev = &selected_brushes;
		selected_brushes.next = selected_brushes.prev = b;
	}
	else
	{
		//
		// change the selected brushes over to the new entity
		//
		for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
		{
			Entity_UnlinkBrush (b);
			e->linkBrush(b);
			Brush_Build( b );	// so the key brush gets a name
		}
	}

	Sys_UpdateWindows (W_ALL);
	return e;
}

void entity_s::linkBrush (edBrush_c *b)
{
	if (b->oprev || b->onext)
		Error ("Entity_LinkBrush: Allready linked");
	b->owner = this;

	b->onext = this->brushes.onext;
	b->oprev = &this->brushes;
	this->brushes.onext->oprev = b;
	this->brushes.onext = b;
}

void Entity_UnlinkBrush (edBrush_c *b)
{
	//if (!b->owner || !b->onext || !b->oprev)
	if (!b->onext || !b->oprev)
		Error ("Entity_UnlinkBrush: Not currently linked");
	b->onext->oprev = b->oprev;
	b->oprev->onext = b->onext;
	b->onext = b->oprev = NULL;
	b->owner = NULL;
}

entity_s	*Entity_Clone (entity_s *e)
{
	entity_s	*n;

	n = new entity_s();
	n->entityId = g_entityId++;
	n->brushes.onext = n->brushes.oprev = &n->brushes;
	n->eclass = e->eclass;

	// add the entity to the entity list
	Entity_AddToList(n, &entities);

	n->keyValues = e->keyValues;
	return n;
}

int GetUniqueTargetId(int iHint)
{
	int iMin, iMax, i;
	BOOL fFound;
	entity_s *pe;
	
	fFound = FALSE;
	pe = entities.next;
	iMin = 0; 
	iMax = 0;
	
	for (; pe != NULL && pe != &entities ; pe = pe->next)
	{
		i = pe->getKeyInt("target");
		if (i)
		{
			iMin = min(i, iMin);
			iMax = max(i, iMax);
			if (i == iHint)
				fFound = TRUE;
		}
	}

	if (fFound)
		return iMax + 1;
	else
		return iHint;
}

entity_s *FindEntity(char *pszKey, char *pszValue)
{
	entity_s *pe;
	
	pe = entities.next;
	
	for (; pe != NULL && pe != &entities ; pe = pe->next)
	{
		if (!strcmp(pe->getKeyValue(pszKey), pszValue))
			return pe;
	}

	return NULL;
}