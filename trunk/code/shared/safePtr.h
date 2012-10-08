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
	safePtrObject_c() {
		safePtrList = 0;
	}
	~safePtrObject_c() {
		nullAllReferences();
	}
	void nullAllReferences();

	void addSafePtr(safePtrBase_c *p) {
		p->next = safePtrList;
		safePtrList = p;
	}
	void removeSafePtr(safePtrBase_c *p) {
		if(safePtrList == 0)
			return;
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
class safePtr_c : public safePtrBase_c {
friend class safePtrObject_c;
	_Ty *myPtr;
public:
	safePtr_c() {
		myPtr = 0;
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
		if(ptr == 0)
			return 0;
		myPtr = ptr;
		myPtr->addSafePtr(this);
		return ptr;
	}
	operator _Ty *() {
		return myPtr;
	}
	_Ty *operator ->() {
		return myPtr;
	}
	const _Ty *operator ->() const {
		return myPtr;
	}
};

inline void safePtrObject_c::nullAllReferences() {
	safePtrBase_c *p = safePtrList;
	while(p) {
		safePtr_c<safePtrObject_c> *sp = (safePtr_c<safePtrObject_c>*)p;
		if(p->next != sp->next)
			__asm int 3
		sp->myPtr = 0;
		p = p->next;
		sp->next = 0;
	}
}

#endif // __SAFEPTR_H__
