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
	rModelAPI_i *viewModel = cgs.gameModels[cg.snap->ps.customViewRModelIndex];
	if(viewModel == 0) {
		CG_FreeViewModelEntity();
		return;
	}
	CG_AllocViewModelEntity();
	// always update viewmodel position
	cg_viewModelEntity->setOrigin(cg.refdef.vieworg);
	cg_viewModelEntity->setAngles(cg.refdefViewAngles);
	// set viewmodel model
	//rModelAPI_i *viewModel = rf->registerModel("models/testweapons/xrealMachinegun/machinegun_view.md5mesh");
	cg_viewModelEntity->setModel(viewModel);
}

