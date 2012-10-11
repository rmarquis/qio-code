/*
============================================================================
Copyright (C) 2012 V.

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
// entDefsList.h - list of entity definitions (keyValues + primitives)
#ifndef __ENTDEFSLIST_H__
#define __ENTDEFSLIST_H__

#include "ePairsList.h"

class entDef_c {
	str className;
	ePairList_c ePairs; // key-values
	// ePrimList_c primitives; // TODO: for .map files

public:
	void setKeyValue(const char *key, const char *value) {
		if(!stricmp(key,"classname")) {
			this->className = value;
			return;
		}
		ePairs.set(key,value);
	}
	const char *getClassName() {
		if(className.length())
			return className;
		return 0;
	}
	u32 getNumKeyValues() const {
		return ePairs.size();
	}
	void getKeyValue(u32 idx, const char **key, const char **value) {
		return ePairs.getKeyValue(idx,key,value);
	}
};

class entDefsList_c {
	arraySTD_c<entDef_c*> entities;
	bool fromString(const char *txt);
public:
	entDefsList_c();
	~entDefsList_c();
	void clear();

	// loads entity list from .map / .bsp / .ent file
	bool load(const char *fname);


	u32 size() const {
		return entities.size();
	}
	entDef_c * operator [] (u32 entNum) {
		return entities[entNum];
	}
	const entDef_c * operator [] (u32 entNum) const {
		return entities[entNum];
	}
};

#endif // __ENTDEFSLIST_H__
