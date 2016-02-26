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
// rf_font.cpp
#include <shared/hashTableTemplate.h>
#include <shared/str.h>
#include <shared/parser.h>
#include <shared/rect.h>
#include <api/vfsAPI.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <api/fontAPI.h>
#include <api/mtrAPI.h>
#include <api/materialSystemAPI.h>

class fontBase_c : public fontAPI_i {
	const str name;
	fontBase_c *hashNext;
public:
	fontBase_c(const char *s) : name(s) {

	}
	virtual const char *getName() const {
		return name;
	}
	void setHashNext(fontBase_c *fb) {
		hashNext = fb;
	}
	fontBase_c *getHashNext() {
		return hashNext;
	}
};
class fontRitual_c : public fontBase_c {
	int indirections[256];
	rect_c rects[256];
	float height;
	mtrAPI_i *mat;
	bool bIsValid;
public:
	fontRitual_c(const char *s) : fontBase_c(s) {
		str path = "fonts/";
		path.append(getName());
		path.append(".RitualFont");
		parser_c p;
		if(p.openFile(path)) {
			g_core->RedWarning("fontRitual_c::fontRitual_c: failed to open %s\n",path.c_str());
			bIsValid = false;
			return;
		}
		if(p.atWord("RitFont")==false) {


		}
		str matName = "gfx/fonts/";
		matName.append(getName());
		mat = g_ms->registerMaterial(matName);
		while(p.atEOF()==false) {
			if(p.atWord("height")) {
				height = p.getFloat();
			} else if(p.atWord("aspect")) {
				p.getFloat();
			} else if(p.atWord("indirections")) {
				if(!p.atChar('{')) {
					g_core->RedWarning("Expected '{' after indirections token for ritualfont %s, found %s\n",
							getName(),p.getToken());
					bIsValid = false;
					return;
				}
				for(u32 i = 0; i < 256; i++) {
					indirections[i] = p.getInteger();
				}
				if(!p.atChar('}')) {
					g_core->RedWarning("Expected '}' at the end of indirections block for ritualfont %s, found %s\n",
							getName(),p.getToken());
					bIsValid = false;
					return;
				}
			} else if(p.atWord("locations")) {
				if(!p.atChar('{')) {

				}
				for(int i = 0; i < 256; i++) {
					if(!p.atChar('{')) {
						g_core->RedWarning("Expected '{' to follow location %i of ritualfont %s, found %s\n",
							i,getName(),p.getToken());
						bIsValid = false;
						return;
					}
					rects[i].setMinX(p.getFloat());
					rects[i].setMinY(p.getFloat());
					rects[i].setW(p.getFloat());
					rects[i].setH(p.getFloat());
					if(!p.atChar('}')) {
						g_core->RedWarning("Expected '}' after location %i of ritualfont %s, found %s\n",
							i,getName(),p.getToken());
						bIsValid = false;
						return;
					}
				}
				if(!p.atChar('}')) {

				}
			} else {
				p.getToken();
			}
		}
		bIsValid = true;
	}
	virtual void drawString(float x, float y, const char *s) const {
		if(bIsValid==false)
			return;
		const char *p = s;
		float nowX = x;
		float nowY = y;
		float w = 8.f;
		float h = 16.f;
		while(*p) {
			int ch = indirections[*p];
			if(ch != -1) {
				const rect_c &r = rects[ch];
				float s0 = float(rects[ch].getX())/float(mat->getImageWidth());
				float t0 = float(rects[ch].getY())/float(mat->getImageHeight());
				float s1 = float(rects[ch].getMaxX())/float(mat->getImageWidth());
				float t1 = float(rects[ch].getMaxY())/float(mat->getImageHeight());
				rf->drawStretchPic(nowX,nowY,w,h,s0,t0,s1,t1,mat);
			}
			nowX += w;
			p++;
		}
	}
};
hashTableTemplateExt_c<fontBase_c> rf_fonts;

fontAPI_i *RF_RegisterFont(const char *name) {
	fontBase_c *f = rf_fonts.getEntry(name);
	if(f)
		return f;
	str path = "fonts/";
	path.append(name);
	path.append(".RitualFont");
	if(g_vfs->FS_FileExists(path)) {
		f = new fontRitual_c(name);
	}
	rf_fonts.addObject(f);
	return f;
}
