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
#include <api/skelModelAPI.h>
#include <api/modelDeclAPI.h>
#include <api/declManagerAPI.h>

// for bsp inline models
void model_c::initInlineModel(class rBspTree_c *pMyBSP, u32 myBSPModNum) {
	this->type = MOD_BSP;
	this->myBSP = pMyBSP;
	this->bspModelNum = myBSPModNum;
}
// for proc inline models
void model_c::initProcModel(class procTree_c *pMyPROC, class r_model_c *modPtr) {
	this->type = MOD_PROC;
	this->myProcTree = pMyPROC;
	this->procModel = modPtr;
	this->bb = modPtr->getBounds();
}
void model_c::initStaticModel(class r_model_c *myNewModelPtr) {
	this->type = MOD_STATIC;
	this->staticModel = myNewModelPtr;
	this->bb = myNewModelPtr->getBounds();
}
u32 model_c::getNumSurfaces() const {
	if(type == MOD_BSP) {
		return 1; // FIXME?
	} else if(type == MOD_STATIC) {
		return staticModel->getNumSurfs();
	} else if(type == MOD_PROC) {
		return staticModel->getNumSurfs();
	} else if(type == MOD_SKELETAL) {
		return skelModel->getNumSurfs();
	} else if(type == MOD_DECL) {
		return declModel->getNumSurfaces();
	}
	return 0;
}
void model_c::addModelDrawCalls(const class rfSurfsFlagsArray_t *extraSfFlags) {
	if(type == MOD_BSP) {
		myBSP->addModelDrawCalls(bspModelNum);
	} else if(type == MOD_STATIC) {
		staticModel->addDrawCalls(extraSfFlags);
	} else if(type == MOD_PROC) {
		staticModel->addDrawCalls(extraSfFlags);
	}
}
bool model_c::rayTrace(class trace_c &tr) const {
	if(type == MOD_BSP) {
		return myBSP->traceRayInlineModel(bspModelNum,tr);
	} else if(type == MOD_STATIC) {
		return staticModel->traceRay(tr);
	} else if(type == MOD_PROC) {
		return staticModel->traceRay(tr);
	} else {

		return false;
	}
}

bool model_c::createStaticModelDecal(class simpleDecalBatcher_c *out, const class vec3_c &pos,
								 const class vec3_c &normal, float radius, class mtrAPI_i *material) {
	if(type == MOD_BSP) {
		return myBSP->createInlineModelDecal(bspModelNum,out,pos,normal,radius,material);
	} else if(type == MOD_STATIC) {
		return staticModel->createDecal(out,pos,normal,radius,material);
	} else if(type == MOD_PROC) {
		return staticModel->createDecal(out,pos,normal,radius,material);
	}
	return false;
}
class skelModelAPI_i *model_c::getSkelModelAPI() const {
	if(type == MOD_SKELETAL) {
		return skelModel;
	}
	if(type == MOD_DECL) {
		return declModel->getSkelModel();
	}
	return 0;
}
const class skelAnimAPI_i *model_c::getDeclModelAFPoseAnim() const {
	if(type == MOD_DECL) {
		return declModel->getSkelAnimAPIForAlias("af_pose");
	}
	return 0;
}
void model_c::clear() {
	if(type == MOD_BSP) {
		// bsp inline models are fried in rf_bsp.cpp
	} else if(type == MOD_STATIC) {
		delete staticModel;
		staticModel = 0;
	} else if(type == MOD_SKELETAL) {
		delete skelModel;
		skelModel = 0;
	} else if(type == MOD_PROC) {
		// proc inline models are fried in rf_proc.cpp
	}
}

static hashTableTemplateExt_c<model_c> rf_models;

model_c *RF_AllocModel(const char *modName) {
	model_c *check = (model_c*)RF_FindModel(modName);
	if(check) {
		g_core->Print(S_COLOR_RED,"RF_AllocModel: model %s already exist. Overwriting.\n",modName);
		check->clear();
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
			ret->bb = ret->staticModel->getBounds();
			ret->type = MOD_STATIC; // that's a valid model
		}
	} else if(g_modelLoader->isSkelModelFile(modName)) {
		ret->skelModel = g_modelLoader->loadSkelModelFile(modName);
		if(ret->skelModel) {
			ret->type = MOD_SKELETAL; // that's a valid model
//			ret->bb = ret->skelModel->getEstimatedBounds();
			ret->bb.fromRadius(96.f);
		} else {
			g_core->Print(S_COLOR_RED"Loading of skeletal model %s failed\n",modName);
		}
	} else {
		ret->declModel = g_declMgr->registerModelDecl(modName);
		if(ret->declModel) {
			ret->type = MOD_DECL;
//			ret->bb = ret->declModel->getEstimatedBounds();
			ret->bb.fromRadius(96.f);
		}
	}
	return ret;
}

void RF_ClearModels() {
	for(u32 i = 0; i < rf_models.size(); i++) {
		model_c *m = rf_models[i];
		m->clear();
		delete m;
		rf_models[i] = 0;
	}
	rf_models.clear();
}







