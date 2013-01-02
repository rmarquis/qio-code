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
// rf_skin.cpp - used to remap model materials without modifying their binary files
#include "rf_skin.h"
#include <api/materialSystemAPI.h>
#include <shared/parser.h>
#include <shared/hashTableTemplate.h>

static hashTableTemplateExt_c<rSkinRemap_c> rf_skins;

rSkinRemap_c::rSkinRemap_c() {
	type = SKIN_BAD;
}
class mtrAPI_i *rSkinRemap_c::getMaterial(u32 surfNum) const {
	const rSkinSurface_s &sf = surfSkins[surfNum];
	if(sf.mat == 0) {
		sf.mat = g_ms->registerMaterial(sf.matName);
	}
	return sf.mat;
}
const char *rSkinRemap_c::getMatName(u32 surfNum) const const {
	return surfSkins[surfNum].matName;
}
bool rSkinRemap_c::fromSingleMaterial(const char *matName) {
	surfSkins.resize(1);
	surfSkins[0].matName = matName;
	surfSkins[0].surfName = "*all";
	this->type = SKIN_SINGLEMATERIAL;
	return false; // no error
}
void rSkinRemap_c::addSurfSkin(const char *sfName, const char *matName) {
	rSkinSurface_s &sf = surfSkins.pushBack();
	sf.mat = 0;
	sf.surfName = sfName;
	sf.matName = matName;
}
bool rSkinRemap_c::fromSkinFile(const char *fname) {
	parser_c p;
	if(p.openFile(fname)) {
		return true; // error
	}
	while(p.atEOF() == false) {
		str tok = p.getD3Token();
		if(!Q_stricmpn(tok,"tag_",4)) {
			continue;
		}
		str mat = p.getD3Token();
		addSurfSkin(tok,mat);
	}
	this->type = SKIN_MATLIST;
	return false; // no error
}

rSkinRemap_c *RF_RegisterSkin(const char *skinName) {
	rSkinRemap_c *ret = rf_skins.getEntry(skinName);
	if(ret) {
		if(ret->isValid()) {
			return ret;
		} else {
			return 0;
		}
	}
	ret = new rSkinRemap_c;
	ret->setName(skinName);
	rf_skins.addObject(ret);
	const char *ext = G_strgetExt(skinName);
	bool error;
	if(ext == 0) {
		error = ret->fromSingleMaterial(skinName);
	} else {
		error = ret->fromSkinFile(skinName);
	}
	if(error) {
		return 0;
	}
	return ret;
}
rSkinRemap_c *RF_RegisterSkinForModel(const char *modelName, const char *skinName) {
	str path = modelName;
	path.stripExtension();
	path.append("_");
	path.append(skinName);
	path.append(".skin");
	return RF_RegisterSkin(path);
}





