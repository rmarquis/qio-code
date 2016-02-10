/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// CamWnd.cpp : implementation file
//

#include <stdafx.h>
#include "Radiant.h"
#include "XYWnd.h"
#include "CamWnd.h"
#include "qe3.h"
#include <gl/glu.h>
#include <api/rAPI.h>
#include <api/rbAPI.h>
#include <api/coreAPI.h>
#include <api/entityDeclAPI.h>
#include <math/axis.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void DrawPathLines();

int g_nAngleSpeed = 300;
int g_nMoveSpeed = 400;


/////////////////////////////////////////////////////////////////////////////
// CCamWnd
IMPLEMENT_DYNCREATE(CCamWnd, CWnd);

CCamWnd::CCamWnd()
{
  m_pXYFriend = NULL;
  memset(&m_Camera, 0, sizeof(camera_t));
  m_pSide_select = NULL;
  m_bClipMode = false;
  Cam_Init();
}

CCamWnd::~CCamWnd()
{
}


BEGIN_MESSAGE_MAP(CCamWnd, CWnd)
	//{{AFX_MSG_MAP(CCamWnd)
	ON_WM_KEYDOWN()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_NCCALCSIZE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


LONG WINAPI CamWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    RECT	rect;

    GetClientRect(hWnd, &rect);

    switch (uMsg)
    {
	case WM_KILLFOCUS:
	case WM_SETFOCUS:
		SendMessage( hWnd, WM_NCACTIVATE, uMsg == WM_SETFOCUS, 0 );
		return 0;

	case WM_NCCALCSIZE:// don't let windows copy pixels
		DefWindowProc (hWnd, uMsg, wParam, lParam);
		return WVR_REDRAW;

    }

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}


/////////////////////////////////////////////////////////////////////////////
// CCamWnd message handlers

BOOL CCamWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
  WNDCLASS wc;
  HINSTANCE hInstance = AfxGetInstanceHandle();
  if (::GetClassInfo(hInstance, CAMERA_WINDOW_CLASS, &wc) == FALSE)
  {
    // Register a new class
  	memset (&wc, 0, sizeof(wc));
    wc.style         = CS_NOCLOSE | CS_OWNDC;
    wc.lpszClassName = CAMERA_WINDOW_CLASS;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
    wc.lpfnWndProc   = CamWndProc;
	wc.hInstance = hInstance; // V: added
    if (AfxRegisterClass(&wc) == FALSE)
      Error ("CCamWnd RegisterClass: failed");
  }

  cs.lpszClass = CAMERA_WINDOW_CLASS;
  cs.lpszName = "CAM";
  if (cs.style != QE3_CHILDSTYLE)
    cs.style = QE3_SPLITTER_STYLE;

	BOOL bResult = CWnd::PreCreateWindow(cs);

  // See if the class already exists and if not then we need
  // to register our new window class.
  return bResult;
	
}


void CCamWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags);
}


edBrush_c* g_pSplitList = NULL;

void CCamWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
  bool bPaint = true;
  if (!wglMakeCurrent( dc.m_hDC, g_qeglobals.d_hglrcBase ))
  {
    Sys_Printf("ERROR: wglMakeCurrent failed..\n ");
    Sys_Printf("Please restart Q3Radiant if the camera view is not working\n");
  }
  else
  {
    QE_CheckOpenGLForErrors();
    g_pSplitList = NULL;
    if (g_bClipMode)
    {
      if (g_Clip1.Set() && g_Clip2.Set())
      {
        g_pSplitList = ( (g_pParentWnd->ActiveXY()->GetViewType() == XZ) ? !g_bSwitch : g_bSwitch) ? &g_brBackSplits : &g_brFrontSplits;
      }
    }
		Cam_Draw ();
		QE_CheckOpenGLForErrors();

	glFinish();

		//qwgl
		SwapBuffers(dc.m_hDC);
  }
}


void CCamWnd::SetXYFriend(CXYWnd * pWnd)
{
  m_pXYFriend = pWnd;
}

void CCamWnd::OnDestroy() 
{
	QEW_StopGL(GetSafeHwnd(), g_qeglobals.d_hglrcBase, g_qeglobals.d_hdcBase );
	CWnd::OnDestroy();
}

void CCamWnd::OnClose() 
{
	CWnd::OnClose();
}

extern void Select_ShiftTexture(int x, int y);
extern void Select_RotateTexture(int amt);
extern void Select_ScaleTexture(int x, int y);
void CCamWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
  CRect r;
  GetClientRect(r);
  if (GetCapture() == this && (GetKeyState(VK_MENU) & 0x8000) && !((GetKeyState(VK_SHIFT) & 0x8000) || (GetKeyState(VK_CONTROL) & 0x8000)))
  {
    if (GetKeyState(VK_CONTROL) & 0x8000)
      Select_RotateTexture(point.y - m_ptLastCursor.y);
    else
    if (GetKeyState(VK_SHIFT) & 0x8000)
      Select_ScaleTexture(point.x - m_ptLastCursor.x, m_ptLastCursor.y - point.y);
    else
      Select_ShiftTexture(point.x - m_ptLastCursor.x, m_ptLastCursor.y - point.y);
  }
  else
  {
    Cam_MouseMoved(point.x, r.bottom - 1 - point.y, nFlags);
  }
  m_ptLastCursor = point;
}

void CCamWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
  m_ptLastCursor = point;
  OriginalMouseDown(nFlags, point);
}

void CCamWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
  OriginalMouseUp(nFlags, point);
}

void CCamWnd::OnMButtonDown(UINT nFlags, CPoint point) 
{
  OriginalMouseDown(nFlags, point);
}

void CCamWnd::OnMButtonUp(UINT nFlags, CPoint point) 
{
  OriginalMouseUp(nFlags, point);
}

void CCamWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
  OriginalMouseDown(nFlags, point);
}

void CCamWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
  OriginalMouseUp(nFlags, point);
}

int CCamWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	g_qeglobals.d_hdcBase = GetDC()->m_hDC;
	QEW_SetupPixelFormat(g_qeglobals.d_hdcBase, true);

  if ((g_qeglobals.d_hglrcBase = wglCreateContext(g_qeglobals.d_hdcBase)) == 0)
	  Error("wglCreateContext failed");
 // 
 // HGLRC qioRC;
	//rf->getHGLRC(&qioRC);
	//if (!wglShareLists(qioRC, g_qeglobals.d_hglrcBase))
	//  Error( "wglShareLists in CZWnd::OnCreate failed");


  if (!wglMakeCurrent(g_qeglobals.d_hdcBase, g_qeglobals.d_hglrcBase))
	  Error ("wglMakeCurrent failed");


	//
	// create GL font
	//
  HFONT hfont = ::CreateFont(
	  12,	// logical height of font 
		 6,	// logical average character width 
		 0,	// angle of escapement 
		 0,	// base-line orientation angle 
		 0,	// font weight 
		 0,	// italic attribute flag 
		 0,	// underline attribute flag 
		 0,	// strikeout attribute flag 
		 0,	// character set identifier 
		 0,	// output precision 
		 0,	// clipping precision 
		 0,	// output quality 
		 0,	// pitch and family 
		 "system font" // pointer to typeface name string 
		 	);

	if (!hfont)
	  Error( "couldn't create font" );

  ::SelectObject(g_qeglobals.d_hdcBase, hfont);

	if ((g_qeglobals.d_font_list = glGenLists (256)) == 0)
	  Error( "couldn't create font dlists" );
			
	// create the bitmap display lists
	// we're making images of glyphs 0 thru 255
  
  if (g_PrefsDlg.m_bBuggyICD)
  {
	  if ( !wglUseFontBitmaps (g_qeglobals.d_hdcBase, 1, 255, g_qeglobals.d_font_list-1) )
	    Error( "wglUseFontBitmaps faileD" );
  }
  else
  {
	  if ( !wglUseFontBitmaps (g_qeglobals.d_hdcBase, 1, 255, g_qeglobals.d_font_list) )
	    Error( "wglUseFontBitmaps faileD" );
  }
	
	// indicate start of glyph display lists
	glListBase (g_qeglobals.d_font_list);

	// report OpenGL information
	Sys_Printf ("GL_VENDOR: %s\n", glGetString (GL_VENDOR));
	Sys_Printf ("GL_RENDERER: %s\n", glGetString (GL_RENDERER));
	Sys_Printf ("GL_VERSION: %s\n", glGetString (GL_VERSION));
	Sys_Printf ("GL_EXTENSIONS: %s\n", glGetString (GL_EXTENSIONS));

  g_qeglobals.d_hwndCamera = GetSafeHwnd();

  g_core->EditorInitRenderer();

	return 0;
}

void CCamWnd::OriginalMouseUp(UINT nFlags, CPoint point)
{
  CRect r;
  GetClientRect(r);
  Cam_MouseUp(point.x, r.bottom - 1 - point.y, nFlags);
	if (!(nFlags & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)))
  	ReleaseCapture ();
}

void CCamWnd::OriginalMouseDown(UINT nFlags, CPoint point)
{
  //if (GetTopWindow()->GetSafeHwnd() != GetSafeHwnd())
  //  BringWindowToTop();
  CRect r;
  GetClientRect(r);
  SetFocus();
	SetCapture();
  //if (!(GetKeyState(VK_MENU) & 0x8000))
	  Cam_MouseDown (point.x, r.bottom - 1 - point.y, nFlags);
}

void CCamWnd::Cam_Init()
{
	//m_Camera.draw_mode = cd_texture;
	m_Camera.timing = false;
	m_Camera.origin[0] = 0;
	m_Camera.origin[1] = 20;
	m_Camera.origin[2] = 46;
	m_Camera.color[0] = 0.3;
	m_Camera.color[1] = 0.3;
	m_Camera.color[2] = 0.3;
}



void CCamWnd::Cam_ChangeFloor (bool up)
{
	edBrush_c	*b;
	float	d, bestd, current;
	vec3_t	start, dir;

	start[0] = m_Camera.origin[0];
	start[1] = m_Camera.origin[1];
	start[2] = 8192;
	dir[0] = dir[1] = 0;
	dir[2] = -1;

	current = 8192 - (m_Camera.origin[2] - 48);
	if (up)
		bestd = 0;
	else
		bestd = 16384;

	for (b=active_brushes.next ; b != &active_brushes ; b=b->next)
	{
		if ( !Brush_Ray (start, dir, b, &d) )
			continue;
		if (up && d < current && d > bestd)
			bestd = d;
		if (!up && d > current && d < bestd)
			bestd = d;
	}

	if (bestd == 0 || bestd == 16384)
		return;

	m_Camera.origin[2] += current - bestd;
	Sys_UpdateWindows (W_CAMERA|W_Z_OVERLAY);
}


void CCamWnd::Cam_PositionDrag()
{
	int	x, y;
	Sys_GetCursorPos (&x, &y);
	if (x != m_ptCursor.x || y != m_ptCursor.y)
	{
		x -= m_ptCursor.x;
		m_Camera.origin.vectorMA(m_Camera.origin, x, m_Camera.right);
		y -= m_ptCursor.y;
		m_Camera.origin[2] -= y;
    SetCursorPos(m_ptCursor.x, m_ptCursor.y);
		Sys_UpdateWindows (W_CAMERA | W_XY_OVERLAY);
	}
}


void CCamWnd::Cam_MouseControl (float dtime)
{
	int		xl, xh;
	int		yl, yh;
	float	xf, yf;
	if (g_PrefsDlg.m_nMouseButtons == 2)
	{
		if (m_nCambuttonstate != (MK_RBUTTON | MK_SHIFT))
			return;
	}
	else
	{
		if (m_nCambuttonstate != MK_RBUTTON)
			return;
	}

	//xf = (float)(m_ptButton.x - m_Camera.width/2) / (m_Camera.width/2);
	//yf = (float)(m_ptButton.y - m_Camera.height/2) / (m_Camera.height/2);

	RECT r;
	GetWindowRect(&r);

	int	x, y;
	Sys_GetCursorPos (&x, &y);
	x -= r.left;
	y -= r.top;
	xf = x - m_Camera.width/2;
	yf = m_Camera.height/2 - y;

  //Sys_Printf("xf-%f  yf-%f  xl-%i  xh-i%  yl-i%  yh-i%\n",xf,yf,xl,xh,yl,yh);
#if 0
	// strafe
	if (buttony < yl && (buttonx < xl || buttonx > xh))
		VectorMA (camera.origin, xf*dtime*g_nMoveSpeed, camera.right, camera.origin);
	else
#endif
	{
#if 0
		xf *= 1.0 - fabs(yf);
		if (xf < 0)
		{
			xf += 0.1;
			if (xf > 0)
				xf = 0;
		}
		else
		{
			xf -= 0.1;
			if (xf < 0)
				xf = 0;
		}

		VectorMA (m_Camera.origin, yf*dtime*g_nMoveSpeed, m_Camera.forward, m_Camera.origin);
		m_Camera.angles[YAW] += xf*-dtime*g_nAngleSpeed;
#else
		m_Camera.angles[YAW] -= xf;
		// V: in Q3Rad there was +yf, but for Qio renderer is -yf
		m_Camera.angles[PITCH] -= yf;
		if(m_Camera.angles[PITCH] > 88)
			m_Camera.angles[PITCH] = 88;
		else if(m_Camera.angles[PITCH] < -88)
			m_Camera.angles[PITCH] = -88;
		SetCursorPos(r.left+m_Camera.width/2,  r.top+m_Camera.height/2);
#endif
	}
	int nUpdate = (g_PrefsDlg.m_bCamXYUpdate) ? (W_CAMERA | W_XY) : (W_CAMERA);
	Sys_UpdateWindows (nUpdate);
	g_pParentWnd->PostMessage(WM_TIMER, 0, 0);
}



void CCamWnd::Cam_MouseDown(int x, int y, int buttons)
{
	vec3_c		dir;
	float		f, r, u;
	int			i;

	//
	// calc ray direction
	//
	u = (float)(y - m_Camera.height/2) / (m_Camera.width/2);
	r = (float)(x - m_Camera.width/2) / (m_Camera.width/2);
	f = 1;

	for (i=0 ; i<3 ; i++)
		dir[i] = m_Camera.forward[i] * f + m_Camera.right[i] * r + m_Camera.up[i] * u;
	dir.normalize();

	GetCursorPos(&m_ptCursor);

	m_nCambuttonstate = buttons;
	m_ptButton.x = x;
	m_ptButton.y = y;

	// LBUTTON = manipulate selection
	// shift-LBUTTON = select
	// middle button = grab texture
	// ctrl-middle button = set entire brush to texture
	// ctrl-shift-middle button = set single face to texture
	int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;
	if ((buttons == MK_LBUTTON)
		        || (buttons == (MK_LBUTTON | MK_SHIFT))
		        || (buttons == (MK_LBUTTON | MK_CONTROL))
		        || (buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT))
		        || (buttons == nMouseButton)
		        || (buttons == (nMouseButton|MK_SHIFT))
		        || (buttons == (nMouseButton|MK_CONTROL))
		        || (buttons == (nMouseButton|MK_SHIFT|MK_CONTROL)))
	{

    if (g_PrefsDlg.m_nMouseButtons == 2 && (buttons == (MK_RBUTTON | MK_SHIFT)))
		  Cam_MouseControl (0.1);
    else
    {
      // something global needs to track which window is responsible for stuff
      Patch_SetView(W_CAMERA);
		  Drag_Begin (x, y, buttons, m_Camera.right, m_Camera.up,	m_Camera.origin, dir);
    }
    return;
	}

	if (buttons == MK_RBUTTON)
	{
		Cam_MouseControl (0.1);
		return;
	}
}


void CCamWnd::Cam_MouseUp (int x, int y, int buttons)
{
	m_nCambuttonstate = 0;
	Drag_MouseUp (buttons);
}


void CCamWnd::Cam_MouseMoved (int x, int y, int buttons)
{
	m_nCambuttonstate = buttons;
	if (!buttons) {
		return;
	}
	m_ptButton.x = x;
	m_ptButton.y = y;

	if (buttons == (MK_RBUTTON|MK_CONTROL) )
	{
		Cam_PositionDrag ();
		Sys_UpdateWindows (W_XY|W_CAMERA|W_Z);
		return;
	}

	GetCursorPos(&m_ptCursor);

	if (buttons & (MK_LBUTTON | MK_MBUTTON) )
	{
		Drag_MouseMoved (x, y, buttons);
		Sys_UpdateWindows (W_XY|W_CAMERA|W_Z);
	}
}




#include <gl/glut.h>
void DrawLightRadius(entity_s *e)
{	
	str s = e->getKeyValue("light");
	float radius;
	if(s.size()) 
		radius = atof(s);
	else
		radius = 512.f;

	const vec3_c &at = e->getOrigin();	

	
///	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(at.getX(),at.getY(),at.getZ());
	glutSolidSphere(atof(s),16,16);
	//glTranslatef(-at.getX(),-at.getY(),-at.getZ());
	glPopMatrix();
}
BOOL FilterBrush(edBrush_c *pb);

void CCamWnd::Cam_Draw()
{
	double	start, end;

	if (!active_brushes.next)
		return;	// not valid yet
	
	if (m_Camera.timing)
		start = Sys_DoubleTime ();


	//
	// clear
	//
	QE_CheckOpenGLForErrors();
	
	//rf->getBackend()->set
	//glClearColor (g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][0],
	//	g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][1],
	//	g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][2], 0);
	rf->getBackend()->setBDrawEditorImageStages(true);
	rf->getBackend()->setViewPort(m_Camera.width, m_Camera.height);
	rf->beginFrame();
	rf->setRenderTimeMsec(clock());

	// V: setup projection
	projDef_s pd;
	pd.zFar = 8000.f;
	pd.zNear = 1.f;
	pd.fovX = 90.f;
	pd.calcFovYForViewPort(m_Camera.width,m_Camera.height);
	rf->setupProjection3D(&pd);
	// V: setup camera orientation
	rf->setup3DView(m_Camera.origin,m_Camera.angles,false);
	const axis_c &cax = rf->getCameraAxis();
    m_Camera.forward = cax.getForward();
	m_Camera.right = -cax.getLeft();
	m_Camera.up = cax.getUp();

	// V: this will draw all Qio rStaticModelAPI_i instances,
	// which are used now for brushes, patches and light shapes.
	rf->draw3DView();

	rf->getBackend()->unbindMaterial();

	float redc[4] = { 1, 0, 0, 1 };
	rf->getBackend()->setColor4(redc);
	rf->getBackend()->setAdditiveBlending();
	int nCount = g_SelectedFaces.GetSize();
	for (int i = 0; i < nCount; i++)
	{
		face_s *face = reinterpret_cast<face_s*>(g_SelectedFaces.GetAt(i));
		rf->getBackend()->drawWinding(face->face_winding->getXYZs(),face->face_winding->size(),face->face_winding->getStride());
	}
	rf->getBackend()->unbindMaterial();

	if (g_qeglobals.d_select_mode == sel_vertex)
	{
		glPointSize (4);
		glColor3f (0,1,0);
		glBegin (GL_POINTS);
		for (u32 i=0 ; i<g_qeglobals.d_numpoints ; i++)
			glVertex3fv (g_qeglobals.d_points[i]);
		glEnd ();
		glPointSize (1);
	}
	else if (g_qeglobals.d_select_mode == sel_edge)
	{
		float	*v1, *v2;
		
		glPointSize (4);
		glColor3f (0,0,1);
		glBegin (GL_POINTS);
		for (u32 i=0 ; i<g_qeglobals.d_numedges ; i++)
		{
			v1 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p1];
			v2 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p2];
			glVertex3f ( (v1[0]+v2[0])*0.5,(v1[1]+v2[1])*0.5,(v1[2]+v2[2])*0.5);
		}
		glEnd ();
		glPointSize (1);
	}

	glFinish();
	QE_CheckOpenGLForErrors();
	//	Sys_EndWait();
	if (m_Camera.timing)
	{
		end = Sys_DoubleTime ();
		Sys_Printf ("Camera: %i ms\n", (int)(1000*(end-start)));
	}
}
void CCamWnd::Cam_Draw_Old()
{
	edBrush_c	*brush;
	face_s	*face;
	double	start, end;
	int		i;
	
	if (!active_brushes.next)
		return;	// not valid yet
	

	if (m_Camera.timing)
		start = Sys_DoubleTime ();
	
	//
	// clear
	//
	QE_CheckOpenGLForErrors();
	
	//rf->getBackend()->set
	//glClearColor (g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][0],
	//	g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][1],
	//	g_qeglobals.d_savedinfo.colors[COLOR_CAMERABACK][2], 0);
	rf->getBackend()->setViewPort(m_Camera.width, m_Camera.height);
	rf->beginFrame();
	rf->setRenderTimeMsec(clock());

	// V: setup projection
	projDef_s pd;
	pd.zFar = 8000.f;
	pd.zNear = 1.f;
	pd.fovX = 90.f;
	pd.calcFovYForViewPort(m_Camera.width,m_Camera.height);
	rf->setupProjection3D(&pd);
	// V: setup camera orientation
	rf->setup3DView(m_Camera.origin,m_Camera.angles,false);
	const axis_c &cax = rf->getCameraAxis();
    m_Camera.forward = cax.getForward();
	m_Camera.right = -cax.getLeft();
	m_Camera.up = cax.getUp();

	aabb clip;
	clip.fromPointAndRadius(m_Camera.origin,1024.f);
	rf->draw3DView();

	m_TransBrushes.clear();
	arraySTD_c<entity_s*> lights;
	for (brush = active_brushes.next ; brush != &active_brushes ; brush=brush->next)
	{
		if(brush->getBounds().intersect(clip)==false)
			continue;
		
		if (FilterBrush (brush))
			continue;
		
		if (brush->getFirstFace()->d_texture && (brush->getFirstFace()->d_texture->hasEditorTransparency() && brush->getFirstFace()->d_texture->getEditorTransparency() != 1.0))
		{
			m_TransBrushes.push_back(brush);
		} 
		else 
		{
			//--      if (brush->patchBrush)
			//--			  m_TransBrushes [ m_nNumTransBrushes++ ] = brush;
			//--      else
			Brush_Draw(brush,false);
		}
		if(brush->owner->getEntityClass()->hasEditorFlagLight())
		{
			lights.push_back(brush->owner);
		}
	}
#if 0
	glEnable ( GL_BLEND );
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for ( i = 0; i < m_TransBrushes.size(); i++ ) 
		Brush_Draw (m_TransBrushes[i]);
	
	glMatrixMode(GL_PROJECTION);
#endif
	//
	// now draw selected brushes
	//
	edBrush_c* pList = (g_bClipMode && g_pSplitList) ? g_pSplitList : &selected_brushes;
	float red[4] = { 1.f, 0.5, 0.5, 1.f };
	rf->getBackend()->setColor4(red);
	for (brush = pList->next ; brush != pList ; brush=brush->next)
	{
		//DrawLightRadius(brush);
		//if (brush->patchBrush && g_qeglobals.d_select_mode == sel_curvepoint)
		//  continue;
		
		Brush_Draw(brush,true);
	}
	rf->getBackend()->setColor4(0);
#if 0		
	// edge / vertex flags
	
	if (g_qeglobals.d_select_mode == sel_vertex)
	{
		glPointSize (4);
		glColor3f (0,1,0);
		glBegin (GL_POINTS);
		for (i=0 ; i<g_qeglobals.d_numpoints ; i++)
			glVertex3fv (g_qeglobals.d_points[i]);
		glEnd ();
		glPointSize (1);
	}
	else if (g_qeglobals.d_select_mode == sel_edge)
	{
		float	*v1, *v2;
		
		glPointSize (4);
		glColor3f (0,0,1);
		glBegin (GL_POINTS);
		for (i=0 ; i<g_qeglobals.d_numedges ; i++)
		{
			v1 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p1];
			v2 = g_qeglobals.d_points[g_qeglobals.d_edges[i].p2];
			glVertex3f ( (v1[0]+v2[0])*0.5,(v1[1]+v2[1])*0.5,(v1[2]+v2[2])*0.5);
		}
		glEnd ();
		glPointSize (1);
	}
	
	//
	// draw pointfile
	//

	glEnable(GL_DEPTH_TEST);
	

	DrawPathLines ();
	
	
	
//	if (g_qeglobals.d_pointfile_display_list)
//	{
////		Pointfile_Draw();
//		//		glCallList (g_qeglobals.d_pointfile_display_list);
//	}
	
	// bind back to the default texture so that we don't have problems
	// elsewhere using/modifying texture maps between contexts
	glBindTexture( GL_TEXTURE_2D, 0 );
	
#if 0
	// area selection hack
	if (g_qeglobals.d_select_mode == sel_area)
	{
		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(0.0, 0.0, 1.0, 0.25);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glRectfv(g_qeglobals.d_vAreaTL, g_qeglobals.d_vAreaBR);
		glDisable (GL_BLEND);
	}
#endif
	
#endif
	rf->getBackend()->unbindMaterial();
	glFinish();
	QE_CheckOpenGLForErrors();
	//	Sys_EndWait();
	if (m_Camera.timing)
	{
		end = Sys_DoubleTime ();
		Sys_Printf ("Camera: %i ms\n", (int)(1000*(end-start)));
	}
}


void CCamWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
  CRect rect;
  GetClientRect(rect);
	m_Camera.width = rect.right;
	m_Camera.height = rect.bottom;
	InvalidateRect(NULL, false);
}

void CCamWnd::BenchMark()
{
	PAINTSTRUCT	ps;
  CRect rct;
  GetWindowRect(rct);
  long lStyle = ::GetWindowLong(GetSafeHwnd(), GWL_STYLE);
  ::SetWindowLong(GetSafeHwnd(), GWL_STYLE, QE3_CHILDSTYLE);
  CWnd* pParent = GetParent();
  SetParent(g_pParentWnd);
  MoveWindow(CRect(30, 30, 400, 400), TRUE);

  BeginPaint(&ps);
  if (!wglMakeCurrent(ps.hdc, g_qeglobals.d_hglrcBase))
		Error ("wglMakeCurrent failed in Benchmark");
  
	glDrawBuffer (GL_FRONT);
	double dStart = Sys_DoubleTime ();
	for (int i=0 ; i < 100 ; i++)
	{
		m_Camera.angles[YAW] = i*4;
		Cam_Draw();
	}
//	wgl
	SwapBuffers(ps.hdc);
	glDrawBuffer (GL_BACK);
	double dEnd = Sys_DoubleTime ();
	EndPaint(&ps);
	Sys_Printf ("%5.2f seconds\n", dEnd - dStart);
  ::SetWindowLong(GetSafeHwnd(), GWL_STYLE, lStyle);
  SetParent(pParent);
  MoveWindow(rct, TRUE);
}

void CCamWnd::ReInitGL()
{

  wglMakeCurrent(0,0);
	QEW_SetupPixelFormat(GetDC()->m_hDC, true);
  if (!wglMakeCurrent(g_qeglobals.d_hdcBase, g_qeglobals.d_hglrcBase))
	  Error ("wglMakeCurrent failed");

  return;

  long lStyle = ::GetWindowLong(GetSafeHwnd(), GWL_STYLE);
  int nID = ::GetWindowLong(GetSafeHwnd(), GWL_ID);
  CWnd* pParent = GetParent();
  CRect rctClient;
  GetClientRect(rctClient);
  DestroyWindow();
  Create(CAMERA_WINDOW_CLASS, "", lStyle, rctClient, pParent, nID);
}

void CCamWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags, false);
}

void ComputeAxisBase(const vec3_c &inNormal, vec3_c &texS, vec3_c &texT);

// Timo
// brush primitive texture shifting, using camera view to select translations :
void CCamWnd::ShiftTexture_BrushPrimit(face_s *f, int x, int y)
{
	vec3_c texS,texT;
	vec3_c viewX,viewY;
	int XS,XT,YS,YT;
	int outS,outT;
#ifdef _DEBUG
	if (!g_qeglobals.m_bBrushPrimitMode)
	{
		Sys_Printf("Warning : unexpected call to CCamWnd::ShiftTexture_BrushPrimit with brush primitive mode disbaled\n");
		return;
	}
#endif
	// compute face axis base
	ComputeAxisBase( f->plane.norm, texS, texT );
	// compute camera view vectors
	viewY = m_Camera.up;
	viewX = m_Camera.right;
	// compute best vectors
	ComputeBest2DVector( viewX, texS, texT, XS, XT );
	ComputeBest2DVector( viewY, texS, texT, YS, YT );
	// check this is not a degenerate case
	if ( ( XS == YS ) && ( XT == YT ) )
	{
#ifdef _DEBUG
		Sys_Printf("Warning : degenerate best vectors axis base in CCamWnd::ShiftTexture_BrushPrimit\n");
#endif
		// forget it
		f->shiftTexture_BrushPrimit(x, y );
		return;
	}
	// compute best fitted translation in face axis base
	outS = XS*x + YS*y;
	outT = XT*x + YT*y;
	// call actual texture shifting code
	f->shiftTexture_BrushPrimit(outS, outT );
}
