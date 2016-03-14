/*
============================================================================
Copyright (C) 2013 V.

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
// stringList.h - simple string list.
// performs much faster than arraySTD_c<str>
#ifndef __SHARED_STRINGLIST_H__
#define __SHARED_STRINGLIST_H__

#include "perStringCallback.h"

class stringList_c : public perStringCallbackListener_i {
	arraySTD_c<char*> list;
	bool bIgnoreDuplicates;

	// perStringCallbackListener_i impl
	virtual void perStringCallback(const char *s) {
		addString(s);
	}
public:
	stringList_c() {
		bIgnoreDuplicates = false;
	}
	stringList_c(const stringList_c &other) {
		list.resize(other.list.size());
		for(u32 i = 0; i < other.list.size(); i++) {
			list[i] = strdup(other.list[i]);
		}
	}
	~stringList_c() {
		freeMemory();
	}
	void setIgnoreDuplicates(bool bSet) {
		bIgnoreDuplicates = bSet;
	}
	void freeMemory() {
		for(u32 i = 0; i < list.size(); i++) {
			free(list[i]);
		}
		list.clear();
	}
	void iterateStringList(void (*callback)(const char *s)) {
		for(u32 i = 0; i < list.size(); i++) {
			callback(list[i]);
		}
	}
	int findIndex(const char *s) const {
		for(u32 i = 0; i < list.size(); i++) {
			const char *check = list[i];
			if(!_stricmp(s,check))
				return i;
		}
		return -1;
	}
	void addString(const char *s) {
		if(bIgnoreDuplicates) {
			if(findIndex(s) >= 0)
				return; // already on list
		}
		u32 l = strlen(s);
		char *n = (char*)malloc(l+1);
		strcpy(n,s);
		list.push_back(n);
	}
	const char *getString(u32 i) const {
		if(list.size() <= i)
			return "stringlist_index_out_of_range";
		return list[i];
	}
	static int CompareStringQSort(const void *v0, const void *v1) {
		const char *s0 = *((const char **)v0);
		const char *s1 = *((const char **)v1);
		return _stricmp(s0,s1);
	}
	void sortStrings() {
		qsort(list.getArray(),list.size(),list.getElementSize(),CompareStringQSort);
	}
	u32 size() const {
		return list.size();
	}
};

#endif // __SHARED_STRINGLIST_H__
