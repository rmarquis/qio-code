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

void Eclass_InitForSourceDirectory (char *path);
// this will create a new eclass_s if one is not present
eclass_s *Eclass_ForName (char *name, bool has_brushes);
// this will return 0 if class with given name is not present
eclass_s *Eclass_FindExisting (const char *name);
void EClass_CreateNewFromText(const char *text);


struct entity_s
{
	struct entity_s	*prev, *next;
	brush_s		brushes;					// head/tail of list
	int			undoId, redoId, entityId;	// used for undo/redo
	edVec3_c		origin;
	eclass_s	*eclass;
	epair_s		*epairs;
	vec3_t vRotation;   // valid for misc_models only
	vec3_t vScale;      // valid for misc_models only
};

char 	*ValueForKey (entity_s *ent, const char *key);
void	SetKeyValue (entity_s *ent, const char *key, const char *value);
void 	SetKeyValue (epair_s *&e, const char *key, const char *value);
void 	DeleteKey (entity_s *ent, const char *key);
void 	DeleteKey (epair_s *&e, const char *key);
float	FloatForKey (entity_s *ent, const char *key);
int		IntForKey (entity_s *ent, const char *key);
void 	GetVectorForKey (entity_s *ent, const char *key, vec3_t vec);

void		Entity_Free (entity_s *e);
void		Entity_FreeEpairs(entity_s *e);
int			Entity_MemorySize(entity_s *e);
entity_s	*Entity_Parse (bool onlypairs, brush_s* pList = NULL);
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

//Timo : used for parsing epairs in brush primitive
epair_s* ParseEpair(void);
char *ValueForKey ( epair_s *&e, const char *key);
