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
void AI_RTCW_Base::setKeyValue(const char *key, const char *value) {
	if(!stricmp(key,"skin")) {
		// "skin" "infantryss/assault1"
		// modelName / skinName
		str modelName = value;
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
		setAnimation("alert_bk_1h");
		setRenderModelAttachment(0,"tag_head",headPath);
		const char *skin = strchr(value,'/');
		if(skin) {
			skin++; // skip /
			setRenderModelSkin(skin);
		}
	}
	ModelEntity::setKeyValue(key,value);
}
void AI_RTCW_Base::postSpawn() {
	if(cmod) {
	}
}


