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
// array.h - shared dynamic array class (std::vector style)

#ifndef __SHARED_ARRAY_H__
#define __SHARED_ARRAY_H__

#include "typedefs.h"
#include <vector>

// extended version of classic std::vector
// TODO: replace it with our own array class
template<class _Ty>
class arraySTD_c : public std::vector<typename _Ty> {
public:
	_Ty *getArray() {
		if(this->size() == 0)
			return 0;
		return &(*this)[0];
	}
	const _Ty *getArray() const {
		if(this->size() == 0)
			return 0;
		return &(*this)[0];
	}
	operator void *() {
		return (void*)getArray();
	}
	void add_unique(_Ty add) {
		int sizeNow = size();
		const _Ty *pData = getArray();
		for(int i = 0; i < sizeNow; i++) {
			if(pData[i] == add) {
				return;
			}
		}
		push_back(add);
	}
	void addArray(const std::vector<typename _Ty> &other) {
		int s = other.size();
		for(int i = 0; i < s; i++) {
			push_back(other[i]);
		}
	}
	void addArray(const _Ty *first, u32 count) {
		for(int i = 0; i < count; i++) {
			push_back(first[i]);
		}
	}
	void addArray_unique(const _Ty *first, u32 count) {
		for(int i = 0; i < count; i++) {
			add_unique(first[i]);
		}
	}
	inline int getElementSize() {
		return (sizeof(_Ty));
	}
	void nullMemory() {
		int mySizeInBytes = size()*getElementSize();
		memset(getArray(),0,mySizeInBytes);
	}
	void setMemory(byte p) {
		int mySizeInBytes = size()*getElementSize();
		memset(getArray(),p,mySizeInBytes);
	}
	s32 indexOf(const _Ty &t) const {
		for(u32 i = 0; i < this->size(); i++) {
			if(t == (*this)[i]) {
				return i;
			}
		}
		return -1;
	}
	void reverse() {
		arraySTD_c<_Ty> rev;
		rev.resize(this->size());
		u32 j = 0;
		for(int i = size()-1; i >= 0; i--, j++) {
			rev[j] = (*this)[i];
		}
		*this = rev;
	}
	bool isOnList(const _Ty &t) const {
		for(int i = 0; i < this->size(); i++) {
			if(t == (*this)[i]) {
				return true;
			}
		}
		return false;
	}
	void erase(int index) {
		std::vector<_Ty>::erase(this->begin()+index);
	}
	void remove(const _Ty &t) {
		for(u32 i = 0; i < this->size(); i++) {
			if(t == (*this)[i]) {
				erase(i);
				return;
			}
		}
	}
	void removeByPtr(const _Ty &t) {
		for(int i = 0; i < this->size(); i++) {
			if(&t == &(*this)[i]) {
				erase(i);
				return;
			}
		}
	}
	void removeObject(const _Ty object) {
		for (u32 i = 0; i < size(); i++) {
			if (object == (*this)[i]) {
				erase(i);
				return;
			}
		}
	}
	inline _Ty &pushBack() {
		int oldSize = this->size();
		this->resize(oldSize + 1);
		return (*this)[oldSize];
	}
	void fromData(const int numObjects, const _Ty *ptr) {
		this->reserve(this->size() + numObjects);
		for(int i = 0; i < numObjects; i++) {
			this->push_back(ptr[i]);
		}
	}
	u32 getSizeInBytes() const {
		return sizeof(_Ty) * this->size();
	}
	void purge() {
		for(int i = 0; i < this->size(); i++) {
			if((*this)[i] == 0)
				return;
			delete (*this)[i];
			(*this)[i] = 0;
		}
		clear();
	}
	_Ty getNextOf(const _Ty &t) const {
		if(this->size() <= 1)
			return t;
		int index = this->indexOf(t);
		if(index == this->size() - 1) {
			return (*this)[0];
		}
		return (*this)[index+1];
	}
	_Ty getPrevOf(const _Ty &t) const {
		if(this->size() == 1)
			return t;
		int index = this->indexOf(t);
		if(index == 0) {
			return (*this)[this->size() - 1];
		}
		return (*this)[index-1];
	}
	_Ty &getLast() {
		if(this->size() == 0) {
			return _Ty();;
		}
		return (*this)[this->size()-1];
	}
};

#endif // __SHARED_ARRAY_H__
