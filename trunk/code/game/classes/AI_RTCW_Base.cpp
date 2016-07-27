/*
============================================================================
Copyright (C) 2016 V.

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
// AI_RTCW_Base.cpp
#include "AI_RTCW_Base.h"
#include <api/serverAPI.h>

DEFINE_CLASS(AI_RTCW_Base, "ModelEntity");
 
AI_RTCW_Base::AI_RTCW_Base() {
	health = 100;
	bTakeDamage = true;

}
void AI_RTCW_Base::setSkin(const char *skinName) {
	this->skin = skinName;

	// "skin" "infantryss/assault1"
	// modelName / skinName
	str modelName = skinName;
	modelName.stripAfterFirst('/');
	str modelPath = "models/players/";
	modelPath.append(modelName);
	modelPath.append("/body.mds");
	str headPath = "models/players/";
	headPath.append(modelName);
	headPath.append("/head.mdc");
	setRenderModel(modelPath);
	// old test to play all anims
	//setAnimation(modelPath);
	// This animation name is inside wolfanim.cfg
	//setAnimation("alert_bk_1h");
	if(isLoper()) {
		setAnimation("base_frame");
		setRenderModelAttachment(1,"tag_spinner","models/players/loper/spinner.mdc");
	} else if(isZombie()) {
		setAnimation("alert_idle");
	} else if(isWarZombie()) {
		setAnimation("alert_idle");
	} else if(isSuperSoldier()) {
		setAnimation("alert_idle");
	} else if(isProtoSoldier()) {
		setAnimation("alert_idle");
	} else if(isBossHelga()) {
		setAnimation("idle");
	} else {
		setAnimation("alert_idle_1h");
	}
	
	setRenderModelAttachment(0,"tag_head",headPath);
	const char *skin = strchr(skinName,'/');
	if(skin) {
		skin++; // skip /
		setRenderModelSkin(skin);
	}
}
void AI_RTCW_Base::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"skin")) {
		// "skin" "infantryss/assault1"
		// modelName / skinName
		this->setSkin(value);
	} else {
		ModelEntity::setKeyValue(key,value);
	}
}
void AI_RTCW_Base::postSpawn() {
	if(skin.length()==0) {
		// default skin
		if(isSuperSoldier()) {
			setSkin("supersoldier/default");
		} else if(isProtoSoldier()) {
			setSkin("protosoldier/default");
		} else if(isLoper()) {
			setSkin("loper/default");
		} else if(isZombie()) {
			setSkin("zombie/default");
		} else if(isWarZombie()) {
			setSkin("warrior/default");
		} else if(isBossHelga()) {
			setSkin("beast/default");
		}
	}
	if(cmod) {
	}
}


