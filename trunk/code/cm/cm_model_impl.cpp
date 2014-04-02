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
// cm_model_impl.cpp
#include "cm_local.h"
#include "cm_model.h"
#include <api/skelModelAPI.h>
#include <api/coreAPI.h>

cmObjectBase_c::~cmObjectBase_c() {
	g_core->Print("Freeing %s\n",getName());
	if(parent) {
		cmCompound_c *cmpd = dynamic_cast<cmCompound_c*>(parent);
		cMod_i *cm = dynamic_cast<cMod_i*>(this);
		//cmpd->onChildDestructor(this);
	}
}

cmSkelModel_c::cmSkelModel_c() {
	skel = 0;
}
cmSkelModel_c::~cmSkelModel_c() {
	if(skel) {
		delete skel;
		skel = 0;
	}
}

int cmSkelModel_c::getBoneNumForName(const char *boneName) const {
	return skel->getLocalBoneIndexForBoneName(boneName);
}
