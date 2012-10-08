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
// cm_model.cpp
#include "cm_model.h"
#include <api/coreAPI.h>
#include <shared/hashTableTemplate.h>

static hashTableTemplateExt_c<cmObjectBase_c> cm_models;

cMod_i *CM_FindModelInternal(const char *name) {
	cmObjectBase_c *b = cm_models.getEntry(name);
	cMod_i *ret = dynamic_cast<cMod_i*>(b);
	return ret;
}
void CM_FormatCapsuleModelName(str &out, float h, float r) {
	out = va("_c%f_%f",h,r);
}
void CM_FormatBBExtsModelName(str &out, const float *halfSizes) {
	out = va("_bhe%f_%f_%f",halfSizes[0],halfSizes[1],halfSizes[2]);
}
cmCapsule_i *CM_RegisterCapsule(float height, float radius) {
	str modName;
	CM_FormatCapsuleModelName(modName,height,radius);
	cMod_i *existing = CM_FindModelInternal(modName);
	if(existing) {
		if(existing->getType() != CMOD_CAPSULE) {
			g_core->DropError("CM_RegisterCapsule: found non-capsule model using capsules name syntax");
			return 0;
		}
		return (cmCapsule_c*)existing;
	}
	cmCapsule_c *n = new cmCapsule_c(modName,height,radius);
	cm_models.addObject(n);
	return n;
}
class cmBBExts_i *CM_RegisterBoxExts(float halfSizeX, float halfSizeY, float halfSizeZ) {
	str modName;
	CM_FormatBBExtsModelName(modName,vec3_c(halfSizeX, halfSizeY, halfSizeZ));
	cMod_i *existing = CM_FindModelInternal(modName);
	if(existing) {
		if(existing->getType() != CMOD_BBEXTS) {
			g_core->DropError("CM_RegisterBoxExts: found non-bbexts model using bbexts name syntax");
			return 0;
		}
		return (cmBBExts_i*)existing;
	}
	cmBBExts_c *n = new cmBBExts_c(modName, vec3_c(halfSizeX, halfSizeY, halfSizeZ));
	cm_models.addObject(n);
	return n;
}
class cMod_i *CM_RegisterModel(const char *modName) {
	cMod_i *existing = CM_FindModelInternal(modName);
	if(existing) {
		return existing;
	}
	// check for primitive models
	if(modName[0] == '_') {
		const char *t = modName+1;
		if(!Q_stricmpn(t,"c",1)) {
			// that's a capsule
			float radius, height;
			sscanf(modName,"_c%f_%f",&height,&radius);
			return CM_RegisterCapsule(height,radius);
		} else if(!Q_stricmpn(t,"bhe",3)) {
			// that's a bb defined by halfsizes
			vec3_c halfSizes;
			sscanf(modName,"_bhe%f_%f_%f",&halfSizes.x,&halfSizes.y,&halfSizes.z);
			return CM_RegisterBoxExts(halfSizes.x,halfSizes.y,halfSizes.z);
		} else {
			return 0;
		}
	}
	return 0;
}