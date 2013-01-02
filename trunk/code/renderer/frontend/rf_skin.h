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
// rf_skin.h - used to remap model materials without modifying their binary files
#ifndef __RF_SKIN_H__
#define __RF_SKIN_H__

#include <shared/str.h>
#include <shared/array.h>

enum rSkinType_e {
	SKIN_BAD,
	// that's not a skin file, but a single material name
	SKIN_SINGLEMATERIAL,
	// .skin file - material list
	SKIN_MATLIST, 
};

struct rSkinSurface_s {
	mutable class mtrAPI_i *mat;
	str matName;
	str surfName;

	rSkinSurface_s() {
		mat = 0;
	}
};

class rSkinRemap_c {
	str skinName;
	rSkinType_e type;
	arraySTD_c<rSkinSurface_s> surfSkins;

	rSkinRemap_c *hashNext;
public:
	rSkinRemap_c();

	void setName(const char *newName) {
		skinName = newName;
	}
	const char *getName() const {
		return skinName;
	}
	void setHashNext(rSkinRemap_c *newHashNext) {
		hashNext = newHashNext;
	}
	rSkinRemap_c *getHashNext() {
		return hashNext;
	}

	u32 size() const {
		return surfSkins.size();
	}

	class mtrAPI_i *getMaterial(u32 surfNum) const;
	const char *getMatName(u32 surfNum) const;
	const rSkinSurface_s &getSkinSurf(u32 surfNum) const {
		return surfSkins[surfNum];
	}
	bool isValid() const {
		if(type == SKIN_BAD) {
			return false;
		}
		return true;
	}
	
	bool fromSingleMaterial(const char *matName);
	bool fromSkinFile(const char *fname);
	void addSurfSkin(const char *sfName, const char *matName);

	// used while merging models
	void appendRemap(const rSkinRemap_c *other);
};

rSkinRemap_c *RF_RegisterSkin(const char *skinName);
rSkinRemap_c *RF_RegisterSkinForModel(const char *modelName, const char *skinName);

#endif // __RF_SKIN_H__
