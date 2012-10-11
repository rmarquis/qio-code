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
// ePairsList.h - list of entity key-values
#ifndef __EPAIRSLIST_H__
#define __EPAIRSLIST_H__

#include "str.h"
#include "array.h"

class ePair_c {
friend class ePairList_c;
	str key;
	str val;
public:
	ePair_c(const char *nk, const char *nv) {
		this->key = nk;
		this->val = nv;
	}
	void set(const char *newVal) {
		this->val = newVal;
	}
	const char *getKey() const {
		return key;
	}
	const char *getValue() const {
		return val;
	}
};

class ePairList_c {
	arraySTD_c<ePair_c*> pairs;
	ePair_c *find(const char *key) {
		for(u32 i = 0; i < pairs.size(); i++) {
			ePair_c *ep = pairs[i];
			if(!stricmp(key,ep->key)) {
				return ep;
			}
		}
		return 0;
	}
public:
	ePairList_c() {

	}
	~ePairList_c() {
		for(u32 i = 0; i < pairs.size(); i++) {
			delete pairs[i];
		}
	}
	void set(const char *key, const char *val) {
		ePair_c *ep = find(key);
		if(ep) {
			ep->set(val);
			return;
		}
		ePair_c *np = new ePair_c(key,val);
		pairs.push_back(np);
	}
	void getKeyValue(u32 idx, const char **key, const char **value) {
		if(idx >= pairs.size()) {
			// this should never happen
			*key = 0;
			*value = 0;
			return;
		}
		ePair_c *p = pairs[idx];
		*key = p->getKey();
		*value = p->getValue();
	}
	u32 size() const {
		return pairs.size();
	}
};


#endif // __EPAIRSLIST_H__