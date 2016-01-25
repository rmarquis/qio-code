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
// etCharMgr.h - Enemy Territory Characters Manager
#ifndef __ETCHARMGR_H__
#define __ETCHARMGR_H__

#include "charFile.h"
#include <shared/hashTableTemplate.h>

class etChar_c : public charFile_c {
friend class etCharMgr_c;
	class etChar_c *hashNext;
	class skelModelAPI_i *skelModel;
	class etAnimGroup_c *pAnimationGroup;
public:
	etChar_c();
	~etChar_c();
	bool loadCharacterData();
	const class skelAnimAPI_i *findSkelAnim(const char *animName) const;

	etChar_c *getHashNext() {
		return hashNext;
	}
	void setHashNext(etChar_c *hn) {
		hashNext = hn;
	}
	skelModelAPI_i *getSkelModel() {
		return skelModel;
	}
	bool isValid() const {
		if(skelModel == 0)
			return false;
		return true;
	}
};
class etAnim_c {
friend class etAnimGroup_c;
	str alias;
	class skelAnimAPI_i *anim;
	etAnim_c *hashNext;
public:
	const char *getName() const {
		return alias;
	}
	etAnim_c *getHashNext() {
		return hashNext;
	}
	void setHashNext(etAnim_c *hn) {
		hashNext = hn;
	}
};
class etAnimGroup_c {
	str name;
	etAnimGroup_c *hashNext;
	hashTableTemplateExt_c<etAnim_c> anims;

	bool parseAninc(const char *mdxName, class parser_c &p);
	bool parseAninc(const char *mdxName, const char *anincName);
public:
	~etAnimGroup_c();

	bool parse(const char *fname);

	const skelAnimAPI_i *findSkelAnim(const char *animName) const;

	void setName(const char *s) {
		name = s;
	}
	const char *getName() const {
		return name;
	}
	etAnimGroup_c *getHashNext() {
		return hashNext;
	}
	void setHashNext(etAnimGroup_c *hn) {
		hashNext = hn;
	}
};

class etCharMgr_c {
	hashTableTemplateExt_c<etAnimGroup_c> animGroups;
	hashTableTemplateExt_c<etChar_c> characters;

	etAnimGroup_c *registerAnimationGroup(const char *fname);
public:
	~etCharMgr_c();

	etChar_c *registerCharacter(const char *fname);
};

#endif // __ETCHARMGR_H__
