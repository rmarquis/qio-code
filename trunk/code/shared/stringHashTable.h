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
// stringHashTable.h
#ifndef __SHARED_STRING_HASHTABLE_H__
#define __SHARED_STRING_HASHTABLE_H__

#include <shared/hashTableTemplate.h>
#include <shared/str.h>

class stringRegister_c {
	struct strEntry_s {
		str name;
		u32 index;
		strEntry_s *hashNext;
		inline const char *getName() const {
			return name;
		}
		inline strEntry_s *getHashNext() const {
			return hashNext;
		}
		inline void setHashNext(strEntry_s *newNext) {
			hashNext = newNext;
		}
	};
	hashTableTemplateExt_c<strEntry_s> table;

public:
	stringRegister_c() {

	}
	stringRegister_c(const stringRegister_c &other);
	void operator = (const stringRegister_c &other);

	~stringRegister_c() {
		for(u32 i = 0; i < table.size(); i++) {
			delete table[i];
		}
		table.clear();
	}
	u32 registerString(const char *str) {
		strEntry_s *e = table.getEntry(str);
		if(e == 0) {
			e = new strEntry_s;
			e->name = str;
			e->index = table.size();
			table.addObject(e);
		}
		return e->index;
	}
	bool findString(const char *str, u32 &outIndex) const {
		const strEntry_s *e = table.getEntry(str);
		if(e == 0) {
			return false;
		}
		outIndex = e->index;
		return true; // found
	}
	const char *getString(u32 index) const {
		return table.objectAt(index)->getName();
	}
};

#endif // __SHARED_STRING_HASHTABLE_H__
