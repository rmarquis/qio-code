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
// entDefsList.h - list of entity definitions (keyValues + primitives)
#include "entDefsList.h"
#include "../qcommon/q_shared.h"
#include <api/vfsAPI.h>
#include <api/coreAPI.h>
#include <fileformats/bspFileFormat.h>
#include <shared/parser.h>
#include <shared/str.h>

entDefsList_c::entDefsList_c() {
	
}
entDefsList_c::~entDefsList_c() {
	clear();
}
void entDefsList_c::clear() {
	for(u32 i = 0; i < entities.size(); i++) {
		delete entities[i];
		entities[i] = 0;
	}
	entities.clear();
}
bool entDefsList_c::fromString(const char *text) {
	parser_c p;
	p.setup(text);
	while(p.atEOF() == false) {
		if(p.atChar('{')) {
			entDef_c *ed = new entDef_c;
			this->entities.push_back(ed);
			while(p.atChar('}') == false) {
				if(p.atEOF()) {

					break;
				}
				str key = p.getToken();
				str val = p.getToken();
				ed->setKeyValue(key,val);
			}
		} else {
			p.getToken();
		}
	}
	return false; // OK
}
bool entDefsList_c::load(const char *fname) {
	char buf[256];
	strcpy(buf,"maps/");
	strcat(buf,fname);
	strcat(buf,".bsp");
	fileHandle_t f;
	int len = g_vfs->FS_FOpenFile(buf,&f,FS_READ);
	byte *data = (byte*)malloc(len);
	g_vfs->FS_Read(data,len,f);
	g_vfs->FS_FCloseFile(f);

	const char *text;

	q3Header_s *h = (q3Header_s*)data;
	if(h->ident == BSP_IDENT_IBSP) {
		text = (const char*)h->getLumpData(Q3_ENTITIES);
	} else {
		g_core->Print(S_COLOR_RED"entDefsList_c::load: unknown bsp type\n");
		text = 0;
		free(data);
		return true; // error
	}
	bool error = this->fromString(text);
	free(data);
	return error;
}





