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
// entity.h

#include <shared/ePairsList.h>

void Eclass_InitForSourceDirectory (const char *path);
// this will create a new eclass_s if one is not present
eclass_s *Eclass_ForName (const char *name, bool has_brushes);
// this will return 0 if class with given name is not present
eclass_s *Eclass_FindExisting (const char *name);
void EClass_CreateNewFromText(const char *text);

// allocated counter is only used for debugging
extern int g_allocatedCounter_entity;

struct entity_s
{
	bool bIsLinkedListHeader;
	struct entity_s	*prev, *next;
	brush_s		brushes;					// head/tail of list
	int			undoId, redoId, entityId;	// used for undo/redo
	vec3_c		origin;
	eclass_s	*eclass;
	ePairList_c keyValues;
	vec3_c vRotation;   // valid for misc_models only
	vec3_c vScale;      // valid for misc_models only

	void clearKeyValues() {
		keyValues.clear();
	}
	void deleteKey(const char *k) {
		keyValues.remove(k);
	}
	int getKeyInt(const char *k) const {
		return keyValues.getKeyInt(k);
	}
	int getKeyFloat(const char *k) const {
		return keyValues.getKeyFloat(k);
	}
	bool getKeyVector(const char *key, vec3_t vec) const {
		const char	*k = this->getKeyValue(key);
		sscanf (k, "%f %f %f", &vec[0], &vec[1], &vec[2]);
		return false;
	}
	const char *getKeyValue(const char *k) const {
		const char *r = keyValues.getKeyValue(k);
		if(r == 0)
			return "";
		return r;
	}
	void trackMD3Angles(const char *key, const char *value);
	void setKeyValue(const char *key, const char *value) {
		if (this == NULL)
			return;

		if (!key || !key[0])
			return;

		trackMD3Angles(key, value);

		this->keyValues.set(key,value);
	}
	entity_s(bool bIsLinkedListHeader = false) {
		prev = 0;
		next = 0;
		undoId = 0;
		redoId = 0;
		entityId = 0;
		eclass = 0;

		// mark linked list as empty
		//this->brushes.onext = &this->brushes;
		//this->brushes.oprev = &this->brushes;

		this->bIsLinkedListHeader = bIsLinkedListHeader;
		// allocated counter is only used for debugging
		if(bIsLinkedListHeader==false)
			g_allocatedCounter_entity++;
	}
	~entity_s() {
		if(this->brushes.onext != 0) {
			while (this->brushes.onext != &this->brushes)
				Brush_Free (this->brushes.onext);
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
};

int			Entity_MemorySize(entity_s *e);
entity_s	*Entity_Parse (brush_s* pList = NULL);
void		Entity_Write (entity_s *e, FILE *f, bool use_region);
void		Entity_WriteSelected(entity_s *e, FILE *f);
void		Entity_WriteSelected(entity_s *e, CMemFile*);
entity_s	*Entity_Create (eclass_s *c);
entity_s	*Entity_Clone (entity_s *e);
void		Entity_AddToList(entity_s *e, entity_s *list);
void		Entity_RemoveFromList(entity_s *e);

void		Entity_LinkBrush (entity_s *e, brush_s *b);
void		Entity_UnlinkBrush (brush_s *b);
entity_s	*FindEntity(char *pszKey, char *pszValue);
entity_s	*FindEntityInt(char *pszKey, int iValue);

int GetUniqueTargetId(int iHint);
