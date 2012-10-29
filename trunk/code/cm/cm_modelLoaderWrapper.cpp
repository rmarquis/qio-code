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
// cm_modelLoaderWrapper.cpp - simple wrapper for modelLoaderDLL
// (so it can be used for generating collision models from render models)
#include <api/coreAPI.h>
#include <api/modelLoaderDLLAPI.h>
#include <shared/cmSurface.h>

bool CM_LoadRenderModelToSingleSurface(const char *rModelName, class cmSurface_c &out) {
	if(g_modelLoader->loadStaticModelFile(rModelName,&out)) {
		return true; // error
	}
	return false; // OK
}
