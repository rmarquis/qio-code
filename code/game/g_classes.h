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
// g_classes.h - game classes system
#ifndef __G_CLASSES_H__
#define __G_CLASSES_H__

#include <shared/str.h>

typedef void *(*allocNewInstanceFunc_t)();

class gClassDef_c {
	char className[64];
	char parentClass[64];

	allocNewInstanceFunc_t newInstance;
	gClassDef_c *nextDef;
public:
	gClassDef_c(const char *newClassName, const char *newParentClass, allocNewInstanceFunc_t allocNewInstanceFunc);

	void *allocNewInstance() {
		return newInstance();
	}
	const char *getClassName() const {
		return className;
	}
	bool hasClassName(const char *c) const {
		return !_stricmp(className,c);
	}
	gClassDef_c *getNext() {
		return nextDef;
	}
};

class gClassAlias_c {
	char className[64];
	char classAlias[64];
	gClassAlias_c *next;
public:
	gClassAlias_c(const char *newClassName, const char *newClassAlias);

	const char *getClassAlias() const {
		return classAlias;
	}
	const char *getClassName() const {
		return className;
	}
	gClassAlias_c *getNext() {
		return next;
	}
};

// class declaration should be in the header, inside class body
#define DECLARE_CLASS( className )					\
		static gClassDef_c myClassDef;				\
		static void *allocInstance();				\
		virtual const char *getClassName() const;

// class definition should be in .cpp file
#define DEFINE_CLASS( className, parentClass )					\
	gClassDef_c className::myClassDef(#className,parentClass,	\
					className::allocInstance);					\
		void *className::allocInstance() {						\
			return new className;								\
		}														\
		const char *className::getClassName() const {			\
			return #className;									\
		}


#define DEFINE_CLASS_ALIAS( className, aliasName ) static gClassAlias_c aliasName(#className,#aliasName);

void *G_SpawnClassDef(const char *className);
const char *G_TranslateClassAlias(const char *classNameOrAlias);

#endif // __G_CLASSES_H__
