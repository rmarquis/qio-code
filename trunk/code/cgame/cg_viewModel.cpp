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
// cg_viewmodel.cpp - first person weapon and hands animation
#include "cg_local.h"
#include <shared/autoCvar.h>
#include <api/rAPI.h>
#include <api/rEntityAPI.h>
#include <renderer/rModelAPI.h>
#include <math/matrix.h>
#include <math/axis.h>

static aCvar_c cg_gunX("cg_gunX","0");
static aCvar_c cg_gunY("cg_gunY","0");
static aCvar_c cg_gunZ("cg_gunZ","0");

static class rEntityAPI_i *cg_viewModelEntity = 0;

void CG_FreeViewModelEntity() {
	if(cg_viewModelEntity == 0)
		return;
	rf->removeEntity(cg_viewModelEntity);
	cg_viewModelEntity = 0;
}
void CG_AllocViewModelEntity() {
	if(cg_viewModelEntity)
		return;
	cg_viewModelEntity = rf->allocEntity();
}

void CG_RunViewModel() {
	int viewModelEntity = cg.snap->ps.curWeaponEntNum;
	if(cg_thirdPerson.integer) {
		CG_FreeViewModelEntity();
		if(viewModelEntity != ENTITYNUM_NONE && cg_entities[viewModelEntity].rEnt) {
			cg_entities[viewModelEntity].rEnt->showModel();
		}
		return;
	}
	if(viewModelEntity == ENTITYNUM_NONE) {
		CG_FreeViewModelEntity();
		return;
	}
	if(cg_entities[viewModelEntity].rEnt) {
		cg_entities[viewModelEntity].rEnt->hideModel();
	}
	rModelAPI_i *viewModel;
	if(cg.snap->ps.customViewRModelIndex) {
		viewModel = cgs.gameModels[cg.snap->ps.customViewRModelIndex];
	} else {
		viewModel = 0;//cg_entities[viewModelEntity].rEnt->getModel();
	}
	if(viewModel == 0) {
		CG_FreeViewModelEntity();
		return;
	}
	CG_AllocViewModelEntity();

	vec3_c origin = cg.refdefViewOrigin;
	vec3_c angles = cg.refdefViewAngles;

	// set local offset
	vec3_c localOfs(0,0,0);

	localOfs.x += cg_gunX.getFloat();
	localOfs.y += cg_gunY.getFloat();
	localOfs.z += cg_gunZ.getFloat();

	// add local offset to hand origin
	origin.vectorMA(origin,cg.refdefViewAxis[0],localOfs.x);
	origin.vectorMA(origin,cg.refdefViewAxis[1],localOfs.y);
	origin.vectorMA(origin,cg.refdefViewAxis[2],localOfs.z);

	// always update viewmodel position
	cg_viewModelEntity->setOrigin(origin);
	cg_viewModelEntity->setAngles(angles);
	// set viewmodel model
	//rModelAPI_i *viewModel = rf->registerModel("models/testweapons/xrealMachinegun/machinegun_view.md5mesh");
	cg_viewModelEntity->setModel(viewModel);
}

