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
// cg_testModel.cpp - clientside entity/model testing
#include "cg_local.h"
#include "cg_entities.h"
#include <shared/autoCvar.h>
#include <api/rAPI.h>
#include <api/rEntityAPI.h>
#include <api/coreAPI.h>
#include <renderer/rModelAPI.h>
#include <math/matrix.h>
#include <math/axis.h>

static aCvar_c cg_testModel("cg_testModel","");
static aCvar_c cg_testAnim("cg_testAnim","");
static aCvar_c cg_testModelParentEntity("cg_testModelParentEntity","");
static aCvar_c cg_testModelParentTag("cg_testModelParentTag","MG_ATTACHER");
static aCvar_c cg_testModelAttached_extraYaw("cg_testModelAttached_extraYaw","0");
static aCvar_c cg_testModelAttached_extraPitch("cg_testModelAttached_extraPitch","0");
static aCvar_c cg_testModelAttached_extraRoll("cg_testModelAttached_extraRoll","0");
static aCvar_c cg_testModelAttached_extraX("cg_testModelAttached_extraX","0");
static aCvar_c cg_testModelAttached_extraY("cg_testModelAttached_extraY","0");
static aCvar_c cg_testModelAttached_extraZ("cg_testModelAttached_extraZ","0");
static aCvar_c cg_testModel_attachToCamera("cg_testModel_attachToCamera","0");
static aCvar_c cg_testModel_printNumTris("cg_testModel_printNumTris","0");
static aCvar_c cg_testModel_attachmentModel("cg_testModel_attachmentModel","");
static aCvar_c cg_testModel_attachmentTag("cg_testModel_attachmentTag","t_weapon_R");
static aCvar_c cg_testModel_printAnimsList("cg_testModel_printAnimsList","0");

static rEntityAPI_i *cg_testModelEntity = 0;
static rEntityAPI_i *cg_testModelAttachmentEntity = 0;

static void CG_FreeTestModel() {
	if(cg_testModelEntity) {
		rf->removeEntity(cg_testModelEntity);
		cg_testModelEntity = 0;
	}
	//if(cg_testModelAttachmentEntity) {
	//	rf->removeEntity(cg_testModelAttachmentEntity);
	//	cg_testModelAttachmentEntity = 0;
	//}
}
void CG_RunTestModel() {
	if(cg_testModel.getStr()[0] == 0 || cg_testModel.getStr()[0] == '0') {
		CG_FreeTestModel();
		return;
	}
	rModelAPI_i *mod = rf->registerModel(cg_testModel.getStr());
	if(mod == 0 || mod->isValid() == false) {
		CG_FreeTestModel();
		return;
	}
	if(cg_testModelEntity == 0) {
		cg_testModelEntity = rf->allocEntity();
		cg_testModelEntity->setOrigin(cg.refdefViewOrigin);
	}
	cg_testModelEntity->setModel(mod);
	cg_testModelEntity->setAnim(cg_testAnim.getStr());
	if(cg_testModelParentEntity.strLen() && _stricmp(cg_testModelParentEntity.getStr(),"none")) {
		int entNum = atoi(cg_testModelParentEntity.getStr());
		cgEntity_c *cent = &cg_entities[entNum];
		if(cent->getRenderEntity()) {
			matrix_c orMat;
			cent->getRenderEntity()->getBoneWorldOrientation(cg_testModelParentTag.getStr(),orMat);
			cg_testModelEntity->setOrigin(orMat.getOrigin());
			vec3_c angles = orMat.getAngles();
			angles[PITCH] += cg_testModelAttached_extraPitch.getFloat();
			angles[YAW] += cg_testModelAttached_extraYaw.getFloat();
			angles[ROLL] += cg_testModelAttached_extraRoll.getFloat();
			cg_testModelEntity->setAngles(angles);
		}
	} else if(cg_testModel_attachToCamera.getInt()) {
		vec3_c usePos = cg.refdefViewOrigin;
		usePos += cg.refdefViewAxis.getForward() * cg_testModelAttached_extraX.getFloat();
		usePos += cg.refdefViewAxis.getLeft() * cg_testModelAttached_extraY.getFloat();
		usePos += cg.refdefViewAxis.getUp() * cg_testModelAttached_extraZ.getFloat();
#if 1
		matrix_c m;
		m.fromAnglesAndOrigin(cg.refdefViewAngles,usePos);
		m.rotateZ(cg_testModelAttached_extraYaw.getFloat());
		cg_testModelEntity->setOrigin(m.getOrigin());
		cg_testModelEntity->setAngles(m.getAngles());
#else
		cg_testModelEntity->setOrigin(usePos);
		cg_testModelEntity->setAngles(cg.refdefViewAngles+vec3_c(0,cg_testModelAttached_extraYaw.getFloat(),0));
#endif
	}
	
	rModelAPI_i *attachedMod = rf->registerModel(cg_testModel_attachmentModel.getStr());
	if(attachedMod == 0 || attachedMod->isValid() == false) {
		//if(cg_testModelAttachmentEntity) {
		//	rf->removeEntity(cg_testModelAttachmentEntity);
		//	cg_testModelAttachmentEntity = 0;
		//}
		cg_testModelEntity->setAttachment(0,"","");
	} else {
		cg_testModelEntity->setAttachment(0,attachedMod->getName(),cg_testModel_attachmentTag.getStr());
		//if(cg_testModelAttachmentEntity == 0) {
		//	cg_testModelAttachmentEntity = rf->allocEntity();
		//}
		//cg_testModelAttachmentEntity->setModel(mod);
		//cg_testModelEntity->setAttachment
	}
	if(cg_testModel_printNumTris.getInt()) {
		g_core->Print("Testmodel has %i triangles\n",cg_testModelEntity->getEntityTriangleCount());
	}
	if(cg_testModel_printAnimsList.getInt()) {
		for(u32 i = 0; i < mod->getNumAnims(); i++) {
			const char *alias = mod->getAnimAlias(i);
			g_core->Print("Testmodel anim %i of %i - %s\n",i,mod->getNumAnims(),alias);
		}
	}
	//cg_testModelEntity->hideSurface(2);
	//cg_testModelEntity->hideSurface(3);
	//cg_testModelEntity->hideSurface(4);
	//cg_testModelEntity->hideSurface(5);
}