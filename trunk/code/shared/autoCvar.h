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
// autoCvar.h - automatic cvars initialization and synchronization for dll modules
// (one cvar can be statically defined in several modules)
#ifndef __AUTOCVAR_H__
#define __AUTOCVAR_H__

#include "str.h"
#include "cvarModificationCallback.h"

typedef void (*autoCvarModificationCallback_t)(const class aCvar_c *aCvar);

class aCvar_c : public cvarModifyCallback_i {
	str name; // name of variable
	str valStr; // value string
	float valFloat; // value converted to float
	int valInt; // value converted to integer
	int cvarFlags;
	aCvar_c *nextModuleCvar;
	
	struct cvar_s *internalCvar;

	autoCvarModificationCallback_t extraModificationCallback;

	// this is called when cvar is modified trough console
	void onCvarModified(const char *newText);
public:
	aCvar_c(const char *newName, const char *newDefaultStr, int newCvarFlags = 0);

	void setExtraModificationCallback(autoCvarModificationCallback_t newModCallback);
	void setString(const char *newStr);

	int getInt() const {
		return valInt;
	}
	float getFloat() const {
		return valFloat;
	}
	const char *getStr() const {
		return valStr;
	}
	u32 strLen() const {
		return strlen(valStr);
	}

// this should be called once on every module startup
friend void AUTOCVAR_RegisterAutoCvars();
// and this on module shutdown (BEFORE g_cvars api is NULL'ed!)
friend void AUTOCVAR_UnregisterAutoCvars();
};


#endif // __AUTOCVAR_H__
