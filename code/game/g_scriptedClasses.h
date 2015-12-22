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
// g_scriptedClasses.h
#ifndef __G_SCRIPTEDCLASSES_H__
#define __G_SCRIPTEDCLASSES_H__

#include <shared/str.h>
#include <shared/ePairsList.h>

class scriptedClass_c {
	str className;
	str baseClass;
	ePairList_c keyValues;

	scriptedClass_c *hashNext;
public:
	scriptedClass_c() {
		hashNext = 0;
	}
	~scriptedClass_c() {
		
	}
	void setKeyValue(const char *key, const char *value) {
		if(!_stricmp(key,"classname")) {
			this->className = value;
		} else {
			keyValues.set(key,value);
		}
	}
	void setBaseClass(const char *nBaseClassName) {
		this->baseClass = nBaseClassName;
	}
	const char *getBaseClassName() const {
		return this->baseClass;
	}
	const ePairList_c &getKeyValues() const {
		return keyValues;
	}

	const char *getName() const {
		return className;
	}
	void setHashNext(scriptedClass_c *newHashNext) {
		hashNext = newHashNext;
	}
	scriptedClass_c *getHashNext() {
		return hashNext;
	}
};

#endif // __G_SCRIPTEDCLASSES_H__

