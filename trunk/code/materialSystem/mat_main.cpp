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
#include <api/coreAPI.h>
#include <api/vfsAPI.h>
#include <api/rbAPI.h>
#include <api/imgAPI.h>
#include <shared/hashTableTemplate.h>
#include <shared/autoCmd.h>

struct matFile_s {
	str fname;
	str text;

	bool reloadMaterialSourceText() {
		text.clear();
		char *tmpFileData;
		u32 len = g_vfs->FS_ReadFile(this->fname,(void**)&tmpFileData);
		if(tmpFileData == 0) {
			g_core->RedWarning("matFile_s::reloadMaterialSourceText: failed to reload file %s\n",this->fname.c_str());
			return true; // error
		}
		this->text = tmpFileData;
		g_vfs->FS_FreeFile(tmpFileData);
		return false;
	}
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

matFile_s *MAT_FindMatFileForName(const char *fname) {
	for(u32 i = 0; i < matFiles.size(); i++) {
		matFile_s *mf = matFiles[i];
		if(!stricmp(mf->fname,fname)) {
			return mf;
		}
	}
	return 0;
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
void MAT_LoadMaterial(class mtrIMPL_c *mat) {
	if(mat->getName() == 0 || mat->getName()[0] == 0) {
		// this should never happen
		g_core->RedWarning("MAT_LoadMaterial: material name not set! Cannot reload material.\n");
		return;
	}
	// try to load from material text (.shader/.mtr files)
	matTextDef_s text;
	if(MAT_FindMaterialText(mat->getName(),text)) {
		mat->loadFromText(text);
	} else {
		if(mat->isVMTMaterial()) {
			if(mat->loadFromVMTFile()) {
				mat->createFromImage();
			}
		} else {
			// create material directly from image
			mat->createFromImage();
		}
	}
}
mtrIMPL_c *MAT_RegisterMaterial(const char *inMatName) {
	// strip the image name extension (if any)
	str matName = inMatName;
	const char *ext = matName.getExt();
	// strip non-vmt extensions
	if(ext && stricmp(ext,"vmt")) {
		matName.stripExtension();
	}
	mtrIMPL_c *ret = materials.getEntry(matName);
	if(ret) {
		return ret;
	}
	ret = new mtrIMPL_c;
	// set material name first and add it to hash table
	ret->setName(matName);
	materials.addObject(ret);
	// load material from material text (.shader/.mtr file)
	// or (if material text is not present) directly from texture if
	MAT_LoadMaterial(ret);
	return ret;
}
void MAT_ReloadSingleMaterial_internal(mtrIMPL_c *mat) {
	// free old material data (but dont reset material name)
	mat->clear();
	// load material once again
	MAT_LoadMaterial(mat);
}
// reload entire material source text 
// but recreate only material with given name
// (matName is the name of single material)
void MAT_ReloadSingleMaterial(const char *matName) {
	mtrIMPL_c *mat = materials.getEntry(matName);
	if(mat == 0) {
		g_core->RedWarning("MAT_ReloadMaterial: material %s was not loaded.\n",matName);
		return;
	}
	// see if we have to reload entire .mtr text file
	const char *sourceFileName = mat->getSourceFileName();
	matFile_s *mtrTextFile = MAT_FindMatFileForName(sourceFileName);
	if(mtrTextFile) {
		// reload mtr text file
		mtrTextFile->reloadMaterialSourceText();
	}
	// reparse specified material text / reload its single image
	MAT_ReloadSingleMaterial_internal(mat);
}
// reload entire material source text
// and recreate all of the present materials using it
// (mtrSourceFileName is the name of .shader / .mtr file)
void MAT_ReloadMaterialFileSource(const char *mtrSourceFileName) {
	matFile_s *mtrTextFile = MAT_FindMatFileForName(mtrSourceFileName);
	if(mtrTextFile) {
		g_core->Print("MAT_ReloadMaterialFileSource: refreshing material file %s\n",mtrSourceFileName);
		// reload existing mtr text file
		mtrTextFile->reloadMaterialSourceText();
		// reload materials
		u32 c_reloadedMats = 0;
		for(u32 i = 0; i < materials.size(); i++) {
			mtrIMPL_c *m = materials[i];
			if(!stricmp(m->getSourceFileName(),mtrSourceFileName)) {
				MAT_ReloadSingleMaterial_internal(m);
				c_reloadedMats++;
			}
		}
		g_core->Print("MAT_ReloadMaterialFileSource: reloaded %i materials for source file %s\n",c_reloadedMats,mtrSourceFileName);
	} else if(g_vfs->FS_FileExists(mtrSourceFileName)) {
		g_core->Print("MAT_ReloadMaterialFileSource: loading NEW materials source file %s\n",mtrSourceFileName);
		// add a NEW material file
		MAT_CacheMatFileText(mtrSourceFileName);
		return; // nothing else to do:
	} else {
		g_core->RedWarning("MAT_ReloadMaterialFileSource: %s does not exist\n",mtrSourceFileName);
		return;
	}
}
class mtrAPI_i *MAT_CreateHLBSPTexture(const char *newMatName, const byte *pixels, u32 width, u32 height, const byte *palette) {
	mtrIMPL_c *ret = materials.getEntry(newMatName);
	if(ret == 0) {
		ret = new mtrIMPL_c;
		// set material name first and add it to hash table
		ret->setName(newMatName);
		materials.addObject(ret);
	} else {
		// just clear the existing material
		ret->clear();
	}
	byte *converted = 0;
	u32 convertedW, convertedH;
	g_img->convert8BitImageToRGBA32(&converted,&convertedW,&convertedH,pixels,width,height,palette);
	textureAPI_i *tex = MAT_CreateTexture(newMatName,converted,convertedW,convertedH);
	g_img->freeImageData(converted);
	ret->createFromTexturePointer(tex);
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

static void MAT_RefreshSingleMaterial_f() {
	if(g_core->Argc() < 2) {
		g_core->Print("usage: \"mat_refreshSingleMaterial <matName>\"\n");
		return;
	}
	const char *matName = g_core->Argv(1);
	MAT_ReloadSingleMaterial(matName);
}
static void MAT_RefreshMaterialSourceFile_f() {
	if(g_core->Argc() < 2) {
		g_core->Print("usage: \"mat_refreshMaterialSourceFile <mtrFileName>\"\n");
		return;
	}
	const char *mtrFileName = g_core->Argv(1);
	MAT_ReloadMaterialFileSource(mtrFileName);
}
static aCmd_c mat_refreshSingleMaterial_f("mat_refreshSingleMaterial",MAT_RefreshSingleMaterial_f);
static aCmd_c mat_refreshMaterialSourceFile_f("mat_refreshMaterialSourceFile",MAT_RefreshMaterialSourceFile_f);
