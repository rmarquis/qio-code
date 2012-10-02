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
// sdl_gl.cpp - SDL / openGL backend

#define NO_SDL_GLEXT
#include <windows.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include "../client/client.h"
#include "../sys/sys_local.h"
#include "../renderer/qgl.h"
#include "sdl_icon.h"
#include <api/rbAPI.h>
#include <api/iFaceMgrAPI.h>
#include <api/textureAPI.h>
#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <shared/r2dVert.h>

#include "sdl_glConfig.h"


void GLimp_Init();
void GLimp_Shutdown();
void GLimp_EndFrame();

class rbSDLOpenGL_c : public rbAPI_i {
	mtrAPI_i *lastMat;
public:
	rbSDLOpenGL_c() {
		lastMat = 0;
	}
	void checkErrors() {
		int		err;
		char	s[64];

		err = glGetError();
		if ( err == GL_NO_ERROR ) {
			return;
		}
		switch( err ) {
			case GL_INVALID_ENUM:
				strcpy( s, "GL_INVALID_ENUM" );
				break;
			case GL_INVALID_VALUE:
				strcpy( s, "GL_INVALID_VALUE" );
				break;
			case GL_INVALID_OPERATION:
				strcpy( s, "GL_INVALID_OPERATION" );
				break;
			case GL_STACK_OVERFLOW:
				strcpy( s, "GL_STACK_OVERFLOW" );
				break;
			case GL_STACK_UNDERFLOW:
				strcpy( s, "GL_STACK_UNDERFLOW" );
				break;
			case GL_OUT_OF_MEMORY:
				strcpy( s, "GL_OUT_OF_MEMORY" );
				break;
			default:
				Com_sprintf( s, sizeof(s), "%i", err);
				break;
		}
		Com_Printf("GL_CheckErrors: %s\n", s );
	}

	virtual void setMaterial(class mtrAPI_i *mat) {
		lastMat = mat;
	}
	virtual void setColor4(const float *rgba)  {
		if(rgba == 0) {
			float def[] = { 1, 1, 1, 1 };
			rgba = def;
		}
		glColor4fv(rgba);
	}
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices)  {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2,GL_FLOAT,sizeof(r2dVert_s),verts);
		glTexCoordPointer(2,GL_FLOAT,sizeof(r2dVert_s),&verts->texCoords.x);
		checkErrors();
		if(lastMat) {
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			// NOTE: this is the way Q3 draws all the 2d menu graphics
			// (it worked before with bigchars.tga, even when the bigchars
			// shader was missing!!!)
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			for(u32 i = 0; i < lastMat->getNumStages(); i++) {
				const mtrStageAPI_i *s = lastMat->getStage(i);
				textureAPI_i *t = s->getTexture();
				u32 handle = t->getInternalHandleU32();
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D,handle);
				checkErrors();
				glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);
				checkErrors();
			}
			glDisable(GL_BLEND);
			glDisable(GL_TEXTURE_2D);
		} else {
			glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	virtual void beginFrame() {
		// NOTE: for stencil shadows, stencil buffer should be cleared here as well.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0,0,0,0);
	}
	virtual void endFrame() {
		GLimp_EndFrame();
	}	
	virtual void setup2DView() {
		glViewport(0,0,getWinWidth(),getWinHeight());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho (0, getWinWidth(), getWinHeight(), 0, 0, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	virtual void init()  {
		GLimp_Init();
		u32 res = glewInit();
		if (GLEW_OK != res) {
			Com_Error(ERR_DROP,"rbSDLOpenGL_c::init: glewInit() failed. Cannot init openGL renderer\n");
			return;
		}

	}
	virtual void shutdown()  {
		GLimp_Shutdown();
	}
	virtual u32 getWinWidth() const {
		return glConfig.vidWidth;
	}
	virtual u32 getWinHeight() const {
		return glConfig.vidHeight;
	}
	virtual void uploadTextureRGBA(class textureAPI_i *out, const byte *data, u32 w, u32 h) {
		out->setWidth(w);
		out->setHeight(h);
		u32 texID;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
#if 0
		// wtf it doesnt work?
		glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
#else
		gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,w,h,GL_RGBA,GL_UNSIGNED_BYTE, data);
#endif
		checkErrors();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
		//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		checkErrors();
		glBindTexture(GL_TEXTURE_2D, 0);	
		out->setInternalHandleU32(texID);
	}
	virtual void freeTextureData(class textureAPI_i *tex) {
		u32 handle = tex->getInternalHandleU32();
		glDeleteTextures(1,&handle);
		tex->setInternalHandleU32(0);
	}
};

static rbSDLOpenGL_c g_staticSDLOpenGLBackend;

void SDLOpenGL_RegisterBackEnd() {
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)&g_staticSDLOpenGLBackend,RB_SDLOPENGL_API_IDENTSTR);
}