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
// entDef.h
#ifndef __ENTDEF_H__
#define __ENTDEF_H__

#include <api/entDefAPI.h>
#include "ePairsList.h"
#include "str.h"

class entDef_c : public entDefAPI_i {
	str className;
	ePairList_c ePairs; // key-values
	// ePrimList_c primitives; // TODO: for .map files

	bool parseSingleEntDef(class parser_c &p);

public:
	void setKeyValue(const char *key, const char *value) {
		if(!stricmp(key,"classname")
			// NOTE: "spawnclass" keyword is used in Doom3 .def files
			|| !stricmp(key,"spawnclass")
			) {
			this->className = value;
			return;
		}
		ePairs.set(key,value);
	}
	virtual const char *getClassName() const {
		if(className.length())
			return className;
		return 0;
	}
	virtual u32 getNumKeyValues() const {
		return ePairs.size();
	}
	virtual bool hasClassName() const {
		if(className.length())
			return true;
		return false;
	}
	virtual bool hasClassName(const char *s) const {
		if(!stricmp(className.c_str(),s))
			return true;
		return false;
	}
	virtual void getKeyValue(u32 idx, const char **key, const char **value) const {
		return ePairs.getKeyValue(idx,key,value);
	}
	virtual bool getKeyValue(const char *key, int &out) const {
		return ePairs.getKeyValue(key,out);
	}
	virtual bool getKeyValue(const char *key, class vec3_c &out) const {
		return ePairs.getKeyVec3(key,out);
	}
	virtual bool hasKey(const char *key) const {
		return ePairs.hasKey(key);
	}
	virtual const char *getKeyValue(const char *key) const {
		return ePairs.getKeyValue(key);
	}
	bool fromString(const char *txt);
	bool readFirstEntDefFromFile(const char *fileName);
	void fromOtherAPI(const class entDefAPI_i *p);
	void appendOtherAPI_overwrite(const class entDefAPI_i *p);

	void operator = (const entDef_c &other) {
		this->className = other.className;
		this->ePairs = other.ePairs;
	}
};

#endif // __ENTDEF_H__

