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
// hashTableTemplate.h - hash table templates from models/cvars/materials registers.

#ifndef __HASHTABLETEMPLATE_H__
#define __HASHTABLETEMPLATE_H__

#include "array.h"

enum {
	DEFAULT_HASH_TABLE_SIZE = 1024,
};

template <class TYPE>
class hashTableTemplate_c {
	//friend TYPE;
protected:
	TYPE *hashTable[DEFAULT_HASH_TABLE_SIZE];
	mutable int currentHash;
	void addEntry(TYPE *en) {
		en->setHashNext(hashTable[currentHash]);
		hashTable[currentHash] = en;
	}
public:
	int hashForString(const char *str) const {
		int hash = 0;
		int i = 0;
		while (str[i] != '\0') {
			char letter = tolower(str[i]);
			if (letter =='.')
				break; // don't include extension
			if (letter =='\\')
				letter = '/'; // fix path names
			hash+=(long)(letter)*(i+119);
			i++;
		}
		hash &= (DEFAULT_HASH_TABLE_SIZE-1);		
		return hash;
	}
	hashTableTemplate_c() {
		memset(hashTable,0,sizeof(hashTable));
	}
	TYPE **getTable() {
		return hashTable;
	}
	TYPE *getEntry(const char *entryName) {
		if(entryName == 0 || entryName[0] == 0)
			return 0;
		currentHash = hashForString(entryName);
		TYPE *ret;
		for (ret = hashTable[currentHash]; ret; ret = ret->getHashNext()) {
			//if(compareEntryWithNameStr(ret,entryName))
			if(!_stricmp(ret->getName(),entryName))
				return ret;
		}
		return 0;
	}
	const TYPE *getEntry(const char *entryName) const {
		if(entryName == 0 || entryName[0] == 0)
			return 0;
		currentHash = hashForString(entryName);
		TYPE *ret;
		for (ret = hashTable[currentHash]; ret; ret = ret->getHashNext()) {
			//if(compareEntryWithNameStr(ret,entryName))
			if(!_stricmp(ret->getName(),entryName))
				return ret;
		}
		return 0;
	}
	void addObject(TYPE *obj) {
		if(getEntry(obj->getName())) {
			return;
		}
		addEntry(obj);
	}
	inline bool isEntryRegistered(const char *entryName) {
		return getEntry(entryName);
	}
	inline bool isEntryRegistered(TYPE *en) {
		return getEntry(en->name);
	}
	void removeEntry(TYPE *en) {
		currentHash = hashForString(en->getName());
		TYPE *other = hashTable[currentHash];
		if(other == 0) {
			return;
		}
		if(other == en) {
			hashTable[currentHash] = other->getHashNext();
			return;
		}
		TYPE *prev = other;
		other = other->getHashNext();
		while(other) {
			if(other==en) {
//				G_assert(prev->getHashNext() == other);
				prev->setHashNext(other->getHashNext());
				return;
			}
			prev = other;
			other = other->getHashNext();
		}
	}
	void clear() {
		memset(hashTable,0,sizeof(hashTable));
	}
};

template <class TYPE>
class hashTableTemplateExt_c {
friend TYPE;
	int iIndex;
	hashTableTemplate_c<TYPE> table;
public:
	arraySTD_c<TYPE*> ar;
	TYPE *getEntry(const char *entryName) {
		if(entryName == 0 || entryName[0] == 0)
			return 0;
		return table.getEntry(entryName);
	}
	const TYPE *getEntry(const char *entryName) const {
		if(entryName == 0 || entryName[0] == 0)
			return 0;
		return table.getEntry(entryName);
	}
	bool addObject(TYPE *obj, bool forceAdd = false) {
		if(getEntry(obj->getName())) {
			if(forceAdd == false) {
				return true; // already on list
			}
		}
		table.addObject(obj);
//		G_assert(ar.indexOf(obj)==-1);
		ar.push_back(obj);
		return false;
	}
	inline bool isEntryRegistered(const char *entryName) {
		return table.isEntryRegistered(entryName);
	}
	inline bool isEntryRegistered(TYPE *en) {
		return table.isEntryRegistered(en);
	}
	void removeEntry(TYPE *en) {
		table.removeEntry(en);
		ar.removeObject(en);
	}
	// returns 0 if entry with given name was not found (and thus not removed)
	TYPE *removeEntry(const char *entryName) {
		TYPE *removeMe = getEntry(entryName);
		if(!removeMe)
			return 0;
		removeEntry(removeMe);
		return removeMe;
	}
	static int comp(const void *a, const void *b) {
		TYPE *ta = *(TYPE**)a;	
		TYPE *tb = *(TYPE**)b;	
		return stricmp(ta->getName(),tb->getName());
	}
	void sortArray() {
		qsort(ar.getArray(),ar.size(),ar.getElementSize(),comp);;
	}
	void iterateEntries(void (*callback)(const char *entryName)) {
		for(u32 e = 0; e < ar.size(); e++) {
			callback(ar[e]->name);
		}
	}
	void iterateEntries(void (*callback)(const TYPE *entry)) const {
		for(u32 e = 0; e < ar.size(); e++) {
			callback(ar[e]);
		}
	}
	void iterateEntries(void (*callback)(TYPE *entry)) {
		for(u32 e = 0; e < ar.size(); e++) {
			callback(ar[e]);
		}
	}
	void iterateEntries(void (*callback)(const TYPE *entry, const u32 entryIndexInArray)) {
		for(u32 e = 0; e < ar.size(); e++) {
			callback(ar[e],e);
		}
	}
	void beginIterating() {
		iIndex = 0;
	}
	TYPE *iterateNext() {
		int oldIndex = this->iIndex;
		iIndex++;
		if(oldIndex < ar.size())
			return ar[oldIndex];
		return 0;
	}
	void clear() {
		ar.clear();
		table.clear();
	}
	inline int hashForString(const char *str) {
		return table.hashForString(str);
	}
	inline TYPE **getTable() {
		return table.getTable();
	}
	inline TYPE *objectAt(u32 index) {
		if(ar.size() < index)
			return 0;
		return ar[index];
	}
	inline const TYPE *objectAt(u32 index) const {
		if(ar.size() < index)
			return 0;
		return ar[index];
	}
	// some array_c functionality
	inline u32 size() const {
		return ar.size();
	}
	TYPE* &operator [] (const u32 index) {
		return ar[index];
	}
	const TYPE* operator [] (const u32 index) const {
		return ar[index];
	}
};

#endif // __HASHTABLETEMPLATE_H__
