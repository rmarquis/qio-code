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
// mat_main.cpp - renderer materials managment
#include "mat_local.h"
#include "mat_impl.h"

#include <qcommon/q_shared.h>
#include <api/vfsAPI.h>
#include <api/rbAPI.h>
#include <shared/hashTableTemplate.h>

static hashTableTemplateExt_c<mtrIMPL_c> materials;

void MAT_CacheMatFileText(const char *fname) {
	
}
void MAT_ScanForFiles(const char *path, const char *ext) {
	int numFiles;
	char **fnames = g_vfs->FS_ListFiles(path,ext,&numFiles);
	for(u32 i = 0; i < numFiles; i++) {
		const char *fname = fnames[i];
		MAT_CacheMatFileText(fname);
	}
	g_vfs->FS_FreeFileList(fnames);
}
void MAT_ScanForMaterialFiles() {
	MAT_ScanForFiles("scripts",".shader");
	MAT_ScanForFiles("materials",".mtr");
}

mtrIMPL_c *MAT_RegisterMaterial(const char *matName) {
	mtrIMPL_c *ret = materials.getEntry(matName);
	if(ret) {
		return ret;
	}
	ret = new mtrIMPL_c;
	ret->setName(matName);
	ret->createFromImage();
	materials.addObject(ret);
	return ret;
}
class mtrAPI_i *MAT_RegisterMaterialAPI(const char *matName) {
	return MAT_RegisterMaterial(matName);
}
void MAT_FreeAllMaterials() {
	for(u32 i = 0; i < materials.size(); i++) {
		mtrIMPL_c *m = materials[i];
		delete m;
		materials[i] = 0;
	}
}