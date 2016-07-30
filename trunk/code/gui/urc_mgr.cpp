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
// urc_mgr.cpp - Ultimate ResourCe manager
#include "urc_mgr.h"
#include "urc_element_field.h"
#include "urc_element_pulldown.h"
#include <api/vfsAPI.h>
#include <api/rAPI.h>
#include <api/fontAPI.h>
#include <api/guiAPI.h>
#include <shared/parser.h>
#include <shared/autoCVar.h>

static aCvar_c urc_printVirtualScreenSize("urc_printVirtualScreenSize","0");

urcMgr_c::urcMgr_c() {
	activeField = 0;
	activePullDown = 0;
	curGUIRenderer = 0;
}
void urcMgr_c::precacheURCFile(const char *fname) {
	parser_c p;
	if(p.openFile(fname))
		return;
	if(!p.atWord("menu"))
		return;
	str name = p.getToken();
	g_core->Print("URC file %s has menu %s\n",fname,name.c_str());
	urcNameMappingCache_c *c = nameCache.getEntry(name);
	if(c == 0) {
		c = new urcNameMappingCache_c();
		c->setName(name);
		nameCache.addObject(c);
		c->setFName(fname);
	} else {
		g_core->Print("URC internal name %s already in use in %s, cant load from %s\n",
			name.c_str(),c->getFName(),fname);
	}
}
void urcMgr_c::precacheURCFiles() {
	int numFiles;
	char **fnames = g_vfs->FS_ListFiles("ui/","urc",&numFiles);
	for(u32 i = 0; i < numFiles; i++) {
		const char *fname = fnames[i];
		str fullPath = "ui/";
		fullPath.append(fname);
		precacheURCFile(fullPath);
	}
	g_vfs->FS_FreeFileList(fnames);
}

const char *urcMgr_c::findURCInternalNameForURCFileName(const char *fname) const {
	for(u32 i = 0; i < nameCache.size(); i++) {
		const urcNameMappingCache_c *c = nameCache[i];
		g_core->Print("Comparing %s with %s\n",fname,c->getFName());
		if(!stricmp(c->getFName(),fname))
			return c->getName();
	}
	return 0;
}
const char *urcMgr_c::getURCFileNameForURCInternalName(const char *internalName) const {
	const urcNameMappingCache_c *c = nameCache.getEntry(internalName);
	if(c == 0) {
		static str tmp;
		tmp = "ui/";
		tmp.append(internalName);
		tmp.append(".urc");
		return tmp;
	}
	return c->getFName();
}

class guiRendererDefault_c : public guiRenderer_i {
	float mX, mY;
public:
	void setMousePosReal(float x, float y) {
		mX = x;
		mY = y;
	}
	virtual void drawStretchPic(float x, float y, float w, float h,
		float s1, float t1, float s2, float t2, const char *matName) const {
		rf->drawStretchPic(x,y,w,h,s1,t1,s2,t2,matName);	
	}
	virtual void drawString(class fontAPI_i *f, float x, float y, const char *s) const {
		f->drawString(x,y,s);
	}
	virtual void fillRectRGBA(float x, float y, float w, float h, const float *rgba) const {
		rf->set2DColor(rgba);
		rf->drawStretchPic(x,y,w,h,0,0,1,1,"(1 1 1)");
		rf->set2DColor(0);
	}	
	virtual float getMouseX() const {
		return mX;
	}
	virtual float getMouseY() const {
		return mY;
	}
};
class guiRendererVirtualScreen_c : public guiRenderer_i {
	float mX, mY;
	float virtualHeight;
	float virtualWidth;
	float fracX;
	float fracY;
public:
	guiRendererVirtualScreen_c(float w, float h) {
		this->virtualHeight = h;
		this->virtualWidth = w;
		float rw = rf->getWinWidth();
		float rh = rf->getWinHeight();
		fracX = rw / w;
		fracY = rh / h;
	}
	void setMousePosReal(float x, float y) {
		mX = x/fracX;
		mY = y/fracY;
	}
	virtual void drawStretchPic(float x, float y, float w, float h,
		float s1, float t1, float s2, float t2, const char *matName) const {
		x *= fracX;
		y *= fracY;
		w *= fracX;
		h *= fracY;
		rf->drawStretchPic(x,y,w,h,s1,t1,s2,t2,matName);	
	}
	virtual void fillRectRGBA(float x, float y, float w, float h, const float *rgba) const {
		x *= fracX;
		y *= fracY;
		w *= fracX;
		h *= fracY;
		rf->set2DColor(rgba);
		rf->drawStretchPic(x,y,w,h,0,0,1,1,"(1 1 1)");
		rf->set2DColor(0);
	}
	virtual void drawString(class fontAPI_i *f, float x, float y, const char *s) const {
		x *= fracX;
		y *= fracY;
		f->drawString(x,y,s,fracX,fracY);
	}
	virtual float getMouseX() const {
		return mX;
	}
	virtual float getMouseY() const {
		return mY;
	}
};
void urcMgr_c::setupGUIRendererFor(const class urc_c *urc) {
	if(urc->isUsingVirtualScreen()) {
		static guiRendererVirtualScreen_c rv(urc->getVirtualScreenW(),urc->getVirtualScreenH());
		if(urc_printVirtualScreenSize.getInt()) {
			g_core->Print("urcMgr_c::setupGUIRendererFor: urc %s (filename %s) is using virtual screen size %i %i\n",
				urc->getName(),urc->getFName(),urc->getVirtualScreenW(),urc->getVirtualScreenH());
		}
		rv.setMousePosReal(gui->getMouseX(),gui->getMouseY());
		curGUIRenderer = &rv;
	} else {
		static guiRendererDefault_c rd;
		rd.setMousePosReal(gui->getMouseX(),gui->getMouseY());
		curGUIRenderer = &rd;
	}
}
void urcMgr_c::drawURCs() {

	//for(int i = stack.size()-1; i>=0;i--) {
	for(u32 i = 0; i < stack.size(); i++) {
		setupGUIRendererFor(stack[i]);
		stack[i]->drawURC(this);
	}
	// pull down list on top of everything
	if(activePullDown) {
		activePullDown->drawActivePullDown();
	}
}
void urcMgr_c::popAllMenus() {
	stack.clear();
	setActiveField(0);
}
void urcMgr_c::popMenu() {
	if(stack.size() == 0)
		return;
	stack.pop_back();
	setActiveField(0);
}
void urcMgr_c::pushMenu(const char *name) {
	urc_c *urc = registerURC(name);
	if(urc == 0 || urc->getNumElements() == 0) {
		const char *fixedName = findURCInternalNameForURCFileName(name);
		if(fixedName == 0) {
			str tmp = "ui/";
			tmp.append(name);
			fixedName = findURCInternalNameForURCFileName(tmp);
		}
		urc = registerURC(fixedName);
		if(urc == 0) {
			g_core->RedWarning("Pushmenu: %s is not a valid menu\n",name);
			return;
		}
		g_core->Print("Fixing %s into %s\n",name,fixedName);
		name = fixedName;
	}
	g_core->Print("Pushmenu: pushing %s\n",name);
	stack.push_back(urc);
	// typing is no longer possible in the background menu
	setActiveField(0);
}
void urcMgr_c::onMouseDown(int keyCode, int mouseX, int mouseY) {
	if(stack.size()==0)
		return;
	setupGUIRendererFor(stack[stack.size()-1]);
	// transform from window coordinates to virtual screen coordinates (if needed)
	float tranformedMouseX = curGUIRenderer->getMouseX();
	float tranformedMouseY = curGUIRenderer->getMouseY();
	stack[stack.size()-1]->onMouseDown(keyCode,tranformedMouseX,tranformedMouseY, this);
}
void urcMgr_c::onMouseUp(int keyCode, int mouseX, int mouseY) {
	if(stack.size()==0)
		return;
	if(activePullDown) {
		setActivePullDown(0);
	}
}
void urcMgr_c::onMouseMove(int mouseX, int mouseY) {
	if(stack.size()==0)
		return;
	stack[stack.size()-1]->onMouseMove(mouseX,mouseY);
}
void urcMgr_c::onKeyDown(int keyCode) {
	if(stack.size()==0)
		return;
	stack[stack.size()-1]->onKeyDown(keyCode);
	if(activeField) {
		activeField->onKeyDown(keyCode);
	}
}
urc_c *urcMgr_c::findURCForFileName(const char *fname) {
	for(u32 i = 0; i < loaded.size(); i++) {
		urc_c *urc = loaded[i];
		const char *urcFName = urc->getFName();
		if(!stricmp(urcFName,fname))
			return urc;
	}
	return 0;
}
urc_c *urcMgr_c::registerURC(const char *internalName) {
	if(internalName == 0)
		return 0;
	urc_c *urc = loaded.getEntry(internalName);
	if(urc) {
		return urc;
	}
	urc = new urc_c();
	urc->setName(internalName);
	loaded.addObject(urc);
	// find the .urc file path for internal URC name
	const char *fname = getURCFileNameForURCInternalName(internalName);
	urc->setFName(fname);

	urc->loadURCFile();

	return urc;
}
