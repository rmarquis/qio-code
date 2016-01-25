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
// charFile.h - Enemy Territory .char file class
#ifndef __SHARED_CHARFILE_H__
#define __SHARED_CHARFILE_H__

#include <shared/str.h>

class charFile_c {
protected:
	str fileName;
	str skin;
	str mesh;
	str animationGroup;
	str animationScript;
	str undressedCorpseModel;
	str undressedCorpseSkin;
	str hudHead;
	str hudHeadAnims;
	str hudHeadSkin;
public:

	void setName(const char *s) {
		fileName = s;
	}
	const char *getName() const {
		return fileName;
	}
	bool parseCharFile(const char *fname);

	const char *getAnimationGroupName() const {
		return animationGroup.c_str();
	}
	bool hasDefaultSkinName() const {
		if(skin.size())
			return true;
		return false;
	}
	const char *getDefaultSkinName() const {
		return skin;
	}

};

#endif // __SHARED_CHARFILE_H__

