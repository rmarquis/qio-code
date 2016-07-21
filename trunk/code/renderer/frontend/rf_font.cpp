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
#include <shared/colorTable.h>
#include <api/vfsAPI.h>
#include <api/coreAPI.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>
#include <api/fontAPI.h>
#include <api/mtrAPI.h>
#include <api/materialSystemAPI.h>
#include <shared/textureAllocator.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#pragma comment(lib,"freetype.lib")

FT_Library rf_ft;

class fontBase_c : public fontAPI_i {
	const str name;
	fontBase_c *hashNext;
protected:
	bool bIsValid;
public:
	fontBase_c(const char *s) : name(s) {

	}
	virtual ~fontBase_c() {

	}
	virtual const char *getName() const {
		return name;
	}
	void setupColourCode(int c) const {
		const float *f = g_color_table[c];
		rb->setColor4(f);
	}
	const char *checkForColourCode(const char *p) const {
		if(p[0] == '^' && p[1]) {
			// skip p[0]
			p++;
			// get code
			char colourCode = *p;
			// skip colour code 
			p++;
			// setup
			setupColourCode(colourCode);
		}
		return p;
	}
	void setHashNext(fontBase_c *fb) {
		hashNext = fb;
	}
	fontBase_c *getHashNext() {
		return hashNext;
	}
	bool isValid() const {
		return bIsValid;
	}
};
class fontRitual_c : public fontBase_c {
	int indirections[256];
	rect_c rects[256];
	float height;
	mtrAPI_i *mat;
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
	virtual float getStringWidth(const char *s) const {
		return 0; // TODO
	}
	virtual float getStringHeight(const char *s) const {
		return 0; // TODO
	}
	virtual float drawChar(float x, float y, char s, float sx = 1.f, float sy = 1.f) const {

		return 0.f; // TODO
	}
	virtual void drawString(float x, float y, const char *s, float scaleX = 1.f, float scaleY = 1.f, bool bUseColourCodes = true) const {
		if(bIsValid==false)
			return;
		const char *p = s;
		float nowX = x;
		float nowY = y;
		float w = 8.f;
		float h = 16.f;
		while(*p) {
			if(bUseColourCodes) {
				p = checkForColourCode(p);
				if(p == 0 || *p == 0)
					break;
			}
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
		if(bUseColourCodes) {
			// set back the default colour (white)
			rb->setColor4(0);
		}
	}
};

struct glyphInfo_s {
	float width;
	float rows;
	float left;
	float top;
	long advance_x;
	long advance_y;
	u32 sizes[2]; // = { width, rows }
	u32 pos[2];
	float s0, s1;
	float t0, t1;
};

class image8bit_c {
	byte *data;
	u32 w, h;
public:
	image8bit_c() {
		data = 0;
	}
	~image8bit_c() {
		if(data) {
			free(data);
		}
	}
	void setSize(u32 newSize) {
		w = h = newSize;
		data = (byte*)malloc(newSize*newSize);
		memset(data,0,newSize*newSize);
	}
	void setSubImage(u32 x, u32 y, u32 inW, u32 inH, const byte *ptr) {
		for(u32 i = 0; i < inW; i++) {
			for(u32 j = 0; j < inH; j++) {
				u32 inIndex = inW * j + i;
				u32 outIndex = w * (y+j) + x + i;
				data[outIndex] = ptr[inIndex];
			}
		}
	}
	const byte *getData() const {
		return data;
	}
	u32 getW() const {
		return w;
	}
	u32 getH() const {
		return h;
	}
};
class fontFreeType_c : public fontBase_c {
	glyphInfo_s glyphs[256];
	mtrAPI_i *mat;
	float maxHeight;
public:
	fontFreeType_c(const char *s, FT_Face face) : fontBase_c(s) {
		FT_Set_Pixel_Sizes(face, 0, 14);

		FT_GlyphSlot g = face->glyph;
		maxHeight = 0;

		for(u32 i = 0; i < 128; i++) {
			char ch = i;

			if(ch == '\r') {
				continue;
			}

			/* Try to load and render the character */
			if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
				continue;

			glyphInfo_s &gl = glyphs[i];
			gl.width = g->bitmap.width;
			gl.rows = g->bitmap.rows;
			gl.left = g->bitmap_left;
			gl.top = g->bitmap_top;
			gl.advance_x = g->advance.x;
			gl.advance_y = g->advance.y;
			gl.sizes[0] = gl.width;
			gl.sizes[1] = gl.rows;

			if(gl.rows > maxHeight) {
				maxHeight = gl.rows;
			}
		}


		textureAllocator_c allocator;

		bool bFailed;
		u32 textureSize;
		for(textureSize = 128; textureSize <= 1024; textureSize *= 2) {
			bFailed = false;
			allocator.setupTextureSize(textureSize);
			for(u32 i = 0; i < 128 && !bFailed; i++) {
				char ch = i;

				if(ch == '\r') {
					continue;
				}
				glyphInfo_s &gl = glyphs[i];
				if(allocator.allocTextureBlock(gl.sizes,gl.pos)) {
					bFailed = true;
					break;
				}
			}
			if(!bFailed) {
				break;
			}
		}


		image8bit_c imgData;
		imgData.setSize(textureSize);

		for(u32 i = 0; i < 128; i++) {
			char ch = i;

			if(ch == '\r') {
				continue;
			}
			/* Try to load and render the character */
			if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
				continue;

			glyphInfo_s &gl = glyphs[i];

			imgData.setSubImage(gl.pos[0],gl.pos[1],gl.sizes[0],gl.sizes[1],g->bitmap.buffer);

			gl.s1 = float(gl.pos[0]+gl.sizes[0])/ float(textureSize);
			gl.s0 = float(gl.pos[0])/ float(textureSize);
			gl.t0 = float(gl.pos[1])/ float(textureSize);
			gl.t1 = float(gl.pos[1]+gl.sizes[1])/ float(textureSize);
		}
		g_ms->createTexture(getName(),imgData.getW(),imgData.getH(),imgData.getData(),1);
		mat = g_ms->registerMaterial(getName());
		bIsValid = true;
	}
	virtual float drawChar(float x, float y, char ch, float sx = 1.f, float sy = 1.f) const {
		const glyphInfo_s &gl = glyphs[ch];

		float s0 = gl.s0;
		float t0 = gl.t0;
		float s1 = gl.s1;
		float t1 = gl.t1;

		
		float w = gl.width * sx;
		float h = gl.rows * sy;
		float x2 = x + gl.left * sx;
		float y2 = y - gl.top * sy + maxHeight;

		rf->drawStretchPic(x2,y2,w,h,s0,t0,s1,t1,mat);
	

		return (gl.advance_x >> 6) * sx;
	}
	virtual float getStringWidth(const char *s) const {
		const char *p = s;
		float nowX = 0;
		float nowY = 0;
		while(*p) {
			int ch = *p;
			const glyphInfo_s &gl = glyphs[ch];

			float sx = 1.f;
			float sy = 1.f;
			
			float w = gl.width * sx;
			float h = gl.rows * sy;
			float x2 = nowX + gl.left * sx;
			float y2 = nowY - gl.top * sy + maxHeight;

		
			nowX += (gl.advance_x >> 6) * sx;
			nowY += (gl.advance_y >> 6) * sy;
			p++;
		}
		return nowX;
	}
	virtual float getStringHeight(const char *s) const {
		return 0; // TODO
	}
	virtual void drawString(float x, float y, const char *s, float sx = 1.f, float sy = 1.f, bool bUseColourCodes = true) const {

		const char *p = s;
		float nowX = x;
		float nowY = y;
		while(*p) {
			if(bUseColourCodes) {
				p = checkForColourCode(p);
				if(p == 0 || *p == 0)
					break;
			}
			int ch = *p;

			nowX += drawChar(nowX,nowY,ch);
			p++;
		}
		if(bUseColourCodes) {
			// set back the default colour (white)
			rb->setColor4(0);
		}
	}
};

class fontBad_c : public fontBase_c {
public:
	fontBad_c(const char *s) : fontBase_c(s) {
		bIsValid = false;
	}
	virtual float drawChar(float x, float y, char ch, float sx = 1.f, float sy = 1.f) const {
		return 0.f;
	}
	virtual float getStringWidth(const char *s) const {
		return 0.f;
	}
	virtual float getStringHeight(const char *s) const {
		return 0; // TODO
	}
	virtual void drawString(float x, float y, const char *s, float sx = 1.f, float sy = 1.f, bool bUseColourCodes = true) const {

	}
};
hashTableTemplateExt_c<fontBase_c> rf_fonts;

fontAPI_i *RF_RegisterFont(const char *name) {
	if(0) {
		return 0;
	}
	fontBase_c *f = rf_fonts.getEntry(name);
	if(f) {	
		if(f->isValid() == false)
			return 0;
		return f;
	}

	str path = "fonts/";
	path.append(name);
	path.append(".RitualFont");
	str ritualFontMaterialPath = "gfx/fonts/";
	ritualFontMaterialPath.append(name);
	if(g_vfs->FS_FileExists(path) && g_ms->isMaterialOrImagePresent(ritualFontMaterialPath)) {
		f = new fontRitual_c(name);
	} else {
		if(rf_ft) {
			const char *fontDirs[] = {
				"Fonts/",
				"C:/Windows/Fonts/",
			};
			u32 numFontDirs = sizeof(fontDirs)/sizeof(fontDirs[0]);
			for(u32 i = 0; i < numFontDirs; i++) {
				path = fontDirs[i];
				path.append(name);
				path.setExtension("ttf");
				FT_Face face;
				if(!FT_New_Face(rf_ft, path.c_str(), 0, &face)) {
					// valid
					g_core->Print("RF_RegisterFont: loaded font from %s\n",path.c_str());
					f = new fontFreeType_c(name,face);
					break;
				}
			}
		}
	}
#if 0
	if(f == 0)
		return 0;
#else
	if(f == 0) {
		f = new fontBad_c(name);
	}
#endif
	rf_fonts.addObject(f);
	if(f->isValid() == false)
		return 0;
	return f;
}
void RF_InitFonts() {
	if(FT_Init_FreeType(&rf_ft)) {
		g_core->RedWarning("RF_InitFonts: FT_Init_FreeType failed. Cannot init fonts\n");
		return;
	}
	g_core->RedWarning("RF_InitFonts: FreeType font library successfully loaded!\n");
}
void RF_ShutdownFonts() {
	for(u32 i = 0; i < rf_fonts.size(); i++) {
		delete rf_fonts[i];
	}
	rf_fonts.clear();
	if(rf_ft) {
		FT_Done_FreeType(rf_ft);
		rf_ft = 0;
	}
}
