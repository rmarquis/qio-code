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
// modelLoaderDLLAPI.h - interface of model loading module
#ifndef __MODELLOADERDLLAPI_H__
#define __MODELLOADERDLLAPI_H__

#define MODELLOADERDLL_API_IDENTSTR "ModelLoaderDLLAPI0001"

class modelLoaderDLLAPI_i {
public:
	virtual bool isStaticModelFile(const char *fname) = 0;
	virtual bool loadStaticModelFile(const char *fname, class staticModelCreatorAPI_i *out) = 0;
};

extern class modelLoaderDLLAPI_i *g_modelLoader;

#endif // __MODELLOADERDLLAPI_H__
