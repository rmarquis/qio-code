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
#include <shared/parser.h>



class tikiParser_c : public parser_c {
	str curPath;

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
			} else if(atWord("skelmodel")) {
				// used for skeletal TIKIs,
				// this keyword should not be used
				// when .md3/.tan files are used for animations
				curPath = getToken();
			} else if(atWord("surface")) {
				str surf, mat;
				getToken(surf);
				if(atWord("shader")) {
					getToken(mat);
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
			// parse optional animation flags/params
			while(isAtEOL()==false) {
				if(atWord("default_angles")) {

				} else if(atWord("crossblend")) {
					getFloat();
				} else if(atWord("notimecheck")) {
					
				} else {
					g_core->RedWarning("tikiParser_c::parseAnimations: unexpected animation param %s for alias '%s', found '%s' at line %i of %s\n",
						getToken(),alias.c_str(),file.c_str(),getCurrentLineNumber(),getDebugFileName());
					return true;
				}
			}
			// after animation alias-fname pair there is an optional commands block
			if(atChar('{')) {
				if(parseAnimationBlock()) {
					return true;
				}
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
	bool parseAnimationBlock() {

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
	bool parseTIKI(const char *fname) {
		if(openFile(fname)) {

			return true;
		}
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
				getToken();
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
		tikiParser_c tp;
		tp.parseTIKI("models/projectile_cluster.tik");
		tp.parseTIKI("models/plant_tree.tik");
		tp.parseTIKI("models/otto.tik");
		tp.parseTIKI("models/plant_fruit3.tik");
		tp.parseTIKI("models/shield.tik");
		tp.parseTIKI("models/weapon_axe.tik");
		tp.parseTIKI("models/weapon_firesword.tik");
		tp.parseTIKI("models/shgliek.tik");
		tp.parseTIKI("models/weapon_uzi.tik");
		tp.parseTIKI("models/xyz.tik");
		tp.parseTIKI("models/shgliek_noise.tik");
		tp.parseTIKI("models/tr_vine.tik");
		tp.parseTIKI("models/weapon_gun.tik");
		tp.parseTIKI("models/lochnar.tik");
		tp.parseTIKI("models/julie_alpha.tik");
		tp.parseTIKI("models/julie_base.tik");
		tp.parseTIKI("models/julie_battle.tik");
		tp.parseTIKI("models/fx_fire3d.tik");
		tp.parseTIKI("models/creeper.tik");
		g_core->Print("TIKI module ready!\n");
	}
	virtual void shutdown() {
		g_core->Print("====== tikiAPIImpl_c::shutdown() ===== \n");
	}
	virtual class cMod_i *registerModel(const char *modName) {

		return 0;
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

