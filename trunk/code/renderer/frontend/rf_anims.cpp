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
// rf_anims.cpp
#include "rf_anims.h"
#include <shared/hashTableTemplate.h>
#include <api/modelLoaderDLLAPI.h>
#include <api/skelAnimAPI.h>

void rfAnimation_c::clear() {
	if(api) {
		delete api;
		api = 0;
	}
}

static hashTableTemplateExt_c<rfAnimation_c> rf_animations;

rfAnimation_c *RF_RegisterAnimation(const char *animName) {
	rfAnimation_c *ret = rf_animations.getEntry(animName);
	if(ret) {
		return ret;
	}
	ret = new rfAnimation_c;
	ret->api = g_modelLoader->loadSkelAnimFile(animName);
	ret->name = animName;
	return ret;
}
const skelAnimAPI_i *RF_RegisterAnimation_GetAPI(const char *animName) {
	rfAnimation_c *rfAnim = RF_RegisterAnimation(animName);
	if(rfAnim && rfAnim->getAPI()) {
		return rfAnim->getAPI();
	}
	return 0;
}
void RF_ClearAnims() {
	for(u32 i = 0; i < rf_animations.size(); i++) {
		rfAnimation_c *a = rf_animations[i];
		a->clear();
		delete a;
		rf_animations[i] = 0;
	}
	rf_animations.clear();
}
