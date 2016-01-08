/*
============================================================================
Copyright (C) 2015 V.

This file is part of Qio source code.

Qio source code is free software; you can redistribute it 
and/or modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

Qio source code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
or simply visit <http://www.gnu.org/licenses/>.
============================================================================
*/
// ed_entity.h
#ifndef __EDITOR_ENTITY_H__
#define __EDITOR_ENTITY_H__

#include "ed_brush.h"
#include <shared/ePairsList.h>
#include <api/rEntityAPI.h>

// allocated counter is only used for debugging
extern int g_allocatedCounter_entity;

class entity_s {
	bool bIsLinkedListHeader;
	ePairList_c keyValues;
	vec3_c		origin;
	class entityDeclAPI_i *eclass;
	rEntityAPI_i *rEnt;
public:
	class entity_s	*prev, *next;
	edBrush_c		brushes;					// head/tail of list
	int			undoId, redoId, entityId;	// used for undo/redo
	
	entity_s *getNextEntity() {
		return next;
	}
	const vec3_c &getOrigin() const {
		return origin;
	}
	const rEntityAPI_i *getREntity() const {
		return rEnt;
	}
	void removeREntity();
	void moveOrigin(const vec3_c &delta);
	void setKeyValues(const ePairList_c &newKeyValues) {
		keyValues = newKeyValues;
	}
	const ePairList_c &getKeyValues() const {
		return keyValues;
	}
	const entityDeclAPI_i *getEntityClass() const {
		return eclass;
	}
	void setEntityClass(entityDeclAPI_i *nc) {
		eclass = nc;
	}
	//entityDeclAPI_i *getEntityClass() {
	//	return eclass;
	//}
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
	void onBrushSelectedStateChanged(bool newBIsSelected);
	void trackMD3Angles(const char *key, const char *value);
	void setKeyValue(const char *key, const char *value);
	entity_s(bool bIsLinkedListHeader = false);
	~entity_s();
	u32 getMemorySize() const {
		return sizeof(entity_s);
	}
	void linkBrush (edBrush_c *b);

	

friend entity_s	*Entity_Parse (class parser_c &p, edBrush_c* pList = NULL);
friend void		Entity_Write (entity_s *e, FILE *f, bool use_region);
friend void		Entity_WriteSelected(entity_s *e, FILE *f);
friend void		Entity_WriteSelected(entity_s *e, class CMemFile*);
friend entity_s	*Entity_Create (entityDeclAPI_i *c);
friend entity_s	*Entity_Clone (entity_s *e);
friend void		Entity_AddToList(entity_s *e, entity_s *list);
friend void		Entity_RemoveFromList(entity_s *e);

friend void		Entity_UnlinkBrush (edBrush_c *b);
friend entity_s	*FindEntity(char *pszKey, char *pszValue);

friend int GetUniqueTargetId(int iHint);
};




#endif // __EDITOR_ENTITY_H__

