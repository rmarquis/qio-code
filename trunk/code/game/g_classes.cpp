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
// g_classes.cpp - game classes system
#include "g_local.h"
#include "g_classes.h"
#include "classes/BaseEntity.h"

// class definitions
static gClassDef_c *g_classList = 0;

gClassDef_c::gClassDef_c(const char *newClassName, const char *newParentClass, allocNewInstanceFunc_t allocNewInstanceFunc) {
	strcpy(className,newClassName);
	strcpy(parentClass,newParentClass);

	this->newInstance = allocNewInstanceFunc;
	
	// add to the list
	this->nextDef = g_classList;
	g_classList = this;
}

// class aliases
static gClassAlias_c *g_aliasList = 0;
gClassAlias_c::gClassAlias_c(const char *newClassName, const char *newClassAlias) {
	strcpy(className,newClassName);
	strcpy(classAlias,newClassAlias);

	// add to the list
	this->next = g_aliasList;
	g_aliasList = this;
}

const char *G_TranslateClassAlias(const char *classNameOrAlias) {
	gClassAlias_c *ca = g_aliasList;
	while(ca) {
		if(!_stricmp(ca->getClassAlias(),classNameOrAlias))
			return ca->getClassName();
		ca = ca->getNext();
	}
	return classNameOrAlias;
}

gClassDef_c *G_FindClassDefBasic(const char *origClassName) {
	// translate class aliases to our internal class names
	const char *className = G_TranslateClassAlias(origClassName);
	gClassDef_c *cd = g_classList;
	while(cd) {
		if(!_stricmp(cd->getClassName(),className))
			return cd;
		cd = cd->getNext();
	}
	return 0;
}

void *G_SpawnClassDef(const char *className) {
	gClassDef_c *cd = G_FindClassDefBasic(className);
	if(cd == 0)
		return 0;
	// hack
	if(cd->hasClassName("Player") || cd->hasClassName("FakePlayer")) {	
		edict_s *newEdict = G_Spawn();
		BE_SetForcedEdict(newEdict);
	}
	return cd->allocNewInstance();
}