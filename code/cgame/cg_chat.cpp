/*
============================================================================
Copyright (C) 2013 V.

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
// cg_chat.cpp
#include "cg_local.h"
#include <shared/array.h>
#include <api/rAPI.h>
#include <shared/autoCvar.h>

static aCvar_c cg_chatHeight("cg_chatHeight","300");
static aCvar_c cg_chatMaxHistory("cg_chatMaxHistory","128");

class simpleChat_c {
	arraySTD_c<char *> lines;

public:
	simpleChat_c() {
		//addChatMessage("14:30: Tester 1: Hello world!");
		//addChatMessage("14:35: Tester 2: Hi.");
		//addChatMessage("14:39: Tester 3: Hiho.");
	}
	~simpleChat_c() {
		for(u32 i = 0; i < lines.size(); i++) {
			free(lines[i]);
		}
	}
	void addChatMessage(const char *newString) {
		while(lines.size() > 10 && lines.size() > cg_chatMaxHistory.getInt()) {
			// free the oldest message
			free(lines[0]);
			lines.remove(0);
		}
		lines.push_back(_strdup(newString));
	}
	void drawChat(u32 x, u32 y, u32 w, u32 h) {
		y -= h;
		u32 numLines = h / SMALLCHAR_HEIGHT;
		float color[4] = { 0, 0, 0, 1.f };
		int index = lines.size() - numLines;
		if(index < 0)
			index = 0;
		for(u32 i = 0; i < numLines; i++) {
			if(index >= lines.size())
				break;
			CG_DrawSmallStringColor(x,y,lines[index],color);
			y += SMALLCHAR_HEIGHT;
			index++;
		}
	}
};

static simpleChat_c cg_chat;

void CG_AddChatMessage(const char *msg) {
	cg_chat.addChatMessage(msg);
}

void CG_DrawChat() {
	cg_chat.drawChat(0,480,400,cg_chatHeight.getInt());
}

