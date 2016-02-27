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

#include <ft2build.h>
#include FT_FREETYPE_H
#pragma comment(lib,"freetype.lib")

FT_Library rf_ft;

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
class textureAllocator_c {
	u32 maxTextureSize;
	arraySTD_c<u32> allocated;
public:
	textureAllocator_c() {
		setupTextureSize(64);
	}
	void setupTextureSize(u32 newSize) {
		maxTextureSize = newSize;
		allocated.resize(maxTextureSize);
		allocated.nullMemory();
	}
	bool allocTextureBlock(const u32 inSizes[2], u32 outPos[2]) {
		u32 margin = 1;
		u32 sizes[2] = { inSizes[0]+margin, inSizes[1]+margin };
		u32 best = maxTextureSize;
		for(u32 i = 0; i <= maxTextureSize-sizes[0]; i++ ) {
			u32 best2 = 0;
			u32 j;
			for(j = 0; j < sizes[0]; j++) {
				if(allocated[i+j] >= best) {
					break;
				}
				if(allocated[i+j] > best2) {
					best2 = allocated[i+j];
				}
			}
			if(j == sizes[0])	{	// this is a valid spot
				outPos[0] = i;
				outPos[1] = best = best2;
			}
		}

		if(best + sizes[1] > maxTextureSize) {
			return true;
		}

		for(u32 i = 0; i < sizes[0]; i++) {
			allocated[outPos[0] + i] = best + sizes[1];
		}

		return false;
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
	}
	virtual void drawString(float x, float y, const char *s) const {

		const char *p = s;
		float nowX = x;
		float nowY = y;
		float sx = 1.f;
		float sy = 1.f;
		while(*p) {
			int ch = *p;
		
			const glyphInfo_s &gl = glyphs[ch];

			float s0 = gl.s0;
			float t0 = gl.t0;
			float s1 = gl.s1;
			float t1 = gl.t1;

			
			float w = gl.width * sx;
			float h = gl.rows * sy;
			float x2 = nowX + gl.left * sx;
			float y2 = nowY - gl.top * sy + maxHeight;

			rf->drawStretchPic(x2,y2,w,h,s0,t0,s1,t1,mat);
		
			nowX += (gl.advance_x >> 6) * sx;
			nowY += (gl.advance_y >> 6) * sy;
			p++;
		}
	}
};
hashTableTemplateExt_c<fontBase_c> rf_fonts;

fontAPI_i *RF_RegisterFont(const char *name) {
	if(0) {
		return 0;
	}
	fontBase_c *f = rf_fonts.getEntry(name);
	if(f)
		return f;
	str path = "fonts/";
	path.append(name);
	path.append(".RitualFont");
	if(g_vfs->FS_FileExists(path)) {
		f = new fontRitual_c(name);
	} else {
		path = "C:/Windows/Fonts/";
		path.append(name);
		path.setExtension("ttf");
		FT_Face face;
		if(!FT_New_Face(rf_ft, path.c_str(), 0, &face)) {
			// valid
			f = new fontFreeType_c(name,face);
		}
	}
	if(f == 0)
		return 0;
	rf_fonts.addObject(f);
	return f;
}
void RF_InitFonts() {
	if(FT_Init_FreeType(&rf_ft)) {
		g_core->RedWarning("RF_InitFonts: FT_Init_FreeType failed. Cannot init fonts\n");
		return;
	}
	g_core->RedWarning("RF_InitFonts: FreeType font library successfully loaded!\n");
}
