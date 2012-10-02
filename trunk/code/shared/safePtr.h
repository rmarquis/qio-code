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
// safePtr.h - safe pointer system
#ifndef __SAFEPTR_H__
#define __SAFEPTR_H__

#include "typedefs.h"

class safePtrBase_c {
friend class safePtrObject_c;
	safePtrBase_c *next;
};

class safePtrObject_c {
	class safePtrBase_c *safePtrList;
public:
	void addSafePtr(safePtrBase_c *p) {
		p->next = safePtrList;
		safePtrList = p;
	}
	void removeSafePtr(safePtrBase_c *p) {
		if(p == safePtrList) {
			safePtrList = p->next;
			p->next = 0;
			return;
		}
		safePtrBase_c *prev = safePtrList;
		safePtrBase_c *it = safePtrList->next;
		while(it) {
			safePtrBase_c *next = it->next;
			if(it == p) {
				prev->next = it->next;
				p->next = 0;
				return;
			}
			prev = it;
			it = next;
		}
		//printf("safePtrObject_c::removeSafePtr: pointer %i not found on list!\n",p);
	}
	u32 countReferences() const {
		const safePtrBase_c *p = safePtrList;
		u32 ret = 0;
		while(p) {
			ret++;
			p = p->next;
		}
		return ret;
	}
};

template<class _Ty>
class safePtr_c {
	_Ty *myPtr;
public:
	safePtr_c() {
		myPtr = 0;
		next = 0;
	}
	~safePtr_c() {
		nullPtr();
	}
	void nullPtr() {
		if(myPtr == 0)
			return;
		safePtrObject_c *p = myPtr;
		p->removeSafePtr(this);
	}
	_Ty *operator = (_Ty *ptr) {
		if(myPtr) {
			nullPtr();
		}
		myPtr = ptr;
		myPtr->addSafePtr(this);
	}
	operator _Ty *() {
		return myPtr;
	}
};

#endif // __SAFEPTR_H__
