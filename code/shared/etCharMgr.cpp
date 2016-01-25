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
// etCharMgr.cpp - Enemy Territory Characters Manager
#include "etCharMgr.h"
#include <api/modelLoaderDLLAPI.h>
#include <api/skelModelAPI.h>
#include <api/skelAnimAPI.h>
#include <shared/parser.h>

bool etAnimGroup_c::parseAninc(const char *mdxName, const char *anincName) {
	parser_c p;
	if(p.openFile(anincName)) {
		g_core->RedWarning("etAnimGroup_c::parseAninc: failed to open %s\n",anincName);
		return true;
	}
	if(parseAninc(mdxName,p))
		return true;
	return false;
}
bool etAnimGroup_c::parseAninc(const char *mdxName, class parser_c &p) {
	skelAnimAPI_i *mdx = g_modelLoader->loadSkelAnimFile(mdxName);
	if(mdx == 0) {
	//	return true; 
	}
	while(p.atEOF() == false) {
		if(p.atWord("}"))
			break; // V: not used now... but it's logical if the #include was to be skipped?
		str alias = p.getToken();
		u32 firstFrame = p.getInteger();
		u32 len = p.getInteger();
		u32 looping = p.getInteger();
		float fps = p.getFloat();
		float moveSpeed = p.getFloat();
		u32 transition = p.getInteger();
		u32 reversed = p.getInteger();

		if(mdx) {
			skelAnimAPI_i *subAnim = mdx->createSubAnim(firstFrame,len);
			etAnim_c *a = new etAnim_c;
			a->alias = alias;
			a->anim = subAnim;
			this->anims.addObject(a);
		}
	}
	if(mdx) {
		delete mdx;
	}
	return false;
}
const skelAnimAPI_i *etAnimGroup_c::findSkelAnim(const char *animName) const {
	const etAnim_c *a = anims.getEntry(animName);
	if(a == 0)
		return 0;
	return a->anim;
}
bool etAnimGroup_c::parse(const char *fname) {
	parser_c p;
	if(p.openFile(fname)) {
		return true;
	}
	if(p.atWord("animGroup") == false) {
		g_core->RedWarning("etAnimGroup_c::parse: expected 'animGroup', found %s in %s\n",p.getToken(),fname);
		return true;
	}
	if(p.atWord("{") == false) {
		g_core->RedWarning("etAnimGroup_c::parse: expected '{', found %s in %s\n",p.getToken(),fname);
		return true;
	}
	while(p.atWord("}") == false) {
		if(p.atWord("animFile")) {
			str mdxFile = p.getToken();
			if(p.atWord("{") == false) {
				g_core->RedWarning("etAnimGroup_c::parse: expected '{', found %s in %s\n",p.getToken(),fname);
				return true;
			}
			if(p.atWord("#include") == false) {
				g_core->RedWarning("etAnimGroup_c::parse: expected '{', found %s in %s\n",p.getToken(),fname);
				return true;
			}
			str anincFile = p.getToken();
			if(p.atWord("}") == false) {
				g_core->RedWarning("etAnimGroup_c::parse: expected '{', found %s in %s\n",p.getToken(),fname);
				return true;
			}
			g_core->Print("MDX file %s, aninc file %s\n",mdxFile.c_str(),anincFile.c_str());

			parseAninc(mdxFile,anincFile);

		} else {
			g_core->RedWarning("etAnimGroup_c::parse: unknown token %s found in %s\n",p.getToken(),fname);
		}
	}
	return false;
}


etChar_c::etChar_c() {
	skelModel = 0;
}
etChar_c::~etChar_c() {
	delete skelModel;
}
const skelAnimAPI_i *etChar_c::findSkelAnim(const char *animName) const {
	if(pAnimationGroup == 0)
		return 0;
	return pAnimationGroup->findSkelAnim(animName);
}
bool etChar_c::loadCharacterData() {
	skelModel = g_modelLoader->loadSkelModelFile(mesh);

	return false;
}


etAnimGroup_c *etCharMgr_c::registerAnimationGroup(const char *fname) {
	etAnimGroup_c *a = animGroups.getEntry(fname);
	if(a)
		return a;
	a = new etAnimGroup_c();
	a->setName(fname);
	animGroups.addObject(a);
	if(a->parse(fname)) {
		return 0;
	}
	return a;
}
etChar_c *etCharMgr_c::registerCharacter(const char *fname) {
	etChar_c *c = characters.getEntry(fname);
	if(c) {
		if(c->isValid()) {
			return c;
		}
		return 0;
	}
	c = new etChar_c();
	c->setName(fname);
	characters.addObject(c);
	if(c->parseCharFile(fname)) {
		return 0; // error
	}
	if(c->loadCharacterData()) {
		return 0;
	}
	c->pAnimationGroup = registerAnimationGroup(c->getAnimationGroupName());
	return c;
}

