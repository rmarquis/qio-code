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
// declManagerIMPL.cpp
#include <qcommon/q_shared.h>
#include "declManagerIMPL.h"
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/modelDeclAPI.h>
#include <api/modelLoaderDLLAPI.h>
#include <api/skelModelAPI.h>
#include <api/entityDeclAPI.h>
#include <shared/parser.h>
#include <shared/ePairsList.h>
#include <shared/entDef.h>

struct declTextDef_s {
	const char *sourceFile; // name of the source .def file
	const char *p; // pointer to the first '{' (the one after material name)
	const char *textBase; // pointer to the start of cached material file text

	void clear() {
		sourceFile = 0;
		p = 0;
		textBase = 0;
	}
	declTextDef_s() {
		clear();
	}
};

class animEvent_c {
friend class modelDecl_c;
	int frameNum;
	str text;
};
class animDef_c : public animDefAPI_i {
friend class modelDecl_c;
	str animAlias;
	str animFile;
	class skelAnimAPI_i *anim;
	arraySTD_c<animEvent_c> events; 
public:
	animDef_c() {
		anim = 0;
	}

	// animDefAPI_i impl
	virtual const char *getAlias() const {
		return animAlias;
	}
	virtual const char *getAnimFileName() const {
		return animFile;
	}
	virtual class skelAnimAPI_i *getAnim() const {
		return anim;
	}
};
class declRefState_c {
	bool referencedByClient;
	bool referencedByServer;
public:
	declRefState_c() {
		referencedByClient = false;
		referencedByServer = false;
	}
	void setReferencedByClient() {
		referencedByClient = true;
	}
	void setReferencedByServer() {
		referencedByServer = true;
	}
	void clearServerRef() {
		referencedByServer = false;
	}
	void clearClientRef() {
		referencedByClient = false;
	}
	bool isReferenced() const {
		if(referencedByClient)
			return true;
		if(referencedByServer)
			return true;
		return false;
	}
};
class modelDecl_c : public modelDeclAPI_i, public declRefState_c {
	str declName;
	str meshName;
	vec3_c offset;
	arraySTD_c<animDef_c> anims;
	modelDecl_c *hashNext;
	bool cached;
	class skelModelAPI_i *skelModel;
public:
	modelDecl_c() {
		cached = false;
		skelModel = 0;
	}
	void precache() {
		if(cached)
			return;
		skelModel = g_modelLoader->loadSkelModelFile(meshName);
	}
	bool parse(const char *text, const char *textBase, const char *fname) {
		parser_c p;
		p.setup(textBase, text);
		p.setDebugFileName(fname);
		while(p.atWord_dontNeedWS("}") == false) {
			if(p.atEOF()) {
				g_core->RedWarning("modelDecl_c::parse: unexpected EOF hit while parsing model %s declaration in file %s\n",
					getName(),fname);
				break;
			}
			if(p.atWord("mesh")) {
				meshName = p.getToken();
			} else if(p.atWord("offset")) {
				p.getFloatMat_braced(offset,3);
			} else if(p.atWord("channel")) {
				p.skipLine();
			} else if(p.atWord("anim")) {
				animDef_c &newAD = anims.pushBack();
				newAD.animAlias = p.getToken();
				newAD.animFile = p.getToken();
				// sometimes there is no space between "smth/anim.md5anim" and '{' ...
				if(newAD.animFile.isLastChar('{') || p.atWord("{")) {
					newAD.animFile.stripTrailing("{");
					while(p.atWord("}") == false) {
						if(p.atWord("frame")) {
							animEvent_c &ev = newAD.events.pushBack();
							ev.frameNum = p.getInteger();
							ev.text = p.getLine();
						} else if(p.atWord("rate")) {
							float rate = p.getFloat();
						} else {
							p.skipLine();
						}
					}
				}
			} else {
				int line = p.getCurrentLineNumber();
				const char *unk = p.getToken();
				g_core->RedWarning("modelDecl_c::parse: unknown token %s at line %i of %s\n",unk,line,fname);
			}
		}
		return false; // no error
	}
	bool isValid() const {
		if(meshName.length() == 0)
			return false;
		return true;
	}
	void setDeclName(const char *newName) {
		declName = newName;
	}
	const char *getName() const {
		return declName;
	}
	modelDecl_c *getHashNext() {
		return hashNext;
	}
	void setHashNext(modelDecl_c *newHashNext) {
		hashNext = newHashNext;
	}

	// modelDeclAPI_i impl
	const char *getModelDeclName() {
		return declName;
	}
	const char *getMeshName() {
		return meshName;
	}
	class skelModelAPI_i *getSkelModel() {
		precache();
		return skelModel;
	}
	u32 getNumSurfaces() {
		precache();
		if(skelModel == 0)
			return 0;
		return skelModel->getNumSurfs();
	}
};
class entityDecl_c : public entityDeclAPI_i, public declRefState_c  {
	str declName;
	entDef_c entDef;
	entityDecl_c *hashNext;

	virtual const char *getDeclName() const {
		return declName;
	}

	virtual const class entDefAPI_i *getEntDefAPI() const {
		return &entDef;
	}
public:	
	bool parse(const char *text, const char *textBase, const char *fname) {
		parser_c p;
		p.setup(textBase, text);
		p.setDebugFileName(fname);
		while(p.atWord_dontNeedWS("}") == false) {
			if(p.atEOF()) {
				g_core->RedWarning("entityDecl_c::parse: unexpected EOF hit while parsing entityDef %s declaration in file %s\n",
					getDeclName(),fname);
				break;
			}
			str key = p.getToken();
			str val = p.getToken();
			entDef.setKeyValue(key,val);
		}
		return false;
	}
	bool isValid() const {
		if(entDef.getNumKeyValues())
			return true;
		return false;
	}	
	void setDeclName(const char *newName) {
		declName = newName;
	}
	const char *getName() const {
		return declName;
	}
	entityDecl_c *getHashNext() {
		return hashNext;
	}
	void setHashNext(entityDecl_c *newHashNext) {
		hashNext = newHashNext;
	}
};

const char *declManagerIMPL_c::findDeclInText(const char *declName, const char *declType, const char *text) {
	u32 declNameLen = strlen(declName);
	u32 declTypeLen = strlen(declType);
	const char *p = text;
	while(*p) {
		if(!Q_stricmpn(p,declName,declNameLen) && G_isWS(p[declNameLen])) {
			// check the decl type
			const char *declTypeStringEnd = p;
			while(G_isWS(*declTypeStringEnd)) {
				if(declTypeStringEnd == text) {
					declTypeStringEnd = 0;
					break;
				}
				declTypeStringEnd--;
			}
			if(declTypeStringEnd == 0)
				continue;
			const char *declTypeStringStart = declTypeStringEnd - declTypeLen - 1;
			if(declTypeStringStart < text)
				continue;
			if(Q_stricmpn(declTypeStringStart,declType,declTypeLen))
				continue; // decl type didnt match
			const char *declNameStart = p;
			p += declNameLen;
			p = G_SkipToNextToken(p);
			if(*p != '{') {
				continue;
			}
			const char *brace = p;
			p++;
			G_SkipToNextToken(p);
			// now we're sure that 'p' is at valid material text,
			// so we can start parsing
			return brace+1;
		}
		p++;
	}
	return 0;
}

bool declManagerIMPL_c::findDeclText(const char *declName, const char *declType, declTextDef_s &out) {
	for(u32 i = 0; i < defFiles.size(); i++) {
		defFile_s *mf = defFiles[i];
		const char *p = findDeclInText(declName,declType,mf->text);
		if(p) {
			out.p = p;
			out.textBase = mf->text;
			out.sourceFile = mf->fname;
			return true;
		}
	}
	return false;
}

class modelDeclAPI_i *declManagerIMPL_c::_registerModelDecl(const char *name, qioModule_e userModule) {
	modelDecl_c *ret = modelDecls.getEntry(name);
	if(ret) {
		if(QM_IsServerSide(userModule) == false) {
			ret->setReferencedByClient();
		} else {
			ret->setReferencedByServer();
		}
		if(ret->isValid())
			return ret;
		return 0;
	}
	declTextDef_s txt;
	ret = new modelDecl_c;
	ret->setDeclName(name);
	modelDecls.addObject(ret);
	if(findDeclText(name,"model",txt) == false) {
		return 0;
	}
	ret->parse(txt.p,txt.textBase,txt.sourceFile);
	if(QM_IsServerSide(userModule) == false) {
		ret->setReferencedByClient();
	} else {
		ret->setReferencedByServer();
	}
	if(ret->isValid())
		return ret;
	return 0;
}
class entityDeclAPI_i *declManagerIMPL_c::_registerEntityDecl(const char *name, qioModule_e userModule) {
	entityDecl_c *ret = entityDecls.getEntry(name);
	if(ret) {
		if(QM_IsServerSide(userModule) == false) {
			ret->setReferencedByClient();
		} else {
			ret->setReferencedByServer();
		}
		if(ret->isValid())
			return ret;
		return 0;
	}
	declTextDef_s txt;
	ret = new entityDecl_c;
	ret->setDeclName(name);
	entityDecls.addObject(ret);
	if(findDeclText(name,"entityDef",txt) == false) {
		return 0;
	}
	ret->parse(txt.p,txt.textBase,txt.sourceFile);
	if(QM_IsServerSide(userModule) == false) {
		ret->setReferencedByClient();
	} else {
		ret->setReferencedByServer();
	}
	if(ret->isValid())
		return ret;
	return 0;
}
void declManagerIMPL_c::removeUnrefrencedDecls() {
	for(int i = 0; i < entityDecls.size(); i++) {
		entityDecl_c *ed = entityDecls[i];
		if(ed->isReferenced() == false) {
			entityDecls.removeEntry(ed);
			i--;
			delete ed;
		}
	}
	for(int i = 0; i < modelDecls.size(); i++) {
		modelDecl_c *md = modelDecls[i];
		if(md->isReferenced() == false) {
			modelDecls.removeEntry(md);
			i--;
			delete md;
		}
	}
}
void declManagerIMPL_c::onGameShutdown() {
	for(u32 i = 0; i < entityDecls.size(); i++) {
		entityDecl_c *ed = entityDecls[i];
		ed->clearServerRef();
	}
	for(u32 i = 0; i < modelDecls.size(); i++) {
		modelDecl_c *md = modelDecls[i];
		md->clearServerRef();
	}
	removeUnrefrencedDecls();
}
void declManagerIMPL_c::onRendererShutdown() {
	for(u32 i = 0; i < entityDecls.size(); i++) {
		entityDecl_c *ed = entityDecls[i];
		ed->clearClientRef();
	}
	for(u32 i = 0; i < modelDecls.size(); i++) {
		modelDecl_c *md = modelDecls[i];
		md->clearClientRef();
	}	
	removeUnrefrencedDecls();
}
void declManagerIMPL_c::cacheDefFileText(const char *fname) {
	char *data;
	u32 len = g_vfs->FS_ReadFile(fname,(void**)&data);
	if(data == 0)
		return;
	totalDefBytes += len;
	g_core->Print("Caching .def file: %s... - %i bytes\n",fname,len);
	defFile_s *df = new defFile_s;
	df->fname = fname;
	df->text = data;
	g_vfs->FS_FreeFile(data);
	defFiles.push_back(df);
}
void declManagerIMPL_c::init() {
	g_core->Print("----- Initializing Decls -----\n");
	int numFiles;
	totalDefBytes = 0;
	char **fnames = g_vfs->FS_ListFiles("def/",".def",&numFiles);
	for(u32 i = 0; i < numFiles; i++) {
		const char *fname = fnames[i];
		str fullPath = "def/";
		fullPath.append(fname);
		cacheDefFileText(fullPath);
	}
	g_core->Print("----- Total %i decl bytes in %i def files -----\n",totalDefBytes,numFiles);
	g_vfs->FS_FreeFileList(fnames);
#if 0
	registerModelDecl("monster_qwizard");

#endif
}

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
modelLoaderDLLAPI_i *g_modelLoader = 0;
// exports
static declManagerIMPL_c g_staticDeclMgr;
declManagerAPI_i *g_declMgr = &g_staticDeclMgr;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)g_declMgr,DECL_MANAGER_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_modelLoader,MODELLOADERDLL_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_DECL_MANAGER;
}

