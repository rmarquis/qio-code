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
	// this will create a new eclass_t if one is not present
    eclass_t *Eclass_ForName (char *name, bool has_brushes);
	// this will return 0 if class with given name is not present
	eclass_t *Eclass_FindExisting (const char *name);
	void EClass_CreateNewFromText(const char *text);
    
    
    typedef struct entity_s
    {
    	struct entity_s	*prev, *next;
    	brush_t		brushes;					// head/tail of list
     	int			undoId, redoId, entityId;	// used for undo/redo
    	edVec3_c		origin;
    	eclass_t	*eclass;
    	epair_t		*epairs;
    	eclass_t  *md3Class;
      vec3_t vRotation;   // valid for misc_models only
      vec3_t vScale;      // valid for misc_models only
    } entity_t;
    
    char 	*ValueForKey (entity_t *ent, const char *key);
    void	SetKeyValue (entity_t *ent, const char *key, const char *value);
    void 	SetKeyValue (epair_t *&e, const char *key, const char *value);
    void 	DeleteKey (entity_t *ent, const char *key);
    void 	DeleteKey (epair_t *&e, const char *key);
    float	FloatForKey (entity_t *ent, const char *key);
    int		IntForKey (entity_t *ent, const char *key);
    void 	GetVectorForKey (entity_t *ent, const char *key, vec3_t vec);
    
    void		Entity_Free (entity_t *e);
    void		Entity_FreeEpairs(entity_t *e);
    int			Entity_MemorySize(entity_t *e);
    entity_t	*Entity_Parse (bool onlypairs, brush_t* pList = NULL);
    void		Entity_Write (entity_t *e, FILE *f, bool use_region);
    void		Entity_WriteSelected(entity_t *e, FILE *f);
    void		Entity_WriteSelected(entity_t *e, CMemFile*);
    entity_t	*Entity_Create (eclass_t *c);
    entity_t	*Entity_Clone (entity_t *e);
    void		Entity_AddToList(entity_t *e, entity_t *list);
    void		Entity_RemoveFromList(entity_t *e);
    
    void		Entity_LinkBrush (entity_t *e, brush_t *b);
    void		Entity_UnlinkBrush (brush_t *b);
    entity_t	*FindEntity(char *pszKey, char *pszValue);
    entity_t	*FindEntityInt(char *pszKey, int iValue);
    
    int GetUniqueTargetId(int iHint);
    bool Eclass_hasModel(eclass_t *e, class edVec3_c &vMin, edVec3_c &vMax);
    eclass_t* GetCachedModel(entity_t *pEntity, const char *pName, class edVec3_c &vMin, edVec3_c &vMax);
    
    //Timo : used for parsing epairs in brush primitive
    epair_t* ParseEpair(void);
    char *ValueForKey ( epair_t *&e, const char *key);
