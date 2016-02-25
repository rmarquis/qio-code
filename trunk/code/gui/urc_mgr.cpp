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
// urc_mgr.cpp - Ultimate ResourCe manager
#include "urc_mgr.h"
#include <api/vfsAPI.h>
#include <shared/parser.h>

void urcMgr_c::precacheURCFile(const char *fname) {
	parser_c p;
	if(p.openFile(fname))
		return;
	if(!p.atWord("menu"))
		return;
	str name = p.getToken();
	g_core->Print("URC file %s has menu %s\n",fname,name.c_str());
	urcNameMappingCache_c *c = nameCache.getEntry(name);
	if(c == 0) {
		c = new urcNameMappingCache_c();
		c->setName(name);
		nameCache.addObject(c);
	}
	c->setFName(fname);
}
void urcMgr_c::precacheURCFiles() {
	int numFiles;
	char **fnames = g_vfs->FS_ListFiles("ui/","urc",&numFiles);
	for(u32 i = 0; i < numFiles; i++) {
		const char *fname = fnames[i];
		str fullPath = "ui/";
		fullPath.append(fname);
		precacheURCFile(fullPath);
	}
	g_vfs->FS_FreeFileList(fnames);
}

const char *urcMgr_c::getURCFileNameForURCInternalName(const char *internalName) const {
	const urcNameMappingCache_c *c = nameCache.getEntry(internalName);
	if(c == 0) {
		static str tmp;
		tmp = "ui/";
		tmp.append(internalName);
		tmp.append(".urc");
		return tmp;
	}
	return c->getFName();
}
urc_c *urcMgr_c::registerURC(const char *internalName) {
	urc_c *urc = loaded.getEntry(internalName);
	if(urc) {
		return urc;
	}
	urc = new urc_c();
	urc->setName(internalName);
	loaded.addObject(urc);
	// find the .urc file path for internal URC name
	const char *fname = getURCFileNameForURCInternalName(internalName);
	urc->setFName(fname);

	urc->loadURCFile();

	return urc;
}
