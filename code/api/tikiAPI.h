/*
============================================================================
Copyright (C) 2016 V.

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
// tikiAPI.h - Ritual's .tik file format support
#ifndef __API_TIKI_H__
#define __API_TIKI_H__


#include "iFaceBase.h"
#include <shared/typedefs.h>

#define TIKI_API_IDENTSTR "TIKI0001"

class tiki_i {

};

class tikiAPI_i : public iFaceBase_i {
public:
	virtual void init() = 0;
	virtual void shutdown() = 0;
	virtual class tiki_i *registerModel(const char *modName) = 0;
};

extern tikiAPI_i *tiki;

#endif // __API_TIKI_H__
