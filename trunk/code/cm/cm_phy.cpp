/*
============================================================================
Copyright (C) 2015 V.

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
// cm_phy.cpp
#include "cm_local.h"
#include <api/coreAPI.h>
#include <api/cmAPI.h>
#include <api/vfsAPI.h>
#include <shared/str.h>
#include <shared/readStream.h>

class cMod_i *CM_LoadModelFromPHYFile(const char *fname) {
#if 0
	return 0;
#else
	readStream_c r;
	if(r.loadFromFile(fname)) {
		return 0;
	}
	u32 fileSize = r.getTotalLen();
	u32 headerSize = r.readInt();
	int id = r.readInt();
	u32 numSolids = r.readInt();
	u32 checkSum = r.readInt();
	if(r.getPos() != headerSize) {
		g_core->RedWarning("CM_LoadModelFromPHYFile: %s has extra data after base header\n",fname);
	}
	r.setPos(headerSize);
	for(u32 i = 0; i < checkSum; i++) {
		u32 ofsStart = r.getPos();
		u32 size = r.readInt();
		int solidID = r.readInt();
		short solidVersion = r.readS16();
		short modelType = r.readS16();
		int surfaceSize = r.readInt();
		vec3_c dragAxisAreas;
		r.readVec3(dragAxisAreas);
		int axisMapSize = r.readInt();
		g_core->Print("CM_LoadModelFromPHYFile: ofsStart %i\n",ofsStart);
	}
	return 0;
#endif
};