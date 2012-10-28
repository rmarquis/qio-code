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
// staticModelCreatorAPI.h
#ifndef __STATICMODELCREATORAPI_H__
#define __STATICMODELCREATORAPI_H__

struct simpleVert_s {
	vec3_c xyz;
	vec2_c tc;
};

// this class is used by modelLoader.dll
// to pass model file data to cm/renderer modules
class staticModelCreatorAPI_i {
public:
	virtual void addTriangle(const char *matName, const struct simpleVert_s &v0,
		const struct simpleVert_s &v1, const struct simpleVert_s &v2) = 0;
};

#endif // __STATICMODELCREATORAPI_H__
