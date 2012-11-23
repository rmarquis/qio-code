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
#include <api/imgAPI.h>
#include <shared/hashTableTemplate.h>

struct matFile_s {
	str fname;
	str text;
};

static hashTableTemplateExt_c<mtrIMPL_c> materials;
static arraySTD_c<matFile_s*> matFiles;

void MAT_CacheMatFileText(const char *fname) {
	char *data;
	u32 len = g_vfs->FS_ReadFile(fname,(void**)&data);
	if(data == 0)
		return;
	matFile_s *mf = new matFile_s;
	mf->fname = fname;
	mf->text = data;
	g_vfs->FS_FreeFile(data);
	matFiles.push_back(mf);
}
const char *MAT_FindMaterialDefInText(const char *matName, const char *text) {
	u32 matNameLen = strlen(matName);
	const char *p = text;
	while(*p) {
		if(!Q_stricmpn(p,matName,matNameLen) && G_isWS(p[matNameLen])) {
			const char *matNameStart = p;
			p += matNameLen;
			p = G_SkipToNextToken(p);
			if(*p != '{') {
				continue;
			}
			const char *brace = p;
			p++;
			G_SkipToNextToken(p);
			// now we're sure that 'p' is at valid material text,
			// so we can start parsing
			return brace;
		}
		p++;
	}
	return 0;
}

bool MAT_FindMaterialText(const char *matName, matTextDef_s &out) {
	for(u32 i = 0; i < matFiles.size(); i++) {
		matFile_s *mf = matFiles[i];
		const char *p = MAT_FindMaterialDefInText(matName,mf->text);
		if(p) {
			out.p = p;
			out.textBase = mf->text;
			out.sourceFile = mf->fname;
			return true;
		}
	}
	return false;
}

void MAT_ScanForFiles(const char *path, const char *ext) {
	int numFiles;
	char **fnames = g_vfs->FS_ListFiles(path,ext,&numFiles);
	for(u32 i = 0; i < numFiles; i++) {
		const char *fname = fnames[i];
		str fullPath = path;
		fullPath.append(fname);
		MAT_CacheMatFileText(fullPath);
	}
	g_vfs->FS_FreeFileList(fnames);
}
void MAT_ScanForMaterialFiles() {
	MAT_ScanForFiles("scripts/",".shader");
	MAT_ScanForFiles("materials/",".mtr");
}
mtrIMPL_c *MAT_RegisterMaterial(const char *matName) {
	mtrIMPL_c *ret = materials.getEntry(matName);
	if(ret) {
		return ret;
	}
	ret = new mtrIMPL_c;
	ret->setName(matName);
	materials.addObject(ret);

	// try to load from material text (.shader/.mtr files)
	matTextDef_s text;
	if(MAT_FindMaterialText(matName,text)) {
		ret->loadFromText(text);
		return ret;
	}
	// create material directly from image
	ret->createFromImage();
	return ret;
}
class mtrAPI_i *MAT_RegisterMaterialAPI(const char *matName) {
	return MAT_RegisterMaterial(matName);
}
bool MAT_IsMaterialOrImagePresent(const char *matName) {
	// try to load from material text (.shader/.mtr files)
	matTextDef_s text;
	if(MAT_FindMaterialText(matName,text)) {
		return true; // OK, found
	}
	// see if the image with such name (or similiar, extension can be different!) exists
	byte *data = 0;
	u32 w, h;
	const char *fixedPath = g_img->loadImage(matName,&data,&w,&h);
	if(data == 0) {
		return false;
	}
	g_img->freeImageData(data);
	return true; // OK, texture image exists
}
void MAT_FreeAllMaterials() {
	for(u32 i = 0; i < materials.size(); i++) {
		mtrIMPL_c *m = materials[i];
		delete m;
		materials[i] = 0;
	}
}
