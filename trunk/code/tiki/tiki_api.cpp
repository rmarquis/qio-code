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
// tiki_api.cpp - TIKI API implementation
#include "tiki_local.h"
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/cmAPI.h>
#include <api/tikiAPI.h>
#include <api/modelLoaderDLLAPI.h>
#include <api/moduleManagerAPI.h>
#include <api/modelPostProcessFuncs.h>
#include <api/kfModelAPI.h>
#include <api/skelAnimAPI.h>
#include <shared/parser.h>
#include <shared/ePairsList.h>

class tikiBuilder_i {

public:
	virtual void addAnim(class tikiAnimBase_c *ta) = 0;
	virtual void addSkelModel(const char *skelModel) = 0;
	virtual void addRemap(const char *surf, const char *mat) = 0;
};

class tikiAnim_i {

public:
	virtual const char *getAlias() const = 0;
	virtual class kfModelAPI_i *getKFModel() const = 0;
};

class tikiAnimBase_c : public tikiAnim_i {
protected:
	str alias;
	str fileName;
public:
	~tikiAnimBase_c() {

	}
	virtual const char *getAlias() const {
		return alias;
	}

};
enum tikiType_e {
	TT_BAD,
	TT_SKELETAL,
	TT_KEYFRAMED,
};
class simpleTIKI_c : public tiki_i, public tikiBuilder_i {
	tikiType_e type;
	arraySTD_c<tikiAnimBase_c*> anims;
	ePairList_c remaps;

public:
	virtual bool isSkeletal() const {
		return type == TT_SKELETAL;
	}
	virtual bool isKeyframed() const {
		return true;
	}
	virtual void applyMaterialRemapsTo(class modelPostProcessFuncs_i *out) const {
		for(u32 i = 0; i < remaps.size(); i++) {
			const char *s, *m;
			remaps.getKeyValue(i,&s,&m);
			int sfIndex = out->findSurface(s);
			if(sfIndex != -1)
				out->setSurfaceMaterial(sfIndex,m);
		}
	}
	virtual class kfModelAPI_i *getAnimKFModel(u32 animNum) const {
		if(animNum >= anims.size())
			return 0;
		return anims[animNum]->getKFModel();
	}
	virtual void addAnim(class tikiAnimBase_c *ta) {
		anims.push_back(ta);
	}
	virtual void addSkelModel(const char *skelModel) {
		
	}
	virtual void addRemap(const char *surf, const char *mat) {
		remaps.set(surf,mat);
	}
};
class tikiAnimSkeletal_c : public tikiAnimBase_c {
	class skelAnimAPI_i *skelAnim;
public:
	tikiAnimSkeletal_c(const char *fname) {
		this->fileName = fname;
		skelAnim = g_modelLoader->loadSkelAnimFile(fname);
	}
	~tikiAnimSkeletal_c() {
		delete skelAnim;
	}


	virtual class kfModelAPI_i *getKFModel() const {
		return 0;
	}
};
class tikiAnimKeyFramed_c : public tikiAnimBase_c {
	class kfModelAPI_i *kfModel;
public:
	tikiAnimKeyFramed_c(const char *fname) {
		this->fileName = fname;
		kfModel = g_modelLoader->loadKeyFramedModelFile(fname);
		if(kfModel == 0) {
			g_core->RedWarning("tikiAnimKeyFramed_c::tikiAnimKeyFramed_c: failed to load %s\n",fname);
		}
	}
	~tikiAnimKeyFramed_c() {
		delete kfModel;
	}


	virtual class kfModelAPI_i *getKFModel() const {
		return kfModel;
	}
};

class tikiParser_c : public parser_c {
	str curPath;
	tikiBuilder_i *out;

	bool parseSetup() {
		while(atChar('}')==false) {
			if(atWord("scale")) {
				float sc = getFloat();
			} else if(atWord("lod_scale")) {
				// models/obj_wrench.tik
				float sc = getFloat();
			} else if(atWord("radius")) {
				// models/otto.tik
				float sc = getFloat();
			} else if(atWord("path")) {
				curPath = getToken();
				if(curPath.isLastChar('/')==false && curPath.isLastChar('\\')==false) {
					curPath.appendChar('/');
				}
				curPath.backSlashesToSlashes();
			} else if(atWord("skelmodel")) {
				// used for skeletal TIKIs,
				// this keyword should not be used
				// when .md3/.tan files are used for animations
				getToken();
			} else if(atWord("surface")) {
				str surf, mat;
				getToken(surf);
				if(atWord("shader")) {
					getToken(mat);
					if(out) {
						out->addRemap(surf,mat);
					}
				} else if(atWord("flags")) {
					// models/julie_base.txt
					getToken(mat);
				} else {

				}
			} else {
				g_core->RedWarning("tikiParser_c::parseSetup: unknown token '%s' at line %i of %s\n",
					getToken(),getCurrentLineNumber(),getDebugFileName());
				return true;
			}
		}
		return false;
	}
	bool parseAnimations() {
		while(atChar('}')==false) {
			//if(atWord("$include")) {
			//	str fileName;
			//	getToken(fileName);

			//	continue;
			//}
			str alias, file;
			getToken(alias);
			if(alias[0] == '{' || alias[1] == '}') {
				g_core->RedWarning("tikiParser_c::parseAnimations: expected animation alias, found '%s' at line %i of %s\n",
					alias.c_str(),getCurrentLineNumber(),getDebugFileName());
				return true;
			}
			getToken(file);
			if(file[0] == '{' || file[1] == '}') {
				g_core->RedWarning("tikiParser_c::parseAnimations: expected animation file name for alias '%s', found '%s' at line %i of %s\n",
					alias.c_str(),file.c_str(),getCurrentLineNumber(),getDebugFileName());
				return true;
			}
			str fullPath = curPath;
			fullPath.append(file);
			tikiAnimBase_c *ta;
			if(g_modelLoader->isKeyFramedModelFile(fullPath)) {
				ta = new tikiAnimKeyFramed_c(fullPath);
			} else if(g_modelLoader->isSkelAnimFile(fullPath)) {
				ta = new tikiAnimSkeletal_c(fullPath);
			} else {
				g_core->RedWarning("tikiParser_c::parseAnimations: unsupported animation file type - alias '%s', file '%s' at line %i of %s\n",
					alias.c_str(),fullPath.c_str(),getCurrentLineNumber(),getDebugFileName());
				return true;
			}
			// parse optional animation flags/params
			while(isAtEOL()==false) {
				if(atWord("default_angles")) {

				} else if(atWord("crossblend")) {
					getFloat();
				} else if(atWord("notimecheck")) {
					
				} else {
					g_core->RedWarning("tikiParser_c::parseAnimations: unexpected animation param %s for alias '%s', found '%s' at line %i of %s\n",
						getToken(),alias.c_str(),fullPath.c_str(),getCurrentLineNumber(),getDebugFileName());
					return true;
				}
			}
			// after animation alias-fname pair there is an optional commands block
			if(atChar('{')) {
				if(parseAnimationBlock(ta)) {
					delete ta;
					return true;
				}
			}
			if(out) {
				out->addAnim(ta);
			} else {
				delete ta;
			}
			
		}
		return false;
	}
	bool parseInitCommands() {
		while(atChar('}')==false) {
			if(atWord("classname")) {
				getToken();
			} else {
				const char *eventStr = getLine();
			}
		}
		return false;
	}
	bool parseAnimationBlock(class tikiAnimBase_c *ta) {

		while(atChar('}')==false) {
			if(atWord("server")) {
				if(atChar('{')==false) {
					g_core->RedWarning("tikiParser_c::parseAnimationBlock: expected '{' to follow animation 'server' section, found '%s' at line %i of %s\n",
						getToken(),getCurrentLineNumber(),getDebugFileName());
					return true;
				}
				if(parseInitCommands()) {
					g_core->RedWarning("tikiParser_c::parseAnimationBlock: failed to parse 'server' section of %s\n",
						getDebugFileName());
					return true;
				}
			} else if(atWord("client")) {
				if(atChar('{')==false) {
					g_core->RedWarning("tikiParser_c::parseAnimationBlock: expected '{' to follow 'client' section, found '%s' at line %i of %s\n",
						getToken(),getCurrentLineNumber(),getDebugFileName());
					return true;
				}
				if(parseInitCommands()) {
					g_core->RedWarning("tikiParser_c::parseAnimationBlock: failed to parse 'client' section of %s\n",
						getDebugFileName());
					return true;
				}
			} else {
				g_core->RedWarning("tikiParser_c::parseAnimationBlock: unknown token '%s' at line %i of %s\n",
					getToken(),getCurrentLineNumber(),getDebugFileName());
				return true;
			}
		}
		return false;
	}
	bool parseInit() {
		while(atChar('}')==false) {
			if(atWord("server")) {
				if(atChar('{')==false) {
					g_core->RedWarning("tikiParser_c::parseInit: expected '{' to follow 'server' section, found '%s' at line %i of %s\n",
						getToken(),getCurrentLineNumber(),getDebugFileName());
					return true;
				}
				if(parseInitCommands()) {
					g_core->RedWarning("tikiParser_c::parseInit: failed to parse 'server' section of %s\n",
						getDebugFileName());
					return true;
				}
			} else if(atWord("client")) {
				if(atChar('{')==false) {
					g_core->RedWarning("tikiParser_c::parseInit: expected '{' to follow 'client' section, found '%s' at line %i of %s\n",
						getToken(),getCurrentLineNumber(),getDebugFileName());
					return true;
				}
				if(parseInitCommands()) {
					g_core->RedWarning("tikiParser_c::parseInit: failed to parse 'client' section of %s\n",
						getDebugFileName());
					return true;
				}
			} else {
				g_core->RedWarning("tikiParser_c::parseInit: unknown token '%s' at line %i of %s\n",
					getToken(),getCurrentLineNumber(),getDebugFileName());
				return true;
			}
		}
		return false;
	}
public:
	bool parseTIKI(const char *fname, tikiBuilder_i *no = 0) {
		if(openFile(fname)) {

			return true;
		}
		this->out = no;
		if(atWord("TIKI")) {

		}
		while(atEOF()==false) {
			if(atWord("setup")) {
				if(atChar('{')==false) {
					g_core->RedWarning("tikiParser_c::parseTIKI: expected '{' to follow 'setup' section, found '%s' at line %i of %s\n",
						getToken(),getCurrentLineNumber(),getDebugFileName());
					return true;
				}
				if(parseSetup()) {
					g_core->RedWarning("tikiParser_c::parseTIKI: failed to parse 'setup' section of %s\n",
						getDebugFileName());
					return true;
				}
			} else if(atWord("animations")) {
				if(atChar('{')==false) {
					g_core->RedWarning("tikiParser_c::parseTIKI: expected '{' to follow 'animations' section, found '%s' at line %i of %s\n",
						getToken(),getCurrentLineNumber(),getDebugFileName());
					return true;
				}
				if(parseAnimations()) {
					g_core->RedWarning("tikiParser_c::parseTIKI: failed to parse 'animations' section of %s\n",
						getDebugFileName());
					return true;
				}
			} else if(atWord("init")) {
				if(atChar('{')==false) {
					g_core->RedWarning("tikiParser_c::parseTIKI: expected '{' to follow 'init' section, found '%s' at line %i of %s\n",
						getToken(),getCurrentLineNumber(),getDebugFileName());
					return true;
				}
				if(parseInit()) {
					g_core->RedWarning("tikiParser_c::parseTIKI: failed to parse 'init' section of %s\n",
						getDebugFileName());
					return true;
				}
			} else if(atWord("$define")) {
				getToken();
				getToken();
			} else if(atWord("$include")) {
				str fileName;
				getToken(fileName);

				g_core->Print("tikiParser_c::parseTIKI: including %s from %s\n",fileName.c_str(),fname);
				tikiParser_c tp2;
				// keep the path setting
				// This is needed for eg. models/weapon_uzi.tik
				tp2.curPath = this->curPath;
				if(tp2.parseTIKI(fileName,this->out)) {

				}

			} else {
				g_core->RedWarning("tikiParser_c::parseTIKI: unknown token '%s' at line %i of %s\n",
					getToken(),getCurrentLineNumber(),getDebugFileName());
				return true;
			}
		}
		g_core->Print("tikiParser_c::parseTIKI: successfully parsed %s\n",getDebugFileName());
		return false;
	}
};

class tikiAPIImpl_c : public tikiAPI_i {
public:
	tikiAPIImpl_c() {

	}
	~tikiAPIImpl_c() {
	}

	virtual void init() {
		g_core->Print("====== tikiAPIImpl_c::init() ===== \n");
		registerModel("models/projectile_cluster.tik");
		registerModel("models/plant_tree.tik");
		registerModel("models/otto.tik");
		registerModel("models/plant_fruit3.tik");
		registerModel("models/shield.tik");
		registerModel("models/weapon_axe.tik");
		registerModel("models/weapon_firesword.tik");
		registerModel("models/shgliek.tik");
		registerModel("models/weapon_uzi.tik");
		registerModel("models/xyz.tik");
		registerModel("models/shgliek_noise.tik");
		registerModel("models/tr_vine.tik");
		registerModel("models/weapon_gun.tik");
		registerModel("models/lochnar.tik");
		registerModel("models/julie_alpha.tik");
		registerModel("models/julie_base.tik");
		registerModel("models/julie_battle.tik");
		registerModel("models/fx_fire3d.tik");
		registerModel("models/creeper.tik");
		g_core->Print("TIKI module ready!\n");
	}
	virtual void shutdown() {
		g_core->Print("====== tikiAPIImpl_c::shutdown() ===== \n");
	}
	virtual class tiki_i *registerModel(const char *modName) {
		simpleTIKI_c *st = new simpleTIKI_c();
		tikiParser_c tp;
		tp.parseTIKI(modName,st);

		return st;
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
cmAPI_i *cm = 0;
modelLoaderDLLAPI_i *g_modelLoader = 0;
moduleManagerAPI_i *g_moduleMgr = 0;

// exports
static tikiAPIImpl_c g_staticTIKIAPI;
tikiAPI_i *tiki = &g_staticTIKIAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)tiki,TIKI_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_moduleMgr,MODULEMANAGER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_modelLoader,MODELLOADERDLL_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&cm,CM_API_IDENTSTR);

	//TIKI_AddConsoleCommands();
}

qioModule_e IFM_GetCurModule() {
	return QMD_TIKI;
}

