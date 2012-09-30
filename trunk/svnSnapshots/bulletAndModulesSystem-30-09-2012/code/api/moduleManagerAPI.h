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
// moduleManagerAPI.h - DLL managment

#ifndef __MODULEMANAGERAPI_H__
#define __MODULEMANAGERAPI_H__

#include "iFaceBase.h"

#define MODULEMANAGER_API_IDENTSTR "ModuleManagerAPI0001"

class moduleAPI_i {
public:
	virtual const char *getName() const = 0;
};

class moduleManagerAPI_i : public iFaceBase_i {
public:
	virtual class moduleAPI_i *load(const char *moduleName) = 0;
	virtual void unload(class moduleAPI_i **mPtr) = 0;
	virtual class moduleAPI_i *restart(class moduleAPI_i *np, bool unPure) = 0;
};

extern moduleManagerAPI_i *g_moduleMgr;

#endif // __MODULEMANAGERAPI_H__
