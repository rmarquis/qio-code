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
// sdlSharedAPI.h - SDL code shared between GL and DX9 backends
#ifndef __SDLSHAREDAPI_H__
#define __SDLSHAREDAPI_H__

#include <shared/typedefs.h>
#include "iFaceBase.h"

#define SHARED_SDL_API_IDENTSTRING "SharedSDLAPI0001"

class sdlSharedAPI_i : public iFaceBase_i {
public:
	virtual u32 getWinWidth() const = 0;
	virtual u32 getWinHeigth() const = 0;

	// SDL window controls
	virtual void endFrame() = 0;
	virtual void init() = 0;
	virtual void shutdown() = 0;
};

extern sdlSharedAPI_i *g_sharedSDLAPI;

#endif // __SDLSHAREDAPI_H__
