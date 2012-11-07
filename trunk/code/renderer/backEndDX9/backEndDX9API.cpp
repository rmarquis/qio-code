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
// backEndDX9API.cpp
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

#include <shared/r2dVert.h>
#include <math/matrix.h>
#include <math/axis.h>
#include <renderer/rVertexBuffer.h>
#include <renderer/rIndexBuffer.h>

#ifdef USE_LOCAL_HEADERS
#	include "SDL.h"
#else
#	include <SDL.h>
#endif

#include <d3dx9.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

const DWORD R2DVERT_FVF = D3DFVF_XYZ | D3DFVF_TEX2;
const DWORD RVERT_FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2;

// interface manager (import)
class iFaceMgrAPI_i *g_iFaceMan = 0;
// imports
vfsAPI_s *g_vfs = 0;
cvarsAPI_s *g_cvars = 0;
coreAPI_s *g_core = 0;
inputSystemAPI_i * g_inputSystem = 0;
sysEventCasterAPI_c *g_sysEventCaster = 0;

//#include <windows.h> // DX9 is windows only
//
//static int prevMouseX = -999;
//static int prevMouseY = -999;
//LRESULT CALLBACK DX9WindowWNDProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	//switch (uMsg) {
//	//	case WM_MOUSEMOVE:
//	//	{
//	//		int xPos = LOWORD(lParam); 
//	//		int yPos = HIWORD(lParam); 
//	//		if(prevMouseX == -999) {
//	//			prevMouseX = xPos;
//	//			prevMouseY = yPos;
//	//			return 0;
//	//		}
//	//		
//	//		int deltaX = xPos - prevMouseX;
//	//		int deltaY = yPos - prevMouseY;
//
//	//		prevMouseX = xPos;
//	//		prevMouseY = yPos;
//
//	//		return 0;
//	//	}
//	//}
//
//
//
//	// pass all unhandled messages to DefWindowProc
//	return DefWindowProc(hWnd,uMsg,wParam,lParam);
//}
class rbDX9_c : public rbAPI_i {
	IDirect3D9* pD3D;
	IDirect3DDevice9* pDev;
	HWND hWnd;
	int dxWidth, dxHeight;
	class mtrAPI_i *curMat;
public:
	rbDX9_c() {
		hWnd = 0;
		curMat = 0;
	}
	void initDX9State() {
		pDev->SetRenderState(D3DRS_ZENABLE, true);
		pDev->SetRenderState(D3DRS_AMBIENT,RGB(255,255,255));
		pDev->SetRenderState(D3DRS_LIGHTING,false);
		pDev->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
		pDev->SetRenderState(D3DRS_ZFUNC,D3DCMP_LESSEQUAL);

		pDev->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);                
		pDev->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC); 
		pDev->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC); 
	}
	bool createWindow(const char *title, int width, int height, int colorBits, bool useFullscreen) {
		/// CREATE WINDOW
		/*HINSTANCE hProg = GetModuleHandle(0);
		WNDCLASS wc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hProg;
		wc.lpfnWndProc = DX9WindowWNDProc;
		wc.lpszClassName = "WndClass";
		wc.lpszMenuName = 0;
		wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.hIcon = LoadIcon(hProg, IDI_WINLOGO);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.hCursor = LoadCursor(hProg, IDC_ARROW);

 		RegisterClass(&wc);
		hWnd = CreateWindowEx(0, "WndClass", title, WS_OVERLAPPEDWINDOW,
  			0, 0, width, height, 0, 0, hProg, 0);*/
		if( SDL_Init( SDL_INIT_VIDEO ) < 0 || !SDL_GetVideoInfo() )
			return 0; 
		SDL_SetVideoMode( width, height, SDL_GetVideoInfo()->vfmt->BitsPerPixel, SDL_RESIZABLE );

		hWnd = GetActiveWindow();

		ShowWindow(hWnd, 5);

		// create Dx9 device
		pD3D = Direct3DCreate9(D3D_SDK_VERSION);

		D3DPRESENT_PARAMETERS d3dpp;
		ZeroMemory(&d3dpp, sizeof(d3dpp));
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.Windowed = true;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
		// this turns off V-sync (60 fps limit)
		d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		dxWidth = d3dpp.BackBufferWidth = width; // CreateDevice will crash if its set to 0 while Windowed is set to true
		dxHeight = d3dpp.BackBufferHeight = height;

		// add z buffer for depth tests
		d3dpp.EnableAutoDepthStencil = true;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

		pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, 
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDev);

		ShowWindow(hWnd,SW_SHOW);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);

		initDX9State();

		return false;
	}

	virtual void setMaterial(class mtrAPI_i *mat, class textureAPI_i *lightmap) {
		curMat = mat;
	}
	virtual void unbindMaterial() {
		curMat = 0;
	}
	virtual void setColor4(const float *rgba) {
	}
	virtual void setBindVertexColors(bool bBindVertexColors) {
	}
	virtual void draw2D(const struct r2dVert_s *verts, u32 numVerts, const u16 *indices, u32 numIndices) {
		pDev->SetFVF(R2DVERT_FVF);

		pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		pDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		if(curMat) {
			const mtrStageAPI_i *s = curMat->getStage(0);
			IDirect3DTexture9 *texDX9 = (IDirect3DTexture9 *)s->getTexture()->getInternalHandleV();
			pDev->SetTexture(0,texDX9);
			pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,numVerts,numIndices/3,indices,D3DFMT_INDEX16,
				verts,sizeof(r2dVert_s));
		} else {
			pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,numVerts,numIndices/3,indices,D3DFMT_INDEX16,
				verts,sizeof(r2dVert_s));
		}
		pDev->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	}
	virtual void drawElements(const class rVertexBuffer_c &verts, const class rIndexBuffer_c &indices) {
		if(curMat) {
			const mtrStageAPI_i *s = curMat->getStage(0);
			IDirect3DTexture9 *texDX9 = (IDirect3DTexture9 *)s->getTexture()->getInternalHandleV();
			pDev->SetTexture(0,texDX9);
			pDev->SetFVF(RVERT_FVF);

			pDev->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST,0,verts.size(),indices.getNumIndices()/3,
				indices.getArray(),
				indices.getDX9IndexType(),verts.getArray(),sizeof(rVert_c));
		}

		
	}
	virtual void beginFrame() {
		pDev->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xfff000ff, 1, 0);
		pDev->BeginScene();
	}
	virtual void endFrame() {
		pDev->EndScene();
		pDev->Present(0, 0, 0, 0);
	}
	virtual void setup2DView() {
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
	matrix_c viewMatrix;
	matrix_c worldMatrix;
	virtual void setup3DView(const class vec3_c &newCamPos, const class axis_c &camAxis) {
		// transform by the camera placement and view axis
		viewMatrix.invFromAxisAndVector(camAxis,newCamPos);
		// convert to gl coord system
		viewMatrix.toGL();

		pDev->SetTransform(D3DTS_VIEW, (const D3DMATRIX *)&viewMatrix);

		setupWorldSpace();
	}
	virtual void setupProjection3D(const struct projDef_s *pd) {
		matrix_c proj;
		proj.setupProjection(pd->fovX,pd->fovY,pd->zNear,pd->zFar);
		pDev->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX *)&proj);
	}
	virtual void drawCapsuleZ(const float *xyz, float h, float w) {
	}
	virtual void drawBoxHalfSizes(const float *halfSizes) {
	}
	virtual void drawLineFromTo(const float *from, const float *to, const float *colorRGB) {
	}
	virtual void drawBBLines(const class aabb &bb) {
	}
	// used while drawing world surfaces and particles
	virtual void setupWorldSpace() {
		worldMatrix.identity();

		pDev->SetTransform(D3DTS_WORLD, (const D3DMATRIX *)&worldMatrix);
	}
	// used while drawing entities
	virtual void setupEntitySpace(const class axis_c &axis, const class vec3_c &origin) {
		worldMatrix.fromAxisAndOrigin(axis,origin);

		pDev->SetTransform(D3DTS_WORLD, (const D3DMATRIX *)&worldMatrix);
	}
	// same as above but with angles instead of axis
	virtual void setupEntitySpace2(const class vec3_c &angles, const class vec3_c &origin) {
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
				outPixel[0] = inPixel[2];
				outPixel[1] = inPixel[1];
				outPixel[2] = inPixel[0];
				outPixel[3] = inPixel[3];
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
	virtual void uploadLightmapRGB(class textureAPI_i *out, const byte *data, u32 w, u32 h) {
	}
	virtual void freeTextureData(class textureAPI_i *tex) {
		IDirect3DTexture9 *texD9 = (IDirect3DTexture9 *)tex->getInternalHandleV();
		if(texD9 == 0)
			return;
		texD9->Release();
		tex->setInternalHandleV(0);
	}

	// vertex buffers (VBOs)
	virtual bool createVBO(class rVertexBuffer_c *vbo) {
		return false;
	}
	virtual bool destroyVBO(class rVertexBuffer_c *vbo) {
		return false;
	}

	// index buffers (IBOs)
	virtual bool createIBO(class rIndexBuffer_c *ibo) {
		return false;
	}
	virtual bool destroyIBO(class rIndexBuffer_c *ibo) {
		return false;
	}

	virtual void init() {
		if(hWnd == 0) {
			createWindow("Test",640,480,32,false);
		}
		// This depends on SDL_INIT_VIDEO, hence having it here
		g_inputSystem->IN_Init();
	}
	virtual void shutdown() {
		if(hWnd == 0) {
			g_core->RedWarning("rbDX9_c::shutdown: hWnd is already NULL\n");
			return;
		}
		pDev->Release();
		pD3D->Release();
		g_inputSystem->IN_Shutdown();

		SDL_QuitSubSystem( SDL_INIT_VIDEO );
		hWnd = 0;
		pDev = 0;
		pD3D = 0;
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
	g_iFaceMan->registerIFaceUser(&g_sysEventCaster,SYSEVENTCASTER_API_IDENTSTR);
}

qioModule_e IFM_GetCurModule() {
	return QMD_REF_BACKEND_DX9;
}

