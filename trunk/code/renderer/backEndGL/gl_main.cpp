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
// gl_main.cpp - SDL / openGL backend

#include "gl_local.h"
#include "gl_shader.h"
#include <api/coreAPI.h>
#include <api/rbAPI.h>
#include <api/iFaceMgrAPI.h>
#include <api/textureAPI.h>
#include <api/cubeMapAPI.h>
#include <api/mtrAPI.h>
#include <api/mtrStageAPI.h>
#include <api/sdlSharedAPI.h>
#include <api/materialSystemAPI.h>
#include <api/rAPI.h>
#include <shared/r2dVert.h>
#include <math/matrix.h>
#include <math/axis.h>
#include <math/aabb.h>
#include <math/plane.h>

#include <materialSystem/mat_public.h> // alphaFunc_e etc
#include <renderer/rVertexBuffer.h>
#include <renderer/rIndexBuffer.h>
#include <renderer/rPointBuffer.h>

#include <shared/cullType.h>
#include <renderer/drawCallSort.h>

#include <shared/autoCvar.h>
#include <api/rLightAPI.h>
#include <api/occlusionQueryAPI.h>

#include <shared/byteRGB.h>
#include <shared/textureWrapMode.h>
#include <shared/fColor4.h>
#include <shared/ast_input.h>

static aCvar_c rb_showTris("rb_showTris","0");
static aCvar_c rb_showNormals("rb_showNormals","0");
// use a special GLSL shader to show normal vectors as colors
static aCvar_c rb_showNormalColors("rb_showNormalColors","0");
static aCvar_c gl_callGLFinish("gl_callGLFinish","0");
static aCvar_c gl_checkForGLErrors("gl_checkForGLErrors","1");
static aCvar_c rb_printMemcpyVertexArrayBottleneck("rb_printMemcpyVertexArrayBottleneck","0");
static aCvar_c rb_gpuTexGens("rb_gpuTexGens","0");
static aCvar_c rb_ignoreRGBGens("rb_ignoreRGBGens","1");
static aCvar_c rb_ignoreRGBGenVertex("rb_ignoreRGBGenVertex","0");
static aCvar_c rb_ignoreRGBGenWave("rb_ignoreRGBGenWave","1");
static aCvar_c rb_printRGBGenWaveMaterials("rb_printRGBGenWaveMaterials","0");
static aCvar_c rb_ignoreRGBGenConst("rb_ignoreRGBGenConst","0");
static aCvar_c rb_ignoreRGBGenAST("rb_ignoreRGBGenAST","0");
// always use GLSL shaders, even if they are not needed for any material effects
static aCvar_c gl_alwaysUseGLSLShaders("gl_alwaysUseGLSLShaders","0");
static aCvar_c rb_showDepthBuffer("rb_showDepthBuffer","0");
static aCvar_c rb_verboseDrawElements("rb_verboseDrawElements","0");
static aCvar_c rb_ignoreBumpMaps("rb_ignoreBumpMaps","0");
static aCvar_c rb_ignoreHeightMaps("rb_ignoreHeightMaps","0");
static aCvar_c rb_ignoreDeluxeMaps("rb_ignoreDeluxeMaps","0");
// use relief mapping raycast function to handle heightMaps
// (if disabled, a simple bad-looking trick is used instead)
static aCvar_c rb_useReliefMapping("rb_useReliefMapping","1");
static aCvar_c rb_showBumpMaps("rb_showBumpMaps","0");
static aCvar_c rb_showHeightMaps("rb_showHeightMaps","0");
static aCvar_c rb_showLightMaps("rb_showLightMaps","0");
static aCvar_c rb_showDeluxeMaps("rb_showDeluxeMaps","0");
static aCvar_c rb_verboseBindShader("rb_verboseBindShader","0");
static aCvar_c rb_ignoreLightmaps("rb_ignoreLightmaps","0");
static aCvar_c rb_printFrameTriCounts("rb_printFrameTriCounts","0");
static aCvar_c rb_printFrameVertCounts("rb_printFrameVertCounts","0");
static aCvar_c rb_ignoreDrawCalls3D("rb_ignoreDrawCalls3D","0");
// enables DEBUG_IGNOREANGLEFACTOR macro for all shaders
static aCvar_c rb_dynamicLighting_ignoreAngleFactor("rb_dynamicLighting_ignoreAngleFactor","0");
// enables DEBUG_IGNOREDISTANCEFACTOR macro for all shaders
static aCvar_c rb_dynamicLighting_ignoreDistanceFactor("rb_dynamicLighting_ignoreDistanceFactor","0");
static aCvar_c rb_debugStageConditions("rb_debugStageConditions","0");
static aCvar_c rb_showShadowVolumes("rb_showShadowVolumes","0");
// used to see which surfaces has a material with blendfunc
static aCvar_c rb_skipStagesWithoutBlendFunc("rb_skipStagesWithoutBlendFunc","0");
static aCvar_c rb_skipStagesWithBlendFunc("rb_skipStagesWithBlendFunc","0");
static aCvar_c rb_ignoreDoom3AlphaTest("rb_ignoreDoom3AlphaTest","0");
static aCvar_c rb_printMaterialDepthWrite("rb_printMaterialDepthWrite","0");
static aCvar_c rb_forceTwoSided("rb_forceTwoSided","0");
static aCvar_c rb_printLightingPassDrawCalls("rb_printLightingPassDrawCalls","0");
static aCvar_c rb_shadowMapBlur("rb_shadowMapBlur","1");
static aCvar_c rb_printBlendAfterLightingDrawCalls("rb_printBlendAfterLightingDrawCalls","0");
static aCvar_c rb_printIBOStats("rb_printIBOStats","0");
static aCvar_c rb_forceBlur("rb_forceBlur","0");
static aCvar_c rb_useDepthCubeMap("rb_useDepthCubeMap","0");
static aCvar_c rb_printShadowVolumeDrawCalls("rb_printShadowVolumeDrawCalls","0");
static aCvar_c rb_skipMaterialsWithCubeMaps("rb_skipMaterialsWithCubeMaps","0");
static aCvar_c rb_skipMirrorMaterials("rb_skipMirrorMaterials","0");
static aCvar_c gl_ignoreClipPlanes("gl_ignoreClipPlanes","0");
static aCvar_c gl_clipPlaneEpsilon("gl_clipPlaneEpsilon","0");
static aCvar_c rb_blurScale("rb_blurScale","0.2");
static aCvar_c rb_forceBloom("rb_forceBloom","0");
static aCvar_c rb_showSpotLightShadows("rb_showSpotLightShadows","0");
static aCvar_c rb_showPointLightShadows("rb_showPointLightShadows","0");

#define MAX_TEXTURE_SLOTS 32

#ifndef offsetof
#ifdef  _WIN64
#define offsetof(s,m)   (size_t)( (ptrdiff_t)&reinterpret_cast<const volatile char&>((((s *)0)->m)) )
#else
#define offsetof(s,m)   (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
#endif
#endif // not defined offsetof

class glOcclusionQuery_c : public occlusionQueryAPI_i {
	u32 oqHandle;
	bool waitingForResult;
	mutable u32 lastResult;
public:
	glOcclusionQuery_c();
	~glOcclusionQuery_c();
	virtual void generateOcclusionQuery();
	virtual void assignSphereQuery(const vec3_c &p, float radius);
	virtual u32 getNumSamplesPassed() const;
	virtual bool isResultAvailable() const;
	virtual u32 getPreviousResult() const;
	virtual u32 waitForLatestResult() const;
};
// fbo-replacement for screen buffer
class fboScreen_c {
	u32 textureHandle;
	u32 fboHandle;
	u32 depthBufferHandle;
	u32 w,h;
public:
	fboScreen_c();
	~fboScreen_c();

	bool create(u32 newW, u32 newH);
	void destroy();

	u32 getFBOHandle() {
		return fboHandle;
	}
	u32 getTextureHandle() {
		return textureHandle;
	}
	u32 getW() const {
		return w;
	}
	u32 getH() const {
		return h;
	}
};
// depth-only FBO
class fboDepth_c {
	u32 textureHandle;
	u32 fboHandle;
	u32 w,h;
public:
	fboDepth_c();
	~fboDepth_c();

	bool create(u32 newW, u32 newH);
	void destroy();

	u32 getFBOHandle() {
		return fboHandle;
	}
	u32 getTextureHandle() {
		return textureHandle;
	}
};
// six texture2D FBOs composed into cubemap
class depthCubeFBOs_c {
	fboDepth_c sides[6];
	u32 w, h;
public:
	depthCubeFBOs_c();
	~depthCubeFBOs_c();

	bool create(u32 newW, u32 newH);
	void destroy();

	u32 getSideFBOHandle(u32 sideNum) {
		return sides[sideNum].getFBOHandle();
	}
	u32 getSideTextureHandle(u32 sideNum) {
		return sides[sideNum].getTextureHandle();
	}
};

class depthCubeMap_c {
	u32 fboHandle;
	u32 textureHandle;
	u32 w, h;
public:
	depthCubeMap_c() {

	}	
	~depthCubeMap_c() {
		destroy();
	}
	u32 getTextureHandle() {
		return textureHandle;
	}
	bool create(u32 newW, u32 newH) {
		if(newW == w && newH == h)
			return false;
		w = newW;
		h = newH;
		// Create the FBO
		glGenFramebuffers(1, &fboHandle);

		// Create the depth buffer
		glGenTextures(1, &textureHandle);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandle);
	    
		for (u32 i = 0 ; i < 6 ; i++) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		}

		glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
		for (u32 i = 0 ; i < 6 ; i++) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, textureHandle, 0);
		}

		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (Status != GL_FRAMEBUFFER_COMPLETE) {
			destroy();
			printf("FB error, status: 0x%x\n", Status);
			return false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return false;
	}
	void destroy() {
		w = 0; h = 0;
		if(textureHandle) {
			glDeleteTextures(1,&textureHandle);
			textureHandle = 0;
		}
		if(fboHandle) {
			glDeleteFramebuffers(1,&fboHandle);
			fboHandle = 0;
		}
	}
	u32 getHandle() {
		return fboHandle;
	}
};
struct texState_s {
	bool enabledTexture2D;
	u32 index;
	matrixExt_c mat;

	texState_s() {
		enabledTexture2D = false;
		index = 0;
	}
};
struct texCoordSlotState_s {	
	texCoordSlotState_s() {
		texCoordArrayEnabled = false;
	}
	bool texCoordArrayEnabled;
};
#define CHECK_GL_ERRORS checkForGLErrorsInternal(__FUNCTION__,__LINE__);

struct rbCounters_s {
	u32 c_materialDrawCalls;
	// vert/index counts at drawElements entry
	u32 c_inputVerts;
	u32 c_inputTris;
	// total draw counts (drawElements sometimes needs to draw the same surface several times)
	u32 c_totalVerts;
	u32 c_totalTris;
	u32 c_totalVertsVBO;
	u32 c_totalTrisVBO;
	//u32 c_singleStageMaterials;
	//u32 c_multiStageMaterials;
	//u32 c_gpuVertexBuffers;
	//u32 c_cpuVertexBuffers;
	u32 c_gpuIndexBuffers;
	u32 c_cpuIndexBuffers;

	void clear() {
		memset(this,0,sizeof(*this));
	}
};

matrix_c rb_shadowMappingBias(0.5, 0.0, 0.0, 0.0, 
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0);

class rbSDLOpenGL_c : public rbAPI_i {
	// gl limits
	int maxActiveTextureSlots;
	int maxActiveTextureCoords;
	// gl state
	texState_s texStates[MAX_TEXTURE_SLOTS];
	texCoordSlotState_s texCoordStates[MAX_TEXTURE_SLOTS];
	int curTexSlot;
	int highestTCSlotUsed;
	int curTexCoordsSlot;
	// materials
	//safePtr_c<mtrAPI_i> lastMat;
	mtrAPI_i *lastMat;
	textureAPI_i *lastLightmap;
	textureAPI_i *lastDeluxemap;
	bool bindVertexColors;
	bool bHasVertexColors;
	drawCallSort_e curDrawCallSort;
	int forcedMaterialFrameNum;
	int curCubeMapSide;
	int curShadowMapW;
	int curShadowMapH;
	// for point light shadow maps
	depthCubeFBOs_c depthCubeFBOs; //6x depth FBO
	depthCubeMap_c depthCubeMap; // depth cubemap
	// for spotlight or sunlight (directional light)
	fboDepth_c depthFBO;
	fboDepth_c depthFBO_lod1;
	bool bDepthFBODrawnThisFrame;
	bool bDepthFBOLod1DrawnThisFrame;
	fboScreen_c screenFBO;
	fboScreen_c screenFBO2;
	// 1/4 size of the screen...
	fboScreen_c screenFBO_quarter;
	fboScreen_c screenFBO_64x64;
	float averageScreenLuminance;
	bool bDrawingSky;
	u32 viewPortWidth;
	u32 viewPortHeight;
	bool bSkipStaticEnvCubeMapStages;
	bool bHasSunLight;
	class vec3_c sunColor;
	class vec3_c sunDirection;
	aabb sunLightBounds;
	int shadowMapLOD;
	// matrices
	matrix_c worldModelMatrix;
	matrix_c resultMatrix;
	matrix_c projectionMatrix;
	axis_c viewAxis; // viewer's camera axis
	vec3_c camOriginWorldSpace;
	vec3_c camOriginEntitySpace;
	matrix_c camMatrixInEntitySpace;
	bool usingWorldSpace;
	axis_c entityAxis;
	vec3_c entityOrigin;
	matrix_c entityMatrix;
	matrix_c entityRotationMatrix;
	matrix_c entityMatrixInverse;

	matrix_c savedCameraProjection;
	matrix_c savedCameraView;

	matrix_c sunLightView;
	matrix_c sunLightProjection;
	matrix_c sunLightView_lod1;
	matrix_c sunLightProjection_lod1;
	
	bool boundVBOVertexColors;
	const rVertexBuffer_c *boundVBO;
	// per-surface color
	fcolor4_c lastSurfaceColor;

	const class rIndexBuffer_c *boundIBO;
	u32 boundGPUIBO;
	u32 boundFBO;

	bool bBoundLightmapCoordsToFirstTextureSlot;
	bool bVertexAttribLocationsEnabled[16];

	bool backendInitialized;

	float timeNowSeconds;
	bool isMirror;
	u32 portalDepth;
	bool bRendererMirrorThisFrame;
	int r_shadows;
	aabb sunShadowBounds[2];
	// for glColorMask changes
	bool colorMaskState[4];

	// for Doom3 material expressions
	astInput_c materialVarList;

	// counters
	u32 c_frame_vbsReusedByDifferentDrawCall;
	rbCounters_s counters;

public:
	rbSDLOpenGL_c() {
		lastMat = 0;
		lastLightmap = 0;
		lastDeluxemap = 0;
		bindVertexColors = 0;
		curTexSlot = 0;
		curTexCoordsSlot = 0;
		highestTCSlotUsed = 0;
		boundVBO = 0;
		boundGPUIBO = 0;
		boundIBO = 0;
		backendInitialized = false;
		curLight = 0;
		isMirror = false;
		forcedMaterialFrameNum = -1;
		bRendererMirrorThisFrame = false;
		bDepthFBODrawnThisFrame = false;
		bDepthFBOLod1DrawnThisFrame = false;
		boundFBO = 0;
		r_shadows = 0;
		bBoundLightmapCoordsToFirstTextureSlot = false;
		memset(bVertexAttribLocationsEnabled,0,sizeof(bVertexAttribLocationsEnabled));
		bDrawingSky = false;
		colorMaskState[0] = colorMaskState[1] = colorMaskState[2] = colorMaskState[3] = false;
		bSkipStaticEnvCubeMapStages = false;
		bHasSunLight = false;
	}
	virtual backEndType_e getType() const {
		return BET_GL;
	}
	void checkForGLErrorsInternal(const char *functionName, u32 line) {
		if(gl_checkForGLErrors.getInt() == 0)
			return;

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
		g_core->Print("GL_CheckErrors (%s:%i): %s\n", functionName, line, s );
	}
	virtual bool isGLSLSupportAvaible() const {
		// TODO: do a better check to see if GLSL shaders are supported
		if(glCreateProgram == 0) {
			return false;
		}
		if(glCreateShader == 0) {
			return false;
		}
		return true;
	}
	// returns true if "texgen environment" q3 shader effect can be done on GPU
	virtual bool gpuTexGensSupported() const {
		if(rb_gpuTexGens.getInt() == 0)
			return false;
		if(isGLSLSupportAvaible() == false)
			return false;
		return true;
	}
	virtual bool areTangentsNeededForMaterial(const class mtrAPI_i *mat) const {
		if(mat->hasStageOfType(ST_SPECULARMAP))
			return true;
		if(mat->hasStageOfType(ST_BUMPMAP))
			return true;
		if(mat->hasStageOfType(ST_HEIGHTMAP))
			return true;
		return false;
	}
	// glViewPort changes
	void setGLViewPort(u32 newWidth, u32 newHeight) {
		viewPortWidth = newWidth;
		viewPortHeight = newHeight;
		glViewport(0,0,newWidth,newHeight);
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
			curTexSlot = slot;
		}
		CHECK_GL_ERRORS;
	}
	// this is used for texture coordinates
	void selectTexCoordSlot(int slot) {
		if(curTexCoordsSlot != slot) {
			glClientActiveTexture(GL_TEXTURE0 + slot);
			curTexCoordsSlot = slot;
		}
		CHECK_GL_ERRORS;
	}
	void bindTex(int slot, u32 tex) {
		if(slot > MAX_TEXTURE_SLOTS) {
			g_core->RedWarning("rbSDLOpenGL_c::bindTex: bad slot %i\n",slot);
			return;
		}
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
		CHECK_GL_ERRORS;
	}
	void unbindTex(int slot) {
		texState_s *s = &texStates[slot];
		if(s->enabledTexture2D == false && s->index == 0)
			return;

		selectTex(slot);
		if(s->enabledTexture2D == true) {
			glDisable(GL_TEXTURE_2D);
			s->enabledTexture2D = false;
		}
		if(s->index != 0) {
			glBindTexture(GL_TEXTURE_2D,0);
			s->index = 0;
		}
		CHECK_GL_ERRORS;
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
		CHECK_GL_ERRORS;
	}
	//
	// depthRange changes
	//
	float depthRangeNearVal;
	float depthRangeFarVal;
	void setDepthRange(float nearVal, float farVal) {
		if(nearVal == depthRangeNearVal && farVal == depthRangeFarVal)
			return; // no change
		glDepthRange(nearVal,farVal);
		depthRangeNearVal = nearVal;
		depthRangeFarVal = farVal;
	}
	//
	// alphaFunc changes
	//
	alphaFunc_e prevAlphaFunc;
	float alphaFuncCustomValue;
	void setAlphaFunc(alphaFunc_e newAlphaFunc, float customVal = 0.f) {
		if(prevAlphaFunc == newAlphaFunc) {
			if(newAlphaFunc != AF_D3_ALPHATEST)
				return; // no change
			// see if D3 alphaTest value has changed
			if(alphaFuncCustomValue == customVal)
				return; // no change
		}
		if(newAlphaFunc == AF_NONE) {
			glDisable(GL_ALPHA_TEST);
		} else if(newAlphaFunc == AF_GT0) {
			if(prevAlphaFunc == AF_NONE) {
				glEnable(GL_ALPHA_TEST);
			}
			glAlphaFunc( GL_GREATER, 0.0f ); 
		} else if(newAlphaFunc == AF_GE128) {
			if(prevAlphaFunc == AF_NONE) {
				glEnable(GL_ALPHA_TEST);
			}
			glAlphaFunc( GL_GREATER, 0.5f ); 
		} else if(newAlphaFunc == AF_LT128) {
			if(prevAlphaFunc == AF_NONE) {
				glEnable(GL_ALPHA_TEST);
			}
			glAlphaFunc( GL_LESS, 0.5f ); 
		} else if(newAlphaFunc == AF_D3_ALPHATEST) {
			// set the custom alphaTest value provided by Doom3 material
			if(prevAlphaFunc == AF_NONE) {
				glEnable(GL_ALPHA_TEST);
			}
			glAlphaFunc( GL_GREATER, customVal ); 
			alphaFuncCustomValue = customVal;
		}
		prevAlphaFunc = newAlphaFunc;
	}
	void turnOffAlphaFunc() {
		setAlphaFunc(AF_NONE);
	}	
	//
	// GL_BLEND changes
	//
	bool blendEnable;
	void enableGLBlend() {
		if(blendEnable)
			return;
		glEnable(GL_BLEND);
		blendEnable = true;
	}
	void disableGLBlend() {
		if(blendEnable==false)
			return;
		glDisable(GL_BLEND);
		blendEnable = false;

		blendSrc = BM_NOT_SET;
		blendDst = BM_NOT_SET;
	}
	//
	// GL_NORMAL_ARRAY changes
	//
	bool glNormalArrayEnabled;
	void enableNormalArray() {
		if(glNormalArrayEnabled)
			return;
		glEnable(GL_NORMAL_ARRAY);
		glNormalArrayEnabled = true;
	}
	void disableNormalArray() {
		if(glNormalArrayEnabled==false)
			return;
		glDisable(GL_NORMAL_ARRAY);
		glNormalArrayEnabled = false;
	}
	static int blendModeEnumToGLBlend(int in) {
		static int blendTable [] = {
			0, // BM_NOT_SET
			GL_ZERO,
			GL_ONE,
			GL_ONE_MINUS_SRC_COLOR,
			GL_ONE_MINUS_DST_COLOR,
			GL_ONE_MINUS_SRC_ALPHA,
			GL_ONE_MINUS_DST_ALPHA,
			GL_DST_COLOR,
			GL_DST_ALPHA,
			GL_SRC_COLOR,
			GL_SRC_ALPHA,
			GL_SRC_ALPHA_SATURATE,
		};
		return blendTable[in];
	}
	short blendSrc;
	short blendDst;
	void setBlendFunc( short src, short dst ) {
		if( blendSrc != src || blendDst != dst ) {
			blendSrc = src;
			blendDst = dst;
			if(src == BM_NOT_SET && dst == BM_NOT_SET) {
				//setGLDepthMask(true);
				disableGLBlend(); //glDisable( GL_BLEND );
			} else {
				enableGLBlend(); // glEnable( GL_BLEND );
				glBlendFunc( blendModeEnumToGLBlend(blendSrc), blendModeEnumToGLBlend(blendDst) );
				//setGLDepthMask(false);
			}
		}
		CHECK_GL_ERRORS;
	}
	// tex coords arrays
	void enableTexCoordArrayForCurrentTexCoordSlot() {
		texCoordSlotState_s *s = &texCoordStates[curTexCoordsSlot];
		if(s->texCoordArrayEnabled==true)
			return;
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		s->texCoordArrayEnabled = true;
	}
	void disableTexCoordArrayForCurrentTexCoordSlot() {
		texCoordSlotState_s *s = &texCoordStates[curTexCoordsSlot];
		if(s->texCoordArrayEnabled==false)
			return;
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		s->texCoordArrayEnabled = false;
	}
	void disableTexCoordArrayForTexSlot(u32 slotNum) {
		texCoordSlotState_s *s = &texCoordStates[slotNum];
		if(s->texCoordArrayEnabled==false)
			return;
		selectTexCoordSlot(slotNum);
		disableTexCoordArrayForCurrentTexCoordSlot();
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
	bool polygonOffsetEnabled;
	// polygon offset (for decals)
	void setPolygonOffset(float factor, float units) {
		if(polygonOffsetEnabled == false) {
			polygonOffsetEnabled = true;
			glEnable(GL_POLYGON_OFFSET_FILL);
		}
		glPolygonOffset(factor,units);
	}
	void turnOffPolygonOffset() {
		if(polygonOffsetEnabled == true) {
			glDisable(GL_POLYGON_OFFSET_FILL);
			polygonOffsetEnabled = false;
		}
	}
	// glDepthMask
	bool bDepthMask;
	void setGLDepthMask(bool bOn) {
		if(bOn == bDepthMask) {
			return;
		}
		bDepthMask = bOn;
		glDepthMask(bDepthMask);
		CHECK_GL_ERRORS;
	}
	// glColorMask
	void setColorMask(bool r, bool g, bool b, bool a) {
		if(r != colorMaskState[0] || g != colorMaskState[1] ||
			b != colorMaskState[2] || a != colorMaskState[3]) {
			colorMaskState[0] = r;
			colorMaskState[1] = g;
			colorMaskState[2] = b;
			colorMaskState[3] = a;
			glColorMask(r,g,b,a);
		}
	}
	// GL_CULL
	cullType_e prevCullType;
	void glCull(cullType_e cullType) {
		if(prevCullType == cullType) {
			return;
		}
		if(cullType == CT_TWO_SIDED) {
			glDisable(GL_CULL_FACE);
		} else {
			if(prevCullType == CT_TWO_SIDED) {
				glEnable(GL_CULL_FACE);
			}
			//if(isMirror) {
			if(portalDepth % 2 == 1) {
				// swap CT_FRONT with CT_BACK for mirror views
				if(cullType == CT_BACK_SIDED) {
					glCullFace(GL_FRONT);
				} else {
					glCullFace(GL_BACK);
				}
			} else {
				if(cullType == CT_BACK_SIDED) {
					glCullFace(GL_BACK);
				} else {
					glCullFace(GL_FRONT);
				}
			}
		}
		prevCullType = cullType;
		CHECK_GL_ERRORS;
	}
	// GL_STENCIL_TEST
	bool stencilTestEnabled;
	void setGLStencilTest(bool bOn) {
		if(stencilTestEnabled == bOn)
			return;
		stencilTestEnabled = bOn;
		if(bOn) {
			glEnable(GL_STENCIL_TEST);
		} else {
			glDisable(GL_STENCIL_TEST);
		}
	}
	virtual void setMaterial(class mtrAPI_i *mat, class textureAPI_i *lightmap, class textureAPI_i *deluxemap) {
		lastMat = mat;
		if(rb_ignoreLightmaps.getInt()) {
			lastLightmap = 0;
		} else {
			lastLightmap = lightmap;
		}
		lastDeluxemap = deluxemap;
	}
	virtual void unbindMaterial() {
		disableAllTextures();
		turnOffAlphaFunc();
		disableColorArray();	
		turnOffPolygonOffset();
		turnOffTextureMatrices();
		setBlendFunc(BM_NOT_SET,BM_NOT_SET);
		lastMat = 0;
		lastLightmap = 0;
		lastDeluxemap = 0;
	}
	virtual void beginDrawingSky() {
		bDrawingSky = true;
	}
	virtual void endDrawingSky() {
		bDrawingSky = false;
	}
	virtual void setColor4f(float r, float g, float b, float a)  {
		//if(lastSurfaceColor[0] == r && lastSurfaceColor[1] == g && lastSurfaceColor[2] == b && lastSurfaceColor[3] == a)
		//	return;
		glColor4f(r,g,b,a);
		lastSurfaceColor.set(r,g,b,a);
	}
	virtual void setColor4(const float *rgba)  {
		lastSurfaceColor.fromColor4f(rgba);
		if(rgba == 0) {
			setColor4f(1, 1, 1, 1);
			return;
		}
		setColor4f(rgba[0],rgba[1],rgba[2],rgba[3]);
	}
	virtual void setBindVertexColors(bool bBindVertexColors) {
		this->bHasVertexColors = bBindVertexColors;
		this->bindVertexColors = bBindVertexColors;
	}
	virtual void setCurrentDrawCallSort(enum drawCallSort_e sort) {
		this->curDrawCallSort = sort;
	}
	virtual void setRShadows(int newRShadows) {
		this->r_shadows = newRShadows;
	}
	virtual void setSunShadowBounds(const class aabb bounds[]) {
		memcpy(sunShadowBounds,bounds,sizeof(sunShadowBounds));
	}
	void bindFBO(u32 glHandle) {
		if(boundFBO == glHandle)
			return;
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, glHandle);
		CHECK_GL_ERRORS;
		boundFBO = glHandle;
	}
	void stopDrawingToFBOAndRestoreCameraMatrices() {
		// unbind the FBO
		if(shouldDrawToScreenFBO()) {
			bindFBO(screenFBO.getFBOHandle());
		} else {
			bindFBO(0);
		}
		// restore viewport
		setGLViewPort(getWinWidth(),getWinHeight());

		// restore camera view and projection matrices
		projectionMatrix = savedCameraProjection;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glLoadMatrixf(projectionMatrix);

		worldModelMatrix = savedCameraView;
		//setupWorldSpace();
	}
	virtual void setCurrentDrawCallCubeMapSide(int iCubeSide) {
		if(iCubeSide == this->curCubeMapSide) {
			return; // no change
		}
		if(iCubeSide == -1) {
			this->stopDrawingToFBOAndRestoreCameraMatrices();
	
			this->curCubeMapSide = -1;

			CHECK_GL_ERRORS;
			return;
		}
		if(this->curCubeMapSide == -1) {
			
			selectTex(3);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			// ensure that FBO is ready
			if(curLight->isSpotLight()) {
				depthFBO.create(curShadowMapW,curShadowMapH);
			} else {
				if(rb_useDepthCubeMap.getInt()) {
					//if(1) {
					//	depthCubeMap.destroy();
					//}
					depthCubeMap.create(curShadowMapW,curShadowMapH);
				} else {
					//if(1) {
					//	depthCubeFBOs.destroy();
					//}
					depthCubeFBOs.create(curShadowMapW,curShadowMapH);
				}
			}
		}
		// bind the FBO
		if(curLight->isSpotLight()) {
			bindFBO(depthFBO.getFBOHandle());	
		} else {
			if(rb_useDepthCubeMap.getInt()) {
				bindFBO(depthCubeMap.getHandle());				
				glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+iCubeSide, depthCubeMap.getTextureHandle(), 0);
			} else {
				bindFBO(depthCubeFBOs.getSideFBOHandle(iCubeSide));	
			}
		}
		// set viewport
		setGLViewPort(curShadowMapW,curShadowMapH);
		// clear buffers
		// Depth mask must be set to true before calling glClear
		// (otherwise GL_DEPTH_BUFFER_BIT would be ignored)
		////if(!bDepthMask)
		////	g_core->Print("Depth mask was off\n");
		setGLDepthMask(true); // this is necessary here!
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		if(this->curCubeMapSide == -1) {
			// save camera matrices
			savedCameraProjection = projectionMatrix;
			savedCameraView = worldModelMatrix;
		}

		// set the light view matrices
		if(curLight->isSpotLight()) {
			worldModelMatrix = curLight->getSpotLightView();
			projectionMatrix = curLight->getSMLightProj();
		} else {
			worldModelMatrix = curLight->getSMSideView(iCubeSide);
			projectionMatrix = curLight->getSMLightProj();
		}
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glLoadMatrixf(projectionMatrix);

		this->curCubeMapSide = iCubeSide;
		CHECK_GL_ERRORS;
	}
	virtual void setCurLightShadowMapSize(int newW, int newH) {
		curShadowMapW = newW;
		curShadowMapH = newH;
	}
	virtual void setSunParms(bool newBHasSunLight, const class vec3_c &newSunColor, const class vec3_c &newSunDirection, const class aabb &newSunBounds) {
		bHasSunLight = newBHasSunLight;
		sunColor = newSunColor;
		sunDirection = newSunDirection;
		sunLightBounds = newSunBounds;
	}
	virtual void setForcedMaterialMapFrame(int animMapFrame) {
		this->forcedMaterialFrameNum = animMapFrame;
	}
	void disableAllVertexAttribs() {
		for(int loc = 0; loc < 16; loc++) {
			if(bVertexAttribLocationsEnabled[loc] == true) {
				bVertexAttribLocationsEnabled[loc] = false;
				glDisableVertexAttribArray(loc);
			}
		}
	}
	void enableVertexAttrib(int loc) {
		if(loc < 0)
			return;
		if(bVertexAttribLocationsEnabled[loc] == false) {
			bVertexAttribLocationsEnabled[loc] = true;
			glEnableVertexAttribArray(loc);
		}
	}
	void bindVertexBuffer(const class rVertexBuffer_c *verts, bool bindLightmapCoordsToFirstTextureSlot = false) {
		if(boundVBO == verts) {
			if(boundVBOVertexColors == bindVertexColors) {
				if(bBoundLightmapCoordsToFirstTextureSlot == bindLightmapCoordsToFirstTextureSlot) {
					c_frame_vbsReusedByDifferentDrawCall++;
					return; // already bound
				}
			} else {

			}
		}
		
		disableAllVertexAttribs();

		u32 h = verts->getInternalHandleU32();
		glBindBufferARB(GL_ARRAY_BUFFER,h);
		if(h == 0) {
			// buffer wasnt uploaded to GPU
			glVertexPointer(3,GL_FLOAT,sizeof(rVert_c),verts->getArray());
			selectTexCoordSlot(0);
			enableTexCoordArrayForCurrentTexCoordSlot();
			if(bindLightmapCoordsToFirstTextureSlot) {
				glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),&verts->getArray()->lc.x);
				disableTexCoordArrayForTexSlot(1);
			} else {
				glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),&verts->getArray()->tc.x);
				selectTexCoordSlot(1);
				enableTexCoordArrayForCurrentTexCoordSlot();
				glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),&verts->getArray()->lc.x);
			}
			if(bindVertexColors) {
				enableColorArray();
				glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(rVert_c),&verts->getArray()->color[0]);
			} else {
				disableColorArray();
			}
			enableNormalArray();
			glNormalPointer(GL_FLOAT,sizeof(rVert_c),&verts->getArray()->normal.x);
			// bind tangents and binormals for bump/paralax mapping effects
			if(curShader) {
				int tangentsLocation = curShader->getAtrTangentsLocation();
				int binormalsLocation = curShader->getAtrBinormalsLocation();
				if(tangentsLocation >= 0) {
					enableVertexAttrib(tangentsLocation);
					glVertexAttribPointer(tangentsLocation, 3, GL_FLOAT, true, sizeof(rVert_c), &verts->getArray()->tan.x);
				}
				if(binormalsLocation >= 0) {
					enableVertexAttrib(binormalsLocation);
					glVertexAttribPointer(binormalsLocation, 3, GL_FLOAT, true, sizeof(rVert_c), &verts->getArray()->bin.x);
				}
			}
		} else {
			glVertexPointer(3,GL_FLOAT,sizeof(rVert_c),0);
			selectTexCoordSlot(0);
			enableTexCoordArrayForCurrentTexCoordSlot();
			if(bindLightmapCoordsToFirstTextureSlot) {
				glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),(void*)offsetof(rVert_c,lc));
				disableTexCoordArrayForTexSlot(1);
			} else {
				glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),(void*)offsetof(rVert_c,tc));
				selectTexCoordSlot(1);
				enableTexCoordArrayForCurrentTexCoordSlot();
				glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),(void*)offsetof(rVert_c,lc));
			}
			if(bindVertexColors) {
				enableColorArray();
				glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(rVert_c),(void*)offsetof(rVert_c,color));
			} else {
				disableColorArray();
			}
			enableNormalArray();
			glNormalPointer(GL_FLOAT,sizeof(rVert_c),(void*)offsetof(rVert_c,normal));
			// bind tangents and binormals for bump/paralax mapping effects
			if(curShader) {
				int tangentsLocation = curShader->getAtrTangentsLocation();
				int binormalsLocation = curShader->getAtrBinormalsLocation();
				if(tangentsLocation >= 0) {
					enableVertexAttrib(tangentsLocation);
					glVertexAttribPointer(tangentsLocation, 3, GL_FLOAT, true, sizeof(rVert_c), (void*)offsetof(rVert_c,tan));
				}
				if(binormalsLocation >= 0) {
					enableVertexAttrib(binormalsLocation);
					glVertexAttribPointer(binormalsLocation, 3, GL_FLOAT, true, sizeof(rVert_c), (void*)offsetof(rVert_c,bin));
				}
			}
		}
		boundVBOVertexColors = bindVertexColors;
		bBoundLightmapCoordsToFirstTextureSlot = bindLightmapCoordsToFirstTextureSlot;
		boundVBO = verts;
		CHECK_GL_ERRORS;
	}
	void unbindVertexBuffer() {
		if(boundVBO == 0)
			return;
		if(boundVBO->getInternalHandleU32()) {
			glBindBufferARB(GL_ARRAY_BUFFER,0);
		}
		glVertexPointer(3,GL_FLOAT,sizeof(rVert_c),0);
		selectTexCoordSlot(0);
		disableTexCoordArrayForCurrentTexCoordSlot();
		glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),0);
		selectTexCoordSlot(1);
		disableTexCoordArrayForCurrentTexCoordSlot();
		glTexCoordPointer(2,GL_FLOAT,sizeof(rVert_c),0);
		glColorPointer(4,GL_UNSIGNED_BYTE,sizeof(rVert_c),0);
		disableColorArray();
		boundVBO = 0;
		CHECK_GL_ERRORS;
	}
	void unbindIBO() {
		if(boundGPUIBO) {
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER,0);
			boundGPUIBO = 0;
		}		
		boundIBO = 0;
	}
	void bindIBO(const class rIndexBuffer_c *indices) {
		if(boundIBO == indices)
			return;
		if(indices == 0) {
			unbindIBO();
			return;
		}
		u32 h = indices->getInternalHandleU32();
		if(h != boundGPUIBO) {
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER,h);
			boundGPUIBO = h;
		}
		boundIBO = indices;
	}
	void drawCurIBO() {
		if(rb_ignoreDrawCalls3D.getInt())
			return;
		if(boundGPUIBO == 0) {
			glDrawElements(GL_TRIANGLES, boundIBO->getNumIndices(), boundIBO->getGLIndexType(), boundIBO->getVoidPtr());
			counters.c_cpuIndexBuffers++;
		} else {
			glDrawElements(GL_TRIANGLES, boundIBO->getNumIndices(), boundIBO->getGLIndexType(), 0);
			counters.c_gpuIndexBuffers++;
		}
		CHECK_GL_ERRORS;
		if(gl_callGLFinish.getInt()==2) {
			glFinish();
		}
		if(boundVBO) {
			if(boundVBO->getInternalHandleU32()) {
				counters.c_totalVertsVBO += boundVBO->size();
			}
			counters.c_totalVerts += boundVBO->size();
		}
		if(boundGPUIBO) {
			counters.c_totalTrisVBO += boundIBO->getNumTriangles();
		}
		counters.c_totalTris += boundIBO->getNumTriangles();
	}
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices)  {
		stopDrawingShadowVolumes();
		bindShader(0);
		CHECK_GL_ERRORS;

		glVertexPointer(2,GL_FLOAT,sizeof(r2dVert_s),verts);
		selectTexCoordSlot(0);
		enableTexCoordArrayForCurrentTexCoordSlot();
		CHECK_GL_ERRORS;
		glTexCoordPointer(2,GL_FLOAT,sizeof(r2dVert_s),&verts->texCoords.x);
		disableNormalArray();
		CHECK_GL_ERRORS;
		if(lastMat) {
			// NOTE: this is the way Q3 draws all the 2d menu graphics
			// (it worked before with bigchars.tga, even when the bigchars
			// shader was missing!!!)
			//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			setBlendFunc(BM_SRC_ALPHA,BM_ONE_MINUS_SRC_ALPHA);
			for(u32 i = 0; i < lastMat->getNumStages(); i++) {
				const mtrStageAPI_i *s = lastMat->getStage(i);
				setAlphaFunc(s->getAlphaFunc());
				//const blendDef_s &bd = s->getBlendDef();
				//setBlendFunc(bd.src,bd.dst);
				textureAPI_i *t = s->getTexture(this->timeNowSeconds);
				bindTex(0,t->getInternalHandleU32());
				CHECK_GL_ERRORS;
				glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);
				CHECK_GL_ERRORS;
			}
			//glDisable(GL_BLEND);
		} else {
			glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, indices);
		}
	}
	const rLightAPI_i *curLight;
	class glShader_c *curShader;
	bool bDrawOnlyOnDepthBuffer;
	bool bDrawingShadowVolumes;
	bool bDrawingSunShadowMapPass;
	virtual void setCurLight(const class rLightAPI_i *light) {
		if(this->curLight == light)
			return;
		this->curLight = light;
		// clear stencil buffer after changing the light
		// FIXME: do it only if rf_shadows==1?
		glClear(GL_STENCIL_BUFFER_BIT);
	}
	virtual void setBDrawOnlyOnDepthBuffer(bool bNewDrawOnlyOnDepthBuffer) {
		bDrawOnlyOnDepthBuffer = bNewDrawOnlyOnDepthBuffer;
	}
	virtual void setBDrawingSunShadowMapPass(bool bNewDrawingSunShadowMapPass, int newSunShadowMapLOD) {
		if(bDrawingSunShadowMapPass == bNewDrawingSunShadowMapPass
			&& shadowMapLOD == newSunShadowMapLOD) {
			return;
		}
		if(bNewDrawingSunShadowMapPass) {
			u32 shadowMapSize = 1024;
			if(newSunShadowMapLOD <= 0) {
				depthFBO.create(shadowMapSize,shadowMapSize);		
				bindFBO(depthFBO.getFBOHandle());
				bDepthFBODrawnThisFrame = true;
			} else if(newSunShadowMapLOD == 1) {
				depthFBO_lod1.create(shadowMapSize,shadowMapSize);		
				bindFBO(depthFBO_lod1.getFBOHandle());
				bDepthFBOLod1DrawnThisFrame = true;
			}
			// set viewport
			setGLViewPort(shadowMapSize,shadowMapSize);
			// clear buffers
			// Depth mask must be set to true before calling glClear
			// (otherwise GL_DEPTH_BUFFER_BIT would be ignored)
			////if(!bDepthMask)
			////	g_core->Print("Depth mask was off\n");
			setGLDepthMask(true); // this is necessary here!
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			// save camera matrices
			if(bDrawingSunShadowMapPass == false) {
				savedCameraProjection = projectionMatrix;
				savedCameraView = worldModelMatrix;
			}


			matrix_c tempView;
			tempView.setupLookAtRH(camOriginWorldSpace, -sunDirection, worldModelMatrix.getForward());

			aabb bb = sunLightBounds;
			aabb cropBounds;
			for(u32 i = 0; i < 8; i++) {
				vec3_c p = bb.getPoint(i);
				float p2[4] = { p.x, p.y, p.z, 1.f };
				float transf[4];
				tempView.transformVec4(p2,transf);					
				transf[0] /= transf[3];
				transf[1] /= transf[3];
				transf[2] /= transf[3];
				cropBounds.addPoint(transf);
			}	
			matrix_c tempProjection;
			tempProjection.setupOrthogonalProjectionRH(cropBounds.mins[0], cropBounds.maxs[0], cropBounds.mins[1], cropBounds.maxs[1], -cropBounds.maxs[2], -cropBounds.mins[2]);
			
			if(newSunShadowMapLOD <= 0) {
				sunLightProjection = tempProjection;
				sunLightView = tempView;
			} else if(newSunShadowMapLOD == 1) {
				sunLightProjection_lod1 = tempProjection;
				sunLightView_lod1 = tempView;
			}

			worldModelMatrix = tempView;
			projectionMatrix = tempProjection;

			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glLoadMatrixf(projectionMatrix);
		} else {
			this->stopDrawingToFBOAndRestoreCameraMatrices();
		}
		bDrawingSunShadowMapPass = bNewDrawingSunShadowMapPass;
		shadowMapLOD = newSunShadowMapLOD;
	}
	void bindShader(class glShader_c *newShader) {
		if(glUseProgram == 0) {
			if(newShader) {
				g_core->RedWarning("Tried to bind a GLSL shader without having GLSL support (shader %s)\n",newShader->getName());
			}
			return;
		}
		curShader = newShader;
		if(newShader == 0) {
			if(rb_verboseBindShader.getInt()) {
				g_core->Print("Unbinding shader..\n");
			}
			glUseProgram(0);
			disableAllVertexAttribs();
		} else {
			if(rb_verboseBindShader.getInt()) {
				g_core->Print("Binding shader %s..\n",newShader->getName());
			}
			glUseProgram(newShader->getGLHandle());
			if(newShader->sColorMap != -1) {
				glUniform1i(newShader->sColorMap,0);
			}
			if(newShader->sColorMap2 != -1) {
				glUniform1i(newShader->sColorMap2,1);
			}
			if(newShader->sLightMap != -1) {
				glUniform1i(newShader->sLightMap,1);
			}
			if(newShader->sBumpMap != -1) {
				glUniform1i(newShader->sBumpMap,2);
			}
			if(newShader->sHeightMap != -1) {
				glUniform1i(newShader->sHeightMap,3);
			}
			if(newShader->sDeluxeMap != -1) {
				glUniform1i(newShader->sDeluxeMap,4);
			}
			if(newShader->sCubeMap != -1) {
				glUniform1i(newShader->sCubeMap,0);
			}
			// pass the sunlight (directional light) paremeters to shader
			if(bHasSunLight) {
				if(newShader->u_sunDirection != -1) {
					if(usingWorldSpace) {
						glUniform3f(newShader->u_sunDirection,sunDirection.x,sunDirection.y,sunDirection.z);
					} else {
						vec3_c dirLocal;
						entityMatrixInverse.transformNormal(sunDirection,dirLocal);
						glUniform3f(newShader->u_sunDirection,dirLocal.x,dirLocal.y,dirLocal.z);
					}
				}
				if(newShader->u_sunColor != -1) {
					glUniform3f(newShader->u_sunColor,sunColor.x,sunColor.y,sunColor.z);
				}
			}
			// pass the point/spot light parameters to shader
			if(curLight) {
				if(newShader->uLightOrigin != -1) {
					const vec3_c &xyz = curLight->getOrigin();
					if(usingWorldSpace) {
						glUniform3f(newShader->uLightOrigin,xyz.x,xyz.y,xyz.z);
					} else {
						vec3_c xyzLocal;
						entityMatrixInverse.transformPoint(xyz,xyzLocal);
						glUniform3f(newShader->uLightOrigin,xyzLocal.x,xyzLocal.y,xyzLocal.z);
					}
				}
				if(newShader->uLightRadius != -1) {
					glUniform1f(newShader->uLightRadius,curLight->getRadius());
				}
				if(newShader->u_lightDir != -1) {
					const vec3_c &xyz = curLight->getSpotLightDir();
					vec3_c xyzLocal;
					entityMatrixInverse.transformNormal(xyz,xyzLocal);
					glUniform3f(newShader->u_lightDir,xyzLocal.x,xyzLocal.y,xyzLocal.z);
				}
				if(newShader->u_spotLightMaxCos != -1) {
					glUniform1f(newShader->u_spotLightMaxCos,curLight->getSpotLightMaxCos());
				}
				if(r_shadows == 2) {	
					if(curLight->isSpotLight()) {
						matrix_c lProj = curLight->getSMLightProj();;
						matrix_c lView = curLight->getSpotLightView();
						matrix_c res = rb_shadowMappingBias * lProj * lView * entityMatrix;

						u32 texSlot = 1;
						setTextureMatrixCustom(texSlot,res);

						u32 slot = 3;
						bindTex(slot,depthFBO.getTextureHandle());
						glUniform1i(newShader->u_spotLightShadowMap,slot);
					} else {
						if(rb_useDepthCubeMap.getInt()) {
							selectTex(3);
							glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap.getTextureHandle());
							glUniform1i(newShader->u_shadowCubeMap,3);
						} else {
							for(u32 i = 0; i < 6; i++) {
								matrix_c lProj = curLight->getSMLightProj();;
								matrix_c lView = curLight->getSMSideView(i);

								matrix_c res = rb_shadowMappingBias * lProj * lView * entityMatrix;


								u32 texSlot = 1 + i;
								setTextureMatrixCustom(texSlot,res);

								u32 slot = 3+i;
								bindTex(slot,depthCubeFBOs.getSideTextureHandle(i));
								glUniform1i(newShader->u_shadowMap[i],slot);
							}
						}
					}
				}
			} 
			if(bHasSunLight && r_shadows == 2 && newShader->u_directionalShadowMap >= 0) {
				matrix_c res = rb_shadowMappingBias * sunLightProjection * sunLightView * entityMatrix;
				setTextureMatrixCustom(1,res);

				bindTex(3,depthFBO.getTextureHandle());
				glUniform1i(newShader->u_directionalShadowMap,3);

				// also bind LOD 1 fbo
				if(newShader->u_directionalShadowMap_lod1 >= 0) {
					if(bDepthFBOLod1DrawnThisFrame == false) {
						// should never happen
						g_core->RedWarning("Required LOD 1 FBO was not drwan this frame\n");
					} else {
						matrix_c resLod1 = rb_shadowMappingBias * sunLightProjection_lod1 * sunLightView_lod1 * entityMatrix;
						setTextureMatrixCustom(2,resLod1);
						bindTex(4,depthFBO_lod1.getTextureHandle());
						glUniform1i(newShader->u_directionalShadowMap_lod1,4);
					}
				}
			}
			if(newShader->uViewOrigin != -1) {
				glUniform3f(newShader->uViewOrigin,
					this->camOriginEntitySpace.x,
					this->camOriginEntitySpace.y,
					this->camOriginEntitySpace.z);
			}
			if(newShader->u_materialColor != -1) {
				glUniform4fv(newShader->u_materialColor,1,lastSurfaceColor.toPointer());
			}
			if(newShader->u_entityMatrix != -1) {
				glUniformMatrix4fv(newShader->u_entityMatrix,1,false,entityMatrix);
			}
			if(newShader->u_entityRotationMatrix != -1) {
				glUniformMatrix4fv(newShader->u_entityRotationMatrix,1,false,entityRotationMatrix);
			}
			if(newShader->u_blurScale != -1) {
				glUniform1f(newShader->u_blurScale,getBlurScale());
			}
			if(newShader->u_averageScreenLuminance != -1) {
				glUniform1f(newShader->u_averageScreenLuminance,getAverageScreenLuminance());
			}
			if(newShader->u_shadowMapLod0Maxs != -1) {
				glUniform3fv(newShader->u_shadowMapLod0Maxs,1,sunShadowBounds[0].maxs);
			}
			if(newShader->u_shadowMapLod0Mins != -1) {
				glUniform3fv(newShader->u_shadowMapLod0Mins,1,sunShadowBounds[0].mins);
			}
		}
	}
	// temporary vertex buffer for stages that requires CPU 
	// vertex calculations, eg. texgen enviromental, etc.
	rVertexBuffer_c stageVerts;

	textureAPI_i *getStageTextureInternal(const mtrStageAPI_i *stage) {
		if(forcedMaterialFrameNum == -1) {
			// use material time to get animated texture frame
			return stage->getTexture(this->timeNowSeconds);
		} else {
			// use texture frame set by cgame
			return stage->getTextureForFrameNum(forcedMaterialFrameNum);
		}
	}
	void setupStageAlphaFunc(const mtrStageAPI_i *s) {
		alphaFunc_e newAlphaFunc = s->getAlphaFunc();
		if(newAlphaFunc == AF_D3_ALPHATEST) {
			// evaluate Doom3 alpha value expression at runtime (AST)
			float alphaValue = s->evaluateAlphaTestValue(&materialVarList);
			// and then set it
			if(rb_ignoreDoom3AlphaTest.getInt()) {
				setAlphaFunc(AF_NONE);
			} else {
				setAlphaFunc(AF_D3_ALPHATEST,alphaValue);
			}
		} else {
			// set the classic Quake3 GT0 / LT128 / GT128 alphaFunc
			setAlphaFunc(s->getAlphaFunc());
		}
	}
	bool bDeformsDone;
	virtual void drawElements(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		if(indices.getNumIndices() == 0)
			return;
		if(verts.size() == 0)
			return;

		if(rb_skipMirrorMaterials.getInt()) {
			if(lastMat->isMirrorMaterial())
				return;
		}
		if(rb_skipMaterialsWithCubeMaps.getInt()) {
			if(lastMat && lastMat->hasStageWithCubeMap())
				return;
		}
		// first apply deforms
		if(bDeformsDone == false && lastMat && lastMat->hasDeforms()) {
			// right now we're only supporting the autoSprite deform
			if(lastMat->hasDeformOfType(DEFORM_AUTOSPRITE)) {
				rVertexBuffer_c vertsCopy;
				rIndexBuffer_c indicesCopy;
				vertsCopy = verts;
				vec3_c left = camMatrixInEntitySpace.getLeft();
				vec3_c up = camMatrixInEntitySpace.getUp();
				// see if we can apply the autosprite deform
				if(vertsCopy.applyDeformAutoSprite(left,up)==false) {
					// reorder indices, this is necessary in some cases
					u16 *pIndices = indicesCopy.initU16((verts.size()/4)*6);
					for(u32 i = 0; i < verts.size(); i += 4) {
						pIndices[6*(i>>2)+0] = i;
						pIndices[6*(i>>2)+1] = i+1;
						pIndices[6*(i>>2)+2] = i+2;

						pIndices[6*(i>>2)+3] = i;
						pIndices[6*(i>>2)+4] = i+2;
						pIndices[6*(i>>2)+5] = i+3;
					}
					bDeformsDone = true;
					// use new vertex/index buffers to render elements
					drawElements(vertsCopy,indicesCopy);
					return;
				}
			}
		}
		bDeformsDone = false;

		counters.c_materialDrawCalls++;
		counters.c_inputVerts += verts.size();
		counters.c_inputTris += indices.getNumTriangles();

		stopDrawingShadowVolumes();

		if(rb_verboseDrawElements.getInt()) { 
			g_core->Print("rbSDLOpenGL_c::drawElements: %s - bDrawOnlyOnDepthBuffer %i, curCubeMapSide %i\n",lastMat->getName(),bDrawOnlyOnDepthBuffer,curCubeMapSide);
		}
		
		if(bDrawOnlyOnDepthBuffer) {
			if(lastMat->hasStageWithoutCustomProgram() == false)
				return;
			if(bSkipStaticEnvCubeMapStages) {
				if(lastMat->hasOnlyStagesOfType(ST_ENV_CUBEMAP)) {
					return;
				}
			}

			if(bRendererMirrorThisFrame) {
				if(lastMat->isMirrorMaterial()) {
					setColorMask(false, false, false, false);
				} else {
					// non-mirrored view should never be blended with mirror view,
					// so draw all non-mirrored surfaces in draw color
					setColorMask(true, true, true, true);
					this->setColor4f(0,0,0,0);
				}
			} else {
				setColorMask(false, false, false, false);
			}
			//if(lastMat->isMirrorMaterial()) {
			//	setGLDepthMask(false);
			//} else {
			//	setGLDepthMask(true);
			//}
			setGLDepthMask(true);
			if(curCubeMapSide >= 0 && rb_useDepthCubeMap.getInt()) {
				glShader_c *sh = GL_RegisterShader("drawToShadowMap");
				bindShader(sh);
			} else {
				bindShader(0);
			}
			bindVertexBuffer(&verts);
			bindIBO(&indices);
			//turnOffPolygonOffset();
			if(lastMat->getPolygonOffset()) {
				this->setPolygonOffset(-1,-2);
			} else {
				this->turnOffPolygonOffset();
			}
			turnOffTextureMatrices();

			// check if material has an alpha test stage
			// TODO: draw all of them, not only the first one?
			const mtrStageAPI_i *alphaStage = lastMat->getFirstStageWithAlphaFunc();
			if(alphaStage == 0) {
				// material has no alpha test stages, we can draw without textures
				turnOffAlphaFunc();
				disableAllTextures();
			} else {
				// material has alpha test stage, we need to bind its texture
				// and draw surface with alpha test enabled.
				setupStageAlphaFunc(alphaStage);
				disableAllTextures();
				bindTex(0,alphaStage->getTexture(0)->getInternalHandleU32());
			}

			if(curCubeMapSide >= 0 || bDrawingSunShadowMapPass) {
				// invert culling for shadow mapping 
				// (because we want to get the depth of backfaces to avoid epsilon issues)
				if(lastMat) {
					if(lastMat->getCullType() == CT_TWO_SIDED) {
						// this is needed for eg. tree leaves/branches
						glCull(CT_TWO_SIDED);
					} else {
						glCull(CT_BACK_SIDED);
					}
				} else {
					glCull(CT_BACK_SIDED);
				}
			} else {
				if(lastMat) {
					if(rb_forceTwoSided.getInt()) {
						glCull(CT_FRONT_SIDED);
					} else {
						glCull(lastMat->getCullType());
					}
				} else {
					glCull(CT_FRONT_SIDED);
				}
			}
		///	setDepthRange(0.f,1.f);
		//	glEnable(GL_DEPTH_TEST);

			setBlendFunc(BM_NOT_SET,BM_NOT_SET);
			drawCurIBO();
			return;
		}
		if(rb_printLightingPassDrawCalls.getInt()) {
			if(curLight) {
				g_core->Print("LightingPass: %s (%i tris, %i verts)\n",lastMat->getName(),indices.getNumTriangles(),verts.size());
			}
		}
		// now it's done by frontend, and the color is not always 1 1 1 1
		//this->setColor4f(1.f,1.f,1.f,1.f);
		setColorMask(true, true, true, true);
		if(bDrawingSky) {
			// fathest depth range
			setDepthRange(1.f,1.f);
		} else {
			// default depth range
			setDepthRange(0.f,1.f);
		}

		if(curDrawCallSort == DCS_BLEND_AFTER_LIGHTING) {
			// disable stencil buffer
			setGLStencilTest(false);
			if(rb_printBlendAfterLightingDrawCalls.getInt()) {
				g_core->Print("rbSDLOpenGL_c::drawElements: blendAfterLighting surface with material %s\n",lastMat->getName());
			}	
		}

		if(rb_showNormalColors.getInt()) {
			glShader_c *sh = GL_RegisterShader("showNormalColors");
			if(sh) {
				turnOffAlphaFunc();
				turnOffPolygonOffset();
				turnOffTextureMatrices();
				disableAllTextures();
				bindShader(sh);
				bindVertexBuffer(&verts);
				bindIBO(&indices);
				drawCurIBO();
				return;
			}
		}

		if(lastMat) {
			if(rb_forceTwoSided.getInt()) {
				glCull(CT_TWO_SIDED);
			} else {
				glCull(lastMat->getCullType());
			}
		} else {
			glCull(CT_FRONT_SIDED);
		}

		//bindVertexBuffer(&verts);
		bindIBO(&indices);

		if(lastMat) {
			if(lastMat->getPolygonOffset()) {
				this->setPolygonOffset(-1,-2);
			} else {
				this->turnOffPolygonOffset();
			}
			u32 numMatStages = lastMat->getNumStages();
			for(u32 i = 0; i < numMatStages; i++) {
				// get the material stage
				const mtrStageAPI_i *s = lastMat->getStage(i);
				// see if stage condition is met (Doom3 'if' materials)
				if(s->hasIFCondition()) {
					if(s->conditionMet(&materialVarList) == false) {
						if(rb_debugStageConditions.getInt()) {
							g_core->Print("Skipping stage %i of material %s because condition is NOT met\n",i,lastMat->getName());
						}
						continue; // skip this stage
					}
					if(rb_debugStageConditions.getInt()) {
						g_core->Print("Drawing stage %i of material %s because condition is met\n",i,lastMat->getName());
					}
				}
				if(rb_skipStagesWithoutBlendFunc.getInt()) {
					if(s->getBlendDef().isNonZero() == false)
						continue;
				}
				if(rb_skipStagesWithBlendFunc.getInt()) {
					if(s->getBlendDef().isNonZero())
						continue;
				}
				// skip blendend stages while drawing a light pass
				// (some of Q3 materials has mixed blended stages with non-blended ones)
				if(lastMat->hasBlendFunc() && lastMat->hasStageWithoutBlendFunc() && curLight) {
					if(numMatStages - 1 != i) 
						continue;
				}
				// skip stages that are usind custom Doom3 gpu programs
				if(s->isUsingCustomProgram())
					continue;
				// get material stage type
				enum stageType_e stageType = s->getStageType();
				// get the bumpmap substage of current stage
				const mtrStageAPI_i *bumpMap;
				if(rb_ignoreBumpMaps.getInt()) {
					bumpMap = 0;
				} else {
					bumpMap = s->getBumpMap();
				}
				// get the heightmap substage of current stage
				const mtrStageAPI_i *heightMap;
				if(rb_ignoreHeightMaps.getInt()) {
					heightMap = 0;
				} else {
					heightMap = s->getHeightMap();
				}	
				// get the deluxeMap
				const textureAPI_i *deluxeMap;
				if(rb_ignoreDeluxeMaps.getInt()) {
					deluxeMap = 0;
				} else {
					deluxeMap = lastDeluxemap;
				}
				if(rb_showBumpMaps.getInt()) {
					if(bumpMap) {
						s = bumpMap;
						bumpMap = 0;
					}
					heightMap = 0;
				} else if(rb_showHeightMaps.getInt()) {
					if(heightMap) {
						s = heightMap;
						heightMap = 0;
					}
					bumpMap = 0;
				}
				class cubeMapAPI_i *cubeMap;
				if(stageType == ST_ENV_CUBEMAP) {
					// this is set while rebuilding the cubemaps
					if(bSkipStaticEnvCubeMapStages)
						continue;
					vec3_c p;
					verts.getCenter(indices,p);
					cubeMap = rf->getNearestEnvCubeMapImage(p);
					// fall back to reflection stage type
					stageType = ST_CUBEMAP_REFLECTION;
				} else {
					cubeMap = s->getCubeMap();
				}

				// set the alphafunc
				setupStageAlphaFunc(s);

				// set the blendfunc
				if(curLight == 0) {
					const blendDef_s &bd = s->getBlendDef();
					setBlendFunc(bd.src,bd.dst);
				} else {
					// light interactions are appended with addictive blending
					setBlendFunc(BM_ONE,BM_ONE);
				}

				// set depthmask
#if 1
				// test; it seems beam shader should be drawn with depthmask off..
				//if(!stricmp(lastMat->getName(),"textures/sfx/beam")) {
				//if(s->getBlendDef().isNonZero()) {
				// (only if not doing multipass lighting)
				if(curLight == 0) {
					if(rb_printMaterialDepthWrite.getInt()) {
						g_core->Print("Material: %s, depthWrite: %i\n",lastMat->getName(),s->getDepthWrite());
					}
					if(s->getDepthWrite()==false) {
						setGLDepthMask(false);
					} else {
						setGLDepthMask(true);
					}
				} else {
					// lighting passes (and shadow volume drawcalls)
					// should be done with glDepthMask off
					setGLDepthMask(false);
				}
#endif
				// set colormask (it might be set in Doom3 materials)
				// this is used in eg. Doom3 BFG blast material (models/md5/weapons/bfg_view/viewbfg.md5mesh)
				setColorMask(!s->getColorMaskRed(),!s->getColorMaskGreen(),
					!s->getColorMaskBlue(),!s->getColorMaskAlpha());

				if(s->hasTexMods()) {
					matrix_c mat;
					s->applyTexMods(mat,this->timeNowSeconds,&materialVarList);
					this->setTextureMatrixCustom(0,mat);
				} else {
					this->setTextureMatrixIdentity(0);
				}
				bool bindLightmapCoordinatesToFirstTextureSlot = false;
				if(rb_showLightMaps.getInt() && lastLightmap) {
					goto drawOnlyLightmap;
				} else if(rb_showDeluxeMaps.getInt() && lastDeluxemap) {
					// bind lightmap coordinates to first texture slot.
					// draw ONLY DELUXEMAP
					if(lastDeluxemap) {
						bindTex(0,lastDeluxemap->getInternalHandleU32());
					} else {
						bindTex(0,0);
					}
					bindLightmapCoordinatesToFirstTextureSlot = true;
					unbindTex(1);
				} else if(stageType == ST_COLORMAP_LIGHTMAPPED) {
					// draw multitextured surface with
					// - colormap at slot 0
					// - lightmap at slot 1
					textureAPI_i *t = getStageTextureInternal(s);
					bindTex(0,t->getInternalHandleU32());
					if(lastLightmap) {
						bindTex(1,lastLightmap->getInternalHandleU32());
					} else {
						bindTex(1,0);
					}
				} else if(stageType == ST_LIGHTMAP) {
drawOnlyLightmap:
					// bind lightmap to first texture slot.
					// draw ONLY lightmap
					if(lastLightmap) {
						bindTex(0,lastLightmap->getInternalHandleU32());
					} else {
						bindTex(0,0);
					}
					bindLightmapCoordinatesToFirstTextureSlot = true;
					unbindTex(1);
				} else {
					// draw colormap only
					textureAPI_i *t = getStageTextureInternal(s);
					bindTex(0,t->getInternalHandleU32());
					unbindTex(1);
				}
				//if(bumpMap/* && curLight*/) {
				if(bumpMap && (curLight || deluxeMap)) {
					textureAPI_i *bumpMapTexture = bumpMap->getTexture(this->timeNowSeconds);
					bindTex(2,bumpMapTexture->getInternalHandleU32());
				} else {
					unbindTex(2);
				}
				// using heightmap requires GLSL support
				if(heightMap && glUseProgram) {
					textureAPI_i *heightMapTexture = heightMap->getTexture(this->timeNowSeconds);
					bindTex(3,heightMapTexture->getInternalHandleU32());
				} else {
					unbindTex(3);
				}
				// using deluxeMap requires GLSL support
				if(deluxeMap && glUseProgram) {
					bindTex(4,deluxeMap->getInternalHandleU32());
				} else {
					unbindTex(4);
				}
				// using cubeMap requires GLSL support
				if(cubeMap && glUseProgram) {
					selectTex(0);
					glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->getInternalHandleU32());
				} else {
					selectTex(0);
					glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
				}
				// use given vertex buffer (with VBOs created) if we dont have to do any material calculations on CPU
				const rVertexBuffer_c *selectedVertexBuffer = &verts;

				// see if we have to modify current vertex array on CPU
				// TODO: do deforms in GLSL shader
				if(s->hasTexGen() && (this->gpuTexGensSupported()==false)) {
					// copy vertices data (first big CPU bottleneck)
					// (but only if we havent done this already)
					if(selectedVertexBuffer == &verts) {
						stageVerts = verts;
						selectedVertexBuffer = &stageVerts;
						if(rb_printMemcpyVertexArrayBottleneck.getInt()) {
							g_core->Print("RB_GL: Copying %i vertices to draw material %s\n",verts.size(),lastMat->getName());
						}
					}
					// apply texgen effect (new texcoords)
					if(s->getTexGen() == TCG_ENVIRONMENT) {
						if(indices.getNumIndices() < verts.size()) {
							// if there are more vertices than indexes, we're sure that some of verts are unreferenced,
							// so we dont have to calculate texcoords for EVERY ONE of them
							stageVerts.calcEnvironmentTexCoordsForReferencedVertices(indices,this->camOriginEntitySpace);
						} else {
							stageVerts.calcEnvironmentTexCoords(this->camOriginEntitySpace);
						}
					}
				}
				// by default dont use vertex colors...
				// (right now it overrides the setting from frontend)
				bindVertexColors = false;
				if(s->hasRGBGen() && (rb_ignoreRGBGens.getInt() != 0)) {
					if(s->getRGBGenType() == RGBGEN_IDENTITY) {
						bindVertexColors = false;
					} else if(s->getRGBGenType() == RGBGEN_VERTEX && (rb_ignoreRGBGenVertex.getInt() == 0)) {
						// just use vertex colors from VBO,
						// nothing to calculate on CPU
						bindVertexColors = true;
					} else if(s->getRGBGenType() == RGBGEN_WAVE && (rb_ignoreRGBGenWave.getInt() == 0)) {
						// NOTE: "rgbGen wave inversesawtooth 0 1 0 8" 
						// and "rgbGen wave sawtooth 0 1 0 8"
						// are used in Quake3 "rocketExplosion" material from gfx.shader
						float val = s->getRGBGenWaveValue(this->timeNowSeconds*0.001f);
						byte valAsByte = val * 255.f;
#if 1
						bindVertexColors = true;
						// copy vertices data (first big CPU bottleneck)
						// (but only if we havent done this already)
						if(selectedVertexBuffer == &verts) {
							stageVerts = verts;
							selectedVertexBuffer = &stageVerts;
							if(rb_printMemcpyVertexArrayBottleneck.getInt()) {
								g_core->Print("Copying %i vertices to draw material %s\n",verts.size(),lastMat->getName());
							}
						}
						stageVerts.setVertexColorsToConstValue(valAsByte);
						stageVerts.setVertexAlphaToConstValue(255);
#else
						this->setColor4f(val,val,val,1.f);
#endif
						if(rb_printRGBGenWaveMaterials.getInt()) {
							g_core->Print("Material %s has rgbGen wave\n",lastMat->getName());
						}
					} else if(1 && s->getRGBGenType() == RGBGEN_CONST && (rb_ignoreRGBGenConst.getInt() == 0)) {
						bindVertexColors = true;
						// copy vertices data (first big CPU bottleneck)
						// (but only if we havent done this already)
						if(selectedVertexBuffer == &verts) {
							stageVerts = verts;
							selectedVertexBuffer = &stageVerts;
							if(rb_printMemcpyVertexArrayBottleneck.getInt()) {
								g_core->Print("Copying %i vertices to draw material %s\n",verts.size(),lastMat->getName());
							}
						}
						// get the constant color
						byteRGB_s col;
						vec3_c colFloats;
						s->getRGBGenConstantColor3f(colFloats);
						col.fromFloats(colFloats);
						stageVerts.setVertexColorsToConstValues(col);
						stageVerts.setVertexAlphaToConstValue(255);				
					} else if(1 && s->getRGBGenType() == RGBGEN_AST && (rb_ignoreRGBGenAST.getInt() == 0)) {
						bindVertexColors = true;
						// copy vertices data (first big CPU bottleneck)
						// (but only if we havent done this already)
						if(selectedVertexBuffer == &verts) {
							stageVerts = verts;
							selectedVertexBuffer = &stageVerts;
							if(rb_printMemcpyVertexArrayBottleneck.getInt()) {
								g_core->Print("Copying %i vertices to draw material %s\n",verts.size(),lastMat->getName());
							}
						}
						// get the constant color
						byteRGB_s col;
						vec3_c colFloats;
						// evaluate RGB expression of Doom3 material
						s->evaluateRGBGen(&materialVarList,colFloats);
						col.fromFloats(colFloats);
						stageVerts.setVertexColorsToConstValues(col);
						stageVerts.setVertexAlphaToConstValue(255);
					} else if(0) {
						//bindVertexColors = true;
						//// copy vertices data (first big CPU bottleneck)
						//// (but only if we havent done this already)
						//if(selectedVertexBuffer == &verts) {
						//	stageVerts = verts;
						//	selectedVertexBuffer = &stageVerts;
						//	if(rb_printMemcpyVertexArrayBottleneck.getInt()) {
						//		g_core->Print("Copying %i vertices to draw material %s\n",verts.size(),lastMat->getName());
						//	}
						//}
						//// apply rgbGen effect (new rgb colors)
						////bindStageColors = true; // FIXME, it's already set!!!
						//enum rgbGen_e rgbGenType = s->getRGBGenType();
						//if(rgbGenType == RGBGEN_CONST) {
						//	byteRGB_s col;
						//	vec3_c colFloats;
						//	s->getRGBGenConstantColor3f(colFloats);
						//	col.fromFloats(colFloats);
						//	stageVerts.setVertexColorsToConstValues(col);
						//	stageVerts.setVertexAlphaToConstValue(255);
						////} else if(rgbGenType == RGBGEN_WAVE) {
						////} else if(rgbGenType == RGBGEN_VERTEX) {

						//} else {

						//}
					}
				} else {
#if 1
					if(s->getRGBGenType() == RGBGEN_IDENTITY) {
						bindVertexColors = false;
					} else if(s->getRGBGenType() == RGBGEN_VERTEX && (rb_ignoreRGBGenVertex.getInt() == 0)) {
						// just use vertex colors from VBO,
						// nothing to calculate on CPU
						bindVertexColors = true;
					} else 
#endif
					{
					// if (rgbGen is not set) and (lightmap is not present) and (vertex colors are present)
					// -> enable drawing with vertex colors
					if(bHasVertexColors && (lastLightmap == 0) && (curLight == 0)) {
						// this is a fix for q3 static "md3" models 
						// (those loaded directly from .bsp file and not from .md3 file)
						bindVertexColors = true;
					}
					}
				}
	
				bool modifiedVertexArrayOnCPU = (selectedVertexBuffer != &verts);

				// see if we have to bind a GLSL shader
				glShader_c *selectedShader = 0;
				if(stageType == ST_CUBEMAP_SKYBOX) {
					glslPermutationFlags_s glslShaderDesc;
					selectedShader = GL_RegisterShader("skyboxCubeMap",&glslShaderDesc);
					if(selectedShader) {
						bindShader(selectedShader);
					} else {
						g_core->RedWarning("Cannot render ST_CUBEMAP_SKYBOX because skyboxCubeMap shader is missing.\n");
						continue;
					}
				} else if(stageType == ST_CUBEMAP_REFLECTION) {
					glslPermutationFlags_s glslShaderDesc;
					selectedShader = GL_RegisterShader("reflectionCubeMap",&glslShaderDesc);
					if(selectedShader) {
						bindShader(selectedShader);
					} else {
						g_core->RedWarning("Cannot render ST_CUBEMAP_REFLECTION because reflectionCubeMap shader is missing.\n");
						continue;
					}	
				} else if(curLight) {
					// TODO: add Q3 material effects handling to per pixel lighting GLSL shader....
					glslPermutationFlags_s pf;
					if(r_shadows == 2) {
						if(curLight->isSpotLight()) {
							pf.spotLightShadowMapping = true;
							pf.debug_showSpotLightShadows = rb_showSpotLightShadows.getInt();
						} else {
							pf.pointLightShadowMapping = true;
							pf.debug_showPointLightShadows = rb_showPointLightShadows.getInt();
						}
					}
					if(bumpMap) {
						pf.hasBumpMap = true;
					}
					if(heightMap) {
						pf.hasHeightMap = true;
					}
					pf.useReliefMapping = rb_useReliefMapping.getInt();
					pf.debug_ignoreAngleFactor = rb_dynamicLighting_ignoreAngleFactor.getInt();
					pf.debug_ignoreDistanceFactor = rb_dynamicLighting_ignoreDistanceFactor.getInt();
					pf.isSpotLight = (curLight->getLightType() == LT_SPOTLIGHT);
					pf.enableShadowMappingBlur = rb_shadowMapBlur.getInt();
					pf.useShadowCubeMap = rb_useDepthCubeMap.getInt();
					pf.isTwoSided = this->prevCullType == CT_TWO_SIDED;
					/*if(prevAlphaFunc == AF_D3_ALPHATEST) {
						pf.hasDoom3AlphaTest = true;
						pf.alphaTestValue = alphaFuncCustomValue;
					}*/

					selectedShader = GL_RegisterShader("perPixelLighting",&pf);
					if(selectedShader) {
						bindShader(selectedShader);
					} else {
						g_core->RedWarning("Cannot render light interaction because perPixelLighting shader is missing.\n");
					}
				} else if(lastMat->isPortalMaterial() == false &&
					(
					gl_alwaysUseGLSLShaders.getInt() ||
					(modifiedVertexArrayOnCPU == false && (s->hasTexGen() && this->gpuTexGensSupported()))
					||
					(heightMap != 0)
					||
					((bumpMap != 0) && (lastDeluxemap != 0))
					||
					bHasSunLight
					)
					) {
					glslPermutationFlags_s glslShaderDesc;
					if(stageType == ST_COLORMAP_LIGHTMAPPED && lastLightmap) {
						glslShaderDesc.hasLightmap = true;
					}
					if(bindVertexColors) {
						glslShaderDesc.hasVertexColors = true;
					}
					if(s->hasTexGen()) {
						glslShaderDesc.hasTexGenEnvironment = true;
					}
					if(heightMap) {
						glslShaderDesc.hasHeightMap = true;
					}
					if(bumpMap) {
						glslShaderDesc.hasBumpMap = true;
					}
					if(deluxeMap) {
						glslShaderDesc.hasDeluxeMap = true;
					}
					if(lastSurfaceColor.isFullBright() == false) {
						glslShaderDesc.hasMaterialColor = true;
					}
					if(bHasSunLight) {
						glslShaderDesc.hasSunLight = true;
						if(r_shadows == 2) {
							glslShaderDesc.hasDirectionalShadowMapping = true;
							if(bDepthFBOLod1DrawnThisFrame) {
								glslShaderDesc.bHasShadowMapLod1 = true;
							}
						}
					}
					glslShaderDesc.enableShadowMappingBlur = rb_shadowMapBlur.getInt();
					glslShaderDesc.useReliefMapping = rb_useReliefMapping.getInt();
					glslShaderDesc.debug_ignoreAngleFactor = rb_dynamicLighting_ignoreAngleFactor.getInt();
					glslShaderDesc.debug_ignoreDistanceFactor = rb_dynamicLighting_ignoreDistanceFactor.getInt();

					selectedShader = GL_RegisterShader("genericShader",&glslShaderDesc);
					if(selectedShader) {
						bindShader(selectedShader);
					}
				} else if(cubeMap) {
					glslPermutationFlags_s glslShaderDesc;
					selectedShader = GL_RegisterShader("cubeMapShader",&glslShaderDesc);
					if(selectedShader) {
						bindShader(selectedShader);
					} else {
						g_core->RedWarning("Cannot render stage with cubemap because cubeMapShader shader is missing.\n");
						continue;
					}	
				} else {
					bindShader(0);
				}
				CHECK_GL_ERRORS;

				// draw the current material stage using selected vertex buffer.
				bindVertexBuffer(selectedVertexBuffer,bindLightmapCoordinatesToFirstTextureSlot);

				drawCurIBO();
				CHECK_GL_ERRORS;
			}
		} else {
			if(curLight == 0) {
				setBlendFunc(BM_NOT_SET,BM_NOT_SET);
			} else {
				// light interactions are appended with addictive blending
				setBlendFunc(BM_ONE,BM_ONE);
			}
			drawCurIBO();
		}
		if(rb_showTris.getInt()) {
			// temporary disable stencil test for drawing triangle outlines
			if(stencilTestEnabled) {
				glDisable(GL_STENCIL_TEST);
			}
			this->unbindMaterial();
			this->bindShader(0);
			this->setColor4f(1,1,1,1);
			glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
			if(rb_showTris.getInt()==1)
				setDepthRange( 0, 0 ); 
			drawCurIBO();	
			if(rb_showTris.getInt()==1)
				setDepthRange( 0, 1 ); 
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			// restore it back
			if(stencilTestEnabled) {
				glEnable(GL_STENCIL_TEST);
			}
		}
		if(rb_showNormals.getInt()) {
			// temporary disable stencil test for drawing normal vectors
			if(stencilTestEnabled) {
				glDisable(GL_STENCIL_TEST);
			}
			this->unbindMaterial();
			this->bindShader(0);
			this->setColor4f(1,1,1,1);
			unbindIBO();
			unbindVertexBuffer();
			if(rb_showNormals.getInt()==2)
				setDepthRange( 0, 0 ); 
			glBegin(GL_LINES);
			for(u32 i = 0; i < verts.size(); i++) {
				const rVert_c &v = verts[i];
				vec3_c to = v.xyz + v.normal * 10.f;
				glVertex3fv(v.xyz);
				glVertex3fv(to);
			}
			glEnd();
			if(rb_showNormals.getInt()==2)
				setDepthRange( 0, 1 ); 
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
			// restore it back
			if(stencilTestEnabled) {
				glEnable(GL_STENCIL_TEST);
			}
		}
	}
	virtual void drawElementsWithSingleTexture(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices, class textureAPI_i *tex) {
		if(tex == 0)
			return;

		this->glCull(CT_TWO_SIDED);

		stopDrawingShadowVolumes();

		disableAllTextures();
		bindVertexBuffer(&verts);
		bindIBO(&indices);			
		bindTex(0,tex->getInternalHandleU32());
		if(bDrawingSky) {
			// fathest depth range
			setDepthRange(1.f,1.f);
		} else {
			// default depth range
			setDepthRange(0.f,1.f);
		}
		drawCurIBO();
	}
	void startDrawingShadowVolumes() {
		if(bDrawingShadowVolumes == true)
			return;
		bindShader(0);
		disableAllTextures();
		disableNormalArray();
		disableTexCoordArrayForTexSlot(1);
		disableTexCoordArrayForTexSlot(0);
		unbindVertexBuffer();
		unbindMaterial();
		// stencil buffer is now cleared when a current light is changed
		// (that's because there might be some light that has no shadows)
        //glClear(GL_STENCIL_BUFFER_BIT);
		if(rb_showShadowVolumes.getInt()) {
			setColor4f(1,1,0,0.5f);
			setBlendFunc(BM_ONE,BM_ONE);
			glDepthFunc(GL_LEQUAL); 
			glCull(CT_FRONT_SIDED);
			setColorMask(true, true, true, true); 
			if(rb_showShadowVolumes.getInt() == 2)	
				glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		} else {
			glDepthFunc(GL_LESS); // We change the z-testing function to LESS, to avoid little bugs in shadow
			setColorMask(false, false, false, false); // We dont draw it to the screen
			glStencilFunc(GL_ALWAYS, 0, 0); // We always draw whatever we have in the stencil buffer
			setGLDepthMask(false);
			setGLStencilTest(true);
		}

		bDrawingShadowVolumes = true;
	}
	void stopDrawingShadowVolumes() {
		if(bDrawingShadowVolumes == false)
			return;

		if(rb_showShadowVolumes.getInt() == 2)	
			glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

		// We draw our lighting now that we created the shadows area in the stencil buffer
        glDepthFunc(GL_LEQUAL); // we put it again to LESS or EQUAL (or else you will get some z-fighting)
		glCull(CT_FRONT_SIDED);// glCullFace(GL_FRONT);//BACK); // we draw the front face
        setColorMask(true, true, true, true); // We enable color buffer
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // Drawing will not affect the stencil buffer
        glStencilFunc(GL_EQUAL, 0x0, 0xff); // And the most important thing, the stencil function. Drawing if equal to 0

		bDrawingShadowVolumes = false;
	}
	virtual void drawIndexedShadowVolume(const class rPointBuffer_c *points, const class rIndexBuffer_c *indices) {
		if(indices->getNumIndices() == 0)
			return;
		if(points->size() == 0)
			return;
		if(rb_printShadowVolumeDrawCalls.getInt()) {
			g_core->Print("Indexed Shadow Volume: %i points, %i tris\n",points->size(),indices->getNumTriangles());
		}

		startDrawingShadowVolumes();

		bindIBO(indices);
		glVertexPointer(3,GL_FLOAT,sizeof(hashVec3_c),points->getArray());
		
		if(rb_showShadowVolumes.getInt()) {
			// draw once and we're done
			drawCurIBO(); // draw the shadow volume
			return;
		}
		glCull(CT_BACK_SIDED);
		glStencilOp(GL_KEEP, GL_INCR, GL_KEEP); // increment if the depth test fails

		drawCurIBO(); // draw the shadow volume

		glCull(CT_FRONT_SIDED);
		glStencilOp(GL_KEEP, GL_DECR, GL_KEEP); // decrement if the depth test fails

		drawCurIBO(); // draw the shadow volume

		//glVertexPointer(3,GL_FLOAT,sizeof(hashVec3_c),0);
		//bindIBO(0);
	}
	bool isUsingBlur() const {
		if(rb_forceBlur.getInt())
			return true;
		return false;
	}
	bool shouldDrawToScreenFBO() {
		if(isUsingBlur()) {
			return true;
		}
		return false;
	}
	float getBlurScale() const {
		return rb_blurScale.getFloat();
	}
	float getAverageScreenLuminance() const {
		return averageScreenLuminance;
	}
	virtual void beginFrame() {
		if(shouldDrawToScreenFBO()) {
			screenFBO.create(this->viewPortWidth,this->viewPortHeight);
			bindFBO(screenFBO.getFBOHandle());
		} else {
			bindFBO(0);
		}

		// NOTE: for stencil shadows, stencil buffer should be cleared here as well.
		if(1) {
		    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		} else {
			glClear(GL_COLOR_BUFFER_BIT);
		}
		clearDepthBuffer();
		glClearColor(0,0,0,0);

		materialVarList.setTableList(g_ms->getTablesAPI());
		materialVarList.setVariable("time",timeNowSeconds);

		// reset counters
		c_frame_vbsReusedByDifferentDrawCall = 0;
	}
	virtual void endFrame() {
		if(rb_showDepthBuffer.getInt()) {
			static arraySTD_c<float> pPixels;
			pPixels.resize(this->getWinWidth()*this->getWinHeight()*4);
			glReadPixels( 0, 0, this->getWinWidth(), this->getWinHeight(), GL_DEPTH_COMPONENT, GL_FLOAT, (void*)pPixels );
			glRasterPos2f(0,0);
			bindShader(0);
			setup2DView();
			glDrawPixels( this->getWinWidth(), this->getWinHeight(), GL_DEPTH_COMPONENT, GL_FLOAT, pPixels );
		}
		if(gl_callGLFinish.getInt()) {
			glFinish();
		}
		bRendererMirrorThisFrame = false;
		bDepthFBODrawnThisFrame = false;
		bDepthFBOLod1DrawnThisFrame = false;
		g_sharedSDLAPI->endFrame();
		if(rb_printFrameTriCounts.getInt()) {
			g_core->Print("%i input tris / %i total tris (%i in IBOs)\n",counters.c_inputTris,counters.c_totalTris,counters.c_totalTrisVBO);
		}
		if(rb_printFrameVertCounts.getInt()) {
			g_core->Print("%i input verts / %i total verts (%i in VBOs)\n",counters.c_inputVerts,counters.c_totalVerts,counters.c_totalVertsVBO);
		}
		if(rb_printIBOStats.getInt()) {
			g_core->Print("%i GPU ibos, %i CPU ibos\n",counters.c_gpuIndexBuffers,counters.c_cpuIndexBuffers);
		}
		counters.clear();
	}	
	virtual void clearDepthBuffer() {
		if(bDepthMask == false) {
			glDepthMask(true);
		}
		// glClear(GL_DEPTH_BUFFER_BIT) doesnt work when glDepthMask is false...
		glClear(GL_DEPTH_BUFFER_BIT);
		if(bDepthMask == false) {
			glDepthMask(false);
		}
	}
	void drawFullScreenQuad() {
		glBegin(GL_QUADS);
			glTexCoord2f(0.0f, 1.0f);
			glVertex2f(0, 0);
			glTexCoord2f(0.0f, 0.0f);
			glVertex2f(0, getWinHeight());
			glTexCoord2f(1.0f, 0.0f);	
			glVertex2f(getWinWidth(), getWinHeight());
			glTexCoord2f(1.0f, 1.0f);
			glVertex2f(getWinWidth(), 0);
		glEnd();
	}
	void calculateLuminance(float &min, float &max, float &avg) {
#if 1
		static arraySTD_c<float> image;
		u32 pixelCount = viewPortWidth * viewPortHeight;
		u32 componentCount = pixelCount * 4;
		if(componentCount > image.size()) {
			image.resize(componentCount);
		}
		glReadPixels(0, 0, viewPortWidth, viewPortHeight, GL_RGBA, GL_FLOAT, image);

		vec3_c lum(0.2125f, 0.7154f, 0.0721f);
		min = 0.f;
		float sum = 0.0f;
		max = 0.0f;
		for(u32 i = 0; i < componentCount; i += 4) {
			const vec3_c *col = (const vec3_c*)(&image[i]);

			float luminance = col->dotProduct(lum) + 0.0001f;
			if(luminance > max)
				max = luminance;

			sum += log(luminance);
		}
		sum /= (pixelCount);
		avg = exp(sum);
#else
		static arraySTD_c<byte> image;
		u32 pixelCount = viewPortWidth * viewPortHeight;
		u32 componentCount = pixelCount * 4;
		if(componentCount > image.size()) {
			image.resize(componentCount);
		}
		glReadPixels(0, 0, viewPortWidth, viewPortHeight, GL_RGBA, GL_BYTE, image);

		vec3_c lum(0.2125f, 0.7154f, 0.0721f);
		min = 0.f;
		float sum = 0.0f;
		max = 0.0f;
		for(u32 i = 0; i < componentCount; i += 4) {
			vec3_c col(image[i],image[i+1],image[i+2]);
			col *= (1.f/255.f);

			float luminance = col.dotProduct(lum) + 0.0001f;
			if(luminance > max)
				max = luminance;

			sum += log(luminance);
		}
		sum /= (pixelCount);
		avg = exp(sum);
#endif
	}
	virtual void setup2DView() {		
		stopDrawingShadowVolumes();
		bindShader(0);
		setGLDepthMask(true);
		setGLStencilTest(false);
		disablePortalClipPlane();
		setIsMirror(false);

#if 0
		screenFBO_64x64.create(64,64);
		
		u32 curFBO = boundFBO;
		glBindFramebufferEXT(GL_READ_FRAMEBUFFER, curFBO);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, screenFBO_64x64.getFBOHandle());
		glBlitFramebufferEXT(0, 0, screenFBO.getW(),screenFBO.getH(),
								0, 0, screenFBO_64x64.getW(),screenFBO_64x64.getH(),
								GL_COLOR_BUFFER_BIT,
								GL_LINEAR);
		glBindFramebufferEXT(GL_READ_FRAMEBUFFER, 0);
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);

		bindFBO(screenFBO_quarter.getFBOHandle());
		setGLViewPort(screenFBO_64x64.getW(),screenFBO_64x64.getH());

		float lMin, lMax, lAvg;
		calculateLuminance(lMin,lMax,lAvg);

		bindFBO(curFBO);
	
		g_core->Print("Max %f, avg %f\n",lMax,lAvg);

		averageScreenLuminance = lAvg;
#endif
		setGLViewPort(getWinWidth(),getWinHeight());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho (0, getWinWidth(), getWinHeight(), 0, 0, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		// depth test is not needed while drawing 2d graphics
		glDisable(GL_DEPTH_TEST);
		// disable materials
		turnOffPolygonOffset();
		// rVertexBuffers are used only for 3d
		unbindVertexBuffer();
		bindIBO(0); // we're not using rIndexBuffer_c system for 2d graphics
		glCull(CT_TWO_SIDED);

		if(isUsingBlur() && boundFBO) {
			glslPermutationFlags_s p;

			if(rb_forceBloom.getInt() == 0) {
				p.bHorizontalPass = true;
				glShader_c *sh = GL_RegisterShader("blur",&p);
				if(sh) {
					bindShader(sh);
				}
				screenFBO2.create(this->viewPortWidth,this->viewPortHeight);
				bindFBO(screenFBO2.getFBOHandle());

				bindTex(0,screenFBO.getTextureHandle());
				drawFullScreenQuad();

				p.bHorizontalPass = false;
				sh = GL_RegisterShader("blur",&p);
				if(sh) {
					bindShader(sh);
				}
				bindFBO(0);
				bindTex(0,screenFBO2.getTextureHandle());
				drawFullScreenQuad();
			} else {
				screenFBO2.create(this->viewPortWidth,this->viewPortHeight);
				screenFBO_quarter.create(this->viewPortWidth/4,this->viewPortHeight/4);
				
				glBindFramebufferEXT(GL_READ_FRAMEBUFFER, screenFBO.getFBOHandle());
				glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, screenFBO_quarter.getFBOHandle());
				glBlitFramebufferEXT(0, 0, screenFBO.getW(),screenFBO.getH(),
										0, 0, screenFBO_quarter.getW(),screenFBO_quarter.getH(),
										GL_COLOR_BUFFER_BIT,
										GL_LINEAR);
				glBindFramebufferEXT(GL_READ_FRAMEBUFFER, 0);
				glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, 0);

				glShader_c *sh = GL_RegisterShader("bloom",&p);
				if(sh) {
					bindShader(sh);
				}
				bindFBO(screenFBO2.getFBOHandle());
				bindTex(0,screenFBO.getTextureHandle());
				bindTex(1,screenFBO_quarter.getTextureHandle());
				drawFullScreenQuad();
				bindTex(1,0);

				
				p.bHorizontalPass = true;
				sh = GL_RegisterShader("blur",&p);
				if(sh) {
					bindShader(sh);
				}
				bindFBO(screenFBO.getFBOHandle());

				bindTex(0,screenFBO2.getTextureHandle());
				drawFullScreenQuad();

				p.bHorizontalPass = false;
				sh = GL_RegisterShader("blur",&p);
				if(sh) {
					bindShader(sh);
				}
				bindFBO(0);
				bindTex(0,screenFBO.getTextureHandle());
				drawFullScreenQuad();
			}

		}
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

		camMatrixInEntitySpace.fromAxisAndOrigin(viewAxis,camOriginWorldSpace);
		camOriginEntitySpace = this->camOriginWorldSpace;
		entityAxis.identity();
		entityOrigin.zero();
		entityMatrix.identity();
		entityRotationMatrix.identity();
		entityMatrixInverse.identity();

		usingWorldSpace = true;
	}
	virtual void setupEntitySpace(const axis_c &axis, const vec3_c &origin) {
		entityAxis = axis;
		entityOrigin = origin;

		entityMatrix.fromAxisAndOrigin(axis,origin);
		entityRotationMatrix.fromAxisAndOrigin(axis,vec3_c(0,0,0));
		entityMatrixInverse = entityMatrix.getInversed();
		camMatrixInEntitySpace.fromAxisAndOrigin(viewAxis,camOriginWorldSpace);
		camMatrixInEntitySpace = entityMatrixInverse * camMatrixInEntitySpace;

		entityMatrixInverse.transformPoint(camOriginWorldSpace,camOriginEntitySpace);

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
		camOriginWorldSpace = newCamPos;
		viewAxis = newCamAxis;

		// transform by the camera placement and view axis
		this->worldModelMatrix.invFromAxisAndVector(newCamAxis,newCamPos);
		// convert to gl coord system
		this->worldModelMatrix.toGL();

		setupWorldSpace();

		glEnable(GL_DEPTH_TEST);
	}
	virtual void setupProjection3D(const projDef_s *pd) {
		//frustum.setup(fovX, fovY, zFar, axis, origin);
		projectionMatrix.setupProjection(pd->fovX,pd->fovY,pd->zNear,pd->zFar);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glLoadMatrixf(projectionMatrix);
	}
	virtual void drawCapsuleZ(const float *xyz, float h, float w) {
		this->bindIBO(0);
		this->unbindVertexBuffer();

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
		this->bindIBO(0);
		this->unbindVertexBuffer();
		this->glCull(CT_TWO_SIDED);

		glutSolidCube(halfSizes[0]*2);
	}
	virtual void drawLineFromTo(const float *from, const float *to, const float *colorRGB, float lineWidth) {
		this->bindIBO(0);
		this->unbindVertexBuffer();
		this->unbindMaterial();

		setColor4f(colorRGB[0],colorRGB[1],colorRGB[2],1.f);
		glLineWidth(lineWidth);
		glBegin(GL_LINES);
		glVertex3fv(from);
		glVertex3fv(to);
		glEnd();
		glLineWidth(1.f);
	}	
	virtual void drawBBLines(const class aabb &bb) {
		bindShader(0);

		vec3_c verts[8];
		for(u32 i = 0; i < 8; i++) {
			verts[i] = bb.getPoint(i);
		}
		glBegin(GL_LINES);
		// mins.z
		glVertex3fv(verts[0]);
		glVertex3fv(verts[1]);
		glVertex3fv(verts[0]);
		glVertex3fv(verts[2]);
		glVertex3fv(verts[2]);
		glVertex3fv(verts[3]);
		glVertex3fv(verts[3]);
		glVertex3fv(verts[1]);
		// maxs.z
		glVertex3fv(verts[4]);
		glVertex3fv(verts[5]);
		glVertex3fv(verts[4]);
		glVertex3fv(verts[6]);
		glVertex3fv(verts[6]);
		glVertex3fv(verts[7]);
		glVertex3fv(verts[7]);
		glVertex3fv(verts[5]);
		// mins.z -> maxs.z
		glVertex3fv(verts[0]);
		glVertex3fv(verts[4]);
		glVertex3fv(verts[1]);
		glVertex3fv(verts[5]);
		glVertex3fv(verts[2]);
		glVertex3fv(verts[6]);
		glVertex3fv(verts[3]);
		glVertex3fv(verts[7]);
		glEnd();
	}        
	virtual void drawWinding(const class vec3_c *p, u32 numPoints, u32 stride) {
		glCull(CT_TWO_SIDED);
		glBegin(GL_TRIANGLE_FAN);
		for(u32 i = 0; i < numPoints; i++) {
			glVertex3fv(p->floatPtr());
			p = (const vec3_c*)(((const byte*)p)+stride);
		}
		glEnd();
	}
	virtual bool areGPUOcclusionQueriesSupported() const {
		int bitsSupported = 0;
        glGetQueryiv(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, &bitsSupported);
        if (bitsSupported == 0) {
           return false;
        }
		return true;
	}
	virtual class occlusionQueryAPI_i *allocOcclusionQuery() {
		if(areGPUOcclusionQueriesSupported() == false)
			return 0;
		glOcclusionQuery_c *ret = new glOcclusionQuery_c;
		ret->generateOcclusionQuery();
		return ret;
	}  
	virtual void setRenderTimeSeconds(float newCurTime) {
		this->timeNowSeconds = newCurTime;
	}
	virtual void setIsMirror(bool newBIsMirror) {
		if(newBIsMirror == this->isMirror)
			return;
		if(this->isMirror == true) 
			bRendererMirrorThisFrame = true;
		this->isMirror = newBIsMirror;
		// force cullType reset, because mirror views
		// must have CT_BACK with CT_FRONT swapped
		this->prevCullType = CT_NOT_SET;
	}
	virtual void setPortalDepth(u32 nPortalDepth) {
		portalDepth = nPortalDepth;
	}
	// used eg. for mirrors
	virtual void setPortalClipPlane(const class plane_c &pl, bool bEnabled) {
		double plane2[4];
#if 0
		plane2[0] = viewAxis[0].dotProduct(pl.norm);
		plane2[1] = viewAxis[1].dotProduct(pl.norm);
		plane2[2] = viewAxis[2].dotProduct(pl.norm);
		plane2[3] = pl.norm.dotProduct(camOriginWorldSpace) - pl.dist;
#else
		plane2[0] = -pl.norm.x;
		plane2[1] = -pl.norm.y;
		plane2[2] = -pl.norm.z;
		// add epsilon value, this might fix "blinking" mirrors
		plane2[3] = -pl.dist + gl_clipPlaneEpsilon.getFloat();
#endif
		if(bEnabled && (gl_ignoreClipPlanes.getInt()==0)) {
			glClipPlane (GL_CLIP_PLANE0, plane2);
			glEnable (GL_CLIP_PLANE0);
		} else {
			glDisable (GL_CLIP_PLANE0);	
		}
	}
	virtual void disablePortalClipPlane() {
		glDisable (GL_CLIP_PLANE0);	
	}
	virtual void init()  {
		if(backendInitialized) {
			g_core->Error(ERR_DROP,"rbSDLOpenGL_c::init: already initialized\n");
			return;		
		}
		// cvars
		AUTOCVAR_RegisterAutoCvars();

		// init SDL window
		g_sharedSDLAPI->init();

		u32 res = glewInit();
		if (GLEW_OK != res) {
			g_core->Error(ERR_DROP,"rbSDLOpenGL_c::init: glewInit() failed. Cannot init openGL renderer\n");
			return;
		}
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,&maxActiveTextureSlots);
		glGetIntegerv(GL_MAX_TEXTURE_COORDS,&maxActiveTextureCoords);

		// ensure that all the states are reset after vid_restart
		memset(texStates,0,sizeof(texStates));
		curTexSlot = -1;
		highestTCSlotUsed = 0;
		// materials
		lastMat = 0;
		lastLightmap = 0;
		lastDeluxemap = 0;
		bindVertexColors = 0;
		boundVBO = 0;
		boundIBO = 0;
		boundGPUIBO = false;
		blendSrc = -2;
		blendDst = -2;
		blendEnable = false;
		curLight = 0;
		bDrawOnlyOnDepthBuffer = false;
		bDrawingShadowVolumes = false;
		bDrawingSunShadowMapPass = false;
		bDepthMask = true;
		prevCullType = CT_NOT_SET;
		stencilTestEnabled = 0;
		forcedMaterialFrameNum = -1;
		curShader = 0;
		lastSurfaceColor.setFullBright();
		counters.clear();
		//glShadeModel( GL_SMOOTH );
		glDepthFunc( GL_LEQUAL );
		glEnableClientState(GL_VERTEX_ARRAY);

		// THE hack below is not needed now,
		// lightmapped surfaces were too dark because
		// they were drawn with vertex colors enabled
		// Right now, when I have added basic 
		// "rgbGen" Q3 shader keyword handling,
		// vertex colors are enabled only
		// when they are really needed...
#if 0
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
		backendInitialized = true;
	}
	virtual void shutdown(bool destroyWindow)  {
		if(backendInitialized == false) {
			g_core->Error(ERR_DROP,"rbSDLOpenGL_c::shutdown: already shutdown\n");
			return;		
		}
		depthCubeFBOs.destroy();
		screenFBO.destroy();
		GL_ShutdownGLSLShaders();
		AUTOCVAR_UnregisterAutoCvars();
		for(int i = 8; i >= 0; i--) {
			glActiveTexture(GL_TEXTURE0+i);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		lastMat = 0;
		lastLightmap = 0;
		lastDeluxemap = 0;
		if(destroyWindow) {
			g_sharedSDLAPI->shutdown();
		}
		backendInitialized = false;
	}
	virtual u32 getWinWidth() const {
		//return glConfig.vidWidth;
		return g_sharedSDLAPI->getWinWidth();
	}
	virtual u32 getWinHeight() const {
		//return glConfig.vidHeight;
		return g_sharedSDLAPI->getWinHeigth();
	}
	virtual byte *getScreenShotRGB(u32 *w, u32 *h) const {
		glFinish();

		u32 width = this->viewPortWidth;
		u32 height = this->viewPortHeight;
		byte *pixels = new byte[3 * width * height];
		if(w) {
			*w = width;
		}		
		if(h) {
			*h = height;
		}		
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		return pixels;
	}
	virtual void freeScreenShotData(byte *b) {
		delete [] b;
	};
	virtual void setViewPort(u32 newWidth, u32 newHeight) { 
		setGLViewPort(newWidth,newHeight);
	}
	virtual void setBSkipStaticEnvCubeMapStages(bool newBSkipStaticEnvCubeMapStages) {
		bSkipStaticEnvCubeMapStages = newBSkipStaticEnvCubeMapStages;
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
		CHECK_GL_ERRORS;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		enum textureWrapMode_e wrapMode = out->getWrapMode();
		if(wrapMode == TWM_CLAMP_TO_EDGE) {
			// this is used for skyboxes 
			// (without it they are shown with strange artifacts at texture edges)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		} else if(wrapMode == TWM_CLAMP) {
			// this is used for sprites
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
		}
		//glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		CHECK_GL_ERRORS;
		glBindTexture(GL_TEXTURE_2D, 0);	
		out->setInternalHandleU32(texID);
	}
	virtual void freeTextureData(class textureAPI_i *tex) {
		u32 handle = tex->getInternalHandleU32();
		glDeleteTextures(1,&handle);
		tex->setInternalHandleU32(0);
	}
	virtual void uploadLightmap(class textureAPI_i *out, const byte *data, u32 w, u32 h, bool rgba) {
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
		if(rgba) {
			glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);	
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);	
		}
		CHECK_GL_ERRORS;
		glBindTexture(GL_TEXTURE_2D, 0);
		out->setInternalHandleU32(texID);	
	}
	virtual void uploadCubeMap(class cubeMapAPI_i *out, const imageData_s *in) {
		u32 cubemap;
		glGenTextures(1, &cubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// set textures
		for (u32 i = 0; i < 6; ++i)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, in[i].w, in[i].h, 0, GL_RGBA, GL_UNSIGNED_BYTE, in[i].pic);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		out->setInternalHandleU32(cubemap);
	}
	virtual void freeCubeMap(class cubeMapAPI_i *cm) {
		u32 cubemap = cm->getInternalHandleU32();
		if(cubemap == 0)
			return;
		glDeleteTextures(1,&cubemap);
		cm->setInternalHandleU32(0);
	}
	virtual bool createVBO(class rVertexBuffer_c *vbo) {
		if(vbo->getInternalHandleU32()) {
			destroyVBO(vbo);
		}
		if(glGenBuffersARB == 0)
			return true; // VBO not supported
		u32 h;
		glGenBuffersARB(1,&h);
		vbo->setInternalHandleU32(h);
		glBindBufferARB(GL_ARRAY_BUFFER, h);
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(rVert_c)*vbo->size(), vbo->getArray(), GL_STATIC_DRAW);
		glBindBufferARB(GL_ARRAY_BUFFER, 0);
		return false; // ok
	}
	virtual bool destroyVBO(class rVertexBuffer_c *vbo) {
		if(vbo == 0)
			return true; // NULL ptr
		if(glDeleteBuffersARB == 0)
			return true; // no VBO support
		u32 h = vbo->getInternalHandleU32();
		if(h == 0)
			return true; // buffer wasnt uploaded to gpu
		glDeleteBuffersARB(1,&h);
		vbo->setInternalHandleU32(0);
		return false; // ok
	}
	virtual bool createIBO(class rIndexBuffer_c *ibo) {
		if(ibo->getInternalHandleU32()) {
			destroyIBO(ibo);
		}
		if(glGenBuffersARB == 0)
			return true; // VBO not supported
		u32 h;
		glGenBuffersARB(1,&h);
		ibo->setInternalHandleU32(h);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, h);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, ibo->getSizeInBytes(), ibo->getArray(), GL_STATIC_DRAW);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
		return false; // ok		
	}
	virtual bool destroyIBO(class rIndexBuffer_c *ibo) {
		if(ibo == 0)
			return true; // NULL ptr
		if(glDeleteBuffersARB == 0)
			return true; // no VBO support
		u32 h = ibo->getInternalHandleU32();
		if(h == 0)
			return true; // buffer wasnt uploaded to gpu
		glDeleteBuffersARB(1,&h);
		ibo->setInternalHandleU32(0);
		return false; // ok
	}
};

static rbSDLOpenGL_c g_staticSDLOpenGLBackend;

glOcclusionQuery_c::glOcclusionQuery_c() {
	oqHandle = 0;
	lastResult = 0xffffffff;
	waitingForResult = false;
}
glOcclusionQuery_c::~glOcclusionQuery_c() {

}
void glOcclusionQuery_c::generateOcclusionQuery() {
	glGenQueriesARB(1, &oqHandle);
}
void glOcclusionQuery_c::assignSphereQuery(const vec3_c &p, float radius) {
	// stop drawing shadow volumes
	g_staticSDLOpenGLBackend.stopDrawingShadowVolumes();
	g_staticSDLOpenGLBackend.setColorMask(false,false,false,false);
	g_staticSDLOpenGLBackend.setGLDepthMask(false);
	g_staticSDLOpenGLBackend.glCull(CT_TWO_SIDED);
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, oqHandle);
	glTranslatef(p.x,p.y,p.z);
	glutSolidSphere(radius, 20, 20);
	glTranslatef(-p.x,-p.y,-p.z);
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);
	g_staticSDLOpenGLBackend.setGLDepthMask(true);
	g_staticSDLOpenGLBackend.setColorMask(true,true,true,true);
	waitingForResult = true;
}
u32 glOcclusionQuery_c::getNumSamplesPassed() const {
	if(waitingForResult == false) {
		return this->lastResult;
	}
	u32 resultSamples;
	glGetQueryObjectuivARB(oqHandle, GL_QUERY_RESULT_ARB, &resultSamples);
	this->lastResult = resultSamples;
	return resultSamples;
}
bool glOcclusionQuery_c::isResultAvailable() const {
	u32 bAvail;
	glGetQueryObjectuivARB(oqHandle, GL_QUERY_RESULT_AVAILABLE_ARB, &bAvail);
	if(bAvail)
		return true;
	return false;
}
u32 glOcclusionQuery_c::waitForLatestResult() const {
	u32 bAvail;
	while(1) {
		glGetQueryObjectuivARB(oqHandle, GL_QUERY_RESULT_AVAILABLE_ARB, &bAvail);
		if(bAvail) {
			break;
		}
	}
	return getNumSamplesPassed();
}
u32 glOcclusionQuery_c::getPreviousResult() const {
	return lastResult;
}

fboDepth_c::fboDepth_c() {
	textureHandle = 0;
	fboHandle = 0;
	w = h = 0;
}
fboDepth_c::~fboDepth_c() {
	destroy();
}
bool fboDepth_c::create(u32 newW, u32 newH) {
	if(fboHandle && newW == w && newH == h)
		return false;
	// destroy previously created FBO
	destroy();

	w = newW;
	h = newH;

	// create a depth texture
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

	// normally filtering on depth texture is done bia GL_NEAREST, but Nvidia has a built-in support for Hardware filtering: use GL_LINEAR
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	// two next lines are necessary if we wan to use the convenient shadow2DProj function in the shader.
	// otherwise we have to rely on texture2DProj
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
	
	// stop the silly repating of shadowmap texture outside shadow bounds
	GLfloat border[] = {1.0f,1.0f,1.0f,1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// create a framebuffer object
	glGenFramebuffersEXT(1, &fboHandle);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboHandle);

	// attach the texture to FBO color attachment point
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, textureHandle, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("fboDepth_c::create: failed to create FBO\n");
		destroy();
		return true;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	return false;
}
void fboDepth_c::destroy() {
	if(textureHandle) {
		glDeleteTextures(1,&textureHandle);
		textureHandle = 0;
	}
	if(fboHandle) {
		glDeleteFramebuffers(1,&fboHandle);
		fboHandle = 0;
	}
}
fboScreen_c::fboScreen_c() {
	textureHandle = 0;
	fboHandle = 0;
	depthBufferHandle = 0;
	w = h = 0;
}
fboScreen_c::~fboScreen_c() {
	destroy();
}
bool fboScreen_c::create(u32 newW, u32 newH) {
	if(fboHandle && newW == w && newH == h)
		return false;
	// destroy previously created FBO
	destroy();

	w = newW;
	h = newH;

	// create target texture
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// NULL means reserve texture memory, but texels are undefined
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	//-------------------------
	glGenFramebuffersEXT(1, &fboHandle);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboHandle);
	//Attach 2D texture to this FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureHandle, 0);
	//-------------------------
	glGenRenderbuffersEXT(1, &depthBufferHandle);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBufferHandle);
	bool bNeedsStencilBuffer = true;
	if(bNeedsStencilBuffer == false) {
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, w, h);
		//-------------------------
		//Attach depth buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBufferHandle);
	} else {
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8, w, h);
		//-------------------------
		//Attach depth-stencil buffer to FBO
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBufferHandle);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, depthBufferHandle);
	}

	// check FBO status
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("fboScreen_c::create: failed to create FBO\n");
		destroy();
		return true;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	// switch back to window-system-provided framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	return false;
}	

void fboScreen_c::destroy() {
	if(textureHandle) {
		glDeleteTextures(1,&textureHandle);
		textureHandle = 0;
	}
	if(fboHandle) {
		glDeleteFramebuffers(1,&fboHandle);
		fboHandle = 0;
	}
	if(depthBufferHandle) {
		glDeleteRenderbuffers(1,&depthBufferHandle);
		depthBufferHandle = 0;
	}
}
depthCubeFBOs_c::depthCubeFBOs_c() {
	h = w = 0;
}
depthCubeFBOs_c::~depthCubeFBOs_c() {
	destroy();
}
void depthCubeFBOs_c::destroy() {
	for(u32 i = 0; i < 6; i++) {
		sides[i].destroy();
	}
	w = 0; h = 0;
}
bool depthCubeFBOs_c::create(u32 newW, u32 newH) {
	if(newW == w && newH == h)
		return false; // ok
	destroy();
	bool bError = false;
	for(u32 i = 0; i < 6; i++) {
		if(sides[i].create(newW,newH)) {
			bError = true;
		}
	}
	w = newW;
	h = newH;
	return bError;
}

void SDLOpenGL_RegisterBackEnd() {
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)&g_staticSDLOpenGLBackend,RENDERER_BACKEND_API_IDENTSTR);
}