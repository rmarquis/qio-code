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
// gui_api.cpp - GUI API implementation
#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/fontAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/guiAPI.h>
#include <api/rAPI.h>
#include <api/modelLoaderDLLAPI.h>
#include <api/moduleManagerAPI.h>
#include <client/keyCodes.h>
#include "urc_mgr.h"

void GUI_AddConsoleCommands();
void GUI_RemoveConsoleCommands();

class guiAPIImpl_c : public guiAPI_i {
	urcMgr_c um;
	int mouseX, mouseY;
public:
	guiAPIImpl_c() {
		mouseX = 100;
		mouseY = 100;
	}
	virtual int getMouseX() const {
		return mouseX;
	}
	virtual int getMouseY() const {
		return mouseY;
	}
	void init() {
		um.precacheURCFiles();
		GUI_AddConsoleCommands();
	}
	virtual void shutdownGUI() {
		GUI_RemoveConsoleCommands();
	}
	virtual void drawGUI() {
		//urc_c *u = um.registerURC("main");
	//	u->drawURC();
		um.drawURCs();


		fontAPI_i *f = rf->registerFont("facfont-20");
		if(f) {
			f->drawString(50,50,"This is the font test!");
		}
		fontAPI_i *f2 = rf->registerFont("courier-16");
		if(f2) {
			f2->drawString(50,100,"This is the font test!");
		}
		fontAPI_i *f3 = rf->registerFont("verdana-12");
		if(f3) {
			f3->drawString(50,150,"This is the font test!");
		}
		fontAPI_i *f4 = rf->registerFont("Arial");
		if(f4) {
			f4->drawString(50,200,"This is Arial font test!");
		}
		if(f4 && f3) {
			if(rand()%2) 
				f4->drawString(50,250,"This is random font test!");
			else
				f3->drawString(50,250,"This is random font test!");
		}
		float mouseSize = 30.f;
		rf->drawStretchPic(mouseX,mouseY,mouseSize,mouseSize,0,0,1,1,"gfx/2d/mouse_cursor.tga");
	}
	virtual void onMouseMove(int dX, int dY) { 
		mouseX += dX;
		mouseY += dY;
		if(mouseX < 0)
			mouseX = 0;
		if(mouseY < 0)
			mouseY = 0;
		if(mouseX > rf->getWinWidth())
			mouseX = rf->getWinWidth();
		if(mouseY > rf->getWinHeight())
			mouseY = rf->getWinHeight();
	}
	virtual void onKeyDown(int keyCode) {
		if(keyCode == K_MOUSE1) {
			g_core->Print("LMB on GUI\n");
			um.onMouseDown(keyCode,mouseX,mouseY);
		} else {
			um.onKeyDown(keyCode);
		}
	}
	~guiAPIImpl_c() {
		//GUI_RemoveConsoleCommands();
	}

	void cmdPopAllMenus() {
		um.popAllMenus();
	}
	void cmdPopMenu() {
		um.popMenu();
	}
	void cmdPushMenu(const char *name) {
		um.pushMenu(name);
	}
};

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
rAPI_i *rf;
modelLoaderDLLAPI_i *g_modelLoader = 0;
moduleManagerAPI_i *g_moduleMgr = 0;

// exports
static guiAPIImpl_c g_staticGUIAPI;
guiAPI_i *gui = &g_staticGUIAPI;


void GUI_PushMenu_f() {
	if(g_core->Argc() < 2) {
		g_core->Print("Usage: pushmenu <urc name>\n");
		return;
	}
	g_staticGUIAPI.cmdPushMenu(g_core->Argv(1));
}
void GUI_PopMenu_f() {
	g_staticGUIAPI.cmdPopMenu();
}
void GUI_PopAllMenus_f() {
	g_staticGUIAPI.cmdPopAllMenus();
}
void GUI_AddConsoleCommands() {
	g_core->Cmd_AddCommand("pushmenu",GUI_PushMenu_f);
	g_core->Cmd_AddCommand("popmenu",GUI_PopMenu_f);
	g_core->Cmd_AddCommand("popallmenus",GUI_PopAllMenus_f);
	// TODO: what's the difference between pushmenu_sp and pushmenu?
	g_core->Cmd_AddCommand("pushmenu_sp",GUI_PushMenu_f);
}
void GUI_RemoveConsoleCommands() {
	g_core->Cmd_RemoveCommand("pushmenu");
	g_core->Cmd_RemoveCommand("popmenu");
	g_core->Cmd_RemoveCommand("popallmenus");
}

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)gui,GUI_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_moduleMgr,MODULEMANAGER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_modelLoader,MODELLOADERDLL_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&rf,RENDERER_API_IDENTSTR);

	g_staticGUIAPI.init();

}

qioModule_e IFM_GetCurModule() {
	return QMD_GUI;
}

