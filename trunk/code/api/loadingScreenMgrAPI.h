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
// loadingScreenMgrAPI.h

#ifndef __LOADINGSCREENMGRAPI_H__
#define __LOADINGSCREENMGRAPI_H__

#include "iFaceBase.h"

#define LOADINGSCREENMGR_API_IDENTSTR "LoadingScreenMGRAPI0001"

class loadingScreenMgrAPI_i : public iFaceBase_i {
public:
	virtual void addLoadingString(const char *txt, ...) = 0;
	virtual void clear() = 0;
	virtual void addDrawCalls() = 0;
	virtual bool isEmpty() = 0;
};

// NOTE: this will be NULL on dedicated builds.
// Remember to put your g_loadingScreen->SMTH calls
// inside "if" blocks!
extern loadingScreenMgrAPI_i *g_loadingScreen;

#endif // __LOADINGSCREENMGRAPI_H__
