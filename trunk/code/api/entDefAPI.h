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
// entDefAPI.h
#ifndef __ENTDEFAPI_H__
#define __ENTDEFAPI_H__

#include <shared/typedefs.h>

class entDefAPI_i {
public:
	virtual const char *getClassName() const = 0;
	virtual u32 getNumKeyValues() const = 0;
	virtual bool hasClassName() const = 0;
	virtual bool hasClassName(const char *s) const = 0;
	virtual void getKeyValue(u32 idx, const char **key, const char **value) const = 0;
	// returns true if key was not found
	virtual bool getKeyValue(const char *key, int &out) const = 0;
	virtual bool getKeyValue(const char *key, class vec3_c &out) const = 0;
	virtual bool hasKey(const char *key) const = 0;
	virtual const char *getKeyValue(const char *key) const = 0;
};

#endif // __ENTDEFAPI_H__

