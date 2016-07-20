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
#include <api/skelModelAPI.h>
#include <api/materialSystemAPI.h>
#include <shared/perStringCallback.h>
#include <shared/parser.h>
#include <shared/ePairsList.h>
#include <shared/hashTableTemplate.h>

class tikiBuilder_i {

public:
	virtual void setClassName(const char *s) = 0;
	virtual void addAnim(class tikiAnimBase_c *ta) = 0;
	virtual void setScale(float f) = 0;
	virtual void addSkelModel(const char *skelModel) = 0;
	virtual void addRemap(const char *surf, const char *mat) = 0;
	virtual void addInitCommand(tikiCommandSide_e side, const char *txt) = 0;
};

class tikiAnimCommand_c {
friend class tikiAnimCommands_c;
	int frameNum;
	str commandText;
};
class tikiAnimCommands_c {
	arraySTD_c<tikiAnimCommand_c> commands;
public:
	void addCommand(int frame, const char *txt) {
		tikiAnimCommand_c c;
		c.frameNum = frame;
		c.commandText = txt;
		c.commandText.stripTrailing("\r\n");
		commands.push_back(c);
	}
	void executeCommands(int startFrame,int lastFrame,class perStringCallbackListener_i *cb) const {
		for(u32 i = 0; i < commands.size(); i++) {
			const tikiAnimCommand_c &c = commands[i];

			// HACK? 
			// What about TF_LAST?
			int fixedFrameNum;
			if(c.frameNum == TF_FIRST)
				fixedFrameNum = 0;
			//else if(c.frameNum == TF_LAST)
			//	fixedFrameNum = getNumFrames();
			else 
				fixedFrameNum = c.frameNum;

			if(fixedFrameNum >= lastFrame)
				return;
			if(fixedFrameNum >= startFrame) {
				if(0) {
					g_core->Print("Executing cmd %i of %i (%s) because frameNum %i is inside range <%i,%i)\n",
						i,commands.size(),c.commandText.c_str(),c.frameNum,startFrame,lastFrame);
				}
				if(cb) {
					cb->perStringCallback(c.commandText);
				}
			}
		}
	}
};

class tikiCommands_c {
	arraySTD_c<str> commands;
public:
	void addCommand(const char *txt) {
		str &s = commands.pushBack();
		s = txt;
		s.stripTrailing("\r\n");
	}
	void executeCommands(class perStringCallbackListener_i *cb) const {
		if(cb == 0)
			return;
		for(u32 i = 0; i < commands.size(); i++) {
			if(0) {
				g_core->Print("tikiCommands_c::executeCommands: this %i, cmd %i of %i, txt %s\n",
					this,i,commands.size(),commands[i].c_str());
			}
			cb->perStringCallback(commands[i]);
		}
	}
};
class tikiAnimBase_c : public tikiAnim_i {
	tikiAnimBase_c *hashNext;
protected:
	str alias;
	str fileName;
	tikiAnimCommands_c *serverCommands;
	tikiAnimCommands_c *clientCommands;
public:
	tikiAnimBase_c() {
		serverCommands = 0;
		clientCommands = 0;
	}
	virtual ~tikiAnimBase_c() {
		if(clientCommands) {
			delete clientCommands;
		}
		if(serverCommands) {
			delete serverCommands;
		}
	}	
	virtual void scaleAnimation(float s) = 0;
	void iterateCommands(const tikiAnimCommands_c *s, u32 startTime, u32 endTime, class perStringCallbackListener_i *cb) const {
		if(endTime <= startTime)
			return;
		u32 startFrame = startTime / getFrameTimeMs();
		u32 lastFrame = endTime / getFrameTimeMs();
		s->executeCommands(startFrame,lastFrame,cb);
	}
	void iterateCommands(tikiCommandSide_e side, u32 startTime, u32 endTime, class perStringCallbackListener_i *cb) const {
		if(side == TCS_SERVER) {
			if(serverCommands == 0)
				return;
			iterateCommands(serverCommands,startTime,endTime,cb);
		} else if(side == TCS_CLIENT) {
			if(clientCommands == 0)
				return;
			iterateCommands(clientCommands,startTime,endTime,cb);
		}
	}
	tikiAnimCommands_c *allocServerCommands() {
		if(serverCommands == 0)
			serverCommands = new tikiAnimCommands_c();
		return serverCommands;
	}
	tikiAnimCommands_c *allocClientCommands() {
		if(clientCommands == 0)
			clientCommands = new tikiAnimCommands_c();
		return clientCommands;
	}
	const char *getName() const {
		return alias;
	}
	void setAlias(const char *a) {
		this->alias = a;
	}
	void setHashNext(class tikiAnimBase_c *tb) {
		hashNext = tb;
	}
	tikiAnimBase_c *getHashNext() const {
		return hashNext;
	}
	virtual const char *getAlias() const {
		return alias;
	}

};

class tikiBase_c : public tiki_i {
	str fname;
	tikiBase_c *hashNext;
public:
	virtual ~tikiBase_c() {

	}
	tikiBase_c *getHashNext() {
		return hashNext;
	}
	void setName(const char *s) {
		fname = s;
	}
	void setHashNext(tikiBase_c *hn) {
		hashNext = hn;
	}
	virtual const char *getName() const {
		return fname;
	}
	virtual bool isValid() const = 0;
};
//enum tikiType_e {
//	TT_BAD,
//	TT_SKELETAL,
//	TT_KEYFRAMED,
//};
class simpleTIKI_c : public tikiBase_c, public tikiBuilder_i {
	str className;
//	tikiType_e type;
	//hashTableTemplateExt_c<tikiAnimBase_c> anims;
	arraySTD_c<tikiAnimBase_c*> anims;
	ePairList_c remaps;
	skelModelAPI_i *skelModel;
	tikiCommands_c *serverCommands;
	tikiCommands_c *clientCommands;
	float scale;
public:
	simpleTIKI_c() {
		skelModel = 0;
		serverCommands = 0;
		clientCommands = 0;
		scale = 1.f;
	}
	virtual ~simpleTIKI_c() {
		if(skelModel) {
			delete skelModel;
		}
		for(u32 i = 0; i < anims.size(); i++) {
			delete anims[i];
		}
		if(clientCommands) {
			delete clientCommands;
		}
		if(serverCommands) {
			delete serverCommands;
		}
	}
	virtual void setClassName(const char *s) {
		className = s;
	}
	tikiCommands_c *allocServerCommands() {
		if(serverCommands == 0)
			serverCommands = new tikiCommands_c();
		return serverCommands;
	}
	tikiCommands_c *allocClientCommands() {
		if(clientCommands == 0)
			clientCommands = new tikiCommands_c();
		return clientCommands;
	}
	virtual bool isSkeletal() const {
		return skelModel!=0;
	}
	virtual bool isKeyframed() const {
		return skelModel==0;
	}
	virtual const char *getClassName() const {
		return className;
	}
	virtual const class tikiAnim_i *getAnim(int animIndex) const {
		if(animIndex < 0)
			return 0;
		if(animIndex >= anims.size())
			return 0;
		return anims[animIndex];
	}
	virtual void iterateInitCommands(tikiCommandSide_e side, class perStringCallbackListener_i *cb) const {
		if(side == TCS_SERVER) {
			if(serverCommands==0)
				return;
			serverCommands->executeCommands(cb);
		} else if(side == TCS_CLIENT) {
			if(clientCommands==0)
				return;
			clientCommands->executeCommands(cb);
		}
	}
	virtual int findAnim(const char *animAlias) const { 
		if(animAlias == 0)
			return -1;
		for(u32 i = 0; i < anims.size(); i++) {
			if(!stricmp(anims[i]->getAlias(),animAlias)) {
				return i;
			}
		}
		// game code might request a random animation,
		// for example, "idle" should return "idle1" or "idle2" randomly
		char lastChar = animAlias[strlen(animAlias)-1];
		if(isdigit(lastChar)==false) {
#define MAX_RANDOM_ANIMS_PER_NAME 32
			int candidates[MAX_RANDOM_ANIMS_PER_NAME];
			u32 found = 0;
			u32 aliasLen = strlen(animAlias);
			for(u32 i = 0; i < anims.size(); i++) {
				const char *alias = anims[i]->getAlias();
				if(!strnicmp(alias,animAlias,aliasLen)
					&& isdigit(alias[aliasLen]) && alias[aliasLen+1]==0) {
					if(found < MAX_RANDOM_ANIMS_PER_NAME) {
						candidates[found] = i;
						found++;
					}
				}
			}
			if(found) {
				//u32 chosen = rand() % found;
				u32 chosen = 0; // TODO
				return candidates[chosen];
			}
		}
		return -1;
	}
	virtual int getAnimTotalTimeMs(int animIndex) const {
		if(animIndex < 0)
			return 0;
		if(animIndex >= anims.size())
			return 0;
		return anims[animIndex]->getTotalTimeMs();
	}
	virtual int getBoneNumForName(const char *boneName) const {
		if(skelModel) {
			return skelModel->getLocalBoneIndexForBoneName(boneName);
		}
		return -1;
	}
	virtual void applyMaterialRemapsTo(class modelPostProcessFuncs_i *out) const {
		for(u32 i = 0; i < remaps.size(); i++) {
			const char *s, *m;
			remaps.getKeyValue(i,&s,&m);
			int sfIndex = out->findSurface(s);
			if(sfIndex != -1) {
				// fix TIKI material names
				// (only for FAKK?)
				str tmp;
				if(g_ms) {
					if(skelModel && g_ms->isMaterialOrImagePresent(m)==false) {
						tmp = skelModel->getName();
						tmp.toDir();
						tmp.append(m);
						if(g_ms->isMaterialOrImagePresent(tmp)) {
							m = tmp;
						}
					}
				}
				out->setSurfaceMaterial(sfIndex,m);
			}
		}
	}
	virtual class skelAnimAPI_i *getSkelAnim(u32 animNum) const {
		if(animNum >= anims.size())
			return 0;
		return anims[animNum]->getSkelAnim();
	}
	virtual class kfModelAPI_i *getAnimKFModel(u32 animNum) const {
		if(animNum >= anims.size())
			return 0;
		return anims[animNum]->getKFModel();
	}
	virtual void addAnim(class tikiAnimBase_c *ta) {
		anims.push_back(ta);
		ta->scaleAnimation(scale);
	}
	virtual void setScale(float f) {
		scale = f;
	}
	virtual void addSkelModel(const char *skelModelName) {
		if(skelModel)
			return;
		skelModel = g_modelLoader->loadSkelModelFile(skelModelName);
		if(skelModel) {
			skelModel->scaleXYZ(scale);
		}
	}
	virtual void addRemap(const char *surf, const char *mat) {
		remaps.set(surf,mat);
	}
	virtual void addInitCommand(tikiCommandSide_e side, const char *txt) {
		tikiCommands_c *cmds;
		if(side == TCS_SERVER)
			cmds = allocServerCommands();
		else
			cmds = allocClientCommands();
		cmds->addCommand(txt);
	}
	virtual class skelModelAPI_i *getSkelModel() const {
		return skelModel;
	}

	virtual bool isValid() const {
		if(skelModel)
			return true;
		if(remaps.size())
			return true;
		if(anims.size())
			return true;
		return false;
	}
};
class tikiAnimSkeletal_c : public tikiAnimBase_c {
	class skelAnimAPI_i *skelAnim;
public:
	tikiAnimSkeletal_c(const char *fname) {
		this->fileName = fname;
		skelAnim = g_modelLoader->loadSkelAnimFile(fname);
		if(skelAnim == 0) {
			g_core->RedWarning("tikiAnimSkeletal_c::tikiAnimSkeletal_c: failed to load %s\n",fname);
		}
	}
	virtual ~tikiAnimSkeletal_c() {
		delete skelAnim;
	}	
	virtual void scaleAnimation(float s) {
		if(skelAnim) {
			skelAnim->scaleAnimation(s);
		}
	}

	virtual int getTotalTimeMs() const {
		if(skelAnim == 0)
			return 0;
		return skelAnim->getTotalTimeSec() * 1000;
	}
	virtual int getNumFrames() const {
		if(skelAnim == 0)
			return 0;
		return skelAnim->getNumFrames();
	}
	virtual int getFrameTimeMs() const {
		if(skelAnim == 0)
			return 0;
		return skelAnim->getFrameTime()*1000;
	}

	virtual class skelAnimAPI_i *getSkelAnim() const {
		return skelAnim;
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
	virtual ~tikiAnimKeyFramed_c() {
		delete kfModel;
	}
	virtual void scaleAnimation(float s) {
		if(kfModel) {
			kfModel->scale(s);
		}
	}




	virtual int getTotalTimeMs() const {
		if(kfModel == 0)
			return 0;
		return getNumFrames() * getFrameTimeMs();
	}
	virtual int getNumFrames() const {
		if(kfModel == 0)
			return 0;
		return kfModel->getNumFrames();
	}
	virtual int getFrameTimeMs() const {
		if(kfModel == 0)
			return 0;
		return kfModel->getFrameTime() * 1000;
	}

	virtual class skelAnimAPI_i *getSkelAnim() const {
		return 0;
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
				if(out) {
					out->setScale(sc);
				}
			} else if(atWord("lod_scale")) {
				// models/obj_wrench.tik
				float sc = getFloat();
			} else if(atWord("origin")) {
				// models/weapon_shotgun.tik
				float x = getFloat();
				float y = getFloat();
				float z = getFloat();
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
				str skelModelName;
				// used for skeletal TIKIs,
				// this keyword should not be used
				// when .md3/.tan files are used for animations
				getToken(skelModelName);
				str fp = curPath;
				fp.append(skelModelName);
				if(out) {
					out->addSkelModel(fp);
				}
			} else if(atWord("surface")) {
				str surf, mat;
				getToken(surf);
				if(atWord("shader")) {
					getToken(mat);
					if(out) {
						//if(mat.findToken(".",0,false)) {
							//str tmp = curPath;
							//tmp.append(mat);
							//mat = tmp;
						//}
						out->addRemap(surf,mat);
					}
				} else if(atWord("flags")) {
					// models/julie_base.txt
					getToken(mat);
				} else {

				}
			} else if(atWord("thirdPersonModel")) {
				// for MSR.
				skipLine();
			} else if(atWord("firstpersonmodel")) {
				// for MSR.
				skipLine();
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
			ta->setAlias(alias);
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
	bool parseInitCommands(tikiCommandSide_e side) {
		while(atChar('}')==false) {
			if(atWord("classname")) {
				str className;
				getToken(className);
				out->setClassName(className);
			} else {
				const char *eventStr = getLine();
				if(side == TCS_SERVER) {
					out->addInitCommand(TCS_SERVER,eventStr);
				} else if(side == TCS_CLIENT) {
					out->addInitCommand(TCS_CLIENT,eventStr);
				}
			}
		}
		return false;
	}
	bool parseAnimCommands(class tikiAnimCommands_c *ac) {
		while(atChar('}')==false) {
			int frameNum;
			if(atWord("entry")) {
				frameNum = TF_ENTRY;
			} else if(atWord("exit")) {
				frameNum = TF_EXIT;
			} else if(atWord("first")) {
				frameNum = TF_FIRST;
			} else if(atWord("last")) {
				frameNum = TF_LAST;
			} else {
				frameNum = getInteger();
			}
			const char *l = getLine("}");
			if(0) {
				g_core->Print("parseAnimCommands: frame %i, cmd '%s'\n",
					frameNum,l);
			}
			ac->addCommand(frameNum,l);
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
				if(parseAnimCommands(ta->allocServerCommands())) {
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
				if(parseAnimCommands(ta->allocClientCommands())) {
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
				if(parseInitCommands(TCS_SERVER)) {
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
				if(parseInitCommands(TCS_CLIENT)) {
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
	hashTableTemplateExt_c<tikiBase_c> loaded;
public:
	tikiAPIImpl_c() {

	}
	~tikiAPIImpl_c() {
	}

	virtual void init() {
		g_core->Print("====== tikiAPIImpl_c::init() ===== \n");
		if(0) {
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
		}
		g_core->Print("TIKI module ready!\n");
	}
	virtual void shutdown() {
		g_core->Print("====== tikiAPIImpl_c::shutdown() ===== \n");
		for(u32 i = 0; i < loaded.size(); i++) {
			delete loaded[i];
		}
	}
	virtual class tiki_i *registerModel(const char *modName) {
		tikiBase_c *tb = loaded.getEntry(modName);
		if(tb == 0) {
			simpleTIKI_c *st = new simpleTIKI_c();
			tikiParser_c tp;
			tp.parseTIKI(modName,st);
			tb = st;
			tb->setName(modName);
			loaded.addObject(tb);
		}
		if(tb->isValid() == false)
			return 0;
		return tb;
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
cmAPI_i *cm = 0;
materialSystemAPI_i *g_ms = 0;
modelLoaderDLLAPI_i *g_modelLoader = 0;
moduleManagerAPI_i *g_moduleMgr = 0;

// exports
static tikiAPIImpl_c g_staticTIKIAPI;
tikiAPI_i *g_tikiMgr = &g_staticTIKIAPI;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)g_tikiMgr,TIKI_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_moduleMgr,MODULEMANAGER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_modelLoader,MODELLOADERDLL_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&cm,CM_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_ms,MATERIALSYSTEM_API_IDENTSTR);

	//TIKI_AddConsoleCommands();
}

qioModule_e IFM_GetCurModule() {
	return QMD_TIKI;
}

