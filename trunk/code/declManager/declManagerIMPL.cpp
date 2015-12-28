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
#include <api/skelAnimAPI.h>
#include <api/entityDeclAPI.h>
#include <shared/parser.h>
#include <shared/ePairsList.h>
#include <shared/entDef.h>
#include <shared/autoCvar.h>
#include "q3PlayerModelDecl.h"
#include "articulatedFigure.h"
#include "particleDecl.h"

static aCvar_c decl_entity_printRegisterCalls("decl_entity_printRegisterCalls","0");

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
	animDef_c(const animDef_c &other) {
		this->animAlias = other.animAlias;
		this->animFile = other.animFile;
		this->events = other.events;
		this->anim = 0;
	}
	animDef_c &operator =(const animDef_c &other) {
		this->animAlias = other.animAlias;
		this->animFile = other.animFile;
		this->events = other.events;
		this->anim = 0;
		return *this;
	}

	~animDef_c() {
		if(anim) {
			delete anim;
		}
	}

	void precacheAnim() {
		if(anim)
			return;
		anim = g_modelLoader->loadSkelAnimFile(animFile);
	}

	int getTotalTimeMSec() const {
		if(anim == 0)
			return 0;
		return anim->getTotalTimeSec() * 1000.f;
	}

	// animDefAPI_i impl
	virtual const char *getAlias() const {
		return animAlias;
	}
	virtual const char *getAnimFileName() const {
		return animFile;
	}
	virtual class skelAnimAPI_i *getAnim() const {
		((animDef_c*)this)->precacheAnim();
		return anim;
	}
};
class modelDecl_c : public modelDeclAPI_i, public declRefState_c {
	str declName;
	str meshName;
	vec3_c offset;
	arraySTD_c<animDef_c> anims;
	modelDecl_c *hashNext;
	bool cached;

	class q3PlayerModelAPI_i *q3PlayerModel;
	class skelModelAPI_i *skelModel;
public:
	modelDecl_c() {
		cached = false;
		skelModel = 0;
		q3PlayerModel = 0;
	}
	~modelDecl_c() {
		if(skelModel) {
			delete skelModel;
		}
	}
	void precache() {
		if(cached)
			return;
		cached = true;
		if(meshName.length()) {
			if(g_modelLoader->isSkelModelFile(meshName)) {
				// that's a .md5mesh skeletal model
				skelModel = g_modelLoader->loadSkelModelFile(meshName);
			} else if(g_modelLoader->isStaticModelFile(meshName)) {
				// that's a .lwo / .ase static model
				// TODO
			}
		}
	}
	void setMeshName(const char *newMeshName) {
		meshName = newMeshName;
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
			} else if(p.atWord("inherit")) {
				const char *s = p.getToken();
				class modelDeclAPI_i *inheritAPI = g_declMgr->registerModelDecl(s);
				if(inheritAPI) {
					this->setMeshName(inheritAPI->getMeshName());
					this->offset = inheritAPI->getOffset();
					// hack, we need to append animations
					modelDecl_c *inherit = dynamic_cast<modelDecl_c*>(inheritAPI);
					this->anims.addArray(inherit->anims);
				} else {
					int line = p.getCurrentLineNumber();
					g_core->RedWarning("modelDecl_c::parse: failed to inherit from %s, check line %i of %s\n",s,line,fname);
				}
			} else {
				int line = p.getCurrentLineNumber();
				const char *unk = p.getToken();
				g_core->RedWarning("modelDecl_c::parse: unknown token %s at line %i of %s\n",unk,line,fname);
			}
		}
		return false; // no error
	}
	void setQ3PlayerModel(q3PlayerModelAPI_i *newQ3PModel) {
		this->q3PlayerModel = newQ3PModel;
	}
	bool isValid() const {
		if(meshName.length() == 0) {
			if(this->q3PlayerModel == 0) {
				return false;
			}
		}
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
	virtual const char *getModelDeclName() const {
		return declName;
	}
	virtual const char *getMeshName() const {
		return meshName;
	}
	virtual class skelModelAPI_i *getSkelModel() {
		precache();
		return skelModel;
	}
	virtual const vec3_c &getOffset() const {
		return offset;
	}
	virtual u32 getNumSurfaces() {
		precache();
		if(skelModel) {
			return skelModel->getNumSurfs();
		}
		if(q3PlayerModel) {
			return q3PlayerModel->getNumTotalSurfaces();
		}
		return 0;
	}
	virtual u32 getTotalTriangleCount() {
		precache();
		if(skelModel) {
			return skelModel->getTotalTriangleCount();
		}
		if(q3PlayerModel) {
			return q3PlayerModel->getTotalTriangleCount();
		}
		return 0;
	}
	virtual u32 getNumBones() {
		precache();
		if(skelModel) {
			return skelModel->getNumBones();
		}
		if(q3PlayerModel) {
			return 0;//q3PlayerModel->get();
		}
		return 0;
	}
	virtual int getBoneNumForName(const char *boneName) const {
		(((modelDecl_c*)this)->precache());
		if(skelModel) {
			return skelModel->getLocalBoneIndexForBoneName(boneName);
		}
		if(q3PlayerModel) {
			return q3PlayerModel->getTagNumForName(boneName);
		}
		return -1;
	}
	virtual const char *getBoneName(u32 boneIndex) const {
		if(skelModel == 0)
			return "bad_bone_index";
		return skelModel->getBoneName(boneIndex);
	}
	virtual u32 getNumAnims() const {
		return anims.size();
	}
	const animDef_c *findAnimDef(const char *alias) const {
		for(u32 i = 0; i < anims.size(); i++) {
			if(!_stricmp(anims[i].animAlias,alias)) 
				return &anims[i];
		}
		return 0;
	}
	virtual int getAnimIndexForAnimAlias(const char *alias) const {
		for(u32 i = 0; i < anims.size(); i++) {
			if(!_stricmp(anims[i].animAlias,alias)) 
				return i;
		}
		return -1;
	}
	virtual const class skelAnimAPI_i *getSkelAnimAPIForAlias(const char *alias) const {
		const animDef_c *ad = findAnimDef(alias);
		if(ad == 0)
			return 0;
		return ad->getAnim();
	}
	virtual const class skelAnimAPI_i *getSkelAnimAPIForLocalIndex(int localIndex) const {
		if(localIndex < 0)
			return 0;
		if(localIndex >= anims.size())
			return 0;
		return anims[localIndex].getAnim();
	}
	virtual bool hasAnim(const char *animName) const {
		if(getSkelAnimAPIForAlias(animName))
			return true;
		return false;
	}
	virtual int getAnimationTimeMSec(const char *alias) const {
		const animDef_c *ad = findAnimDef(alias);
		if(ad == 0)
			return 0;
		((animDef_c *)ad)->precacheAnim();
		return ad->getTotalTimeMSec();
	}
	// debug output
	virtual void printBoneNames() const {
		if(skelModel) {
			return skelModel->printBoneNames();
		}
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
	entityDecl_c() {
		hashNext = 0;
	}
	bool parse(const char *text, const char *textBase, const char *fname) {
		parser_c p;
		p.setup(textBase, text);
		p.setDebugFileName(fname);
		str inherit;
		while(p.atWord_dontNeedWS("}") == false) {
			if(p.atEOF()) {
				g_core->RedWarning("entityDecl_c::parse: unexpected EOF hit while parsing entityDef %s declaration in file %s\n",
					getDeclName(),fname);
				break;
			}
			str key = p.getToken();
			str val = p.getToken();
			if(!_stricmp(key,"inherit")) {
				inherit = val;
			} else {
				entDef.setKeyValue(key,val);
			}
		}
		if(inherit.length()) {
			entityDeclAPI_i *inheritDecl = g_declMgr->registerEntityDecl(inherit);
			if(inheritDecl == 0 || (inheritDecl->getEntDefAPI() == 0)) {
				g_core->RedWarning("entityDecl_c::parse: cannot find entityDef \"%s\" for inherit keyword of \"%s\"\n",
					inherit.c_str(),this->declName.c_str());
			} else {
				entDef_c tmp = this->entDef;
				this->entDef.fromOtherAPI(inheritDecl->getEntDefAPI());
				this->entDef.appendOtherAPI_overwrite(&tmp);
			}
		}
		return false;
	}
	bool isValid() const {
		if(entDef.getNumKeyValues())
			return true;
		if(entDef.hasClassName())
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

const char *fileTextDataCache_c::findDeclInText(const char *declName, const char *declType, const char *text) {
	u32 declNameLen = strlen(declName);
	u32 declTypeLen = strlen(declType);
	const char *p = text;
	while(*p) {
		// the opening brace might be directly after decl name (without any whitespaces)
		// eg: "entityDef viewStyle_pistol{"  ...
		if(!Q_stricmpn(p,declName,declNameLen) && (G_isWS(p[declNameLen]) || p[declNameLen] == '{')) {
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
			if(declTypeStringStart < text) {
				p++;
				continue;
			}
			if(Q_stricmpn(declTypeStringStart,declType,declTypeLen)) {
				p++;
				continue; // decl type didnt match
			}
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

bool fileTextDataCache_c::findDeclText(const char *declName, const char *declType, declTextDef_s &out) {
	if(declName == 0 || declName[0] == 0)
		return false;
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
	if(name == 0 || name[0] == 0) {
		g_core->RedWarning("declManagerIMPL_c::_registerModelDecl: NULL name\n");
		return 0;
	}
	if(name[0] == '*') {
		g_core->RedWarning("declManagerIMPL_c::_registerModelDecl: ignoring inline model \"%s\" request.\n",name);
		return 0;
	}
	modelDecl_c *ret = modelDecls.getEntry(name);
	if(ret) {
		ret->setReferencedByModule(userModule);
		if(ret->isValid()) {
			return ret;
		}
		return 0;
	}
	declTextDef_s txt;
	ret = new modelDecl_c;
	ret->setDeclName(name);
	modelDecls.addObject(ret);
	if(name[0] == '$') {
		q3PlayerModelAPI_i *q3PlayerModelDecl = this->_registerQ3PlayerDecl(name,userModule);
		ret->setQ3PlayerModel(q3PlayerModelDecl);
	} else {
		if(defFiles.findDeclText(name,"model",txt) == false) {
			return 0;
		}
		ret->parse(txt.p,txt.textBase,txt.sourceFile);
	}
	ret->setReferencedByModule(userModule);
	if(ret->isValid()) {
		return ret;
	}
	return 0;
}
class entityDeclAPI_i *declManagerIMPL_c::_registerEntityDecl(const char *name, qioModule_e userModule) {
	if(name == 0 || name[0] == 0) {
		g_core->RedWarning("declManagerIMPL_c::_registerEntityDecl: NULL name\n");
		return 0;
	}
	if(decl_entity_printRegisterCalls.getInt()) {
		g_core->Print("declManagerIMPL_c::_registerEntityDecl: registering %s\n",name);
	}
	entityDecl_c *ret = entityDecls.getEntry(name);
	if(ret) {
		ret->setReferencedByModule(userModule);
		if(ret->isValid()) {
			return ret;
		}
		return 0;
	}
	declTextDef_s txt;
	ret = new entityDecl_c;
	ret->setDeclName(name);
	entityDecls.addObject(ret);
	if(defFiles.findDeclText(name,"entityDef",txt) == false) {
		return 0;
	}
	ret->parse(txt.p,txt.textBase,txt.sourceFile);
	ret->setReferencedByModule(userModule);
	if(ret->isValid()) {
		return ret;
	}
	return 0;
}
class afDeclAPI_i *declManagerIMPL_c::_registerAFDecl(const char *name, qioModule_e userModule) {
	if(name == 0 || name[0] == 0) {
		g_core->RedWarning("declManagerIMPL_c::_registerAFDecl: NULL name\n");
		return 0;
	}
	afDecl_c *ret = afDecls.getEntry(name);
	if(ret) {
		ret->setReferencedByModule(userModule);
		if(ret->isValid()) {
			return ret;
		}
		return 0;
	}
	declTextDef_s txt;
	ret = new afDecl_c;
	ret->setDeclName(name);
	afDecls.addObject(ret);
	if(afFiles.findDeclText(name,"articulatedFigure",txt) == false) {
		return 0;
	}
	ret->parse(txt.p,txt.textBase,txt.sourceFile);
	ret->setReferencedByModule(userModule);
	if(ret->isValid()) {
		return ret;
	}
	return 0;
}
class q3PlayerModelAPI_i *declManagerIMPL_c::_registerQ3PlayerDecl(const char *name, qioModule_e userModule) {
	if(name == 0 || name[0] == 0) {
		g_core->RedWarning("declManagerIMPL_c::_registerQ3PlayerDecl: NULL name\n");
		return 0;
	}
	// '$' is no longer needed here
	if(name[0] == '$') {
		name++;
	}
	q3PlayerModelDecl_c *ret = q3PlayerDecls.getEntry(name);
	if(ret) {
		ret->setReferencedByModule(userModule);
		if(ret->isValid()) {
			return ret;
		}
		return 0;
	}
	ret = new q3PlayerModelDecl_c;
	ret->setDeclName(name);
	q3PlayerDecls.addObject(ret);
	ret->loadQ3PlayerDecl();
	ret->setReferencedByModule(userModule);
	if(ret->isValid()) {
		return ret;
	}
	return 0;
}
class particleDeclAPI_i *declManagerIMPL_c::_registerParticleDecl(const char *name, qioModule_e userModule) {
	if(name == 0 || name[0] == 0) {
		g_core->RedWarning("declManagerIMPL_c::_registerParticleDecl: NULL name\n");
		return 0;
	}

	// strip .prt extension
	str fixedName = name;
	fixedName.stripExtension();
	name = fixedName;

	particleDecl_c *ret = prtDecls.getEntry(name);
	if(ret) {
		ret->setReferencedByModule(userModule);
		if(ret->isValid()) {
			return ret;
		}
		return 0;
	}
	declTextDef_s txt;
	ret = new particleDecl_c;
	ret->setDeclName(name);
	prtDecls.addObject(ret);
	if(prtFiles.findDeclText(name,"particle",txt) == false) {
		return 0;
	}
	ret->parse(txt.p,txt.textBase,txt.sourceFile);
	ret->setReferencedByModule(userModule);
	if(ret->isValid()) {
		return ret;
	}
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
	for(int i = 0; i < afDecls.size(); i++) {
		afDecl_c *af = afDecls[i];
		if(af->isReferenced() == false) {
			afDecls.removeEntry(af);
			i--;
			delete af;
		}
	}
	for(int i = 0; i < prtDecls.size(); i++) {
		particleDecl_c *prt = prtDecls[i];
		if(prt->isReferenced() == false) {
			prtDecls.removeEntry(prt);
			i--;
			delete prt;
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
	for(u32 i = 0; i < afDecls.size(); i++) {
		afDecl_c *af = afDecls[i];
		af->clearServerRef();
	}
	for(u32 i = 0; i < q3PlayerDecls.size(); i++) {
		q3PlayerModelDecl_c *pd = q3PlayerDecls[i];
		pd->clearServerRef();
	}
	for(u32 i = 0; i < prtDecls.size(); i++) {
		particleDecl_c *prt = prtDecls[i];
		prt->clearServerRef();
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
	for(u32 i = 0; i < afDecls.size(); i++) {
		afDecl_c *af = afDecls[i];
		af->clearClientRef();
	}
	for(u32 i = 0; i < q3PlayerDecls.size(); i++) {
		q3PlayerModelDecl_c *pd = q3PlayerDecls[i];
		pd->clearClientRef();
	}
	for(u32 i = 0; i < prtDecls.size(); i++) {
		particleDecl_c *prt = prtDecls[i];
		prt->clearClientRef();
	}
	removeUnrefrencedDecls();
}
const char *declManagerIMPL_c::getLoadedEntityDeclName(u32 i) const {
	return entityDecls.objectAt(i)->getName();
}
void declManagerIMPL_c::loadAllEntityDecls() {
	cacheEntDefNamesList();
	for(u32 i = 0; i < entDefNamesList.size(); i++) {
		const char *name = entDefNamesList.getString(i);
		_registerEntityDecl(name,qioModule_e::QMD_DECL_MANAGER); // FIXME - mark module
	}
	entityDecls.sortArray();
}
void declManagerIMPL_c::iterateEntityDefNames(void (*callback)(const char *s)) {
	cacheEntDefNamesList();
	entDefNamesList.iterateStringList(callback);
}
void declManagerIMPL_c::iterateParticleDefNames(void (*callback)(const char *s)) {
	cacheParticleDefNamesList();
	particleDefNamesList.iterateStringList(callback);
}
fileTextDataCache_c::fileTextDataCache_c() {
	totalDefBytes = 0;
}
fileTextDataCache_c::~fileTextDataCache_c() {
	for(u32 i = 0; i < defFiles.size(); i++) {
		delete defFiles[i];
	}
	defFiles.clear();
}
void fileTextDataCache_c::cacheDefFileText(const char *fname) {
	char *data;
	u32 len = g_vfs->FS_ReadFile(fname,(void**)&data);
	if(data == 0)
		return;
	totalDefBytes += len;
	const char *ext = G_strgetExt(fname);
	g_core->Print("Caching .%s file: %s... - %i bytes\n",ext,fname,len);
	defFile_s *df = new defFile_s;
	df->fname = fname;
	df->text = data;
	g_vfs->FS_FreeFile(data);
	defFiles.push_back(df);
}
u32 fileTextDataCache_c::cacheFileList(const char *path, const char *ext) {
	int numFiles;
	totalDefBytes = 0;
	char **fnames = g_vfs->FS_ListFiles(path,ext,&numFiles);
	for(u32 i = 0; i < numFiles; i++) {
		const char *fname = fnames[i];
		str fullPath = path;
		fullPath.append(fname);
		cacheDefFileText(fullPath);
	}
	g_vfs->FS_FreeFileList(fnames);
	return numFiles;
}
void fileTextDataCache_c::listDeclNames(class stringList_c &out, const char *declType) const {
	u32 typeLen = strlen(declType);
	for(u32 i = 0; i < defFiles.size(); i++) {
		const defFile_s *f = defFiles[i];
		const char *fileText = f->text.c_str();
		const char *p = fileText;
		while(*p) {
			if(!_strnicmp(p,declType,typeLen) && G_isWS(p[typeLen])) {
				if(p != fileText && G_isWS(p[-1]) == false) {
					p ++;
					continue;
				}
				p += typeLen;
				while(G_isWS(*p)) {
					p++;
				}
				const char *nameStart = p;
				while(G_isWS(*p) == false) {
					p++;
				}
				str name;
				name.setFromTo(nameStart,p);
				out.addString(name);
			}
			p++;
		}
	}
	out.sortStrings();
}

void declManagerIMPL_c::cacheEntDefNamesList() {
	if(entDefNamesListReady)
		return;
	defFiles.listDeclNames(entDefNamesList,"entityDef");
	entDefNamesListReady = true;
}
void declManagerIMPL_c::cacheParticleDefNamesList() {
	if(particleDefNamesListReady)
		return;
	prtFiles.listDeclNames(particleDefNamesList,"particle");
	particleDefNamesListReady = true;
}

void declManagerIMPL_c::init() {
	entDefNamesListReady = false;
	particleDefNamesListReady = false;

	g_core->Print("----- Initializing Decls -----\n");
	defFiles.cacheFileList("def/",".def");
	afFiles.cacheFileList("af/",".af");
	prtFiles.cacheFileList("particles/",".prt");
	g_core->Print("----- Total %i decl bytes in %i def, %i af and %i prt files -----\n",
		defFiles.getTotalTextSizeInBytes()+afFiles.getTotalTextSizeInBytes()+prtFiles.getTotalTextSizeInBytes(),
		defFiles.getNumFiles(),afFiles.getNumFiles(),prtFiles.getNumFiles());

#if 0
	//registerModelDecl("monster_qwizard");
	registerAFDecl("monster_qwizard");
#endif
	_registerParticleDecl("rocketmuzzle",QMD_DECL_MANAGER);
}
void declManagerIMPL_c::shutdown() {
	for(int i = 0; i < entityDecls.size(); i++) {
		entityDecl_c *ed = entityDecls[i];
		if(ed->isReferenced()) {
			g_core->RedWarning("declManagerIMPL_c::shutdown: freeing %s which is still referenced\n",ed->getName());
		}
		delete ed;
	}
	for(int i = 0; i < modelDecls.size(); i++) {
		modelDecl_c *md = modelDecls[i];
		if(md->isReferenced()) {
			g_core->RedWarning("declManagerIMPL_c::shutdown: freeing %s which is still referenced\n",md->getName());
		}
		delete md;
	}
	for(int i = 0; i < afDecls.size(); i++) {
		afDecl_c *af = afDecls[i];
		if(af->isReferenced()) {
			g_core->RedWarning("declManagerIMPL_c::shutdown: freeing %s which is still referenced\n",af->getName());
		}
		delete af;
	}
	for(int i = 0; i < prtDecls.size(); i++) {
		particleDecl_c *prt = prtDecls[i];
		if(prt->isReferenced()) {
			g_core->RedWarning("declManagerIMPL_c::shutdown: freeing %s which is still referenced\n",prt->getName());
		}
		delete prt;
	}
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

