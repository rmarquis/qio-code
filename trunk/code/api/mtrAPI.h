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
// mat_api.h - material class interface

#ifndef __MTR_API_H__
#define __MTR_API_H__

#include <shared/safePtr.h>

class mtrAPI_i : public safePtrObject_c { 
public:
	virtual const char *getName() const = 0;
	virtual u32 getNumStages() const = 0;
	virtual const class mtrStageAPI_i *getStage(u32 stageNum) const = 0;
	virtual const class skyParmsAPI_i *getSkyParms() const = 0;
	virtual enum drawCallSort_e getSort() const = 0;

	virtual int getImageWidth() const = 0;
	virtual int getImageHeight() const = 0;
};

#endif // __MTR_API_H__
