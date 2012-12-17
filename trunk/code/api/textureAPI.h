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
// textureAPI.h - texture class interface
// Note that a single material (in Q3 called: shader) might use
// multiple textures, even in the single stage.

#ifndef __TEXTUREAPI_H__
#define __TEXTUREAPI_H__

#include <shared/typedefs.h>

class textureAPI_i {
public:
	virtual ~textureAPI_i() {

	}

	// returns the path to the texture file (with extension)
	virtual const char *getName() const = 0;

	virtual u32 getWidth() const = 0;
	virtual u32 getHeight() const = 0;
	virtual void setWidth(u32 newWidth) = 0;
	virtual void setHeight(u32 newHeight) = 0;

	// bClampToEdge should be set to true for skybox textures
	virtual bool getBClampToEdge() const = 0;
	
	virtual void *getInternalHandleV() const = 0;
	virtual void setInternalHandleV(void *newHandle) = 0;
	virtual u32 getInternalHandleU32() const = 0;
	virtual void setInternalHandleU32(u32 newHandle) = 0;
};

#endif // __TEXTUREAPI_H__
