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
// rf_model.cpp
#include "rf_model.h"
#include "rf_surface.h"
#include "rf_bsp.h"
#include <shared/hashTableTemplate.h>
#include <api/coreAPI.h>
#include <api/modelLoaderDLLAPI.h>

void model_c::addModelDrawCalls() {
	if(type == MOD_BSP) {
		myBSP->addModelDrawCalls(bspModelNum);
	} else if(type == MOD_STATIC) {
		staticModel->addDrawCalls();
	}
}
bool model_c::rayTrace(class trace_c &tr) const {
	if(type == MOD_BSP) {
		return myBSP->traceRayInlineModel(bspModelNum,tr);
	} else {

		return false;
	}
}

static hashTableTemplateExt_c<model_c> rf_models;

void RF_ClearModel(model_c *m) {
	// bsp inline models are fried in rf_bsp.cpp
}
model_c *RF_AllocModel(const char *modName) {
	model_c *check = (model_c*)RF_FindModel(modName);
	if(check) {
		g_core->Print(S_COLOR_RED,"RF_AllocModel: model %s already exist. Overwriting.\n",modName);
		RF_ClearModel(check);
		return check;
	}
	model_c *nm = new model_c;
	nm->name = modName;
	rf_models.addObject(nm);
	return nm;
}
rModelAPI_i *RF_FindModel(const char *modName) {
	model_c *ret = rf_models.getEntry(modName);
	return ret;
}
rModelAPI_i *RF_RegisterModel(const char *modName) {
	// see if the model is already loaded
	rModelAPI_i *existing = rf_models.getEntry(modName);
	if(existing) {
		return existing;
	}
	model_c *ret = RF_AllocModel(modName);
	if(g_modelLoader->isStaticModelFile(modName)) {
		ret->staticModel = new r_model_c;
		if(g_modelLoader->loadStaticModelFile(modName,ret->staticModel)) {
			g_core->Print(S_COLOR_RED"Loading of static model %s failed\n",modName);
			delete ret->staticModel;
			ret->staticModel = 0;
		} else {
			ret->type = MOD_STATIC; // that's a valid model
		}
	}
	return ret;
}

void RF_ClearModels() {
	for(u32 i = 0; i < rf_models.size(); i++) {
		model_c *m = rf_models[i];
		RF_ClearModel(m);
		delete m;
		rf_models[i] = 0;
	}
	rf_models.clear();
}







