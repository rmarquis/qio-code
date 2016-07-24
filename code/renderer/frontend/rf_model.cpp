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
#include <shared/autoCmd.h>
#include <api/coreAPI.h>
#include <api/modelLoaderDLLAPI.h>
#include <api/skelModelAPI.h>
#include <api/modelDeclAPI.h>
#include <api/declManagerAPI.h>
#include <api/kfModelAPI.h>
#include <api/q3PlayerModelDeclAPI.h>
#include <api/materialSystemAPI.h>
#include <api/vfsAPI.h>
#include <api/tikiAPI.h>
#include <shared/autoCvar.h>
#include <shared/wolfAnimCfg.h>
#include <shared/etCharMgr.h>

static aCvar_c rf_model_printLoadedKeyFramedModelsFrameCounts("rf_model_printLoadedKeyFramedModelsFrameCounts","0");

#include <api/skelAnimAPI.h>

class rWolfAnimCfg_c : public wolfAnimCfg_c {
	arraySTD_c<skelAnimAPI_i*> loaded;
public:
	bool loadAnimationsData(const char *animFileName) {
		skelAnimAPI_i *base = g_modelLoader->loadSkelAnimFile(animFileName);
		if(base == 0) {
			return true;
		}
		loaded.resize(anims.size());
		for(u32 i = 0; i < anims.size(); i++) {
			loaded[i] = base->createSubAnim(anims[i].firstFrame,anims[i].numFrames);
			if(loaded[i]) {
				loaded[i]->setFrameTime(anims[i].frameTime);
			}
		}
		delete base;
		return false;
	}
	const skelAnimAPI_i *findSkelAnim(const char *animName) const {
		for(u32 i = 0; i < loaded.size(); i++) {
			if(!stricmp(anims[i].name,animName)) {
				return loaded[i];
			}
		}
		return 0;
	}
};
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
// call markAsUsed on every referenced material
void model_c::markModelMaterials() {
	if(this->type == MOD_STATIC) {
		this->staticModel->markMaterials();
	} else {	
		// TODO
	}
}
void model_c::initSprite(const char *matName, float newSpriteRadius) {
	this->type = MOD_SPRITE;
	this->spriteMaterial = g_ms->registerMaterial(matName);
	this->spriteRadius = newSpriteRadius;
	this->bb.fromRadius(newSpriteRadius);
}
bool model_c::hasDefaultSkinName() const {
	if(etChar)
		return etChar->hasDefaultSkinName();
	return false;
}
const char *model_c::getDefaultSkinName() const {
	if(etChar)
		return etChar->getDefaultSkinName();
	return 0;
}
bool model_c::findWolfAnimData(const char *animName, int *firstFrame, int *lastFrame, float *fps) const {
	if(wolfAnim == 0)
		return true;
	return wolfAnim->findWolfAnimData(animName,firstFrame,lastFrame,fps);
}
const skelAnimAPI_i *model_c::findSkelAnim(const char *animName) const {
	if(wolfAnim)
		return wolfAnim->findSkelAnim(animName);
	if(etChar)
		return etChar->findSkelAnim(animName);
	return 0;
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
	} else if(type == MOD_KEYFRAMED) {
		return kfModel->getNumSurfaces();
	} else if(type == MOD_Q3PLAYERMODEL) {
		return q3PlayerModel->getNumTotalSurfaces();
	}
	return 0;
}
u32 model_c::getNumAnims() const {
	if(type == MOD_DECL) {
		return declModel->getNumAnims();
	}
	return 0;
}
bool model_c::hasAnim(const char *animName) const {
	if(type == MOD_DECL) {
		return declModel->hasAnim(animName);
	}
	return 0;	
}
void model_c::addModelDrawCalls(const class rfSurfsFlagsArray_t *extraSfFlags, const vec3_c *extraRGB) {
	if(0) {
		g_core->Print("model_c::addModelDrawCalls: model %s (%i), sfFlags ptr %i\n",this->getName(),this,extraSfFlags);
	}
	if(type == MOD_BSP) {
		myBSP->addModelDrawCalls(bspModelNum);
	} else if(type == MOD_STATIC) {
		staticModel->addDrawCalls(extraSfFlags,false,extraRGB);
	} else if(type == MOD_PROC) {
		staticModel->addDrawCalls(extraSfFlags,false,extraRGB);
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
	if(type == MOD_TIKI) {
		return tiki->getSkelModel();
	}
	return 0;
}
class tiki_i *model_c::getTIKI() const {  
	if(type == MOD_TIKI) {
		return tiki;
	}
	return 0;
}
class modelDeclAPI_i *model_c::getDeclModelAPI() const {  
	if(type == MOD_DECL) {
		return declModel;
	}
	return 0;
}
const class skelAnimAPI_i *model_c::getDeclModelAFPoseAnim() const {
	if(type == MOD_DECL) {
		return declModel->getSkelAnimAPIForAlias("af_pose");
	}
	return 0;
}
class kfModelAPI_i *model_c::getKFModelAPI() const {
	if(type == MOD_KEYFRAMED) {
		return this->kfModel;
	}
	if(type == MOD_TIKI) {
		// this is hacky
		return this->tiki->getAnimKFModel(0);
	}
	return 0;
}
const q3PlayerModelAPI_i *model_c::getQ3PlayerModelAPI() const {
	if(type == MOD_Q3PLAYERMODEL) {
		return this->q3PlayerModel;
	}
	return 0;
}
bool model_c::getTagOrientation(int tagNum, const struct singleAnimLerp_s &legs, const struct singleAnimLerp_s &torso, class matrix_c &out) const {
	if(type == MOD_Q3PLAYERMODEL) {
		return this->q3PlayerModel->getTagOrientation(tagNum,legs,torso,out);
	}
	return true; // error
}	
bool model_c::getTagOrientation(int tagNum, class matrix_c &out) const {
	if(type == MOD_STATIC) {
		return this->staticModel->getTagOrientation(tagNum,out);
	}
	return true; // error
}
bool model_c::getModelData(class staticModelCreatorAPI_i *out) const {
	if(type == MOD_BSP) {
		this->myBSP->getModelData(bspModelNum,out);
		return false;
	} else if(type == MOD_STATIC) {
		this->staticModel->getModelData(out);
		return false;
	} else if(type == MOD_PROC) {
		//this->myProcTree->getModelData(this->m
		this->staticModel->getModelData(out);
		return false;
	}
	return true;
}
void model_c::printBoneNames() const {
	if(type == MOD_DECL) {
		this->declModel->printBoneNames();
	} else if(type == MOD_SKELETAL) {
		this->skelModel->printBoneNames();
	} else {

	}
}
u32 model_c::getTotalVertexCount() const {
	if(type == MOD_DECL) {
		return this->declModel->getTotalVertexCount();
	} else if(type == MOD_SKELETAL) {
		return this->skelModel->getTotalVertexCount();
	} else if(type == MOD_STATIC) {
		return this->staticModel->getTotalVertexCount();
	} else {

	}
	return 0;
}
u32 model_c::getTotalTriangleCount() const {
	if(type == MOD_DECL) {
		return this->declModel->getTotalTriangleCount();
	} else if(type == MOD_SKELETAL) {
		return this->skelModel->getTotalTriangleCount();
	} else if(type == MOD_STATIC) {
		return this->staticModel->getTotalTriangleCount();
	} else if(type == MOD_KEYFRAMED) {
		return this->kfModel->getTotalTriangleCount();
	} else {

	}
	return 0;
}
// will return -1 if bone not found
int model_c::findBone(const char *boneName) const {
	if(type == MOD_SKELETAL) {
		return this->skelModel->getLocalBoneIndexForBoneName(boneName);
	} else if(type == MOD_TIKI) {
		return this->tiki->getBoneNumForName(boneName);
	} else {
		// TODO?
		return -1;
	}
}
bool model_c::hasStageWithoutBlendFunc() const {
	if(type == MOD_STATIC) {
		return this->staticModel->hasStageWithoutBlendFunc();
	} else {
		// TODO?
	}
	return false;
}
bool model_c::isSkeletal() const { 
	if(type == MOD_SKELETAL)
		return true;
	if(type == MOD_DECL) {
		return true; // FIXME?
	}
	if(type == MOD_TIKI) {
		return tiki->isSkeletal();
	}
	return false;
}
bool model_c::isKeyframed() const { 
	if(type == MOD_KEYFRAMED)
		return true;
	if(type == MOD_TIKI) {
		return tiki->isKeyframed();
	}
	return false;
}
void model_c::clear() {
	if(type == MOD_BSP) {
		// bsp inline models are fried in rf_bsp.cpp
	} else if(type == MOD_STATIC) {
		delete staticModel;
		staticModel = 0;
	} else if(type == MOD_SKELETAL) {
		// only if etChar is null, otherwise skelModel is a pointer from etChar class
		if(etChar == 0) {
			delete skelModel;
			skelModel = 0;
		}
	} else if(type == MOD_PROC) {
		// proc inline models are fried in rf_proc.cpp
	}
	if(wolfAnim) {
		delete wolfAnim;
		wolfAnim = 0;
	}
}

static hashTableTemplateExt_c<model_c> rf_models;

model_c *RF_AllocModel(const char *modName) {
	model_c *check = (model_c*)RF_FindModel(modName);
	if(check) {
		g_core->RedWarning("RF_AllocModel: model %s already exist. Overwriting.\n",modName);
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
class strParm_c {
	str name;
	arraySTD_c<str> args;
public:
	void fromStringArray(const arraySTD_c<str> &strings) {
		if(strings.size() == 0) {
			g_core->RedWarning("strParm_c::fromStringArray: strings.size() is 0\n");
			return;
		}
		name = strings[0];
		args.resize(strings.size()-1);
		for(u32 i = 1; i < strings.size(); i++) {
			args[i-1] = strings[i];
		}
	}
	const char *getName() const {
		return name;
	}
	bool getNumArgs() const {
		return args.size();
	}
	const char *getArg(u32 argNum) const {
		return args[argNum];
	}
};
class strParmList_c {
	arraySTD_c<strParm_c> parms;
	strParm_c *getParmForName(const char *parmName) {
		for(u32 i = 0; i < parms.size(); i++) {
			if(!_stricmp(parms[i].getName(),parmName)) {
				return &parms[i];
			}
		}
		return 0;
	}
	const strParm_c *getParmForName(const char *parmName) const {
		for(u32 i = 0; i < parms.size(); i++) {
			if(!_stricmp(parms[i].getName(),parmName)) {
				return &parms[i];
			}
		}
		return 0;
	}
public:
	void addParmString(const arraySTD_c<str> &strings) {
		strParm_c &newParm = parms.pushBack();
		newParm.fromStringArray(strings);
	}
	bool hasKey(const char *keyName) const {
		for(u32 i = 0; i < parms.size(); i++) {
			if(!_stricmp(parms[i].getName(),keyName)) {
				return true;
			}
		}
		return false;
	}
	bool getFloatForKey(const char *keyName, float &out) const {
		const strParm_c *p = getParmForName(keyName);
		if(p == 0) {
			return false; // not found
		}
		if(p->getNumArgs() < 1) {
			return false;
		}
		out = atof(p->getArg(0));
		return true; // ok
	}
};
void P_ProcessFileNameWithParameters(const char *nameWithParameters, str &outRawFName, strParmList_c &outKeys) {
	const char *p = strchr(nameWithParameters,'|');
	if(p == 0) {
		// no parameters
		outRawFName = nameWithParameters;
		return;
	}
	outRawFName.setFromTo(nameWithParameters,p);
	p++; // skip '|'
	const char *start = p;
	arraySTD_c<str> strings;
	do {
		p++;
		if(*p == ',') {
			strings.pushBack().setFromTo(start,p);
			p++;
			start = p;
		} else if(*p == '|' || *p == 0) {
			strings.pushBack().setFromTo(start,p);
			outKeys.addParmString(strings);
			strings.clear();
			if(*p == 0)
				break;
			p++;
			start = p;
		}
	} while(*p);
}

r_model_c *RF_LoadStaticModel(const char *modelName) {
	r_model_c *ret = new r_model_c;
	if(g_modelLoader->loadStaticModelFile(modelName,ret)) {
		g_core->RedWarning("Loading of static model %s failed\n",modelName);
		delete ret;
		return 0;
	} 
	ret->recalcModelTBNs();
	if(ret->getTotalTriangleCount() < 10000) {
		ret->precalculateStencilShadowCaster();
	}
	ret->createVBOsAndIBOs();
	return ret;
}
u32 RF_GetNumAllocatedModels() {
	return rf_models.size();
}
const char *RF_GetAllocatedModelName(u32 i) {
	return rf_models[i]->getName();
}
void RF_MarkModelMaterials() {
	for(u32 i = 0; i < rf_models.size(); i++) {
		model_c *m = rf_models[i];
		m->markModelMaterials();
	}
}
rModelAPI_i *RF_RegisterModel(const char *modNameWithParameters) {
	if(modNameWithParameters == 0)
		return 0;
	if(modNameWithParameters[0] == 0)
		return 0;
	// see if the model is already loaded
	rModelAPI_i *existing = rf_models.getEntry(modNameWithParameters);
	if(existing) {
		return existing;
	}
	// parse optional parameters list
	str modName;
	strParmList_c modelParms;
	P_ProcessFileNameWithParameters(modNameWithParameters,modName,modelParms);

	bool isSprite = modelParms.hasKey("sprite");

	// alloc new model (valid or not)
	model_c *ret = RF_AllocModel(modNameWithParameters);
	if(isSprite) {
		float radius;
		if(modelParms.getFloatForKey("radius",radius) == false) {
			// if radius key was not found, set the default value
			radius = 16.f;
		}
		ret->initSprite(modName,radius);
	} else if(modName[0] == '$') {
		if(g_declMgr == 0) {
			g_core->RedWarning("Cannot load Q3 $player model %s because declManager is not present.\n",modName);
		} else {
			// check for "virtual" Quake3 player models
			const char *playerModelName = modName+1;
			// playerModelName is "sarge", "biker", etc...
			ret->q3PlayerModel = g_declMgr->registerQ3PlayerDecl(playerModelName);
			if(ret->q3PlayerModel == 0) {
				g_core->RedWarning("Loading Q3 $player model %s failed\n",modName);
			} else {
				ret->type = MOD_Q3PLAYERMODEL; // that's a valid model
			}
		}
	} else if(g_modelLoader && g_modelLoader->isKeyFramedModelFile(modName)) {
		ret->kfModel = g_modelLoader->loadKeyFramedModelFile(modName);
		if(ret->kfModel == 0) {
			g_core->RedWarning("Loading keyframed model %s failed\n",modName);
		} else {
			ret->type = MOD_KEYFRAMED; // that's a valid model
			if(rf_model_printLoadedKeyFramedModelsFrameCounts.getInt()) {
				g_core->Print("Loaded keyframed model %s has %i frames\n",modName,ret->kfModel->getNumFrames());
			}	
		}
	} else if(g_modelLoader && g_modelLoader->isStaticModelFile(modName)) {
		ret->staticModel = RF_LoadStaticModel(modNameWithParameters);
		if(ret->staticModel) {
			ret->bb = ret->staticModel->getBounds();
			ret->type = MOD_STATIC; // that's a valid model
		}
	} else if(g_modelLoader && g_modelLoader->isSkelModelFile(modName)) {
		ret->skelModel = g_modelLoader->loadSkelModelFile(modName);
		if(ret->skelModel) {
			ret->type = MOD_SKELETAL; // that's a valid model
//			ret->bb = ret->skelModel->getEstimatedBounds();
			ret->bb.fromRadius(96.f);
		} else {
			g_core->RedWarning("Loading of skeletal model %s failed\n",modName);
		}
	} else if(modName.hasExt("char")) {
		// ET player model - .char file
		static etCharMgr_c test;
		etChar_c *etChar = test.registerCharacter(modName);
		if(etChar) {
			ret->skelModel = etChar->getSkelModel();
			ret->type = MOD_SKELETAL;
			ret->bb.fromRadius(96.f);
			ret->etChar = etChar;
		}
	} else if(modName.hasExt("tik")) {
		static etCharMgr_c test;
		tiki_i *t = g_tikiMgr->registerModel(modName);
		if(t) {
			ret->tiki = t;
			ret->type = MOD_TIKI;
			ret->bb.fromRadius(96.f);
		}
	} else if(g_declMgr) {
		ret->declModel = g_declMgr->registerModelDecl(modName);
		if(ret->declModel) {
			ret->type = MOD_DECL;
//			ret->bb = ret->declModel->getEstimatedBounds();
			ret->bb.fromRadius(96.f);
		}
	}
	// also check for wolfAnim.cfg
	// This is present only for mds models
	if(ret->type == MOD_SKELETAL) {
		str wolfAnimName = ret->getName();
		wolfAnimName.toDir();
		wolfAnimName.append("wolfAnim.cfg");
		if(g_vfs->FS_FileExists(wolfAnimName)) {
			ret->wolfAnim = new rWolfAnimCfg_c();
			// parse wolfAnim.cfg
			if(ret->wolfAnim->parse(wolfAnimName)) {
				delete ret->wolfAnim;
				ret->wolfAnim = 0;
			} else {
				// load animations from mds file into separate classes
				if(ret->wolfAnim->loadAnimationsData(ret->getName())) {
					delete ret->wolfAnim;
					ret->wolfAnim = 0;
				}
			}
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


void RF_ConvertStaticModelToWavefrontOBJ_f() {
	if(g_core->Argc() < 2) {
		g_core->Print("Usage: convertStaticModelToWavefrontOBJ <source model name> [<dest model name>]\n");
		return;
	}
	const char *srcModel = g_core->Argv(1);
	str dstModel;
	if(g_core->Argc() >= 3) {
		dstModel = g_core->Argv(2);
	} else {
		dstModel = srcModel;
		dstModel.stripExtension();
		dstModel.append("_converted.obj");
	}
		
	r_model_c tmp;
	if(g_modelLoader->loadStaticModelFile(srcModel,&tmp)) {
		g_core->RedWarning("Loading of static model %s failed\n",srcModel);
		return;
	} 
	tmp.writeOBJ(dstModel);
}

static aCmd_c rf_convertStaticModelToWavefrontOBJ("convertStaticModelToWavefrontOBJ",RF_ConvertStaticModelToWavefrontOBJ_f);






