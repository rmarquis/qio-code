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
// backEndDX9API.cpp - DirectX9 renderer backend
#include <qcommon/q_shared.h>
#include <api/iFaceMgrAPI.h>
#include <api/vfsAPI.h>
#include <api/cvarAPI.h>
#include <api/coreAPI.h>
#include <api/inputSystemAPI.h>
#include <api/sysEventCasterAPI.h>
#include <api/rbAPI.h>
#include <api/textureAPI.h>
#include <api/mtrStageAPI.h>
#include <api/mtrAPI.h>
#include <api/sdlSharedAPI.h>
#include <api/rbAPI.h>

#include <shared/r2dVert.h>
#include <math/matrix.h>
#include <math/axis.h>
#include <math/plane.h>
#include <renderer/rVertexBuffer.h>
#include <renderer/rIndexBuffer.h>
#include <renderer/rPointBuffer.h>
#include <materialSystem/mat_public.h> // alphaFunc_e etc
#include <api/rLightAPI.h>

#include <shared/cullType.h>
#include <shared/autoCvar.h>
#include <shared/fColor4.h>

#include "dx9_local.h"
#include "dx9_shader.h"

#ifdef USE_LOCAL_HEADERS
#	include "SDL.h"
#else
#	include <SDL.h>
#endif


#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

const DWORD R2DVERT_FVF = D3DFVF_XYZ | D3DFVF_TEX2;
const DWORD RVERT_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2;
// shadow volume vertices dont need anything else
const DWORD RSHADOWVOLUMEVERT_FVF = D3DFVF_XYZ;

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
inputSystemAPI_i * g_inputSystem = 0;
//sysEventCasterAPI_c *g_sysEventCaster = 0;
sdlSharedAPI_i *g_sharedSDLAPI = 0;
rbAPI_i *rb = 0;

static aCvar_c rb_printMemcpyVertexArrayBottleneck("rb_printMemcpyVertexArrayBottleneck","0");
static aCvar_c rb_showTris("rb_showTris","0");
static aCvar_c rb_gpuTexGens("rb_gpuTexGens","1");
// always use HLSL shaders, even if they are not needed for any material effects
static aCvar_c dx9_alwaysUseHLSLShaders("dx9_alwaysUseHLSLShaders","1");

// I need them in dx9_shader.cpp
IDirect3D9 *pD3D = 0;
IDirect3DDevice9 *pDev = 0;

// these are the vertex declarations for HLSL shaders ONLY
D3DVERTEXELEMENT9 dx_rVertexDecl[] =
{
    {0,  0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0},
	{0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,   0},
    {0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
    D3DDECL_END()
};
struct texState_s {
	//bool enabledTexture2D;
	//bool texCoordArrayEnabled;
	//u32 index;
	matrixExt_c mat;

	texState_s() {
		//enabledTexture2D = false;
		///texCoordArrayEnabled = false;
		///index = 0;
	}
};
#define MAX_TEXTURE_SLOTS 4
class rbDX9_c : public rbAPI_i {
	// DirectX9 window
	HWND hWnd;
	int dxWidth, dxHeight;
	// surface material and lightmap
	mtrAPI_i *lastMat;
	textureAPI_i *lastLightmap;
	// vertex declaration - only for HLSL shaders
	IDirect3DVertexDeclaration9 *rVertDecl;
	bool usingWorldSpace;
	bool bDrawingShadowVolumes;
	texState_s texStates[MAX_TEXTURE_SLOTS]; 
	bool bHasVertexColors;
	// cgame time for shader effects (texmods, deforms)
	float timeNowSeconds;
	bool bIsMirror;
	bool bRendererMirrorThisFrame;
	bool bClipPlaneEnabled;
	plane_c clipPlane;
	// matrices
	D3DXMATRIX dxView, dxWorld, dxProj;
	// queried on startup
	D3DCAPS9 devCaps;
	// per-surface color
	fcolor4_c lastSurfaceColor;

	matrix_c viewMatrix;
	matrix_c entityMatrix;
	matrix_c entityMatrixInverse;
	axis_c viewAxis; // viewer's camera axis
	vec3_c camOriginWorldSpace;
	vec3_c camOriginEntitySpace;
public:
	rbDX9_c() {
		hWnd = 0;
		lastMat = 0;
		lastLightmap = 0;
		rVertDecl = 0;
		bClipPlaneEnabled = false;
		bHasVertexColors = false;
		bRendererMirrorThisFrame = false;
		timeNowSeconds = 0.f;
		bIsMirror = false;
	}
	virtual backEndType_e getType() const {
		return BET_DX9;
	}
	bool isHLSLSupportAvaible() const {
		return true; // TODO
	}
	// returns true if "texgen environment" q3 shader effect can be done on GPU
	virtual bool gpuTexGensSupported() const {
		if(rb_gpuTexGens.getInt() == 0)
			return false;
		if(isHLSLSupportAvaible() == false)
			return false;
		return true;
	}
	void initDX9State() {
		if(pDev == 0) {
			g_core->RedWarning("rbDX9_c::initDX9State: pDev is NULL\n");
			return;
		}
		pDev->SetRenderState(D3DRS_ZENABLE, true);
		pDev->SetRenderState(D3DRS_AMBIENT,RGB(255,255,255));
		pDev->SetRenderState(D3DRS_LIGHTING,false);
		pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		pDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL); // less or EQUAL - for multistage materials

		pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);                
		pDev->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC); 
		
		//pDev->SetRenderState( D3DRS_STENCILREF,       0 );
		//pDev->SetRenderState( D3DRS_STENCILMASK,       0 );
		//pDev->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
		//pDev->SetRenderState( D3DRS_CCW_STENCILFUNC,  D3DCMP_ALWAYS );
		//pDev->SetRenderState( D3DRS_CCW_STENCILZFAIL,  D3DSTENCILOP_KEEP );
		//pDev->SetRenderState( D3DRS_CCW_STENCILFAIL,  D3DSTENCILOP_KEEP );
		//pDev->SetRenderState( D3DRS_CCW_STENCILPASS,  D3DSTENCILOP_DECR );
	}

	virtual void setMaterial(class mtrAPI_i *mat, class textureAPI_i *lightmap, class textureAPI_i *deluxemap) {
		lastMat = mat;
		lastLightmap = lightmap;
	}
	virtual void unbindMaterial() {
		lastMat = 0;
		disableLightmap();
		disableBlendFunc();
		turnOffTextureMatrices();
	}
	virtual void setColor4(const float *rgba) {
		lastSurfaceColor.fromColor4f(rgba);
	}
	virtual void setBindVertexColors(bool bBindVertexColors) {
		bHasVertexColors = bBindVertexColors;
	}
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices) {
		disableLightmap();

		pDev->SetFVF(R2DVERT_FVF);
		setCull(CT_FRONT_SIDED);

		setBlendFunc(BM_SRC_ALPHA,BM_ONE_MINUS_SRC_ALPHA);

		if(lastMat) {
			const mtrStageAPI_i *s = lastMat->getStage(0);
			IDirect3DTexture9 *texDX9 = (IDirect3DTexture9 *)s->getTexture()->getInternalHandleV();
			pDev->SetTexture(0,texDX9);
			pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,numVerts,numIndices/3,indices,D3DFMT_INDEX16,
				verts,sizeof(r2dVert_s));
		} else {
			pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,numVerts,numIndices/3,indices,D3DFMT_INDEX16,
				verts,sizeof(r2dVert_s));
		}
	}
	void setLightmap(IDirect3DTexture9 *lightmapDX9) {
		pDev->SetTexture(1,lightmapDX9);

		pDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		pDev->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pDev->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		pDev->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
		pDev->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE );
		pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);

		//pDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE4X );
		pDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		//pDev->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_ADDSIGNED );
		pDev->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		pDev->SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
		pDev->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
		pDev->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );
		pDev->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1);

		pDev->SetTextureStageState( 2, D3DTSS_COLOROP,   D3DTOP_DISABLE );
		pDev->SetTextureStageState( 2, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	}
	void disableLightmap() {
		pDev->SetTexture(1,0);
		pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
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
			pDev->SetRenderState( D3DRS_ALPHATESTENABLE, false );			
		} else if(newAlphaFunc == AF_GT0) {
			pDev->SetRenderState( D3DRS_ALPHAREF, 0x00000000 );
			pDev->SetRenderState( D3DRS_ALPHATESTENABLE, true );
			pDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );	
		} else if(newAlphaFunc == AF_GE128) {
			pDev->SetRenderState( D3DRS_ALPHAREF, 0x00000080 );
			pDev->SetRenderState( D3DRS_ALPHATESTENABLE, true );
			pDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );	
		} else if(newAlphaFunc == AF_LT128) {
			pDev->SetRenderState( D3DRS_ALPHAREF, 0x00000080 );
			pDev->SetRenderState( D3DRS_ALPHATESTENABLE, true );
			pDev->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL );			
		}
		prevAlphaFunc = newAlphaFunc;
	}
	void turnOffAlphaFunc() {
		setAlphaFunc(AF_NONE);
	}
	int blendModeEnumToDX9Blend(int in) {
		static int blendTable [] = {
			0, // BM_NOT_SET
			D3DBLEND_ZERO, // 0
			D3DBLEND_ONE, // 1
			D3DBLEND_INVSRCCOLOR, // 2
			D3DBLEND_INVDESTCOLOR, // 3
			D3DBLEND_INVSRCALPHA, // 4
			D3DBLEND_INVDESTALPHA, // 5 
			D3DBLEND_DESTCOLOR, // 6
			D3DBLEND_DESTALPHA, // 7 
			D3DBLEND_SRCCOLOR, //8 
			D3DBLEND_SRCALPHA, // 9
			D3DBLEND_ZERO//GL_SRC_ALPHA_SATURATE,
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
				pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
				//pDev->SetRenderState(D3DRS_ZENABLE, true);
			} else {
				pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
				//pDev->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
				//pDev->SetRenderState(D3DRS_ZENABLE, false);
				pDev->SetRenderState(D3DRS_SRCBLEND, blendModeEnumToDX9Blend(src));
				pDev->SetRenderState(D3DRS_DESTBLEND, blendModeEnumToDX9Blend(dst));
			}
		}
	}
	void disableBlendFunc() {
		setBlendFunc(BM_NOT_SET,BM_NOT_SET);
	}	
	// D3DRS_STENCILENABLE state
	void setDX9StencilTest(bool bEnable) {
		pDev->SetRenderState(D3DRS_STENCILENABLE,bEnable);
	}
	// glDepthMask equivalent
	// disable/enable writing to depth buffer
	void setDX9DepthMask(bool bEnable) {
		pDev->SetRenderState(D3DRS_ZWRITEENABLE,bEnable);
	}
	// GL_CULL
	cullType_e prevCullType;
	void setCull(cullType_e cullType) {
		if(prevCullType == cullType) {
			return;
		}
		prevCullType = cullType;
		if(cullType == CT_TWO_SIDED) {
			pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
		} else {
			if(bIsMirror) {
				// swap CT_FRONT with CT_BACK for mirror views
				if(cullType == CT_BACK_SIDED) {
					pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
				} else {
					pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
				}
			} else {
				if(cullType == CT_BACK_SIDED) {
					pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CW);
				} else {
					pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
				}
			}
		}
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

//		this->selectTex(slot);
		// FIXME: this is not correct! 
		pDev->SetTransform( D3DTRANSFORMSTATETYPE(D3DTS_TEXTURE0+slot), (D3DMATRIX*)&mat );
		pDev->SetTextureStageState( 0, D3DTSS_TEXTURETRANSFORMFLAGS, 
                                 D3DTTFF_COUNT2 );
	}
	void setTextureMatrixIdentity(u32 slot) {
		texState_s *ts = &this->texStates[slot];
		if(ts->mat.isIdentity())
			return; // no change
		ts->mat.setIdentity();

	//	this->selectTex(slot);
		D3DXMATRIX matTrans;
		D3DXMatrixIdentity(&matTrans);
		pDev->SetTransform( D3DTRANSFORMSTATETYPE(D3DTS_TEXTURE0+slot), &matTrans );
	}
	const rLightAPI_i *curLight;
	bool bDrawOnlyOnDepthBuffer;
	bool bDrawingSky;
	virtual void setCurLight(const class rLightAPI_i *light) {
		this->curLight = light;
	}
	virtual void setBDrawOnlyOnDepthBuffer(bool bNewDrawOnlyOnDepthBuffer) {
		bDrawOnlyOnDepthBuffer = bNewDrawOnlyOnDepthBuffer;
	}	
	virtual void beginDrawingSky() {
		bDrawingSky = true;
	}
	virtual void endDrawingSky() {
		bDrawingSky = false;
	}
	inline void drawIndexedTrimeshInternal(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		if(verts.getInternalHandleVoid() && indices.getInternalHandleVoid()) {
			pDev->SetIndices((IDirect3DIndexBuffer9 *)indices.getInternalHandleVoid());
			pDev->SetStreamSource(0,(IDirect3DVertexBuffer9*)verts.getInternalHandleVoid(),0,sizeof(rVert_c));
			pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,verts.size(),0,indices.getNumIndices()/3);
		} else {
			pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,verts.size(),indices.getNumIndices()/3,
				indices.getArray(),
				indices.getDX9IndexType(),verts.getArray(),sizeof(rVert_c));
		}
	}
	inline void drawIndexedTrimesh(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		drawIndexedTrimeshInternal(verts,indices);
		// show triangle outlines (classic "r_showTris" effect; only for debuging)
		if(rb_showTris.getInt()) {
			pDev->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
			if(rb_showTris.getInt()==1) {
				pDev->SetRenderState(D3DRS_ZENABLE,false);
			}
			disableLightmap();
			disableBlendFunc();
			turnOffTextureMatrices();
			pDev->SetTexture(0,0);
			drawIndexedTrimeshInternal(verts,indices);
			pDev->SetRenderState(D3DRS_FILLMODE,D3DFILL_SOLID);
			if(rb_showTris.getInt()==1) {
				pDev->SetRenderState(D3DRS_ZENABLE,true);
			}
		}
	}
	hlslShader_c *boundShader;
	bool bindHLSLShader(hlslShader_c *newShader, const mtrStageAPI_i *stage) {
		if(newShader == 0) {
			boundShader = 0;
			return true; // error
		}
		if(newShader->isValid() == false) {
			boundShader = 0;
			return true; // error
		}
		// ensure that IDirect3DVertexDeclaration9 is initialized
		// (it's needed only for rendering with HLSL shaders)
		initRVertDecl();
		if(rVertDecl == 0) {
			boundShader = 0;
			return true; // somehow vertex decl creation failed
		}
		// set vertex declation
		pDev->SetVertexDeclaration(rVertDecl);
		// select technique handle
		//D3DXHANDLE hTechnique = newShader->effect->GetTechniqueByName("SimpleTexturing");
		D3DXHANDLE hTechnique = newShader->effect->GetTechnique(0);
		newShader->effect->SetTechnique(hTechnique);
		// set view origin (in entity space)
		newShader->effect->SetValue("viewOrigin", camOriginEntitySpace, sizeof(vec3_c));
		// set final matrix, which we will use in shader to transform vertices
		D3DXMATRIX worldViewProjectionMatrix;
		worldViewProjectionMatrix = dxWorld * dxView * dxProj;
		newShader->effect->SetMatrix("worldViewProjectionMatrix", &worldViewProjectionMatrix);
		// set colorMap and lightMap textures
		if(stage) {
			IDirect3DTexture9 *colorMapDX9 = (IDirect3DTexture9 *)stage->getTexture(this->timeNowSeconds)->getInternalHandleV();
			newShader->effect->SetTexture("colorMapTexture", colorMapDX9);
		} else {
			newShader->effect->SetTexture("colorMapTexture", 0);
		}
		if(lastLightmap) {
			IDirect3DTexture9 *lightMapDX9 = (IDirect3DTexture9 *)lastLightmap->getInternalHandleV();
			newShader->effect->SetTexture("lightMapTexture", lightMapDX9);
		} else {
			newShader->effect->SetTexture("lightMapTexture", 0);
		}
		if(bClipPlaneEnabled) {
			newShader->effect->SetBool("bHasClipPlane",true);
			newShader->effect->SetValue("clipPlaneNormal", clipPlane.norm, sizeof(vec3_c));
			newShader->effect->SetFloat("clipPlaneDist", clipPlane.dist);
		} else {
			newShader->effect->SetBool("bHasClipPlane",false);
		}
		newShader->effect->SetValue("materialColor",lastSurfaceColor.toPointer(),sizeof(lastSurfaceColor));
		// if we're drawing a lighting pass, send light paramters to shader
		if(curLight) {
			const vec3_c &xyz = curLight->getOrigin();
			if(usingWorldSpace) {
				newShader->effect->SetValue("lightOrigin", xyz, sizeof(vec3_c));
			} else {
				matrix_c entityMatrixInv = entityMatrix.getInversed();
				vec3_c xyzLocal;
				entityMatrixInv.transformPoint(xyz,xyzLocal);
				newShader->effect->SetValue("lightOrigin", xyzLocal, sizeof(vec3_c));
			}
			newShader->effect->SetFloat("lightRadius", curLight->getRadius());
		}
		boundShader = newShader;
		// shader is now setup and ready for drawing...
		return false;
	}
	rVertexBuffer_c stageVerts;		
	void drawStageFixedFunctionPipeline(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices, const class mtrStageAPI_i *s) {
		enum stageType_e stageType = s->getStageType();
		IDirect3DTexture9 *texDX9 = (IDirect3DTexture9 *)s->getTexture()->getInternalHandleV();
		// draw without HLSL shader (fixed function pipeline)
		pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
		pDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		if(stageType == ST_COLORMAP_LIGHTMAPPED) {
			// draw multitextured surface with
			// - colormap at slot 0
			// - lightmap at slot 1
			// bind colormap (from material stage)
			pDev->SetTexture(0,texDX9);
			// bind lightmap (only for BSP planar surfaces and bezier patches)
			if(lastLightmap) {
				setLightmap((IDirect3DTexture9 *)lastLightmap->getInternalHandleV());
			} else {
				disableLightmap();
			}
		} else if(stageType == ST_LIGHTMAP) {
			// bind lightmap to FIRST texture slot
			if(lastLightmap) {
				pDev->SetTexture(0,(IDirect3DTexture9 *)lastLightmap->getInternalHandleV());
			} else {
				pDev->SetTexture(0,0);
			}
			// disable second slot lightmap
			disableLightmap();

#if 1
			pDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_PREMODULATE );
#else
			pDev->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
#endif
			// make first texture slot use second slot coordinates (lightmap coords)
			pDev->SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1);
			//// bind lightmap (only for BSP planar surfaces and bezier patches)
			//if(lastLightmap) {
			//	setLightmap((IDirect3DTexture9 *)lastLightmap->getInternalHandleV());
			//} else {
			//	disableLightmap();
			//}
		} else {
			// draw colormap only
			pDev->SetTexture(0,texDX9);
			disableLightmap();
		}
		// use given vertex buffer (with VBOs created) if we dont have to do any material calculations on CPU
		const rVertexBuffer_c *selectedVertexBuffer = &verts;

		// see if we have to modify current vertex array on CPU
		// TODO: do deforms and texgens in HLSL shader
		if(s->hasTexGen() && (this->gpuTexGensSupported()==false)) {
			// copy vertices data (first big CPU bottleneck)
			// (but only if we havent done this already)
			if(selectedVertexBuffer == &verts) {
				stageVerts = verts;
				selectedVertexBuffer = &stageVerts;
				if(rb_printMemcpyVertexArrayBottleneck.getInt()) {
					g_core->Print("RB_DX9: Copying %i vertices to draw material %s\n",verts.size(),lastMat->getName());
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
		drawIndexedTrimesh(*selectedVertexBuffer,indices);
	}	
	void drawStageWithHLSLShader(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices, const class mtrStageAPI_i *s) {
		if(verts.getInternalHandleVoid() && indices.getInternalHandleVoid()) {
			pDev->SetIndices((IDirect3DIndexBuffer9 *)indices.getInternalHandleVoid());
			pDev->SetStreamSource(0,(IDirect3DVertexBuffer9*)verts.getInternalHandleVoid(),0,sizeof(rVert_c));
		}
		u32 numDX9EffectPasses;
		boundShader->effect->Begin(&numDX9EffectPasses,0);
		for(u32 i = 0; i < numDX9EffectPasses; i++) {
			boundShader->effect->BeginPass(i);
			if(verts.getInternalHandleVoid() && indices.getInternalHandleVoid()) {
				pDev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,0,verts.size(),0,indices.getNumIndices()/3);
			} else {
				pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,verts.size(),indices.getNumIndices()/3,
					indices.getArray(),
					indices.getDX9IndexType(),verts.getArray(),sizeof(rVert_c));
			}
			boundShader->effect->EndPass();
		}
		boundShader->effect->End();
	}
	void setDepthRange(float min, float max) {
		D3DVIEWPORT9 v;
		pDev->GetViewport(&v);
		v.MinZ = min;
		v.MaxZ = max;
		pDev->SetViewport(&v);
	}
	virtual void drawElements(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		pDev->SetFVF(RVERT_FVF);

		stopDrawingShadowVolumes();		

		if(lastMat) {
			setCull(lastMat->getCullType());
		} else {
			setCull(CT_FRONT_SIDED);
		}

		if(bDrawOnlyOnDepthBuffer) {
			turnOffAlphaFunc();
			if(bRendererMirrorThisFrame) {
				if(lastMat->isMirrorMaterial()) {
					pDev->SetRenderState(D3DRS_COLORWRITEENABLE,0);
					bindHLSLShader(0,0);
				} else {
					// non-mirrored view should never be blended with mirror view,
					// so draw all non-mirrored surfaces in draw color
					pDev->SetRenderState(D3DRS_COLORWRITEENABLE,
						D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);				
					hlslShader_c *blackFillShader = DX9_RegisterShader("blackFill",0);
					bindHLSLShader(blackFillShader,0);
				}
			} else {
				// set the color mask
				pDev->SetRenderState(D3DRS_COLORWRITEENABLE,0);
				bindHLSLShader(0,0);
			}
			setDX9DepthMask(true);
			if(boundShader) {
				drawStageWithHLSLShader(verts,indices,0);
			} else {
				drawIndexedTrimesh(verts,indices);
			}
			return;
		}
		// set the color mask
		pDev->SetRenderState(D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);

		// adjust the depth range for sky
		if(bDrawingSky) {
			this->setDepthRange(1,1);
		} else {
			this->setDepthRange(0,1);
		}

		if(lastMat) {
			u32 numStages = lastMat->getNumStages();
			for(u32 i = 0; i < numStages; i++) {
				const mtrStageAPI_i *s = lastMat->getStage(i);
				enum stageType_e stageType = s->getStageType();
				IDirect3DTexture9 *texDX9 = (IDirect3DTexture9 *)s->getTexture()->getInternalHandleV();
				
				// set alphafunc (for grates, etc)
				setAlphaFunc(s->getAlphaFunc());
				// set blendfunc (for particles, etc)
				if(curLight) {
					// light interactions are appended with addictive blending
					setBlendFunc(BM_ONE,BM_ONE);
				} else {
					const blendDef_s &bd = s->getBlendDef();
					setBlendFunc(bd.src,bd.dst);
				}
				if(curLight == 0) {
					if(s->getDepthWrite()==false) {
						setDX9DepthMask(false);
					} else {
						setDX9DepthMask(true);
					}
				}

				if(s->hasTexMods()) {
					matrix_c mat;
					s->applyTexMods(mat,this->timeNowSeconds,0);
					this->setTextureMatrixCustom(0,mat);
				} else {
					this->setTextureMatrixIdentity(0);
				}	

				// select the shader

				// true if we have an effect that's faster on GPU path (eg. "texgen environment")
				bool bShouldUseGPU = false;
				// true if we have an effect that must be calculated on CPU
				bool bMustUseCPU = false;
				hlslShader_c *selectedShader = 0;
				hlslPermutationFlags_s shaderDef;
				if(stageType == ST_COLORMAP_LIGHTMAPPED) {
					if(lastLightmap) {
						shaderDef.hasLightmap = true;
					}
				}
				if(stageType == ST_LIGHTMAP) {
					shaderDef.onlyLightmap = true;
					shaderDef.hasLightmap = true;
				}
				if(s->hasTexGen()) {
					// genericShader.fx can do "texgen enviromental" effect on GPU
					shaderDef.hasTexGenEnvironment = true;
					if(this->gpuTexGensSupported()) {
						bShouldUseGPU = true;
					} else {
						bMustUseCPU = true;
					}
				}
				bool bindVertexColors = false;
				if(bHasVertexColors) {
					if(lastLightmap == 0) {
						// if we have vertex colors but dont have lightmap
						bindVertexColors = true; // use vertex colors
					}
				}
				if(bindVertexColors) {
					pDev->SetRenderState(D3DRS_COLORVERTEX,true);
					pDev->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
					shaderDef.hasVertexColors = true;
				} else {
					pDev->SetRenderState(D3DRS_COLORVERTEX,false);
				}
				if(lastSurfaceColor.isFullBright() == false) {
					shaderDef.hasMaterialColor = true;
				}
				if(curLight) {
					selectedShader = DX9_RegisterShader("perPixelLighting",&shaderDef);
				} else {
					if(
						dx9_alwaysUseHLSLShaders.getInt()
						||
						(bMustUseCPU == false && bShouldUseGPU == true)
						) {
						selectedShader = DX9_RegisterShader("genericShader",&shaderDef);
					}
				}
				// bind the selected shader (NULL or valid)
				bindHLSLShader(selectedShader,s); // bindHLSLShader call will set this->boundShader pointer


				if(boundShader == 0) {
					drawStageFixedFunctionPipeline(verts,indices,s);
				} else {
					drawStageWithHLSLShader(verts,indices,s);
				}
			}
		}
	}	
	virtual void drawElementsWithSingleTexture(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices, class textureAPI_i *tex) {

	}	
	void startDrawingShadowVolumes() {
		if(bDrawingShadowVolumes == true)
			return;
		unbindMaterial();
		pDev->Clear(0, 0, D3DCLEAR_STENCIL, 0, 1, 0); // clear the stencil buffer before drawing new light
		pDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESS); // We change the z-testing function to LESS, to avoid little bugs in shadow
		pDev->SetRenderState(D3DRS_COLORWRITEENABLE,0);
		pDev->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS); // always draw to stencil buffer
		pDev->SetRenderState(D3DRS_STENCILREF, 0);
		pDev->SetRenderState(D3DRS_STENCILMASK, 0);
		setDX9DepthMask(false);
		setDX9StencilTest(true);

		bDrawingShadowVolumes = true;
	}
	void stopDrawingShadowVolumes() {
		if(bDrawingShadowVolumes == false)
			return;

		// We draw our lighting now that we created the shadows area in the stencil buffer
		pDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL); // we put it again to LESS or EQUAL (or else you will get some z-fighting)
		setCull(CT_FRONT_SIDED); // we draw the front face
		pDev->SetRenderState(D3DRS_COLORWRITEENABLE,
			D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);

		pDev->SetRenderState( D3DRS_STENCILREF, 0 );
		pDev->SetRenderState( D3DRS_STENCILMASK, 0xffffffff );
		pDev->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
		pDev->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
		pDev->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
		pDev->SetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
		pDev->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_EQUAL );

		bDrawingShadowVolumes = false;
	}
	virtual void drawIndexedShadowVolume(const class rPointBuffer_c *points, const class rIndexBuffer_c *indices) {
		if(points == 0) {
			return;
		}
		if(indices == 0) {
			return;
		}
		if(indices->getNumIndices() == 0) {
			return;
		}
		if(points->size() == 0) {
			return;
		}
		startDrawingShadowVolumes();

		pDev->SetFVF(RSHADOWVOLUMEVERT_FVF);

		// TODO: incr/decr when a depth test fail
		// the current code causes problems when a viewer eye is inside shadow volume

		setCull(CT_FRONT_SIDED);

		pDev->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR );

		pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,points->size(),indices->getNumIndices()/3,indices->getArray(),indices->getDX9IndexType(),
			points->getArray(),sizeof(hashVec3_c)); // draw the shadow volume

		setCull(CT_BACK_SIDED);

		pDev->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR );

		pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,points->size(),indices->getNumIndices()/3,indices->getArray(),indices->getDX9IndexType(),
			points->getArray(),sizeof(hashVec3_c)); // draw the shadow volume
	}
	virtual void beginFrame() {
		pDev->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1, 0);
		pDev->BeginScene();
	}
	virtual void endFrame() {
		bRendererMirrorThisFrame = false;
		pDev->EndScene();
		//g_sharedSDLAPI->endFrame();
		pDev->Present(0, 0, 0, 0);
	}
	virtual void clearDepthBuffer() {
		pDev->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1, 0);
	}
	virtual void setup2DView() {
		setDX9StencilTest(false);
		setDX9DepthMask(true);
		disablePortalClipPlane();
		setIsMirror(false);

		D3DVIEWPORT9 vp;
		vp.X = 0;
		vp.Y = 0;
		vp.Width = dxWidth;
		vp.Height = dxHeight;
		vp.MinZ = 0.f;
		vp.MaxZ = 1.f;
		pDev->SetViewport(&vp);
		D3DXMATRIX ortho;
		//D3DXMatrixOrthoRH(&ortho,win_width,win_height,0,1);
		D3DXMatrixOrthoOffCenterLH(&ortho,0,dxWidth,dxHeight,0,0,1);
		pDev->SetTransform(D3DTS_PROJECTION,&ortho);

		D3DXMATRIX id;
		D3DXMatrixIdentity(&id);
		pDev->SetTransform(D3DTS_WORLD,&id);
		pDev->SetTransform(D3DTS_VIEW,&id);
	}
	virtual void setup3DView(const class vec3_c &newCamPos, const class axis_c &newCamAxis) {
		camOriginWorldSpace = newCamPos;
		viewAxis = newCamAxis;

		// transform by the camera placement and view axis
		viewMatrix.invFromAxisAndVector(newCamAxis,newCamPos);
		// convert to gl coord system
		viewMatrix.toGL();

		dxView = *(const D3DMATRIX *)&viewMatrix;
		pDev->SetTransform(D3DTS_VIEW, (const D3DMATRIX *)&viewMatrix);

		setupWorldSpace();
	}
	virtual void setupProjection3D(const struct projDef_s *pd) {
		matrix_c proj;
		proj.setupProjection(pd->fovX,pd->fovY,pd->zNear,pd->zFar);

		dxProj = *(const D3DMATRIX *)&proj;
		pDev->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX *)&proj);
	}
	virtual void drawCapsuleZ(const float *xyz, float h, float w) {
	}
	virtual void drawBoxHalfSizes(const float *halfSizes) {
	}
	virtual void drawLineFromTo(const float *from, const float *to, const float *colorRGB, float lineWidth) {
	}
	virtual void drawBBLines(const class aabb &bb) {
	}
	virtual void setRenderTimeSeconds(float newCurTime) {
		this->timeNowSeconds = newCurTime;
	}
	virtual void setIsMirror(bool newBIsMirror) {
		if(newBIsMirror == this->bIsMirror)
			return;
		if(this->bIsMirror == true) 
			bRendererMirrorThisFrame = true;
		this->bIsMirror = newBIsMirror;
		// force cullType reset, because mirror views
		// must have CT_BACK with CT_FRONT swapped
		this->prevCullType = CT_NOT_SET;
	}
	virtual void setPortalClipPlane(const class plane_c &pl, bool bEnabled) {
		plane_c plInversed = pl.getOpposite();
		pDev->SetClipPlane(0,plInversed.norm);
		// store plane equation for HLSL shaders
		this->clipPlane = plInversed;
		// set D3D render state for fixed pipeline rendering
		if(bEnabled) {
			pDev->SetRenderState(D3DRS_CLIPPLANEENABLE,true);
			bClipPlaneEnabled = true;
		} else {
			pDev->SetRenderState(D3DRS_CLIPPLANEENABLE,false);
			bClipPlaneEnabled = false;
		}
	}
	virtual void disablePortalClipPlane() {
		pDev->SetRenderState(D3DRS_CLIPPLANEENABLE,false);
		bClipPlaneEnabled = false;
	}
	// used while drawing world surfaces and particles
	virtual void setupWorldSpace() {
		entityMatrix.identity();

		dxWorld = *(const D3DMATRIX *)&entityMatrix;
		pDev->SetTransform(D3DTS_WORLD, (const D3DMATRIX *)&entityMatrix);

		camOriginEntitySpace = this->camOriginWorldSpace;

		usingWorldSpace = true;
	}
	// used while drawing entities
	virtual void setupEntitySpace(const class axis_c &axis, const class vec3_c &origin) {
		entityMatrix.fromAxisAndOrigin(axis,origin);
		entityMatrixInverse = entityMatrix.getInversed();

		entityMatrixInverse.transformPoint(camOriginWorldSpace,camOriginEntitySpace);

		dxWorld = *(const D3DMATRIX *)&entityMatrix;
		pDev->SetTransform(D3DTS_WORLD, (const D3DMATRIX *)&entityMatrix);
		
		usingWorldSpace = false;
	}
	// same as above but with angles instead of axis
	virtual void setupEntitySpace2(const class vec3_c &angles, const class vec3_c &origin) {
		axis_c ax;
		ax.fromAngles(angles);
		setupEntitySpace(ax,origin);
	}

	virtual u32 getWinWidth() const  {
		return dxWidth;
	}
	virtual u32 getWinHeight() const  {
		return dxHeight;
	}

	virtual void uploadTextureRGBA(class textureAPI_i *out, const byte *data, u32 w, u32 h) {
		IDirect3DTexture9 *tex = 0;
		HRESULT hr = pDev->CreateTexture(w,h,0,D3DUSAGE_AUTOGENMIPMAP,D3DFMT_A8R8G8B8,D3DPOOL_MANAGED,&tex,0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadTexture: pDev->CreateTexture failed\n");
			return;
		}
		D3DLOCKED_RECT rect;
		hr = tex->LockRect(0, &rect, 0, 0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadTexture: tex->LockRect failed\n");
			return;
		}
#if 0
		memcpy(rect.pBits,pic,w*h*4);
#else
		byte *outDXData = (byte*)rect.pBits;
		for(u32 i = 0; i < w; i++) {
			for(u32 j = 0; j < h; j++) {
				const byte *inPixel = data + (i * h + j)*4;
				byte *outPixel = outDXData + (i * h + j)*4;
#if 1
				outPixel[0] = inPixel[2];
				outPixel[1] = inPixel[1];
				outPixel[2] = inPixel[0];
				outPixel[3] = inPixel[3];
#else
				outPixel[0] = inPixel[3];
				outPixel[1] = inPixel[0];
				outPixel[2] = inPixel[1];
				outPixel[3] = inPixel[2];
#endif
			}
		}
#endif
		hr = tex->UnlockRect(0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadTexture: tex->UnlockRect(0) failed\n");
			return;
		}
		out->setWidth(w);
		out->setHeight(h);
		out->setInternalHandleV(tex);
	}
	virtual void uploadLightmap(class textureAPI_i *out, const byte *data, u32 w, u32 h, bool rgba) {
		IDirect3DTexture9 *tex = 0;
		// lightmaps dont have alpha channel
		HRESULT hr = pDev->CreateTexture(w,h,0,D3DUSAGE_AUTOGENMIPMAP,D3DFMT_X8R8G8B8,D3DPOOL_MANAGED,&tex,0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadLightmap: pDev->CreateTexture failed\n");
			return;
		}
		D3DLOCKED_RECT rect;
		hr = tex->LockRect(0, &rect, 0, 0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadLightmap: tex->LockRect failed\n");
			return;
		}
		byte *outPicData = (byte*)rect.pBits;
		const byte *inPixel = data;
		byte *outPixel = outPicData;
		for(u32 i = 0; i < w*h; i++) {
			outPixel[0] = inPixel[2];
			outPixel[1] = inPixel[1];
			outPixel[2] = inPixel[0];
			outPixel[3] = 255;
			outPixel += 4;
			inPixel += 3;
		}
		hr = tex->UnlockRect(0);
		if (FAILED(hr)) {
			out->setInternalHandleV(0);
			printf("rbDX9_c::uploadLightmap: tex->UnlockRect(0) failed\n");
			return;
		}
		out->setWidth(w);
		out->setHeight(h);
		out->setInternalHandleV(tex);
	}
	virtual void freeTextureData(class textureAPI_i *tex) {
		IDirect3DTexture9 *texD9 = (IDirect3DTexture9 *)tex->getInternalHandleV();
		if(texD9 == 0)
			return;
		texD9->Release();
		tex->setInternalHandleV(0);
	}

	bool initRVertDecl() {
		if(rVertDecl)
			return false;
		HRESULT hr = pDev->CreateVertexDeclaration(dx_rVertexDecl, &rVertDecl);
		if (FAILED(hr)) {
			g_core->RedWarning("refApiDX9_c::initRVertDecl: pDev->CreateVertexDeclaration failed\n");
			return true;
		}
		return false;
	}

	// vertex buffers (VBOs)
	virtual bool createVBO(class rVertexBuffer_c *ptr) {
		IDirect3DVertexBuffer9 *vbo = 0;
		HRESULT createResult = pDev->CreateVertexBuffer(ptr->getSizeInBytes(), D3DUSAGE_WRITEONLY, RVERT_FVF, D3DPOOL_MANAGED, &vbo, NULL);
		if (FAILED(createResult)) {
			g_core->RedWarning("refApiDX9_c::createVBO: pDev->CreateVertexBuffer failed\n");
			return true;
		}
		void *bufData;
		HRESULT lockResult = vbo->Lock(0,0,&bufData,0);
		if (FAILED(lockResult)) {
			g_core->RedWarning("refApiDX9_c::createVBO: vbo->Lock failed\n");
			return true; 
		}
		memcpy(bufData,ptr->getArray(),ptr->getSizeInBytes());
		HRESULT unlockResult = vbo->Unlock();
		if (FAILED(unlockResult)) {
			g_core->RedWarning("refApiDX9_c::createVBO: vbo->Unlock() failed\n");
			return true;
		}
		ptr->setInternalHandleVoid((void*)vbo);
		return false;
	}
	virtual bool destroyVBO(class rVertexBuffer_c *ptr) {
		if(ptr->getInternalHandleVoid() == 0)
			return true;
		IDirect3DVertexBuffer9 *vbo = (IDirect3DVertexBuffer9 *)ptr->getInternalHandleVoid();
		vbo->Release();
		ptr->setInternalHandleVoid(0);
		return false;
	}

	// index buffers (IBOs)
	virtual bool createIBO(class rIndexBuffer_c *ptr) {	
		IDirect3DIndexBuffer9 *ibo = 0;
		u32 iboSizeInBytes = ptr->getSizeInBytes();
		if(iboSizeInBytes == 0) {
			return true; // pDev->CreateIndexBuffer would fail for 0 bytes
		}
		HRESULT createResult = pDev->CreateIndexBuffer(iboSizeInBytes, D3DUSAGE_WRITEONLY, ptr->getDX9IndexType(), D3DPOOL_MANAGED, &ibo, NULL);
		if (FAILED(createResult)) {
			g_core->RedWarning("refApiDX9_c::createIBO: pDev->CreateIndexBuffer failed\n");
			return true;
		}
		void *bufData;
		HRESULT lockResult = ibo->Lock(0,0,&bufData,0);
		if (FAILED(lockResult)) {
			g_core->RedWarning("refApiDX9_c::createIBO: ibo->Lock failed\n");
			return true;
		}
		memcpy(bufData,ptr->getArray(),iboSizeInBytes);
		HRESULT unlockResult = ibo->Unlock();
		if (FAILED(unlockResult)) {
			g_core->RedWarning("refApiDX9_c::createIBO: ibo->Unlock() failed\n");
			return true;
		}
		ptr->setInternalHandleVoid(ibo);
		return false;
	}
	virtual bool destroyIBO(class rIndexBuffer_c *ibo) {
		if(ibo->getInternalHandleVoid() == 0)
			return false;
		IDirect3DIndexBuffer9 *ib = (IDirect3DIndexBuffer9 *)ibo->getInternalHandleVoid();
		ib->Release();
		ibo->setInternalHandleVoid(0);
		return false;
	}

	virtual void init() {
		// cvars
		AUTOCVAR_RegisterAutoCvars();
		// init SDL window
		g_sharedSDLAPI->init();

		// hack to get HWND (that wasnt needed for GL!)
		// I hope it won't cause any bugs
		hWnd = GetActiveWindow();

		ShowWindow(hWnd, 5);

		// create Dx9 device
		pD3D = Direct3DCreate9(D3D_SDK_VERSION);
		if(pD3D == 0) {
			g_core->RedWarning("rbDX9_c::initDX9State: Direct3DCreate9 failed\n");
			// TODO: return error code
			return;
		}

		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.Windowed = true;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		// this turns off V-sync (60 fps limit)
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		dxWidth = d3dpp.BackBufferWidth = g_sharedSDLAPI->getWinWidth(); // CreateDevice will crash if its set to 0 while Windowed is set to true
		dxHeight = d3dpp.BackBufferHeight = g_sharedSDLAPI->getWinHeigth();

		// add z buffer for depth tests
		d3dpp.EnableAutoDepthStencil = true;
		//d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
		d3dpp.Flags = D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;

		pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, 
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDev);
		if(pDev == 0) {
			g_core->RedWarning("rbDX9_c::initDX9State: pD3D->CreateDevice failed\n");
			// TODO: return error code
			return;
		}

		// get caps
		ZeroMemory( &devCaps, sizeof(devCaps) );
		HRESULT getCapsResult = pDev->GetDeviceCaps(&devCaps);
		if(FAILED(getCapsResult)) {
			g_core->RedWarning("rbDX9_c::init: pDev->GetDeviceCaps FAILED\n");
		} else {

		}
		ShowWindow(hWnd,SW_SHOW);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);

		curLight = 0;
		bDrawOnlyOnDepthBuffer = false;
		rVertDecl = 0;
		bDrawingShadowVolumes = false;
		prevCullType = CT_NOT_SET;
		boundShader = 0;
		bHasVertexColors = false;
		bDrawingSky = false;
		 
		initDX9State();

		if(g_inputSystem == 0) {
			g_core->RedWarning("rbDX9_c::initDX9State: g_inputSystem is NULL\n");
		} else {
			// This depends on SDL_INIT_VIDEO, hence having it here
			g_inputSystem->IN_Init();
		}
	}
	virtual void shutdown(bool destroyWindow) {
		AUTOCVAR_UnregisterAutoCvars();

		lastMat = 0;
		lastLightmap = 0;

		if(pDev) {
			pDev->Release();
			pDev = 0;
		}
		if(pD3D) {
			pD3D->Release();
			pD3D = 0;
		}
		if(rVertDecl) {
			rVertDecl->Release();
			rVertDecl = 0;
		}	
		DX9_ShutdownHLSLShaders();
		hWnd = 0;
		if(destroyWindow) {
			g_sharedSDLAPI->shutdown();
		}
	}
};


static rbDX9_c g_staticDX9Backend;

void ShareAPIs(iFaceMgrAPI_i *iFMA) {
	g_iFaceMan = iFMA;

	// exports
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)&g_staticDX9Backend,RENDERER_BACKEND_API_IDENTSTR);

	// imports
	g_iFaceMan->registerIFaceUser(&g_vfs,VFS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_cvars,CVARS_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_core,CORE_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_inputSystem,INPUT_SYSTEM_API_IDENTSTR);
	//g_iFaceMan->registerIFaceUser(&g_sysEventCaster,SYSEVENTCASTER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_sharedSDLAPI,SHARED_SDL_API_IDENTSTRING);
	g_iFaceMan->registerIFaceUser(&rb,RENDERER_BACKEND_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_REF_BACKEND_DX9;
}

