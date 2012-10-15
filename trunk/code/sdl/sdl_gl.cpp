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
#include <math/matrix.h>
#include <math/axis.h>

#include "sdl_glConfig.h"

#include <materialSystem/mat_public.h> // alphaFunc_e etc
#include <renderer/rVertexBuffer.h>
#include <renderer/rIndexBuffer.h>


void GLimp_Init();
void GLimp_Shutdown();
void GLimp_EndFrame();

#define MAX_TEXTURE_SLOTS 32

struct texState_s {
	bool enabledTexture2D;
	bool texCoordArrayEnabled;
	u32 index;
	matrixExt_c mat;

	texState_s() {
		enabledTexture2D = false;
		texCoordArrayEnabled = false;
		index = 0;
	}
};

class rbSDLOpenGL_c : public rbAPI_i {
	// gl state
	texState_s texStates[MAX_TEXTURE_SLOTS];
	int curTexSlot;
	int highestTCSlotUsed;
	// materials
	safePtr_c<mtrAPI_i> lastMat;
	textureAPI_i *lastLightmap;
	bool bindVertexColors;
	// matrices
	matrix_c worldModelMatrix;
	matrix_c resultMatrix;
	bool usingWorldSpace;
	axis_c entityAxis;
	vec3_c entityOrigin;
	matrix_c entityMatrix;
	
	bool boundVBOVertexColors;
	const rVertexBuffer_c *boundVBO;

	// counters
	u32 c_frame_vbsReusedByDifferentDrawCall;

public:
	rbSDLOpenGL_c() {
		lastMat = 0;
		lastLightmap = 0;
		bindVertexColors = 0;
		curTexSlot = 0;
		highestTCSlotUsed = 0;
		boundVBO = 0;
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
	// GL_COLOR_ARRAY changes
	bool colorArrayActive;
	void enableColorArray() {
		if(colorArrayActive == true)
			return;
		glEnableClientState(GL_COLOR_ARRAY);
		colorArrayActive = true;
	}
	void disableColorArray() {
		if(colorArrayActive == false)
			return;
		glDisableClientState(GL_COLOR_ARRAY);
		colorArrayActive = false;
	}
	//
	// texture changes
	//
	void selectTex(int slot) {
		if(slot+1 > highestTCSlotUsed) {
			highestTCSlotUsed = slot+1;
		}
		if(curTexSlot != slot) {
			glActiveTexture(GL_TEXTURE0 + slot);
			glClientActiveTexture(GL_TEXTURE0 + slot);
			curTexSlot = slot;
		}
	}
	void bindTex(int slot, u32 tex) {
		texState_s *s = &texStates[slot];
		if(s->enabledTexture2D == true && s->index == tex)
			return;

		selectTex(slot);
		if(s->enabledTexture2D == false) {
			glEnable(GL_TEXTURE_2D);
			s->enabledTexture2D = true;
		}
		if(s->index != tex) {
			glBindTexture(GL_TEXTURE_2D,tex);
			s->index = tex;
		}
	}
	void disableAllTextures() {
		texState_s *s = &texStates[0];
		for(int i = 0; i < highestTCSlotUsed; i++, s++) {
			if(s->enabledTexture2D == false && s->index == 0)
				continue;
			selectTex(i);
			if(s->index != 0) {
				glBindTexture(GL_TEXTURE_2D,0);
				s->index = 0;
			}
			if(s->enabledTexture2D) {
				glDisable(GL_TEXTURE_2D);
				s->enabledTexture2D = false;
			}
		}
		highestTCSlotUsed = -1;
	}
	//
	// alphaFunc changes
	//
	alphaFunc_e prevAlphaFunc;
	void setAlphaFunc(alphaFunc_e newAlphaFunc) {
		if(prevAlphaFunc == newAlphaFunc) {
			return; // no change
		}
		if(newAlphaFunc == AF_NONE) {
			glDisable(GL_ALPHA_TEST);
		} else if(newAlphaFunc == AF_GT0) {
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc( GL_GREATER, 0.0f ); 
		} else if(newAlphaFunc == AF_GE128) {
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc( GL_GREATER, 0.5f ); 
		} else if(newAlphaFunc == AF_LT128) {
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc( GL_LESS, 0.5f ); 
		}
		prevAlphaFunc = newAlphaFunc;
	}
	void turnOffAlphaFunc() {
		setAlphaFunc(AF_NONE);
	}
	// tex coords arrays
	void enableTexCoordArrayForCurrentTexSlot() {
		texState_s *s = &texStates[curTexSlot];
		if(s->texCoordArrayEnabled==true)
			return;
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		s->texCoordArrayEnabled = true;
	}
	void disableTexCoordArrayForCurrentTexSlot() {
		texState_s *s = &texStates[curTexSlot];
		if(s->texCoordArrayEnabled==false)
			return;
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		s->texCoordArrayEnabled = false;
	}
	void disableTexCoordArrayForTexSlot(u32 slotNum) {
		texState_s *s = &texStates[slotNum];
		if(s->texCoordArrayEnabled==false)
			return;
		selectTex(slotNum);
		disableTexCoordArrayForCurrentTexSlot();
	}
	// texture matrices
	void turnOffTextureMatrices() {
		for(u32 i = 0; i < MAX_TEXTURE_SLOTS; i++) {
			setTextureMatrixIdentity(i);
		}
	}
	void setTextureMatrixCustom(u32 slot, const matrix_c &mat) {
		texState_s *ts = &this->texStates[slot];
		if(ts->mat.set(mat) == false)
			return; // no change

		this->selectTex(slot);
		glPushAttrib(GL_TRANSFORM_BIT);
		glMatrixMode(GL_TEXTURE);
		glLoadMatrixf(mat);
		glPopAttrib();
	}
	void setTextureMatrixIdentity(u32 slot) {
		texState_s *ts = &this->texStates[slot];
		if(ts->mat.isIdentity())
			return; // no change
		ts->mat.setIdentity();

		this->selectTex(slot);
		glPushAttrib(GL_TRANSFORM_BIT);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glPopAttrib();
	}
	virtual void setMaterial(class mtrAPI_i *mat, class textureAPI_i *lightmap) {
		lastMat = mat;
		lastLightmap = lightmap;
	}
	virtual void unbindMaterial() {
		disableAllTextures();
		turnOffAlphaFunc();
		disableColorArray();
		lastMat = 0;
		lastLightmap = 0;
	}
	virtual void setColor4(const float *rgba)  {
		if(rgba == 0) {
			float def[] = { 1, 1, 1, 1 };
			rgba = def;
		}
		glColor4fv(rgba);
	}
	virtual void setBindVertexColors(bool bBindVertexColors) {
		this->bindVertexColors = bBindVertexColors;
	}
	void bindVertexBuffer(const class rVertexBuffer_c *verts) {
		if(boundVBO == verts) {
			if(boundVBOVertexColors == bindVertexColors) {
				c_frame_vbsReusedByDifferentDrawCall++;
				return; // already bound
			} else {

			}
		}
		u32 h = verts->getInternalHandleU32();
		glBindBuffer(GL_ARRAY_BUFFER,h);
		if(h == 0) {
			// buffer wasnt uploaded to GPU
			glVertexPointer(3,GL_FLOAT,sizeof(rVert_c),verts->getArray());
			selectTex(0);
			enableTexCoordArrayForCurrentTexSlot();
			glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),&verts->getArray()->tc.x);
			selectTex(1);
			enableTexCoordArrayForCurrentTexSlot();
			glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),&verts->getArray()->lc.x);
			if(bindVertexColors) {
				enableColorArray();
				glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(rVert_c),&verts->getArray()->color[0]);
			} else {
				disableColorArray();
			}
		} else {
			glVertexPointer(3,GL_FLOAT,sizeof(rVert_c),0);
			selectTex(0);
			enableTexCoordArrayForCurrentTexSlot();
			glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),(void*)offsetof(rVert_c,tc));
			selectTex(1);
			enableTexCoordArrayForCurrentTexSlot();
			glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),(void*)offsetof(rVert_c,lc));
			if(bindVertexColors) {
				enableColorArray();
				glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(rVert_c),(void*)offsetof(rVert_c,color));
			} else {
				disableColorArray();
			}
		}
		boundVBOVertexColors = bindVertexColors;
		boundVBO = verts;
	}
	void unbindVertexBuffer() {
		if(boundVBO == 0)
			return;
		if(boundVBO->getInternalHandleU32()) {
			glBindBuffer(GL_ARRAY_BUFFER,0);
		}
		glVertexPointer(3,GL_FLOAT,sizeof(rVert_c),0);
		selectTex(0);
		disableTexCoordArrayForCurrentTexSlot();
		glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),0);
		selectTex(1);
		disableTexCoordArrayForCurrentTexSlot();
		glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),0);
		glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(rVert_c),0);
		disableColorArray();
	}
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices)  {
		glVertexPointer(2,GL_FLOAT,sizeof(r2dVert_s),verts);
		selectTex(0);
		enableTexCoordArrayForCurrentTexSlot();
		glTexCoordPointer(2,GL_FLOAT,sizeof(r2dVert_s),&verts->texCoords.x);
		checkErrors();
		if(lastMat) {
			glEnable(GL_BLEND);
			// NOTE: this is the way Q3 draws all the 2d menu graphics
			// (it worked before with bigchars.tga, even when the bigchars
			// shader was missing!!!)
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			for(u32 i = 0; i < lastMat->getNumStages(); i++) {
				const mtrStageAPI_i *s = lastMat->getStage(i);
				setAlphaFunc(s->getAlphaFunc());
				textureAPI_i *t = s->getTexture();
				bindTex(0,t->getInternalHandleU32());
				checkErrors();
				glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);
				checkErrors();
			}
			glDisable(GL_BLEND);
		} else {
			glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);
		}
	}

	virtual void drawElements(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		bindVertexBuffer(&verts);

		checkErrors();
		if(lastMat) {
			for(u32 i = 0; i < lastMat->getNumStages(); i++) {
				const mtrStageAPI_i *s = lastMat->getStage(i);
				setAlphaFunc(s->getAlphaFunc());
				textureAPI_i *t = s->getTexture();
				bindTex(0,t->getInternalHandleU32());
				if(lastLightmap) {
					bindTex(1,lastLightmap->getInternalHandleU32());
				} else {
					bindTex(1,0);
				}
				glDrawElements(GL_TRIANGLES, indices.getNumIndices(), indices.getGLIndexType(), indices.getVoidPtr());
				checkErrors();
			}
		} else {
			glDrawElements(GL_TRIANGLES, indices.getNumIndices(), indices.getGLIndexType(), indices.getVoidPtr());
		}
	}
	virtual void beginFrame() {
		// NOTE: for stencil shadows, stencil buffer should be cleared here as well.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0,0,0,0);

		// reset counters
		c_frame_vbsReusedByDifferentDrawCall = 0;
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
		
		// depth test is not needed while drawing 2d graphics
		glDisable(GL_DEPTH_TEST);
		// rVertexBuffers are used only for 3d
		unbindVertexBuffer();
	}
	matrix_c currentModelViewMatrix;
	void loadModelViewMatrix(const matrix_c &newMat) {
		//if(newMat.compare(currentModelViewMatrix)) 
		//	return;
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(newMat);
		currentModelViewMatrix = newMat;
	}
	virtual void setupWorldSpace() {
		resultMatrix = worldModelMatrix;

		this->loadModelViewMatrix(this->resultMatrix);

		usingWorldSpace = true;
	}
	virtual void setupEntitySpace(const axis_c &axis, const vec3_c &origin) {
		entityAxis = axis;
		entityOrigin = origin;

		entityMatrix.fromAxisAndOrigin(axis,origin);

		this->resultMatrix = this->worldModelMatrix * entityMatrix;

		this->loadModelViewMatrix(this->resultMatrix);

		usingWorldSpace = false;
	}
	virtual void setupEntitySpace2(const class vec3_c &angles, const class vec3_c &origin) {
		axis_c ax;
		ax.fromAngles(angles);
		setupEntitySpace(ax,origin);
	}
	virtual bool isUsingWorldSpace() const {
		return usingWorldSpace;
	}
	virtual const matrix_c &getEntitySpaceMatrix() const {
		// we dont need to call it when usingWorldSpace == true, 
		// just use identity matrix
		assert(usingWorldSpace == false);
		return entityMatrix;
	}
	virtual void setup3DView(const class vec3_c &newCamPos, const class axis_c &newCamAxis) {
		// transform by the camera placement and view axis
		this->worldModelMatrix.invFromAxisAndVector(newCamAxis,newCamPos);
		// convert to gl coord system
		this->worldModelMatrix.toGL();

		setupWorldSpace();

		glEnable(GL_DEPTH_TEST);
	}
	virtual void setupProjection3D(const projDef_s *pd) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		matrix_c proj;
		float viewPortW = this->getWinWidth();
		float viewPortH = this->getWinHeight();
		float x = viewPortW / tan( pd->fovX / 360 * M_PI );
		float fovY = atan2( viewPortH, x );
		fovY = fovY * 360 / M_PI;
		//frustum.setup(fovX, fovY, zFar, axis, origin);
		proj.setupProjection(pd->fovX,fovY,pd->zNear,pd->zFar);
		glLoadMatrixf(proj);
	}
	virtual void drawCapsuleZ(const float *xyz, float h, float w) {
		//glDisable(GL_DEPTH_TEST);
		glTranslatef(xyz[0],xyz[1],xyz[2]);
		// draw lower sphere (sliding on the ground)
		glTranslatef(0,0,-(h*0.5f));
		glutSolidSphere(w,12,12);
		glTranslatef(0,0,(h*0.5f));
		// draw upper sphere (player's "head")
		glTranslatef(0,0,(h*0.5f));
		glutSolidSphere(w,12,12);
		glTranslatef(0,0,-(h*0.5f));
		//glutSolidSphere(32,16,16);

			// draw 'body'
		glTranslatef(0,0,-(h*0.5f));
		GLUquadricObj *obj = gluNewQuadric();
		gluCylinder(obj, w, w, h, 30, 30);
		gluDeleteQuadric(obj);
		glTranslatef(0,0,(h*0.5f));
		glTranslatef(-xyz[0],-xyz[1],-xyz[2]);
	}
	virtual void drawBoxHalfSizes(const float *halfSizes) {
		glutSolidCube(halfSizes[0]*2);
	}
	virtual void init()  {
		GLimp_Init();
		u32 res = glewInit();
		if (GLEW_OK != res) {
			Com_Error(ERR_DROP,"rbSDLOpenGL_c::init: glewInit() failed. Cannot init openGL renderer\n");
			return;
		}
		// ensure that all the states are reset after vid_restart
		memset(texStates,0,sizeof(texStates));
		curTexSlot = -1;
		highestTCSlotUsed = 0;
		// materials
		lastMat = 0;
		lastLightmap = 0;
		bindVertexColors = 0;
		//glShadeModel( GL_SMOOTH );
		//glDepthFunc( GL_LEQUAL );
		
		glEnableClientState(GL_VERTEX_ARRAY);


#if 1
		selectTex(1);
		// increase lightmap brightness. They are very dark when vertex colors are enabled.
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);

		//	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);

		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_MODULATE);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, GL_TEXTURE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_PREVIOUS_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, GL_PREVIOUS_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 4.0f);
		selectTex(0);
#endif
	}
	virtual void shutdown()  {
		lastMat = 0;
		lastLightmap = 0;
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
	virtual void uploadLightmapRGB(class textureAPI_i *out, const byte *data, u32 w, u32 h) {
		out->setWidth(w);
		out->setHeight(h);
		u32 texID;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);	
		checkErrors();
		glBindTexture(GL_TEXTURE_2D, 0);
		out->setInternalHandleU32(texID);	
	}
	virtual bool createVBO(class rVertexBuffer_c *vbo) {
		if(vbo->getInternalHandleU32()) {
			destroyVBO(vbo);
		}
		if(glGenBuffers == 0)
			return true; // VBO not supported
		u32 h;
		glGenBuffers(1,&h);
		vbo->setInternalHandleU32(h);
		glBindBuffer(GL_ARRAY_BUFFER, h);
		glBufferData(GL_ARRAY_BUFFER, sizeof(rVert_c)*vbo->size(), vbo->getArray(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return false; // ok
	}
	virtual bool destroyVBO(class rVertexBuffer_c *vbo) {
		if(vbo == 0)
			return true; // NULL ptr
		if(glDeleteBuffers == 0)
			return true; // no VBO support
		u32 h = vbo->getInternalHandleU32();
		if(h == 0)
			return true; // buffer wasnt uploaded to gpu
		glDeleteBuffers(1,&h);
		vbo->setInternalHandleU32(0);
		return false; // ok
	}

};

static rbSDLOpenGL_c g_staticSDLOpenGLBackend;

void SDLOpenGL_RegisterBackEnd() {
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)&g_staticSDLOpenGLBackend,RB_SDLOPENGL_API_IDENTSTR);
}