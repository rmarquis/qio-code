///*
//============================================================================
//Copyright (C) 2012 V.
//
//This file is part of Qio source code.
//
//Qio source code is free software; you can redistribute it 
//and/or modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//Qio source code is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//See the GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software Foundation,
//Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA,
//or simply visit <http://www.gnu.org/licenses/>.
//============================================================================
//*/
//// cm_anims.cpp
//#include "cm_anims.h"
//#include <shared/hashTableTemplate.h>
//#include <api/modelLoaderDLLAPI.h>
//#include <api/skelAnimAPI.h>
//
//void cmAnimation_c::clear() {
//	if(api) {
//		delete api;
//		api = 0;
//	}
//}
//
//static hashTableTemplateExt_c<cmAnimation_c> cm_animations;
//
//cmAnimation_c *CM_RegisterAnimation(const char *animName) {
//	cmAnimation_c *ret = cm_animations.getEntry(animName);
//	if(ret) {
//		return ret;
//	}
//	ret = new cmAnimation_c;
//	ret->api = g_modelLoader->loadSkelAnimFile(animName);
//	ret->name = animName;
//	cm_animations.addObject(ret);
//	return ret;
//}
//const skelAnimAPI_i *CM_RegisterAnimation_GetAPI(const char *animName) {
//	cmAnimation_c *cmAnim = CM_RegisterAnimation(animName);
//	if(cmAnim && cmAnim->getAPI()) {
//		return cmAnim->getAPI();
//	}
//	return 0;
//}
//void CM_ClearAnims() {
//	for(u32 i = 0; i < cm_animations.size(); i++) {
//		cmAnimation_c *a = cm_animations[i];
//		a->clear();
//		delete a;
//		cm_animations[i] = 0;
//	}
//	cm_animations.clear();
//}
