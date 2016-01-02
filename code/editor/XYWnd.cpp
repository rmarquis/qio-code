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
// XYWnd.cpp : implementation file
//
// QERadiant
//
// 

#include "stdafx.h"
#include "Radiant.h"
#include "XYWnd.h"
#include "qe3.h"
#include "PrefsDlg.h"
#include "DialogInfo.h"

#include <math/math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char* g_pDimStrings[] = {"x:%.f", "y:%.f", "z:%.f"};
const char* g_pOrgStrings[] = {"(x:%.f  y:%.f)", "(x:%.f  z:%.f)", "(y:%.f  z:%.f)"};
CString g_strDim;
CString g_strStatus;

bool g_bCrossHairs = false;
bool g_bScaleMode;
int g_nScaleHow;
bool g_bRotateMode;
bool g_bClipMode;
bool g_bRogueClipMode;
bool g_bSwitch;
CClipPoint g_Clip1;
CClipPoint g_Clip2;
CClipPoint g_Clip3;
CClipPoint* g_pMovingClip;
brush_s g_brFrontSplits(true);
brush_s g_brBackSplits(true);

brush_s g_brClipboard(true);
brush_s g_brUndo(true);
entity_s	g_enClipboard(true);

vec3_c g_vRotateOrigin;
vec3_c g_vRotation;

bool g_bPathMode;
CClipPoint g_PathPoints[256];
CClipPoint* g_pMovingPath;
int g_nPathCount;
int g_nPathLimit;

bool g_bSmartGo;

bool g_bPointMode;
CClipPoint g_PointPoints[512];
CClipPoint* g_pMovingPoint;
int g_nPointCount;
int g_nPointLimit;


const int XY_LEFT = 0x01;
const int XY_RIGHT = 0x02;
const int XY_UP = 0x04;
const int XY_DOWN = 0x08;

PFNPathCallback* g_pPathFunc = NULL;

void AcquirePath(int nCount, PFNPathCallback* pFunc)
{
  g_nPathCount = 0;
  g_nPathLimit = nCount;
  g_pPathFunc = pFunc;
  g_bPathMode = true;
}


CPtrArray g_ptrMenus;

CMemFile g_Clipboard(4096);
CMemFile g_PatchClipboard(4096);

extern int pressx;
extern int pressy;


/////////////////////////////////////////////////////////////////////////////
// CXYWnd

IMPLEMENT_DYNCREATE(CXYWnd, CWnd);

CXYWnd::CXYWnd()
{
  g_brClipboard.next = &g_brClipboard;
  g_brUndo.next = &g_brUndo;
  g_nScaleHow = 0;
  g_bRotateMode = false;
  g_bClipMode = false;
  g_bRogueClipMode = false;
  g_bSwitch = true;
  g_pMovingClip = NULL;
  g_pMovingPath = NULL;
  g_brFrontSplits.next = &g_brFrontSplits;
  g_brBackSplits.next = &g_brBackSplits;
  m_bActive = false;
  //m_bTiming = true;
  m_bTiming = false;
  m_bRButtonDown = false;
  m_nUpdateBits = W_XY;
  g_bPathMode = false;
  g_nPathCount = 0;
  g_nPathLimit = 0;
  m_nTimerID = -1;
  XY_Init();
}

CXYWnd::~CXYWnd()
{
  int nSize = g_ptrMenus.GetSize();
  while (nSize > 0)
  {
    CMenu* pMenu = reinterpret_cast<CMenu*>(g_ptrMenus.GetAt(nSize-1));
    ASSERT(pMenu);
    pMenu->DestroyMenu();
    delete pMenu;
    nSize--;
  }
  g_ptrMenus.RemoveAll();
  m_mnuDrop.DestroyMenu();
}


BEGIN_MESSAGE_MAP(CXYWnd, CWnd)
	//{{AFX_MSG_MAP(CXYWnd)
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL() // V: added for Qio
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_SELECT_MOUSEROTATE, OnSelectMouserotate)
	ON_WM_TIMER()
	ON_WM_KEYUP()
	ON_WM_NCCALCSIZE()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_COMMAND(ID_SELECTION_MAKE_DETAIL, CMainFrame::OnSelectionMakeDetail)
	ON_COMMAND(ID_SELECTION_MAKE_STRUCTURAL, CMainFrame::OnSelectionMakeStructural)
	ON_COMMAND(ID_SELECTION_SELECTCOMPLETETALL, CMainFrame::OnSelectionSelectcompletetall)
	ON_COMMAND(ID_SELECTION_SELECTINSIDE, CMainFrame::OnSelectionSelectinside)
	ON_COMMAND(ID_SELECTION_SELECTPARTIALTALL, CMainFrame::OnSelectionSelectpartialtall)
	ON_COMMAND(ID_SELECTION_SELECTTOUCHING, CMainFrame::OnSelectionSelecttouching)
	ON_COMMAND(ID_SELECTION_UNGROUPENTITY, CMainFrame::OnSelectionUngroupentity)
	//}}AFX_MSG_MAP
  ON_COMMAND_RANGE(ID_ENTITY_START, ID_ENTITY_END, OnEntityCreate)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CXYWnd message handlers
LONG WINAPI XYWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CXYWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
  WNDCLASS wc;
  HINSTANCE hInstance = AfxGetInstanceHandle();
  if (::GetClassInfo(hInstance, XY_WINDOW_CLASS, &wc) == FALSE)
  {
    // Register a new class
  	memset (&wc, 0, sizeof(wc));
    wc.style         = CS_NOCLOSE | CS_OWNDC;
    wc.lpszClassName = XY_WINDOW_CLASS;
    wc.hCursor       = NULL; //LoadCursor (NULL,IDC_ARROW);
    wc.lpfnWndProc   = ::DefWindowProc;
	wc.hInstance = hInstance; // V: added
    if (AfxRegisterClass(&wc) == FALSE)
      Error ("CCamWnd RegisterClass: failed");
  }

  cs.lpszClass = XY_WINDOW_CLASS;
  cs.lpszName = "VIEW";
  if (cs.style != QE3_CHILDSTYLE)
    cs.style = QE3_SPLITTER_STYLE;

	return CWnd::PreCreateWindow(cs);
}

HDC   s_hdcXY;
HGLRC s_hglrcXY;

static unsigned s_stipple[32] =
{
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
	0xaaaaaaaa, 0x55555555,0xaaaaaaaa, 0x55555555,
};

/*
============
WXY_WndProc
============
*/
LONG WINAPI XYWndProc (
    HWND    hWnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    switch (uMsg)
    {
	case WM_DESTROY:
		QEW_StopGL( hWnd, s_hglrcXY, s_hdcXY );
		return 0;

	case WM_NCCALCSIZE:// don't let windows copy pixels
		DefWindowProc (hWnd, uMsg, wParam, lParam);
		return WVR_REDRAW;

	case WM_KILLFOCUS:
	case WM_SETFOCUS:
		SendMessage( hWnd, WM_NCACTIVATE, uMsg == WM_SETFOCUS, 0 );
		return 0;

   	case WM_CLOSE:
        DestroyWindow (hWnd);
		return 0;
    }

	return DefWindowProc (hWnd, uMsg, wParam, lParam);
}


static void WXY_InitPixelFormat( PIXELFORMATDESCRIPTOR *pPFD )
{
	memset( pPFD, 0, sizeof( *pPFD ) );

	pPFD->nSize    = sizeof( PIXELFORMATDESCRIPTOR );
	pPFD->nVersion = 1;
	pPFD->dwFlags  = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pPFD->iPixelType = PFD_TYPE_RGBA;
	pPFD->cColorBits = 24;
	pPFD->cDepthBits = 32;
	pPFD->iLayerType = PFD_MAIN_PLANE;

}

void WXY_Print( void )
{
	DOCINFO di;

	PRINTDLG pd;

	/*
	** initialize the PRINTDLG struct and execute it
	*/
	memset( &pd, 0, sizeof( pd ) );
	pd.lStructSize = sizeof( pd );
	pd.hwndOwner = g_qeglobals.d_hwndXY;
	pd.Flags = PD_RETURNDC;
	pd.hInstance = 0;
	if ( !PrintDlg( &pd ) || !pd.hDC )
	{
		MessageBox( g_qeglobals.d_hwndMain, "Could not PrintDlg()", "QE4 Print Error", MB_OK | MB_ICONERROR );
		return;
	}

	/*
	** StartDoc
	*/
	memset( &di, 0, sizeof( di ) );
	di.cbSize = sizeof( di );
	di.lpszDocName = "QE4";
	if ( StartDoc( pd.hDC, &di ) <= 0 )
	{
		MessageBox( g_qeglobals.d_hwndMain, "Could not StartDoc()", "QE4 Print Error", MB_OK | MB_ICONERROR );
		return;
	}

	/*
	** StartPage
	*/
	if ( StartPage( pd.hDC ) <= 0 )
	{
		MessageBox( g_qeglobals.d_hwndMain, "Could not StartPage()", "QE4 Print Error", MB_OK | MB_ICONERROR );
		return;
	}

	/*
	** read pixels from the XY window
	*/
	{
		int bmwidth = 320, bmheight = 320;
		int pwidth, pheight;

		RECT r;

		GetWindowRect( g_qeglobals.d_hwndXY, &r );

		bmwidth  = r.right - r.left;
		bmheight = r.bottom - r.top;

		pwidth  = GetDeviceCaps( pd.hDC, PHYSICALWIDTH ) - GetDeviceCaps( pd.hDC, PHYSICALOFFSETX );
		pheight = GetDeviceCaps( pd.hDC, PHYSICALHEIGHT ) - GetDeviceCaps( pd.hDC, PHYSICALOFFSETY );

		StretchBlt( pd.hDC,
			0, 0,
			pwidth, pheight,
			s_hdcXY,
			0, 0,
			bmwidth, bmheight,
			SRCCOPY );
	}

	/*
	** EndPage and EndDoc
	*/
	if ( EndPage( pd.hDC ) <= 0 )
	{
		MessageBox( g_qeglobals.d_hwndMain, "QE4 Print Error", "Could not EndPage()", MB_OK | MB_ICONERROR );
		return;
	}

	if ( EndDoc( pd.hDC ) <= 0 )
	{
		MessageBox( g_qeglobals.d_hwndMain, "QE4 Print Error", "Could not EndDoc()", MB_OK | MB_ICONERROR );
		return;
	}
}

int CXYWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

  s_hdcXY = ::GetDC(GetSafeHwnd());
	QEW_SetupPixelFormat(s_hdcXY, false);
	if ( ( s_hglrcXY = wglCreateContext( s_hdcXY ) ) == 0 )
	  Error( "wglCreateContext in WXY_WndProc failed" );

	if (!wglShareLists( g_qeglobals.d_hglrcBase, s_hglrcXY ) )
	  Error( "wglShareLists in WXY_WndProc failed" );

  if (!wglMakeCurrent( s_hdcXY, s_hglrcXY ))
	  Error ("wglMakeCurrent failed");

	glPolygonStipple ((unsigned char *)s_stipple);
	glLineStipple (3, 0xaaaa);
  g_qeglobals.d_hwndXY = GetSafeHwnd();
	return 0;
}

float ptSum(vec3_t pt)
{
  return pt[0] + pt[1] + pt[2];
}

void CXYWnd::DropClipPoint(UINT nFlags, CPoint point)
{
  CRect rctZ;
  GetClientRect(rctZ);
  if (g_pMovingClip)
  {
    SetCapture();
    SnapToPoint (point.x, rctZ.Height() - 1 - point.y , *g_pMovingClip);
  }
  else
  {
    vec3_t* pPt = NULL;
    if (g_Clip1.Set() == false)
    {
      pPt = g_Clip1;
      g_Clip1.Set(true);
      g_Clip1.m_ptScreen = point;
    }
    else 
    if (g_Clip2.Set() == false)
    {
      pPt = g_Clip2;
      g_Clip2.Set(true);
      g_Clip2.m_ptScreen = point;
    }
    else 
    if (g_Clip3.Set() == false)
    {
      pPt = g_Clip3;
      g_Clip3.Set(true);
      g_Clip3.m_ptScreen = point;
    }
    else 
    {
      RetainClipMode(true);
      pPt = g_Clip1;
      g_Clip1.Set(true);
      g_Clip1.m_ptScreen = point;
    }
    SnapToPoint (point.x, rctZ.Height() - 1 - point.y , *pPt);
  }
  Sys_UpdateWindows(XY | W_CAMERA_IFON);
}


void CXYWnd::DropPathPoint(UINT nFlags, CPoint point)
{
  CRect rctZ;
  GetClientRect(rctZ);
  if (g_pMovingPath)
  {
    SetCapture();
    SnapToPoint (point.x, rctZ.Height() - 1 - point.y , *g_pMovingPath);
  }
  else
  {
    g_PathPoints[g_nPathCount].Set(true);
    g_PathPoints[g_nPathCount].m_ptScreen = point;
    SnapToPoint(point.x, rctZ.Height() - 1 - point.y, g_PathPoints[g_nPathCount]);
    g_nPathCount++;
    if (g_nPathCount == g_nPathLimit)
    {
      if (g_pPathFunc)
        g_pPathFunc(true, g_nPathCount);
      g_nPathCount = 0;
      g_bPathMode = false;
      g_pPathFunc = NULL;
    }
  }
  Sys_UpdateWindows(XY | W_CAMERA_IFON);
}

void CXYWnd::AddPointPoint(UINT nFlags, vec3_t* pVec)
{
  g_PointPoints[g_nPointCount].Set(true);
  //g_PointPoints[g_nPointCount].m_ptScreen = point;
  g_PointPoints[g_nPointCount] = *pVec;
  g_PointPoints[g_nPointCount].SetPointPtr(pVec);
  g_nPointCount++;
  Sys_UpdateWindows(XY | W_CAMERA_IFON);
}


void CXYWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
  g_pParentWnd->SetActiveXY(this);
  UndoCopy();

  if (ClipMode() && !RogueClipMode())
  {
    DropClipPoint(nFlags, point);
  }
  else if (PathMode())
  {
    DropPathPoint(nFlags, point);
  }
  else OriginalButtonDown(nFlags, point);
}

void CXYWnd::OnMButtonDown(UINT nFlags, CPoint point) 
{
  OriginalButtonDown(nFlags, point);
}


float Betwixt(float f1, float f2)
{
  if (f1 > f2)
    return f2 + ((f1 - f2) / 2);
  else
    return f1 + ((f2 - f1) / 2);
}

void CXYWnd::ProduceSplits(brush_s** pFront, brush_s** pBack)
{
  *pFront = NULL;
  *pBack = NULL;
  if (ClipMode())
  {
    if (g_Clip1.Set() && g_Clip2.Set())
    {
      face_s face;
      face.planepts[0] = g_Clip1.m_ptClip;
      face.planepts[1] = g_Clip2.m_ptClip;
      face.planepts[2] = g_Clip3.m_ptClip;
      if (selected_brushes.next && (selected_brushes.next->next == &selected_brushes))
      {
        if (g_Clip3.Set() == false)
        {
          if (m_nViewType == XY)
          {
            face.planepts[0][2] = selected_brushes.next->getMins()[2];
            face.planepts[1][2] = selected_brushes.next->getMins()[2];
            face.planepts[2][0] = Betwixt(g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0]);
            face.planepts[2][1] = Betwixt(g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1]);
            face.planepts[2][2] = selected_brushes.next->getMaxs()[2];
          }
          else if (m_nViewType == YZ)
          {
            face.planepts[0][0] = selected_brushes.next->getMins()[0];
            face.planepts[1][0] = selected_brushes.next->getMins()[0];
            face.planepts[2][1] = Betwixt(g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1]);
            face.planepts[2][2] = Betwixt(g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2]);
            face.planepts[2][0] = selected_brushes.next->getMaxs()[0];
          }
          else
          {
            face.planepts[0][1] = selected_brushes.next->getMins()[1];
            face.planepts[1][1] = selected_brushes.next->getMins()[1];
            face.planepts[2][0] = Betwixt(g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0]);
            face.planepts[2][2] = Betwixt(g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2]);
            face.planepts[2][1] = selected_brushes.next->getMaxs()[1];
          }
        }

		selected_brushes.next->splitBrushByFace(&face, pFront, pBack);
      }

    }
  }
}

void CleanList(brush_s* pList)
{
  brush_s* pBrush = pList->next; 
  while (pBrush != NULL && pBrush != pList)
  {
    brush_s* pNext = pBrush->next;
    Brush_Free(pBrush);
    pBrush = pNext;
  }
}

void CXYWnd::ProduceSplitLists()
{
	if (AnyPatchesSelected())
	{
		Sys_Printf("Deslecting patches for clip operation.\n");
		brush_s *next;
		for (brush_s *pb = selected_brushes.next ; pb != &selected_brushes ; pb = next)
		{
			next = pb->next;
			if (pb->patchBrush)
			{
				Brush_RemoveFromList (pb);
				Brush_AddToList (pb, &active_brushes);
				UpdatePatchInspector();
			}
		}
	}

	CleanList(&g_brFrontSplits);
	CleanList(&g_brBackSplits);
	g_brFrontSplits.next = &g_brFrontSplits;
	g_brBackSplits.next = &g_brBackSplits;
	brush_s* pBrush;
	for (pBrush = selected_brushes.next ; pBrush != NULL && pBrush != &selected_brushes ; pBrush=pBrush->next)
	{
		brush_s* pFront = NULL;
		brush_s* pBack = NULL;
		if (ClipMode())
		{
			if (g_Clip1.Set() && g_Clip2.Set())
			{
				face_s face;
				face.planepts[0] = g_Clip1.m_ptClip;
				face.planepts[1] = g_Clip2.m_ptClip;
				face.planepts[2] = g_Clip3.m_ptClip;
				if (g_Clip3.Set() == false)
				{
					if (g_pParentWnd->ActiveXY()->GetViewType() == XY)
					{
						face.planepts[0][2] = pBrush->getMins()[2];
						face.planepts[1][2] = pBrush->getMins()[2];
						face.planepts[2][0] = Betwixt(g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0]);
						face.planepts[2][1] = Betwixt(g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1]);
						face.planepts[2][2] = pBrush->getMaxs()[2];
					}
					else if (g_pParentWnd->ActiveXY()->GetViewType() == YZ)
					{
						face.planepts[0][0] = pBrush->getMins()[0];
						face.planepts[1][0] = pBrush->getMins()[0];
						face.planepts[2][1] = Betwixt(g_Clip1.m_ptClip[1], g_Clip2.m_ptClip[1]);
						face.planepts[2][2] = Betwixt(g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2]);
						face.planepts[2][0] = pBrush->getMaxs()[0];
					}
					else
					{
						face.planepts[0][1] = pBrush->getMins()[1];
						face.planepts[1][1] = pBrush->getMins()[1];
						face.planepts[2][0] = Betwixt(g_Clip1.m_ptClip[0], g_Clip2.m_ptClip[0]);
						face.planepts[2][2] = Betwixt(g_Clip1.m_ptClip[2], g_Clip2.m_ptClip[2]);
						face.planepts[2][1] = pBrush->getMaxs()[1];
					}
				}
				pBrush->splitBrushByFace(&face, &pFront, &pBack);
				if (pBack)
					Brush_AddToList(pBack, &g_brBackSplits);
				if (pFront)
					Brush_AddToList(pFront, &g_brFrontSplits);
			}
		}
	}
}

void Brush_CopyList (brush_s* pFrom, brush_s* pTo)
{
	brush_s* pBrush = pFrom->next; 
	while (pBrush != NULL && pBrush != pFrom)
	{
		brush_s* pNext = pBrush->next;
		Brush_RemoveFromList(pBrush);
		Brush_AddToList(pBrush, pTo);
		pBrush = pNext;
	}
}

void CXYWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
  g_pParentWnd->SetActiveXY(this);
  m_ptDown = point;
  m_bRButtonDown = true;

  if (g_PrefsDlg.m_nMouseButtons == 3) // 3 button mouse 
  {
    if ((GetKeyState(VK_CONTROL) & 0x8000))
    {
      if (ClipMode()) // already there?
        DropClipPoint(nFlags, point);
      else
      {
        SetClipMode(true);
        g_bRogueClipMode = true;
        DropClipPoint(nFlags, point);
      }
      return;
    }
  }
  OriginalButtonDown(nFlags, point);
}

void CXYWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{

	if (ClipMode())
  {
    if (g_pMovingClip)
    {
      ReleaseCapture();
      g_pMovingClip = NULL;
    }
  }
  OriginalButtonUp(nFlags, point);
}

void CXYWnd::OnMButtonUp(UINT nFlags, CPoint point) 
{
  OriginalButtonUp(nFlags, point);
}

void CXYWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
  m_bRButtonDown = false;
  if (point == m_ptDown)    // mouse didn't move
  {
    bool bGo = true;
    if ((GetKeyState(VK_MENU) & 0x8000))
      bGo = false;
    if ((GetKeyState(VK_CONTROL) & 0x8000))
      bGo = false;
    if ((GetKeyState(VK_SHIFT) & 0x8000))
      bGo = false;
    if (bGo)
      HandleDrop();
  }
  OriginalButtonUp(nFlags, point);
}

void CXYWnd::OriginalButtonDown(UINT nFlags, CPoint point)
{
  CRect rctZ;
  GetClientRect(rctZ);
  SetWindowPos(&wndTop, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
  if (::GetTopWindow( g_qeglobals.d_hwndMain ) != GetSafeHwnd())
    ::BringWindowToTop(GetSafeHwnd());
	SetFocus();
	SetCapture();
	XY_MouseDown (point.x, rctZ.Height() - 1 - point.y , nFlags);
  m_nScrollFlags = nFlags;
}          

void CXYWnd::OriginalButtonUp(UINT nFlags, CPoint point)
{
  CRect rctZ;
  GetClientRect(rctZ);
  XY_MouseUp (point.x, rctZ.Height() - 1 - point.y , nFlags);
	if (! (nFlags & (MK_LBUTTON|MK_RBUTTON|MK_MBUTTON)))
		ReleaseCapture ();
}


float fDiff(float f1, float f2)
{
  if (f1 > f2)
    return f1 - f2;
  else
    return f2 - f1;
}


vec3_t tdp;
void CXYWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
  m_ptDown.x = 0;
  m_ptDown.y = 0;

  if ( g_PrefsDlg.m_bChaseMouse == TRUE &&
       (point.x < 0 || point.y < 0 || 
       point.x > m_nWidth || point.y > m_nHeight) &&
       GetCapture() == this)
  {
    float fAdjustment = (g_qeglobals.d_gridsize / 8 * 64) / m_fScale;
    //m_ptDrag = point;
    m_ptDragAdj.x = 0;
    m_ptDragAdj.y = 0;
    if (point.x < 0)
    {
      m_ptDragAdj.x = -fAdjustment;
    }
    else 
    if (point.x > m_nWidth)
    {
      m_ptDragAdj.x = fAdjustment;
    }
    if (point.y < 0)
    {
      m_ptDragAdj.y = -fAdjustment;
    }
    else
    if (point.y > m_nHeight)
    {
      m_ptDragAdj.y = fAdjustment;
    }
    if (m_nTimerID == -1)
    {
      m_nTimerID = SetTimer(100, 50, NULL);
      m_ptDrag = point;
      m_ptDragTotal = 0;
    }
    return;
  }
  //else if (m_nTimerID != -1)
  if (m_nTimerID != -1)
  {
    KillTimer(m_nTimerID);
    pressx -= m_ptDragTotal.x;
    pressy += m_ptDragTotal.y;
    m_nTimerID = -1;
    //return;
  }

  bool bCrossHair = false;
  if (!m_bRButtonDown)
  {
    tdp[0] = tdp[1] = tdp[2] = 0.0;
    SnapToPoint (point.x, m_nHeight - 1 - point.y , tdp);

    g_strStatus.Format("x:: %.1f  y:: %.1f  z:: %.1f", tdp[0], tdp[1], tdp[2]);
    g_pParentWnd->SetStatusText(1, g_strStatus);

    // i need to generalize the point code.. having 3 flavors pretty much sucks.. 
    // once the new curve stuff looks like it is going to stick i will 
    // rationalize this down to a single interface.. 
    if (PointMode())
    {
      if (g_pMovingPoint && GetCapture() == this)
      {
        bCrossHair = true;
        SnapToPoint (point.x, m_nHeight - 1 - point.y , g_pMovingPoint->m_ptClip);
        g_pMovingPoint->UpdatePointPtr();
        Sys_UpdateWindows(XY | W_CAMERA_IFON);
      }
      else
      {
        g_pMovingPoint = NULL;
        int nDim1 = (m_nViewType == YZ) ? 1 : 0;
        int nDim2 = (m_nViewType == XY) ? 1 : 2;
        for (int n = 0; n < g_nPointCount; n++)
        {
          if ( fDiff(g_PointPoints[n].m_ptClip[nDim1], tdp[nDim1]) < 3 &&
               fDiff(g_PointPoints[n].m_ptClip[nDim2], tdp[nDim2]) < 3 )
          {
            bCrossHair = true;
            g_pMovingPoint = &g_PointPoints[n];
          }
        }
      }
    }
    else if (ClipMode())
    {
      if (g_pMovingClip && GetCapture() == this)
      {
        bCrossHair = true;
        SnapToPoint (point.x, m_nHeight - 1 - point.y , g_pMovingClip->m_ptClip);
        Sys_UpdateWindows(XY | W_CAMERA_IFON);
      }
      else
      {
        g_pMovingClip = NULL;
        int nDim1 = (m_nViewType == YZ) ? 1 : 0;
        int nDim2 = (m_nViewType == XY) ? 1 : 2;
        if (g_Clip1.Set())
        {
          if ( fDiff(g_Clip1.m_ptClip[nDim1], tdp[nDim1]) < 3 &&
               fDiff(g_Clip1.m_ptClip[nDim2], tdp[nDim2]) < 3 )
          {
            bCrossHair = true;
            g_pMovingClip = &g_Clip1;
          }
        }
        if (g_Clip2.Set())
        {
          if ( fDiff(g_Clip2.m_ptClip[nDim1], tdp[nDim1]) < 3 &&
               fDiff(g_Clip2.m_ptClip[nDim2], tdp[nDim2]) < 3 )
          {
            bCrossHair = true;
            g_pMovingClip = &g_Clip2;
          }
        }
        if (g_Clip3.Set())
        {
          if ( fDiff(g_Clip3.m_ptClip[nDim1], tdp[nDim1]) < 3 &&
               fDiff(g_Clip3.m_ptClip[nDim2], tdp[nDim2]) < 3 )
          {
            bCrossHair = true;
            g_pMovingClip = &g_Clip3;
          }
        }
      }
      if (bCrossHair == false)
        XY_MouseMoved (point.x, m_nHeight - 1 - point.y , nFlags);
    }
    else if (PathMode())
    {
      if (g_pMovingPath && GetCapture() == this)
      {
        bCrossHair = true;
        SnapToPoint (point.x, m_nHeight - 1 - point.y , g_pMovingPath->m_ptClip);
        Sys_UpdateWindows(XY | W_CAMERA_IFON);
      }
      else
      {
        g_pMovingPath = NULL;
        int nDim1 = (m_nViewType == YZ) ? 1 : 0;
        int nDim2 = (m_nViewType == XY) ? 1 : 2;
        for (int n = 0; n < g_nPathCount; n++)
        {
          if ( fDiff(g_PathPoints[n].m_ptClip[nDim1], tdp[nDim1]) < 3 &&
               fDiff(g_PathPoints[n].m_ptClip[nDim2], tdp[nDim2]) < 3 )
          {
            bCrossHair = true;
            g_pMovingPath = &g_PathPoints[n];
          }
        }
      }
    }
    else
    {
      XY_MouseMoved (point.x, m_nHeight - 1 - point.y , nFlags);
    }
  }
  else 
  {
    XY_MouseMoved (point.x, m_nHeight - 1 - point.y , nFlags);
  }
  if (bCrossHair)
    SetCursor(::LoadCursor(NULL, IDC_CROSS));
  else
    SetCursor(::LoadCursor(NULL, IDC_ARROW));
}
afx_msg BOOL CXYWnd::OnMouseWheel(UINT fFlags, short zDelta, CPoint point) {
	if(zDelta < 0) {
		doZoomOut();
	} else {
		doZoomIn();
	}
	// always remember to redraw the windows after changing camera parameters!
	Sys_UpdateWindows (W_XY|W_XY_OVERLAY);
	return false;
}
void CXYWnd::RetainClipMode(bool bMode)
{
  bool bSave = g_bRogueClipMode;
  SetClipMode(bMode);
  if (bMode == true)
    g_bRogueClipMode = bSave;
  else
    g_bRogueClipMode = false;
}

void CXYWnd::SetClipMode(bool bMode)
{
  g_bClipMode = bMode;
  g_bRogueClipMode = false;
  if (bMode)
  {
    g_Clip1.Reset();
    g_Clip2.Reset();
    g_Clip3.Reset();
    CleanList(&g_brFrontSplits);
    CleanList(&g_brBackSplits);
    g_brFrontSplits.next = &g_brFrontSplits;
    g_brBackSplits.next = &g_brBackSplits;
  }
  else
  {
    if (g_pMovingClip)
    {
      ReleaseCapture();
      g_pMovingClip = NULL;
    }
    CleanList(&g_brFrontSplits);
    CleanList(&g_brBackSplits);
    g_brFrontSplits.next = &g_brFrontSplits;
    g_brBackSplits.next = &g_brBackSplits;
    Sys_UpdateWindows(XY | W_CAMERA_IFON);
  }
}

bool CXYWnd::ClipMode()
{
  return g_bClipMode;
}

bool CXYWnd::RogueClipMode()
{
  return g_bRogueClipMode;
}

bool CXYWnd::PathMode()
{
  return g_bPathMode;
}


bool CXYWnd::PointMode()
{
  return g_bPointMode;
}

void CXYWnd::SetPointMode(bool b)
{
  g_bPointMode = b;
  if (!b)
    g_nPointCount = 0;
}


void CXYWnd::OnPaint() 
{
	int n;
	CPaintDC dc(this); // device context for painting
  bool bPaint = true;
  if (!wglMakeCurrent(dc.m_hDC, s_hglrcXY))
  {
    Sys_Printf("ERROR: wglMakeCurrent failed.. Error:%i\n",glGetError());
    Sys_Printf("Please restart Q3Radiant if the Map view is not working\n");
    bPaint = false;
  }
  if (bPaint)
  {
    QE_CheckOpenGLForErrors();
		XY_Draw ();
    QE_CheckOpenGLForErrors();

    if (m_nViewType != XY)
    {
      glPushMatrix();
      if (m_nViewType == YZ)
        glRotatef (-90,  0, 1, 0);	    // put Z going up
      glRotatef (-90,  1, 0, 0);	    // put Z going up
    }
 
    if (g_bCrossHairs)
    {
	    glColor4f(0.2, 0.9, 0.2, 0.8);
		  glBegin (GL_LINES);
      if (m_nViewType == XY)
      {
        glVertex2f(-16384, tdp[1]);
        glVertex2f(16384, tdp[1]);
        glVertex2f(tdp[0], -16384);
        glVertex2f(tdp[0], 16384);
      }
      else if (m_nViewType == YZ)
      {
        glVertex3f(tdp[0], -16384, tdp[2]);
        glVertex3f(tdp[0], 16384, tdp[2]);
        glVertex3f(tdp[0], tdp[1], -16384);
        glVertex3f(tdp[0], tdp[1], 16384);
      }
      else
      {
        glVertex3f(-16384, tdp[1], tdp[2]);
        glVertex3f( 16384, tdp[1], tdp[2]);
        glVertex3f(tdp[0], tdp[1], -16384);
        glVertex3f(tdp[0], tdp[1], 16384);
      }
		  glEnd();
    }

    if (ClipMode())
    {
      glPointSize (4);
		  glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER]);
		  glBegin (GL_POINTS);
      if (g_Clip1.Set())
			  glVertex3fv (g_Clip1);
      if (g_Clip2.Set())
			  glVertex3fv (g_Clip2);
      if (g_Clip3.Set())
			  glVertex3fv (g_Clip3);
		  glEnd ();
		  glPointSize (1);

      CString strMsg;
      if (g_Clip1.Set())
      {
        glRasterPos3f (g_Clip1.m_ptClip[0]+2, g_Clip1.m_ptClip[1]+2, g_Clip1.m_ptClip[2]+2);
        strMsg = "1";
        //strMsg.Format("1 (%f, %f, %f)", g_Clip1[0], g_Clip1[1], g_Clip1[2]);
	      glCallLists (strMsg.GetLength(), GL_UNSIGNED_BYTE, strMsg);
      }
      if (g_Clip2.Set())
      {
        glRasterPos3f (g_Clip2.m_ptClip[0]+2, g_Clip2.m_ptClip[1]+2, g_Clip2.m_ptClip[2]+2);
        strMsg = "2";
        //strMsg.Format("2 (%f, %f, %f)", g_Clip2[0], g_Clip2[1], g_Clip2[2]);
	      glCallLists (strMsg.GetLength(), GL_UNSIGNED_BYTE, strMsg);
      }
      if (g_Clip3.Set())
      {
        glRasterPos3f (g_Clip3.m_ptClip[0]+2, g_Clip3.m_ptClip[1]+2, g_Clip3.m_ptClip[2]+2);
        strMsg = "3";
        //strMsg.Format("3 (%f, %f, %f)", g_Clip3[0], g_Clip3[1], g_Clip3[2]);
	      glCallLists (strMsg.GetLength(), GL_UNSIGNED_BYTE, strMsg);
      }
      if (g_Clip1.Set() && g_Clip2.Set())
      {
        ProduceSplitLists();
        brush_s* pBrush;
        brush_s* pList = ( (m_nViewType == XZ) ? !g_bSwitch : g_bSwitch) ? &g_brBackSplits : &g_brFrontSplits;
	      for (pBrush = pList->next ; pBrush != NULL && pBrush != pList ; pBrush=pBrush->next)
        {
		      glColor3f (1,1,0);
	        face_s *face;
	        int order;
	        for (face = pBrush->getFirstFace(),order = 0 ; face ; face=face->getNextFace(), order++)
	        {
		        texturedWinding_c* w = face->face_winding;
		        if (!w)
			        continue;
		        // draw the polygon
		        glBegin(GL_LINE_LOOP);
            for (int i=0 ; i<w->size() ; i++)
		          glVertex3fv(w->getXYZ(i));
		        glEnd();
	        }
        }
      }

    }



    if (PathMode())
    {
      glPointSize (4);
		  glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_CLIPPER]);
		  glBegin (GL_POINTS);

      for (n = 0; n < g_nPathCount; n++)
        glVertex3fv(g_PathPoints[n]);
		  glEnd ();
		  glPointSize (1);

      CString strMsg;
      for (n = 0; n < g_nPathCount; n++)
      {
        glRasterPos3f (g_PathPoints[n].m_ptClip[0]+2, g_PathPoints[n].m_ptClip[1]+2, g_PathPoints[n].m_ptClip[2]+2);
        strMsg.Format("%i", n+1);
	      glCallLists (strMsg.GetLength(), GL_UNSIGNED_BYTE, strMsg);
      }

    }
    if (m_nViewType != XY)
      glPopMatrix();

	glFinish();
		//wgl
	SwapBuffers(dc.m_hDC);
    TRACE("XY Paint\n");
  }
 }

void CXYWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags);
}



// FIXME: the brush_s *pBrush is never used. ( Entity_Create uses selected_brushes )
void CreateEntityFromName(char* pName, brush_s* pBrush)
{
	entityDeclAPI_i *pecNew;
	entity_s *petNew;
	if (_stricmp(pName, "worldspawn") == 0)
	{
		MessageBox(g_qeglobals.d_hwndMain, "Can't create an entity with worldspawn.", "info", 0);
		return;
	}
	
	pecNew = g_declMgr->findOrCreateEntityDecl(pName, false);
	
	// create it
	petNew = Entity_Create(pecNew);
	if (petNew == NULL)
	{
		if (!((selected_brushes.next == &selected_brushes)||(selected_brushes.next->next != &selected_brushes)))
		{
			brush_s* b = selected_brushes.next;
			if (b->owner != world_entity && b->owner->eclass->isFixedSize() && pecNew->isFixedSize())
			{
				vec3_c mins, maxs;
				vec3_c origin;
				for (int i=0 ; i<3 ; i++)
					origin[i] = b->getMins()[i] - pecNew->getEditorMins()[i];
				
				mins = pecNew->getEditorMins() + origin;
				maxs = pecNew->getEditorMaxs() + origin;
				texdef_t td;
				td.setName(pecNew->getEditorMaterialName());
				brush_s* nb = Brush_Create (mins, maxs, &td);
				b->owner->linkBrush(nb);
				nb->owner->eclass = pecNew;
				nb->owner->setKeyValue("classname", pName);
				Brush_Free(b);
				Brush_Build(nb);
				Brush_AddToList (nb, &active_brushes);
				Select_Brush(nb);
				return;
			}
		}
		MessageBox(g_qeglobals.d_hwndMain, "Failed to create entity.", "info", 0);
		return;
	}
	
	Select_Deselect ();
	//entity_s* pEntity = world_entity;
	//if (selected_brushes.next != &selected_brushes)
	//	pEntity = selected_brushes.next->owner;
	Select_Brush (petNew->brushes.onext);
	
	if (_stricmp(pName, "misc_model") == 0)
	{
		SetInspectorMode(W_ENTITY);
		PostMessage(g_qeglobals.d_hwndEntity, WM_COMMAND, IDC_BTN_ASSIGNMODEL, 0);
	}
	
}


brush_s* CreateEntityBrush(int x, int y, CXYWnd* pWnd)
{
	vec3_t	mins, maxs;
	int		i;
	float	temp;
	brush_s	*n;

	pWnd->SnapToPoint (x, y, mins);
  x += 32;
  y += 32;
	pWnd->SnapToPoint (x, y, maxs);

  int nDim = (pWnd->GetViewType() == XY) ? 2 : (pWnd->GetViewType() == YZ) ? 0 : 1;
	mins[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_bottom_z/g_qeglobals.d_gridsize));
	maxs[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_top_z/g_qeglobals.d_gridsize));

  if (maxs[nDim] <= mins[nDim])
		maxs[nDim] = mins[nDim] + g_qeglobals.d_gridsize;

	for (i=0 ; i<3 ; i++)
	{
		if (mins[i] == maxs[i])
			maxs[i] += 16;	// don't create a degenerate brush
		if (mins[i] > maxs[i])
		{
			temp = mins[i];
			mins[i] = maxs[i];
			maxs[i] = temp;
		}
	}

	n = Brush_Create (mins, maxs, &g_qeglobals.d_texturewin.texdef);
	if (!n)
		return NULL;

	Brush_AddToList (n, &selected_brushes);
	world_entity->linkBrush(n);
	Brush_Build( n );
  return n;
}

void CreateRightClickEntity(CXYWnd* pWnd, int x, int y, char* pName)
{
  CRect rctZ;
  pWnd->GetClientRect(rctZ);
  brush_s* pBrush = (selected_brushes.next == &selected_brushes) ? CreateEntityBrush(x, rctZ.Height() - 1 - y, pWnd) : selected_brushes.next;
  CreateEntityFromName(pName, pBrush);
  //Select_Brush(pBrush);
}

brush_s* CreateSmartBrush(vec3_t v)
{
	vec3_t	mins, maxs;
	int		i;
	brush_s	*n;

	for (i=0 ; i<3 ; i++)
	{
    mins[i] = v[i] - 16;
    maxs[i] = v[i] + 16;
  }

	n = Brush_Create (mins, maxs, &g_qeglobals.d_texturewin.texdef);
	if (!n)
		return NULL;

	Brush_AddToList(n, &selected_brushes);
	//Entity_LinkBrush(world_entity, n);
	Brush_Build(n);
  return n;
}




CString g_strSmartEntity;
int g_nSmartX;
int g_nSmartY;
bool g_bSmartWaiting;
void _SmartPointDone(bool b, int n)
{
  g_bSmartWaiting = false;
}

void CreateSmartEntity(CXYWnd* pWnd, int x, int y, const char* pName)
{
  g_nSmartX = x;
  g_nSmartY = y;
  g_strSmartEntity = pName;
  if (g_strSmartEntity.Find("Smart_Train") >= 0)
  {
    ShowInfoDialog("Select the path of the train by left clicking in XY, YZ and/or XZ views. You can move an already dropped point by grabbing and moving it. When you are finished, press ENTER to accept and create the entity and path(s), press ESC to abandon the creation");
    g_bPathMode = true;
    g_nPathLimit = 0;
    g_nPathCount = 0;
    g_bSmartGo = true;
  }
  else
  if (g_strSmartEntity.Find("Smart_Monster...") >= 0)
  {
    g_bPathMode = true;
    g_nPathLimit = 0;
    g_nPathCount = 0;
  }
  else
  if (g_strSmartEntity.Find("Smart_Rotating") >= 0)
  {
    g_bSmartWaiting = true;
    ShowInfoDialog("Left click to specify the rotation origin");
    AcquirePath(1, &_SmartPointDone);
    while (g_bSmartWaiting)
    {
      MSG msg;
      if (::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) 
      { 
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    HideInfoDialog();
    CPtrArray array;
    g_bScreenUpdates = false;
    CreateRightClickEntity(g_pParentWnd->ActiveXY(), g_nSmartX, g_nSmartY, "func_rotating");
    array.Add(reinterpret_cast<void*>(selected_brushes.next));
    Select_Deselect();
    brush_s* pBrush = CreateSmartBrush(g_PathPoints[0]);
    array.Add(pBrush);
    Select_Deselect();
    Select_Brush(reinterpret_cast<brush_s*>(array.GetAt(0)));
    Select_Brush(reinterpret_cast<brush_s*>(array.GetAt(1)));
    ConnectEntities();
    g_bScreenUpdates = true;
  }
}


void FinishSmartCreation()
{
	int n;
  CPtrArray array;
  HideInfoDialog();
  brush_s* pEntities = NULL;
  if (g_strSmartEntity.Find("Smart_Train") >= 0)
  {
    g_bScreenUpdates = false;
    CreateRightClickEntity(g_pParentWnd->ActiveXY(), g_nSmartX, g_nSmartY, "func_train");
    array.Add(reinterpret_cast<void*>(selected_brushes.next));
    for (n = 0; n < g_nPathCount; n++)
    {
      Select_Deselect();
      CreateRightClickEntity(g_pParentWnd->ActiveXY(), g_PathPoints[n].m_ptScreen.x,g_PathPoints[n].m_ptScreen.y, "path_corner");
      array.Add(reinterpret_cast<void*>(selected_brushes.next));
    }

    for (n = 0; n < g_nPathCount; n++)
    {
      Select_Deselect();
      Select_Brush(reinterpret_cast<brush_s*>(array.GetAt(n)));
      Select_Brush(reinterpret_cast<brush_s*>(array.GetAt(n+1)));
      ConnectEntities();
    }
    g_bScreenUpdates = true;

  }
  g_nPathCount = 0;
  g_bPathMode = false;
  Sys_UpdateWindows(W_ALL);
}

void CXYWnd::KillPathMode()
{
  g_bSmartGo = false;
  g_bPathMode = false;
  if (g_pPathFunc)
    g_pPathFunc(false, g_nPathCount);
  g_nPathCount = 0;
  g_pPathFunc = NULL;
  Sys_UpdateWindows(W_ALL);
}

// gets called for drop down menu messages
// TIP: it's not always about EntityCreate
void CXYWnd::OnEntityCreate(unsigned int nID) 
{
  if (m_mnuDrop.GetSafeHmenu())
  {
    CString strItem;
    m_mnuDrop.GetMenuString(nID, strItem, MF_BYCOMMAND);

		if (strItem.CompareNoCase("Add to...") == 0)
		{
			//++timo TODO: fill the menu with current groups?
			// this one is for adding to existing groups only
			Sys_Printf("TODO: Add to... in CXYWnd::OnEntityCreate\n");
		}
		else if (strItem.CompareNoCase("Remove") == 0)
		{
			// remove selected brushes from their current group
			brush_s *b;
			for( b = selected_brushes.next; b != &selected_brushes; b = b->next )
			{
				
			}
		}

		//++timo FIXME: remove when all hooks are in
		if (strItem.CompareNoCase("Add to...") == 0
			|| strItem.CompareNoCase("Remove") == 0
			|| strItem.CompareNoCase("Name...") == 0
			|| strItem.CompareNoCase("New group...") == 0)
		{
			Sys_Printf("TODO: hook drop down group menu\n");
			return;
		}
		
    if (strItem.Find("Smart_") >= 0)
    {
      CreateSmartEntity(this, m_ptDown.x, m_ptDown.y, strItem);
    }
    else
    {
      CreateRightClickEntity(this, m_ptDown.x, m_ptDown.y, strItem.GetBuffer(0));
    }
		
    Sys_UpdateWindows(W_ALL);
    //OnLButtonDown((MK_LBUTTON | MK_SHIFT), CPoint(m_ptDown.x+2, m_ptDown.y+2));
  }
}


#include <api/declManagerAPI.h>
void CXYWnd::HandleDrop()
{
  if (g_PrefsDlg.m_bRightClick == false)
    return;
  if (!m_mnuDrop.GetSafeHmenu()) // first time, load it up
  {
    m_mnuDrop.CreatePopupMenu();
    int nID = ID_ENTITY_START;

    CMenu* pChild2 = new CMenu;
    pChild2->CreateMenu();
    pChild2->AppendMenu(MF_STRING, ID_SELECTION_SELECTCOMPLETETALL, "Select Complete Tall");
    pChild2->AppendMenu(MF_STRING, ID_SELECTION_SELECTTOUCHING, "Select Touching");
    pChild2->AppendMenu(MF_STRING, ID_SELECTION_SELECTPARTIALTALL, "Select Partial Tall");
    pChild2->AppendMenu(MF_STRING, ID_SELECTION_SELECTINSIDE, "Select Inside");
    m_mnuDrop.AppendMenu(MF_POPUP, reinterpret_cast<unsigned int>(pChild2->GetSafeHmenu()), "Select");
    m_mnuDrop.AppendMenu(MF_SEPARATOR, nID++, "");

    CMenu* pChild3 = new CMenu;
    pChild3->CreateMenu();
    pChild3->AppendMenu(MF_STRING, ID_DROP_GROUP_ADDTO, "Add to...");
    pChild3->AppendMenu(MF_STRING, ID_DROP_GROUP_REMOVE, "Remove");
    pChild3->AppendMenu(MF_STRING, ID_DROP_GROUP_NAME, "Name...");
    pChild3->AppendMenu(MF_SEPARATOR, nID++, "");
    pChild3->AppendMenu(MF_STRING, ID_DROP_GROUP_NEWGROUP, "New Group...");
    m_mnuDrop.AppendMenu(MF_POPUP, reinterpret_cast<unsigned int>(pChild3->GetSafeHmenu()), "Group");
    m_mnuDrop.AppendMenu(MF_SEPARATOR, nID++, "");

    m_mnuDrop.AppendMenu(MF_STRING, ID_SELECTION_UNGROUPENTITY, "Ungroup Entity");
    m_mnuDrop.AppendMenu(MF_STRING, ID_SELECTION_MAKE_DETAIL, "Make Detail");
    m_mnuDrop.AppendMenu(MF_STRING, ID_SELECTION_MAKE_STRUCTURAL, "Make Structural");
    m_mnuDrop.AppendMenu(MF_SEPARATOR, nID++, "");

    CMenu* pChild = new CMenu;
    pChild->CreateMenu();
    pChild->AppendMenu(MF_STRING, nID++, "Smart_Train");
    //pChild->AppendMenu(MF_STRING, nID++, "Smart_Rotating");
    m_mnuDrop.AppendMenu(MF_POPUP, reinterpret_cast<unsigned int>(pChild->GetSafeHmenu()), "Smart Entities");
    m_mnuDrop.AppendMenu(MF_SEPARATOR, nID++, "");

    pChild = NULL;
    CString strActive;
    CString strLast;
    CString strName;

	for (u32 i = 0; i < g_declMgr->getNumLoadedEntityDecls(); i++)
	{
		const char *name = g_declMgr->getLoadedEntityDeclName(i);
      strLast = strName;
      strName = name;
      int n_ = strName.Find("_");
      if (n_ > 0)
      {
        CString strLeft = strName.Left(n_);
        CString strRight = strName.Right(strName.GetLength() - n_ - 1);
        if (strLeft == strActive) // this is a child
        {
          ASSERT(pChild);
          pChild->AppendMenu(MF_STRING, nID++, strName);
        }
        else
        {
          if (pChild)
          {
            m_mnuDrop.AppendMenu(MF_POPUP, reinterpret_cast<unsigned int>(pChild->GetSafeHmenu()), strActive);
            g_ptrMenus.Add(pChild);
            //pChild->DestroyMenu();
            //delete pChild;
            pChild = NULL;
          }
          strActive = strLeft;
          pChild = new CMenu;
          pChild->CreateMenu();
          pChild->AppendMenu(MF_STRING, nID++, strName);
        }
      }
      else
      {
        if (pChild)
        {
          m_mnuDrop.AppendMenu(MF_POPUP, reinterpret_cast<unsigned int>(pChild->GetSafeHmenu()), strActive);
          g_ptrMenus.Add(pChild);
          //pChild->DestroyMenu();
          //delete pChild;
          pChild = NULL;
        }
        strActive = "";
        m_mnuDrop.AppendMenu(MF_STRING, nID++, strName);
      }
    }
  }

  CPoint ptMouse;
  GetCursorPos(&ptMouse);
  m_mnuDrop.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);
}

void CXYWnd::XY_Init()
{
	m_vOrigin[0] = 0;
	m_vOrigin[1] = 20;
  m_vOrigin[2] = 46;
  m_fScale = 1;
}

void CXYWnd::SnapToPoint (int x, int y, vec3_t point)
{
  if (g_PrefsDlg.m_bNoClamp)
  {
    XY_ToPoint(x, y, point);
  }
  else
  {
    XY_ToGridPoint(x, y, point);
  }
//--  else
//--    XY_ToPoint(x, y, point);
//--    //XY_ToPoint(x, y, point);
}


void CXYWnd::XY_ToPoint (int x, int y, vec3_t point)
{
  float fx = x;
  float fy = y;
  float fw = m_nWidth;
  float fh = m_nHeight;
  if (m_nViewType == XY)
  {
	  point[0] = m_vOrigin[0] + (fx - fw / 2) / m_fScale;
	  point[1] = m_vOrigin[1] + (fy - fh / 2) / m_fScale;
	  //point[2] = 0;
  }
  else if (m_nViewType == YZ)
  {
    ////point[0] = 0;
	  //point[1] = m_vOrigin[0] + (fx - fw / 2) / m_fScale;
	  //point[2] = m_vOrigin[1] + (fy - fh / 2 ) / m_fScale;
	  point[1] = m_vOrigin[1] + (fx - fw / 2) / m_fScale;
	  point[2] = m_vOrigin[2] + (fy - fh / 2 ) / m_fScale;
  }
  else
  {
	  //point[0] = m_vOrigin[0] + (fx - fw / 2) / m_fScale;
	  ////point[1] = 0;
	  //point[2] = m_vOrigin[1] + (fy - fh / 2) / m_fScale;
	  point[0] = m_vOrigin[0] + (fx - fw / 2) / m_fScale;
	  //point[1] = 0;
	  point[2] = m_vOrigin[2] + (fy - fh / 2) / m_fScale;
  }
}


void CXYWnd::XY_ToGridPoint (int x, int y, vec3_t point)
{
  if (m_nViewType == XY)
  {
	  point[0] = m_vOrigin[0] + (x - m_nWidth / 2) / m_fScale;
	  point[1] = m_vOrigin[1] + (y - m_nHeight / 2) / m_fScale;
	  //point[2] = 0;
	  point[0] = floor(point[0] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
	  point[1] = floor(point[1] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
  }
  else if (m_nViewType == YZ)
  {
	  //point[0] = 0;
	  //point[1] = m_vOrigin[0] + (x - m_nWidth / 2) / m_fScale;
	  //point[2] = m_vOrigin[1] + (y - m_nHeight / 2) / m_fScale;
	  point[1] = m_vOrigin[1] + (x - m_nWidth / 2) / m_fScale;
	  point[2] = m_vOrigin[2] + (y - m_nHeight / 2) / m_fScale;
	  point[1] = floor(point[1] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
	  point[2] = floor(point[2] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
  }
  else
  {
	  //point[1] = 0;
	  //point[0] = m_vOrigin[0] + (x - m_nWidth / 2) / m_fScale;
	  //point[2] = m_vOrigin[1] + (y - m_nHeight / 2) / m_fScale;
	  point[0] = m_vOrigin[0] + (x - m_nWidth / 2) / m_fScale;
	  point[2] = m_vOrigin[2] + (y - m_nHeight / 2) / m_fScale;
	  point[0] = floor(point[0] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
	  point[2] = floor(point[2] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
  }
}


void CXYWnd::XY_MouseDown (int x, int y, int buttons)
{

	vec3_c	point;
	vec3_c	origin, dir, right, up;

	m_nButtonstate = buttons;
  m_nPressx = x;
	m_nPressy = y;
	m_vPressdelta.clear();

	XY_ToPoint (x, y, point);
	
  origin = point;

	dir[0] = 0; dir[1] = 0; dir[2] = 0;
  if (m_nViewType == XY)
  {
	  origin[2] = 131072; // max world coord
    dir[2] = -1;
  	right[0] = 1 / m_fScale; 
    right[1] = 0; 
    right[2] = 0;
	  up[0] = 0; 
    up[1] = 1 / m_fScale;
    up[2] = 0;
  }
  else if (m_nViewType == YZ)
  {
    origin[0] = 131072; // max world coord
    dir[0] = -1;
  	right[1] = 1 / m_fScale; 
    right[2] = 0; 
    right[0] = 0;
   	up[0] = 0; 
    up[2] = 1 / m_fScale;
    up[1] = 0;
  }
  else
  {
    origin[1] = 131072; // max world coord
    dir[1] = -1;
  	right[0] = 1 / m_fScale;
    right[2] = 0; 
    right[1] = 0;
	  up[0] = 0; 
    up[2] = 1 / m_fScale;
    up[1] = 0;
  }


	m_bPress_selection = (selected_brushes.next != &selected_brushes);

  GetCursorPos(&m_ptCursor);
	//Sys_GetCursorPos (&m_ptCursor.x, &m_ptCursor.y);

	// lbutton = manipulate selection
	// shift-LBUTTON = select
	if ( (buttons == MK_LBUTTON)
		|| (buttons == (MK_LBUTTON | MK_SHIFT))
		|| (buttons == (MK_LBUTTON | MK_CONTROL))
		|| (buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT)) )
	{	
    Patch_SetView( (m_nViewType == XY) ? W_XY : (m_nViewType == YZ) ? W_YZ : W_XZ);
		Drag_Begin (x, y, buttons, right, up,	origin, dir);
		return;
	}

  int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;

	// control mbutton = move camera
	if (m_nButtonstate == (MK_CONTROL|nMouseButton) )
	{	
	  VectorCopyXY(point, g_pParentWnd->GetCamera()->Camera().origin);
		Sys_UpdateWindows (W_CAMERA|W_XY_OVERLAY);
	}

	// mbutton = angle camera
	if ((g_PrefsDlg.m_nMouseButtons == 3 && m_nButtonstate == MK_MBUTTON) ||
      (g_PrefsDlg.m_nMouseButtons == 2 && m_nButtonstate == (MK_SHIFT|MK_CONTROL|MK_RBUTTON)))
	{	
		point -= g_pParentWnd->GetCamera()->Camera().origin;

    int n1 = (m_nViewType == XY) ? 1 : 2;
    int n2 = (m_nViewType == YZ) ? 1 : 0;
    int nAngle = (m_nViewType == XY) ? YAW : PITCH;
    if (point[n1] || point[n2])
    {
		  g_pParentWnd->GetCamera()->Camera().angles[nAngle] = RAD2DEG(atan2 (point[n1], point[n2]));
		  Sys_UpdateWindows (W_CAMERA_IFON|W_XY_OVERLAY);
    }
	}

	// shift mbutton = move z checker
	if (m_nButtonstate == (MK_SHIFT | nMouseButton))
	{
    if (RotateMode() || g_bPatchBendMode)
    {
		  SnapToPoint (x, y, point);
	    VectorCopyXY(point, g_vRotateOrigin);
      if (g_bPatchBendMode)
      {
        g_vBendOrigin = point;
      }
		  Sys_UpdateWindows (W_XY);
      return;
    }
    else
    {
		  SnapToPoint (x, y, point);
      if (m_nViewType == XY)
      {
		    z.origin[0] = point[0];
		    z.origin[1] = point[1];
      }
      else if (m_nViewType == YZ)
      {
		    z.origin[0] = point[1];
		    z.origin[1] = point[2];
      }
      else
      {
		    z.origin[0] = point[0];
		    z.origin[1] = point[2];
      }
		  Sys_UpdateWindows (W_XY_OVERLAY|W_Z);
		  return;
    }
	}
}


void CXYWnd::XY_MouseUp(int x, int y, int buttons)
{
	Drag_MouseUp (buttons);
	if (!m_bPress_selection)
		Sys_UpdateWindows (W_ALL);
	m_nButtonstate = 0;
  while (::ShowCursor(TRUE) < 0)
    ;
}

bool CXYWnd::DragDelta (int x, int y, vec3_t move)
{
	vec3_c	xvec, yvec, delta;
	int		i;

	xvec[0] = 1 / m_fScale;
	xvec[1] = xvec[2] = 0;
	yvec[1] = 1 / m_fScale;
	yvec[0] = yvec[2] = 0;

	for (i=0 ; i<3 ; i++)
	{
		delta[i] = xvec[i] * (x - m_nPressx) + yvec[i] * (y - m_nPressy);
    if (!g_PrefsDlg.m_bNoClamp)
    {
		  delta[i] = floor(delta[i] / g_qeglobals.d_gridsize + 0.5) * g_qeglobals.d_gridsize;
    }
	}
	move = delta - m_vPressdelta;
	m_vPressdelta = delta;

	if (move[0] || move[1] || move[2])
		return true;
	return false;
}


/*
==============
NewBrushDrag
==============
*/
void CXYWnd::NewBrushDrag (int x, int y)
{
	vec3_c	mins, maxs, junk;
	int		i;
	float	temp;
	brush_s	*n;

	if (!DragDelta (x,y, junk))
		return;

  // delete the current selection
	if (selected_brushes.next != &selected_brushes)
		Brush_Free (selected_brushes.next);
	
  SnapToPoint (m_nPressx, m_nPressy, mins);

  int nDim = (m_nViewType == XY) ? 2 : (m_nViewType == YZ) ? 0 : 1;

	mins[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_bottom_z / g_qeglobals.d_gridsize));
	SnapToPoint (x, y, maxs);
	maxs[nDim] = g_qeglobals.d_gridsize * ((int)(g_qeglobals.d_new_brush_top_z / g_qeglobals.d_gridsize));
	if (maxs[nDim] <= mins[nDim])
		maxs[nDim] = mins[nDim] + g_qeglobals.d_gridsize;

	for (i=0 ; i<3 ; i++)
	{
		if (mins[i] == maxs[i])
			return;	// don't create a degenerate brush
		if (mins[i] > maxs[i])
		{
			temp = mins[i];
			mins[i] = maxs[i];
			maxs[i] = temp;
		}
	}

	n = Brush_Create (mins, maxs, &g_qeglobals.d_texturewin.texdef);
	if (!n)
		return;

  vec3_c vSize = maxs - mins;
  g_strStatus.Format("Size X:: %.1f  Y:: %.1f  Z:: %.1f", vSize[0], vSize[1], vSize[2]);
  g_pParentWnd->SetStatusText(2, g_strStatus);

	Brush_AddToList (n, &selected_brushes);

	world_entity->linkBrush(n);

	Brush_Build( n );

  //	Sys_UpdateWindows (W_ALL);
	Sys_UpdateWindows (W_XY| W_CAMERA);

}

void CXYWnd::XY_MouseMoved (int x, int y, int buttons)
{
	vec3_c	point;


	if (!m_nButtonstate)
  {
    if (g_bCrossHairs)
    {
      ::ShowCursor(FALSE);
  		Sys_UpdateWindows (W_XY | W_XY_OVERLAY);
      ::ShowCursor(TRUE);
    }
		return;
  }

	// lbutton without selection = drag new brush
	//if (m_nButtonstate == MK_LBUTTON && !m_bPress_selection && g_qeglobals.d_select_mode != sel_curvepoint && g_qeglobals.d_select_mode != sel_splineedit)
	if (m_nButtonstate == MK_LBUTTON && !m_bPress_selection && g_qeglobals.d_select_mode == sel_brush)
	{
		NewBrushDrag (x, y);
		return;
	}

	// lbutton (possibly with control and or shift)
	// with selection = drag selection
	if (m_nButtonstate & MK_LBUTTON)
	{
		Drag_MouseMoved (x, y, buttons);
		Sys_UpdateWindows (W_XY_OVERLAY | W_CAMERA_IFON | W_Z);
		return;
	}

  int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;
	// control mbutton = move camera
	if (m_nButtonstate == (MK_CONTROL|nMouseButton) )
	{
		SnapToPoint (x, y, point);
	  VectorCopyXY(point, g_pParentWnd->GetCamera()->Camera().origin);
		Sys_UpdateWindows (W_XY_OVERLAY | W_CAMERA);
		return;
	}

	// shift mbutton = move z checker
	if (m_nButtonstate == (MK_SHIFT|nMouseButton) )
	{
    if (RotateMode() || g_bPatchBendMode)
    {
		  SnapToPoint (x, y, point);
	    VectorCopyXY(point, g_vRotateOrigin);
      if (g_bPatchBendMode)
      {
        g_vBendOrigin = point;
      }
		  Sys_UpdateWindows (W_XY);
      return;
    }
    else
    {
		  SnapToPoint (x, y, point);
      if (m_nViewType == XY)
      {
		    z.origin[0] = point[0];
		    z.origin[1] = point[1];
      }
      else if (m_nViewType == YZ)
      {
		    z.origin[0] = point[1];
		    z.origin[1] = point[2];
      }
      else
      {
		    z.origin[0] = point[0];
		    z.origin[1] = point[2];
      }
    }
		Sys_UpdateWindows (W_XY_OVERLAY|W_Z);
		return;
	}

	// mbutton = angle camera
	if ((g_PrefsDlg.m_nMouseButtons == 3 && m_nButtonstate == MK_MBUTTON) ||
      (g_PrefsDlg.m_nMouseButtons == 2 && m_nButtonstate == (MK_SHIFT|MK_CONTROL|MK_RBUTTON)))
	{	
		SnapToPoint (x, y, point);
		point -= g_pParentWnd->GetCamera()->Camera().origin;

    int n1 = (m_nViewType == XY) ? 1 : 2;
    int n2 = (m_nViewType == YZ) ? 1 : 0;
    int nAngle = (m_nViewType == XY) ? YAW : PITCH;
    if (point[n1] || point[n2])
    {
		  g_pParentWnd->GetCamera()->Camera().angles[nAngle] = RAD2DEG(atan2 (point[n1], point[n2]));
		  Sys_UpdateWindows (W_CAMERA_IFON|W_XY_OVERLAY);
    }
		return;
	}


	// rbutton = drag xy origin
	if (m_nButtonstate == MK_RBUTTON)
	{
		Sys_GetCursorPos (&x, &y);
		if (x != m_ptCursor.x || y != m_ptCursor.y)
		{
      int nDim1 = (m_nViewType == YZ) ? 1 : 0;
      int nDim2 = (m_nViewType == XY) ? 1 : 2;
			m_vOrigin[nDim1] -= (x - m_ptCursor.x) / m_fScale;
			m_vOrigin[nDim2] += (y - m_ptCursor.y) / m_fScale;
	    SetCursorPos (m_ptCursor.x, m_ptCursor.y);
      ::ShowCursor(FALSE);
      //XY_Draw();
      //RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			Sys_UpdateWindows (W_XY | W_XY_OVERLAY);
      //::ShowCursor(TRUE);
		}
		return;
	}

}


/*
============================================================================

DRAWING

============================================================================
*/


/*
==============
XY_DrawGrid
==============
*/
void CXYWnd::XY_DrawGrid()
{
	float	x, y, xb, xe, yb, ye;
	int		w, h;
	char	text[32];

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	w = m_nWidth / 2 / m_fScale;
	h = m_nHeight / 2 / m_fScale;


  int nDim1 = (m_nViewType == YZ) ? 1 : 0;
  int nDim2 = (m_nViewType == XY) ? 1 : 2;
  //int nDim1 = 0;
  //int nDim2 = 1;


	xb = m_vOrigin[nDim1] - w;
	if (xb < region_mins[nDim1])
		xb = region_mins[nDim1];
	xb = 64 * floor (xb/64);

	xe = m_vOrigin[nDim1] + w;
	if (xe > region_maxs[nDim1])
		xe = region_maxs[nDim1];
	xe = 64 * ceil (xe/64);

	yb = m_vOrigin[nDim2] - h;
	if (yb < region_mins[nDim2])
		yb = region_mins[nDim2];
	yb = 64 * floor (yb/64);

	ye = m_vOrigin[nDim2] + h;
	if (ye > region_maxs[nDim2])
		ye = region_maxs[nDim2];
	ye = 64 * ceil (ye/64);

	// draw major blocks

	glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDMAJOR]);

	int stepSize = 64 * 0.1 / m_fScale;
	if (stepSize < 64) {
		stepSize = 64;
	} else {
		int i;
		for (i = 1; i < stepSize; i <<= 1) {
		}
		stepSize = i;
	}

	if ( g_qeglobals.d_showgrid )
	{
		
		glBegin (GL_LINES);
		
		for (x=xb ; x<=xe ; x+=stepSize)
		{
			glVertex2f (x, yb);
			glVertex2f (x, ye);
		}
		for (y=yb ; y<=ye ; y+=stepSize)
		{
			glVertex2f (xb, y);
			glVertex2f (xe, y);
		}
		
		glEnd ();
		
	}

	// draw minor blocks
	if ( m_fScale > .1 && g_qeglobals.d_showgrid && g_qeglobals.d_gridsize * m_fScale >= 4 && 
       g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR] != g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK])
	{
		glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR]);

		glBegin (GL_LINES);
		for (x=xb ; x<xe ; x += g_qeglobals.d_gridsize)
		{
			if ( ! ((int)x & 63) )
				continue;
			glVertex2f (x, yb);
			glVertex2f (x, ye);
		}
		for (y=yb ; y<ye ; y+=g_qeglobals.d_gridsize)
		{
			if ( ! ((int)y & 63) )
				continue;
			glVertex2f (xb, y);
			glVertex2f (xe, y);
		}
		glEnd ();
	}

	// draw coordinate text if needed

	if ( g_qeglobals.d_savedinfo.show_coordinates)
	{
		//glColor4f(0, 0, 0, 0);
		glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDTEXT]);

		for (x=xb ; x<xe ; x+=stepSize)
		{
			glRasterPos2f (x, m_vOrigin[nDim2] + h - 6 / m_fScale);
			sprintf (text, "%i",(int)x);
			glCallLists (strlen(text), GL_UNSIGNED_BYTE, text);
		}
		for (y=yb ; y<ye ; y+=stepSize)
		{
			glRasterPos2f (m_vOrigin[nDim1] - w + 1, y);
			sprintf (text, "%i",(int)y);
			glCallLists (strlen(text), GL_UNSIGNED_BYTE, text);
		}


    if (Active())
		  glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_VIEWNAME]);

    glRasterPos2f ( m_vOrigin[nDim1] - w + 35 / m_fScale, m_vOrigin[nDim2] + h - 20 / m_fScale );

    char cView[20];
    if (m_nViewType == XY)
      strcpy(cView, "XY Top");
    else 
    if (m_nViewType == XZ)
      strcpy(cView, "XZ Front");
    else
      strcpy(cView, "YZ Side");

		glCallLists (strlen(cView), GL_UNSIGNED_BYTE, cView);
    

  }


/*
  if (true)
  {
		glColor3f(g_qeglobals.d_savedinfo.colors[COLOR_GRIDMINOR]);
		glBegin (GL_LINES);
  	glVertex2f (x, yb);
		glVertex2f (x, ye);
    glEnd();
  }
  */

}

/*
==============
XY_DrawBlockGrid
==============
*/
void CXYWnd::XY_DrawBlockGrid()
{
	float	x, y, xb, xe, yb, ye;
	int		w, h;
	char	text[32];

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	w = m_nWidth / 2 / m_fScale;
	h = m_nHeight / 2 / m_fScale;

  int nDim1 = (m_nViewType == YZ) ? 1 : 0;
  int nDim2 = (m_nViewType == XY) ? 1 : 2;

  xb = m_vOrigin[nDim1] - w;
	if (xb < region_mins[nDim1])
		xb = region_mins[nDim1];
	xb = 1024 * floor (xb/1024);

	xe = m_vOrigin[nDim1] + w;
	if (xe > region_maxs[nDim1])
		xe = region_maxs[nDim1];
	xe = 1024 * ceil (xe/1024);

	yb = m_vOrigin[nDim2] - h;
	if (yb < region_mins[nDim2])
		yb = region_mins[nDim2];
	yb = 1024 * floor (yb/1024);

	ye = m_vOrigin[nDim2] + h;
	if (ye > region_maxs[nDim2])
		ye = region_maxs[nDim2];
	ye = 1024 * ceil (ye/1024);

	// draw major blocks

  glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_GRIDBLOCK]);
	glLineWidth (2);

	glBegin (GL_LINES);
	
	for (x=xb ; x<=xe ; x+=1024)
	{
		glVertex2f (x, yb);
		glVertex2f (x, ye);
	}
	for (y=yb ; y<=ye ; y+=1024)
	{
		glVertex2f (xb, y);
		glVertex2f (xe, y);
	}
	
	glEnd ();
	glLineWidth (1);

	// draw coordinate text if needed

	for (x=xb ; x<xe ; x+=1024)
		for (y=yb ; y<ye ; y+=1024)
		{
			glRasterPos2f (x+512, y+512);
			sprintf (text, "%i,%i",(int)floor(x/1024), (int)floor(y/1024) );
			glCallLists (strlen(text), GL_UNSIGNED_BYTE, text);
		}

	glColor4f(0, 0, 0, 0);
}

void CXYWnd::DrawRotateIcon()
{
	float	x, y;

  if (m_nViewType == XY)
  {
    x = g_vRotateOrigin[0];
    y = g_vRotateOrigin[1];
  }
  else if (m_nViewType == YZ)
  {
    x = g_vRotateOrigin[1];
    y = g_vRotateOrigin[2];
  }
  else
  {
    x = g_vRotateOrigin[0];
    y = g_vRotateOrigin[2];
  }

	glEnable (GL_BLEND);
	glDisable (GL_TEXTURE_2D);
	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glDisable (GL_CULL_FACE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (0.8, 0.1, 0.9, 0.25);

  glBegin(GL_QUADS);
	glVertex3f (x-4,y-4,0);
	glVertex3f (x+4,y-4,0);
	glVertex3f (x+4,y+4,0);
	glVertex3f (x-4,y+4,0);
	glEnd ();
	glDisable (GL_BLEND);

	glColor4f (1.0, 0.2, 1.0, 1);
	glBegin(GL_POINTS);
	glVertex3f (x,y,0);
	glEnd ();

#if 0
	glBegin(GL_LINES);
	glVertex3f (x-6,y+6,0);
	glVertex3f (x+6,y+6,0);
	glVertex3f (x-6,y-6,0);
	glVertex3f (x+6,y-6,0);
	glEnd ();
#endif

}

void CXYWnd::DrawCameraIcon()
{
	float	x, y, a;

  if (m_nViewType == XY)
  {
    x = g_pParentWnd->GetCamera()->Camera().origin[0];
		y = g_pParentWnd->GetCamera()->Camera().origin[1];
	  a = DEG2RAD(g_pParentWnd->GetCamera()->Camera().angles[YAW]);;
  }
  else if (m_nViewType == YZ)
  {
	  x = g_pParentWnd->GetCamera()->Camera().origin[1];
		y = g_pParentWnd->GetCamera()->Camera().origin[2];
	  a = DEG2RAD(g_pParentWnd->GetCamera()->Camera().angles[PITCH]);
  }
  else
  {
	  x = g_pParentWnd->GetCamera()->Camera().origin[0];
		y = g_pParentWnd->GetCamera()->Camera().origin[2];
	  a = DEG2RAD(g_pParentWnd->GetCamera()->Camera().angles[PITCH]);
  }

	glColor3f (0.0, 0.0, 1.0);
	glBegin(GL_LINE_STRIP);
	glVertex3f (x-16,y,0);
	glVertex3f (x,y+8,0);
	glVertex3f (x+16,y,0);
	glVertex3f (x,y-8,0);
	glVertex3f (x-16,y,0);
	glVertex3f (x+16,y,0);
	glEnd ();
	
	glBegin(GL_LINE_STRIP);
	glVertex3f (x+48*cos(a+M_PI/4), y+48*sin(a+M_PI/4), 0);
	glVertex3f (x, y, 0);
	glVertex3f (x+48*cos(a-M_PI/4), y+48*sin(a-M_PI/4), 0);
	glEnd ();

#if 0
  char text[128];
	glRasterPos2f (x+64, y+64);
	sprintf (text, "%f",g_pParentWnd->GetCamera()->Camera().angles[YAW]);
	glCallLists (strlen(text), GL_UNSIGNED_BYTE, text);
#endif

}

void CXYWnd::DrawZIcon (void)
{
  if (m_nViewType == XY)
  {
	  float x = z.origin[0];
	  float y = z.origin[1];
	  glEnable (GL_BLEND);
	  glDisable (GL_TEXTURE_2D);
	  glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	  glDisable (GL_CULL_FACE);
	  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  glColor4f (0.0, 0.0, 1.0, 0.25);
	  glBegin(GL_QUADS);
	  glVertex3f (x-8,y-8,0);
	  glVertex3f (x+8,y-8,0);
	  glVertex3f (x+8,y+8,0);
	  glVertex3f (x-8,y+8,0);
	  glEnd ();
	  glDisable (GL_BLEND);

	  glColor4f (0.0, 0.0, 1.0, 1);

	  glBegin(GL_LINE_LOOP);
	  glVertex3f (x-8,y-8,0);
	  glVertex3f (x+8,y-8,0);
	  glVertex3f (x+8,y+8,0);
	  glVertex3f (x-8,y+8,0);
	  glEnd ();

    glBegin(GL_LINE_STRIP);
	  glVertex3f (x-4,y+4,0);
	  glVertex3f (x+4,y+4,0);
	  glVertex3f (x-4,y-4,0);
	  glVertex3f (x+4,y-4,0);
	  glEnd ();
  }
}


/*
==================
FilterBrush
==================
*/
BOOL FilterBrush(brush_s *pb)
{
	if(pb == 0)
		return FALSE;
	if (!pb->owner)
		return FALSE;		// during construction

  if (pb->hiddenBrush)
  {
    return TRUE;
  }

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_CAULK)
  {
    // filter out the brush only if all faces are caulk
    // if not don't hide the whole brush, proceed on a per-face basis (Cam_Draw)
    //++timo TODO: set this as a preference .. show caulk: hide any brush with caulk // don't draw caulk faces
    face_s *f;
    f=pb->getFirstFace();
    while (f)
    {
      if (!strstr(f->getMatName(), "caulk"))
        break;
      f = f->next;
    }
    if (!f)
      return TRUE;

#if 0
    if (strstr(pb->getFirstFace()->getMatName(), "caulk"))
      return TRUE;
#endif

    //++timo FIXME: .. same deal here?
    if (strstr(pb->getFirstFace()->getMatName(), "donotenter"))
      return TRUE;
  }

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_HINT)
  {
    if (strstr(pb->getFirstFace()->getMatName(), "hint"))
      return TRUE;
  }

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_CLIP)
	{
    if (strstr(pb->getFirstFace()->getMatName(), "clip"))
      return TRUE;

    if (strstr(pb->getFirstFace()->getMatName(), "skip"))
      return TRUE;

		//if (!strncmp(pb->getFirstFace()->getMatName(), "clip", 4))
		//	return TRUE;
	}

	//if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_WATER)
	//{
	//	for (face_s* f = pb->getFirstFace() ; f ; f=f->next)
 //   {
 //     if (f->texdef.contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
 //       return TRUE;
 //   }
	//}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_DETAIL)
	{
		if (pb->getFirstFace()->texdef.contents & CONTENTS_DETAIL)
			return TRUE;
	}

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_CURVES)
	{
		if (pb->patchBrush)
			return TRUE;
	}

	if( g_qeglobals.d_savedinfo.exclude & EXCLUDE_TERRAIN )
	{
		if( pb->terrainBrush )
		{
			return TRUE;
		}
	}

	if (pb->owner == world_entity)
	{
		if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_WORLD)
			return TRUE;
		return FALSE;
	}
	else 
  {
    if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_ENT)
    {
		  return (strncmp(pb->owner->eclass->getDeclName(), "func_group", 10));
    }
  }

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_LIGHTS)
	{
    return (pb->owner->eclass->hasEditorFlagLight());
		//if (!strncmp(pb->owner->eclass->getDeclName(), "light", 5))
		//	return TRUE;
	}

	//if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS)
	//{
 //   return (pb->owner->eclass->nShowFlags & ECLASS_PATH);
	//	//if (!strncmp(pb->owner->eclass->getDeclName(), "path", 4))
	//	//	return TRUE;
	//}

	return FALSE;
}

/*
=============================================================

  PATH LINES

=============================================================
*/

/*
==================
DrawPathLines

Draws connections between entities.
Needs to consider all entities, not just ones on screen,
because the lines can be visible when neither end is.
Called for both camera view and xy view.
==================
*/
void DrawPathLines (void)
{
	int		i, j, k;
	vec3_c	mid, mid1;
	entity_s *se, *te;
	brush_s	*sb, *tb;
	const char	*psz;
	vec3_c	dir, s1, s2;
	float	len, f;
	int		arrows;
	int			num_entities;
	arraySTD_c<const char		*> ent_target;
	arraySTD_c<entity_s	*>ent_entity;

	if (g_qeglobals.d_savedinfo.exclude & EXCLUDE_PATHS)
    return;

	ent_entity.resize(g_numentities);
	ent_target.resize(g_numentities);

	num_entities = 0;
	for (te = entities.next ; te != &entities ; te = te->next)
	{
		ent_target[num_entities] = te->getKeyValue("target");
		if (ent_target[num_entities][0])
		{
			ent_entity[num_entities] = te;
			num_entities++;
		}
	}

	for (se = entities.next ; se != &entities ; se = se->next)
	{
		psz = se->getKeyValue("targetname");
	
		if (psz == NULL || psz[0] == '\0')
			continue;
		
		sb = se->brushes.onext;
		if (sb == &se->brushes)
			continue;

		for (k=0 ; k<num_entities ; k++)
		{
		if (strcmp (ent_target[k], psz))
				continue;

			te = ent_entity[k];
			tb = te->brushes.onext;
			if (tb == &te->brushes)
				continue;

			for (i=0 ; i<3 ; i++)
				mid[i] = (sb->getMins()[i] + sb->getMaxs()[i])*0.5; 

			for (i=0 ; i<3 ; i++)
				mid1[i] = (tb->getMins()[i] + tb->getMaxs()[i])*0.5; 

			dir = mid1 - mid;
			len = dir.normalize2();
			s1[0] = -dir[1]*8 + dir[0]*8;
			s2[0] = dir[1]*8 + dir[0]*8;
			s1[1] = dir[0]*8 + dir[1]*8;
			s2[1] = -dir[0]*8 + dir[1]*8;

			glColor3f (se->eclass->getEditorColor()[0], se->eclass->getEditorColor()[1], se->eclass->getEditorColor()[2]);

			glBegin(GL_LINES);
			glVertex3fv(mid);
			glVertex3fv(mid1);

			arrows = (int)(len / 256) + 1;

			for (i=0 ; i<arrows ; i++)
			{
				f = len * (i + 0.5) / arrows;

				for (j=0 ; j<3 ; j++)
					mid1[j] = mid[j] + f*dir[j];
				glVertex3fv (mid1);
				glVertex3f (mid1[0] + s1[0], mid1[1] + s1[1], mid1[2]);
				glVertex3fv (mid1);
				glVertex3f (mid1[0] + s2[0], mid1[1] + s2[1], mid1[2]);
			}

			glEnd();
		}
	}

	return;
}

//=============================================================

// can be greatly simplified but per usual i am in a hurry 
// which is not an excuse, just a fact
void CXYWnd::PaintSizeInfo(int nDim1, int nDim2, const aabb &bounds)
{

  vec3_c vSize = bounds.getSizes();

  glColor3f(g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][0] * .65, 
            g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][1] * .65,
            g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES][2] * .65);

  if (m_nViewType == XY)
  {
		glBegin (GL_LINES);

    glVertex3f(bounds.getMins()[nDim1], bounds.getMins()[nDim2] - 6.0f  / m_fScale, 0.0f);
    glVertex3f(bounds.getMins()[nDim1], bounds.getMins()[nDim2] - 10.0f / m_fScale, 0.0f);

    glVertex3f(bounds.getMins()[nDim1], bounds.getMins()[nDim2] - 10.0f  / m_fScale, 0.0f);
    glVertex3f(bounds.getMaxs()[nDim1], bounds.getMins()[nDim2] - 10.0f  / m_fScale, 0.0f);

    glVertex3f(bounds.getMaxs()[nDim1], bounds.getMins()[nDim2] - 6.0f  / m_fScale, 0.0f);
    glVertex3f(bounds.getMaxs()[nDim1], bounds.getMins()[nDim2] - 10.0f / m_fScale, 0.0f);
  

    glVertex3f(bounds.getMaxs()[nDim1] + 6.0f  / m_fScale, bounds.getMins()[nDim2], 0.0f);
    glVertex3f(bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, bounds.getMins()[nDim2], 0.0f);

    glVertex3f(bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, bounds.getMins()[nDim2], 0.0f);
    glVertex3f(bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, bounds.getMaxs()[nDim2], 0.0f);
  
    glVertex3f(bounds.getMaxs()[nDim1] + 6.0f  / m_fScale, bounds.getMaxs()[nDim2], 0.0f);
    glVertex3f(bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, bounds.getMaxs()[nDim2], 0.0f);

    glEnd();

    glRasterPos3f (Betwixt(bounds.getMins()[nDim1], bounds.getMaxs()[nDim1]),  bounds.getMins()[nDim2] - 20.0  / m_fScale, 0.0f);
    g_strDim.Format(g_pDimStrings[nDim1], vSize[nDim1]);
	  glCallLists (g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);
    
    glRasterPos3f (bounds.getMaxs()[nDim1] + 16.0  / m_fScale, Betwixt(bounds.getMins()[nDim2], bounds.getMaxs()[nDim2]), 0.0f);
    g_strDim.Format(g_pDimStrings[nDim2], vSize[nDim2]);
	  glCallLists (g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

    glRasterPos3f (bounds.getMins()[nDim1] + 4, bounds.getMaxs()[nDim2] + 8 / m_fScale, 0.0f);
    g_strDim.Format(g_pOrgStrings[0], bounds.getMins()[nDim1], bounds.getMaxs()[nDim2]);
	  glCallLists (g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

  }
  else
  if (m_nViewType == XZ)
  {
		glBegin (GL_LINES);

    glVertex3f(bounds.getMins()[nDim1], 0, bounds.getMins()[nDim2] - 6.0f  / m_fScale);
    glVertex3f(bounds.getMins()[nDim1], 0, bounds.getMins()[nDim2] - 10.0f / m_fScale);

    glVertex3f(bounds.getMins()[nDim1], 0,bounds.getMins()[nDim2] - 10.0f  / m_fScale);
    glVertex3f(bounds.getMaxs()[nDim1], 0,bounds.getMins()[nDim2] - 10.0f  / m_fScale);

    glVertex3f(bounds.getMaxs()[nDim1], 0,bounds.getMins()[nDim2] - 6.0f  / m_fScale);
    glVertex3f(bounds.getMaxs()[nDim1], 0,bounds.getMins()[nDim2] - 10.0f / m_fScale);
  

    glVertex3f(bounds.getMaxs()[nDim1] + 6.0f  / m_fScale, 0,bounds.getMins()[nDim2]);
    glVertex3f(bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, 0,bounds.getMins()[nDim2]);

    glVertex3f(bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, 0,bounds.getMins()[nDim2]);
    glVertex3f(bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, 0,bounds.getMaxs()[nDim2]);
  
    glVertex3f(bounds.getMaxs()[nDim1] + 6.0f  / m_fScale, 0,bounds.getMaxs()[nDim2]);
    glVertex3f(bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, 0,bounds.getMaxs()[nDim2]);

    glEnd();

    glRasterPos3f (Betwixt(bounds.getMins()[nDim1], bounds.getMaxs()[nDim1]), 0, bounds.getMins()[nDim2] - 20.0  / m_fScale);
    g_strDim.Format(g_pDimStrings[nDim1], vSize[nDim1]);
	  glCallLists (g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);
    
    glRasterPos3f (bounds.getMaxs()[nDim1] + 16.0  / m_fScale, 0, Betwixt(bounds.getMins()[nDim2], bounds.getMaxs()[nDim2]));
    g_strDim.Format(g_pDimStrings[nDim2], vSize[nDim2]);
	  glCallLists (g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

    glRasterPos3f (bounds.getMins()[nDim1] + 4, 0, bounds.getMaxs()[nDim2] + 8 / m_fScale);
    g_strDim.Format(g_pOrgStrings[1], bounds.getMins()[nDim1], bounds.getMaxs()[nDim2]);
	  glCallLists (g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

  }
  else
  {
		glBegin (GL_LINES);

    glVertex3f(0, bounds.getMins()[nDim1], bounds.getMins()[nDim2] - 6.0f  / m_fScale);
    glVertex3f(0, bounds.getMins()[nDim1], bounds.getMins()[nDim2] - 10.0f / m_fScale);

    glVertex3f(0, bounds.getMins()[nDim1], bounds.getMins()[nDim2] - 10.0f  / m_fScale);
    glVertex3f(0, bounds.getMaxs()[nDim1], bounds.getMins()[nDim2] - 10.0f  / m_fScale);

    glVertex3f(0, bounds.getMaxs()[nDim1], bounds.getMins()[nDim2] - 6.0f  / m_fScale);
    glVertex3f(0, bounds.getMaxs()[nDim1], bounds.getMins()[nDim2] - 10.0f / m_fScale);
  

    glVertex3f(0, bounds.getMaxs()[nDim1] + 6.0f  / m_fScale, bounds.getMins()[nDim2]);
    glVertex3f(0, bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, bounds.getMins()[nDim2]);

    glVertex3f(0, bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, bounds.getMins()[nDim2]);
    glVertex3f(0, bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, bounds.getMaxs()[nDim2]);
  
    glVertex3f(0, bounds.getMaxs()[nDim1] + 6.0f  / m_fScale, bounds.getMaxs()[nDim2]);
    glVertex3f(0, bounds.getMaxs()[nDim1] + 10.0f  / m_fScale, bounds.getMaxs()[nDim2]);

    glEnd();

    glRasterPos3f (0, Betwixt(bounds.getMins()[nDim1], bounds.getMaxs()[nDim1]),  bounds.getMins()[nDim2] - 20.0  / m_fScale);
    g_strDim.Format(g_pDimStrings[nDim1], vSize[nDim1]);
	  glCallLists (g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);
    
    glRasterPos3f (0, bounds.getMaxs()[nDim1] + 16.0  / m_fScale, Betwixt(bounds.getMins()[nDim2], bounds.getMaxs()[nDim2]));
    g_strDim.Format(g_pDimStrings[nDim2], vSize[nDim2]);
	  glCallLists (g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

    glRasterPos3f (0, bounds.getMins()[nDim1] + 4.0, bounds.getMaxs()[nDim2] + 8 / m_fScale);
    g_strDim.Format(g_pOrgStrings[2], bounds.getMins()[nDim1], bounds.getMaxs()[nDim2]);
	  glCallLists (g_strDim.GetLength(), GL_UNSIGNED_BYTE, g_strDim);

  }
}



/*
==============
XY_Draw
==============
*/
long g_lCount = 0;
long g_lTotal = 0;
extern void DrawBrushEntityName (brush_s *b);

void CXYWnd::XY_Draw()
{
  brush_s	*brush;
	float	w, h;
	entity_s	*e;
	double	start, end;
	double	start2, end2;
	vec3_t	mins, maxs;
	int		drawn, culled;
	int		i;

	if (!active_brushes.next)
		return;	// not valid yet

	if (m_bTiming)
		start = Sys_DoubleTime();
	//
	// clear
	//
	m_bDirty = false;

	glViewport(0, 0, m_nWidth, m_nHeight);
	glClearColor (g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][0],
		            g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][1],
		            g_qeglobals.d_savedinfo.colors[COLOR_GRIDBACK][2],0);

  glClear(GL_COLOR_BUFFER_BIT);

	//
	// set up viewpoint
	//
	glMatrixMode(GL_PROJECTION);
  glLoadIdentity ();

	w = m_nWidth / 2 / m_fScale;
	h = m_nHeight / 2/ m_fScale;
  int nDim1 = (m_nViewType == YZ) ? 1 : 0;
  int nDim2 = (m_nViewType == XY) ? 1 : 2;
	mins[0] = m_vOrigin[nDim1] - w;
	maxs[0] = m_vOrigin[nDim1] + w;
	mins[1] = m_vOrigin[nDim2] - h;
	maxs[1] = m_vOrigin[nDim2] + h;


	glOrtho (mins[0], maxs[0], mins[1], maxs[1], -18192, 18192);
  //glRotatef
	//
	// now draw the grid
	//
	XY_DrawGrid ();

	//
	// draw stuff
	//
  glShadeModel (GL_FLAT);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColor3f(0, 0, 0);

  //glEnable (GL_LINE_SMOOTH);

	drawn = culled = 0;

  if (m_nViewType != XY)
  {
    glPushMatrix();
    if (m_nViewType == YZ)
      glRotatef (-90,  0, 1, 0);	    // put Z going up
   //else
      glRotatef (-90,  1, 0, 0);	    // put Z going up
  }

	e = world_entity;
	
  if (m_bTiming)
		start2 = Sys_DoubleTime();

	for (brush = active_brushes.next ; brush != &active_brushes ; brush=brush->next)
	{
		if (brush->getMins()[nDim1] > maxs[0] || 
        brush->getMins()[nDim2] > maxs[1] || 
        brush->getMaxs()[nDim1] < mins[0] || 
        brush->getMaxs()[nDim2] < mins[1])
		{
		  culled++;
		  continue;		// off screen
		}

	 	if (FilterBrush (brush))
	 		continue;

    drawn++;

    if (brush->owner != e && brush->owner)
		{

			glColor3fv(brush->owner->eclass->getEditorColor());
		}
    else
    {
		  glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_BRUSHES]);
    }

    Brush_DrawXY(brush, m_nViewType);
	}
  
  
  if (m_bTiming)
		end2 = Sys_DoubleTime();


  DrawPathLines ();

	//
	// draw pointfile
	//
///	if ( g_qeglobals.d_pointfile_display_list)
///		glCallList (g_qeglobals.d_pointfile_display_list);


  if (!(m_nViewType == XY))
    glPopMatrix();

  //
	// draw block grid
	//
	if ( g_qeglobals.show_blocks)
		XY_DrawBlockGrid ();

	//
	// now draw selected brushes
	//
  if (m_nViewType != XY)
  {
    glPushMatrix();
    if (m_nViewType == YZ)
      glRotatef (-90,  0, 1, 0);	    // put Z going up
    //else
      glRotatef (-90,  1, 0, 0);	    // put Z going up
  }


  glPushMatrix();
	glTranslatef( g_qeglobals.d_select_translate[0], g_qeglobals.d_select_translate[1], g_qeglobals.d_select_translate[2]);

  if (RotateMode())
    glColor3f(0.8, 0.1, 0.9);
  else
  if (ScaleMode())
    glColor3f(0.1, 0.8, 0.1);
  else
    glColor3fv(g_qeglobals.d_savedinfo.colors[COLOR_SELBRUSHES]);


  if (g_PrefsDlg.m_bNoStipple == FALSE)
  {
    glEnable (GL_LINE_STIPPLE);
	  glLineStipple (3, 0xaaaa);
  }
	glLineWidth (2);

	aabb bounds;
  bounds.clear();

  int nSaveDrawn = drawn;
  bool bFixedSize = false;
	for (brush = selected_brushes.next ; brush != &selected_brushes ; brush=brush->next)
	{
		drawn++;
		Brush_DrawXY(brush, m_nViewType);

    if (!bFixedSize)
    {
      if (brush->owner->eclass->isFixedSize())
        bFixedSize = true;
      if (g_PrefsDlg.m_bSizePaint)
      {
			bounds.addBox(brush->getBounds());
      }
    }
	}

  if (g_PrefsDlg.m_bNoStipple == FALSE)
  {
	  glDisable (GL_LINE_STIPPLE);
  }
	glLineWidth (1);

  if (!bFixedSize && !RotateMode() && !ScaleMode() && drawn - nSaveDrawn > 0 && g_PrefsDlg.m_bSizePaint)
    PaintSizeInfo(nDim1, nDim2, bounds);


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

  glPopMatrix();

	glTranslatef (-g_qeglobals.d_select_translate[0], -g_qeglobals.d_select_translate[1], -g_qeglobals.d_select_translate[2]);


  if (!(m_nViewType == XY))
    glPopMatrix();

  // area selection hack
  if (g_qeglobals.d_select_mode == sel_area)
  {
	  glEnable (GL_BLEND);
	  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	  glColor4f(0.0, 0.0, 1.0, 0.25);
    glRectf(g_qeglobals.d_vAreaTL[nDim1], g_qeglobals.d_vAreaTL[nDim2], g_qeglobals.d_vAreaBR[nDim1], g_qeglobals.d_vAreaBR[nDim2]);
	  glDisable (GL_BLEND);
  }


  //
	// now draw camera point
	//
	DrawCameraIcon ();
	DrawZIcon ();

  if (RotateMode())
  {
    DrawRotateIcon();
  }

  glFinish();
	//QE_CheckOpenGLForErrors();

  if (m_bTiming)
	{
		end = Sys_DoubleTime ();
    i = (int)(1000*(end-start));
    int i3 = (int)(1000*(end2-start2));
    g_lCount++;
    g_lTotal += i;
    int i2 = g_lTotal / g_lCount;
		Sys_Printf ("xy: %i ab: %i  avg: %i\n", i, i3, i2);
	}
}

/*
==============
XY_Overlay
==============
*/
void CXYWnd::XY_Overlay()
{
	int	w, h;
	int	r[4];
	static	vec3_c	lastz;
	static	vec3_c	lastcamera;


	glViewport(0, 0, m_nWidth, m_nHeight);

	//
	// set up viewpoint
	//
	glMatrixMode(GL_PROJECTION);
  glLoadIdentity ();

	w = m_nWidth / 2 / m_fScale;
	h = m_nHeight / 2 / m_fScale;

  glOrtho (m_vOrigin[0] - w, m_vOrigin[0] + w	, m_vOrigin[1] - h, m_vOrigin[1] + h, -8000, 8000);
	//
	// erase the old camera and z checker positions
	// if the entire xy hasn't been redrawn
	//
	if (m_bDirty)
	{
		glReadBuffer (GL_BACK);
		glDrawBuffer (GL_FRONT);

		glRasterPos2f (lastz[0]-9, lastz[1]-9);
		glGetIntegerv (GL_CURRENT_RASTER_POSITION,r);
		glCopyPixels(r[0], r[1], 18,18, GL_COLOR);

		glRasterPos2f (lastcamera[0]-50, lastcamera[1]-50);
		glGetIntegerv (GL_CURRENT_RASTER_POSITION,r);
		glCopyPixels(r[0], r[1], 100,100, GL_COLOR);
	}
	m_bDirty = true;

	//
	// save off underneath where we are about to draw
	//
	lastz = z.origin;
	lastcamera = g_pParentWnd->GetCamera()->Camera().origin;

	glReadBuffer (GL_FRONT);
	glDrawBuffer (GL_BACK);

	glRasterPos2f (lastz[0]-9, lastz[1]-9);
	glGetIntegerv (GL_CURRENT_RASTER_POSITION,r);
	glCopyPixels(r[0], r[1], 18,18, GL_COLOR);

	glRasterPos2f (lastcamera[0]-50, lastcamera[1]-50);
	glGetIntegerv (GL_CURRENT_RASTER_POSITION,r);
	glCopyPixels(r[0], r[1], 100,100, GL_COLOR);

	//
	// draw the new icons
	//
	glDrawBuffer (GL_FRONT);

  glShadeModel (GL_FLAT);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColor3f(0, 0, 0);

	DrawCameraIcon ();
	DrawZIcon ();

	glDrawBuffer (GL_BACK);
  glFinish();
}


vec3_c& CXYWnd::GetOrigin()
{
  return m_vOrigin;
}

void CXYWnd::SetOrigin(vec3_t org)
{
	m_vOrigin[0] = org[0];
	m_vOrigin[1] = org[1];
	m_vOrigin[2] = org[2];
}

void CXYWnd::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
  CRect rect;
  GetClientRect(rect);
  m_nWidth = rect.Width();
  m_nHeight = rect.Height();
}

brush_s hold_brushes;
void CXYWnd::Clip()
{
  if (ClipMode())
  {
    hold_brushes.next = &hold_brushes;
    ProduceSplitLists();
    //brush_s* pList = (g_bSwitch) ? &g_brFrontSplits : &g_brBackSplits;
    brush_s* pList;
	if (g_PrefsDlg.m_bSwitchClip)
		pList = ( (m_nViewType == XZ) ? g_bSwitch: !g_bSwitch) ? &g_brFrontSplits : &g_brBackSplits;
	else
		pList = ( (m_nViewType == XZ) ? !g_bSwitch: g_bSwitch) ? &g_brFrontSplits : &g_brBackSplits;

    
	if (pList->next != pList)
    {
      Brush_CopyList(pList, &hold_brushes);
      CleanList(&g_brFrontSplits);
      CleanList(&g_brBackSplits);
      Select_Delete();
      Brush_CopyList(&hold_brushes, &selected_brushes);
      if (RogueClipMode())
        RetainClipMode(false);
      else
        RetainClipMode(true);
      Sys_UpdateWindows(W_ALL);
    }
  }
  else if (PathMode())
  {
    FinishSmartCreation();
    if (g_pPathFunc)
      g_pPathFunc(true, g_nPathCount);
    g_pPathFunc = NULL;
    g_nPathCount = 0;
    g_bPathMode = false;
  }
}

void CXYWnd::SplitClip()
{
	ProduceSplitLists();
	if ((g_brFrontSplits.next != &g_brFrontSplits) &&
		(g_brBackSplits.next != &g_brBackSplits))
	{
		Select_Delete();
		Brush_CopyList(&g_brFrontSplits, &selected_brushes);
		Brush_CopyList(&g_brBackSplits, &selected_brushes);
		CleanList(&g_brFrontSplits);
		CleanList(&g_brBackSplits);
		if (RogueClipMode())
			RetainClipMode(false);
		else
			RetainClipMode(true);
	}
}

void CXYWnd::FlipClip()
{
  g_bSwitch = !g_bSwitch;
  Sys_UpdateWindows(XY | W_CAMERA_IFON);
}


// makes sure the selected brush or camera is in view
void CXYWnd::PositionView()
{
  int nDim1 = (m_nViewType == YZ) ? 1 : 0;
  int nDim2 = (m_nViewType == XY) ? 1 : 2;
  brush_s* b = selected_brushes.next;
  if (b && b->next != b)
  {
	  m_vOrigin[nDim1] = b->getMins()[nDim1];
	  m_vOrigin[nDim2] = b->getMins()[nDim2];
  }
  else
  {
	  m_vOrigin[nDim1] = g_pParentWnd->GetCamera()->Camera().origin[nDim1];
	  m_vOrigin[nDim2] = g_pParentWnd->GetCamera()->Camera().origin[nDim2];
  }
}

void CXYWnd::VectorCopyXY(vec3_t in, vec3_t out)
{
  if (m_nViewType == XY)
  {
	  out[0] = in[0];
	  out[1] = in[1];
  }
  else
  if (m_nViewType == XZ)
  {
	  out[0] = in[0];
	  out[2] = in[2];
  }
  else
  {
	  out[1] = in[1];
	  out[2] = in[2];
  }
}


void CXYWnd::OnDestroy() 
{
	QEW_StopGL( GetSafeHwnd(), s_hglrcXY, s_hdcXY );
	CWnd::OnDestroy();
  // delete this;
}

void CXYWnd::SetViewType(int n) 
{ 
  m_nViewType = n; 
  if (g_pParentWnd->CurrentStyle() == QR_QE4)
  {
    CString str = "YZ Side";
    if (m_nViewType == XY)
      str = "XY Top";
    else if (m_nViewType == XZ)
      str = "XZ Front";
    SetWindowText(str);
  }
};

void CXYWnd::Redraw(unsigned int nBits)
{
  m_nUpdateBits = nBits;
  RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
  m_nUpdateBits = W_XY;
}

bool CXYWnd::RotateMode()
{
  return g_bRotateMode;
}

bool CXYWnd::ScaleMode()
{
  return g_bScaleMode;
}

bool CXYWnd::SetRotateMode(bool bMode)
{
  if (bMode && selected_brushes.next != &selected_brushes)
  {
    g_bRotateMode = true;
    Select_GetTrueMid(g_vRotateOrigin);
    g_vRotation[0] = g_vRotation[1] = g_vRotation[2] = 0.0;
  }
  else 
  {
    if (bMode)
      Sys_Printf("Need a brush selected to turn on Mouse Rotation mode\n");
    g_bRotateMode = false;
  }
  RedrawWindow();
  return g_bRotateMode;
}

void CXYWnd::SetScaleMode(bool bMode)
{
  g_bScaleMode = bMode;
  RedrawWindow();
}




// xy - z
// xz - y
// yz - x

void CXYWnd::OnSelectMouserotate() 
{
	// TODO: Add your command handler code here
	
}

void CleanCopyEntities()
{
	entity_s* pe = g_enClipboard.next;
  while (pe != NULL && pe != &g_enClipboard)
  {
    entity_s* next = pe->next;
	  // delete entity 
	  delete pe;
    pe = next;
  }
  g_enClipboard.next = g_enClipboard.prev = &g_enClipboard;
}

entity_s	*Entity_CopyClone (entity_s *e)
{
	entity_s	*n;

	n = new entity_s();
	n->brushes.onext = n->brushes.oprev = &n->brushes;
	n->eclass = e->eclass;

	// add the entity to the entity list
	n->next = g_enClipboard.next;
	g_enClipboard.next = n;
	n->next->prev = n;
	n->prev = &g_enClipboard;

	// copy key values
	n->keyValues = e->keyValues;
	return n;
}

bool OnList(entity_s* pFind, CPtrArray* pList)
{
  int nSize = pList->GetSize();
  while (nSize-- > 0)
  {
    entity_s* pEntity = reinterpret_cast<entity_s*>(pList->GetAt(nSize));
    if (pEntity == pFind)
      return true;
  }
  return false;
}

void CXYWnd::Copy()
{
#if 1
  CWaitCursor WaitCursor; 
  g_Clipboard.SetLength(0);
  g_PatchClipboard.SetLength(0);
  
  Map_SaveSelected(&g_Clipboard, &g_PatchClipboard);
  bool bClipped = false;
  UINT nClipboard = ::RegisterClipboardFormat("RadiantClippings");
  if (nClipboard > 0)
  {
    if (OpenClipboard())
    {
      ::EmptyClipboard();
  		long lSize = g_Clipboard.GetLength();
      HANDLE h = ::GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, lSize + sizeof(long));
      if (h != NULL)
      {
        unsigned char *cp = reinterpret_cast<unsigned char*>(::GlobalLock(h));
				memcpy(cp, &lSize, sizeof(long));
				cp += sizeof(long);
        g_Clipboard.SeekToBegin();
        g_Clipboard.Read(cp, lSize);
        ::GlobalUnlock(h);
        ::SetClipboardData(nClipboard, h);
        ::CloseClipboard();
        bClipped = true;
      }
    }
  }

  if (!bClipped)
  {
    Sys_Printf("Unable to register Windows clipboard formats, copy/paste between editors will not be possible");
  }

/*
  CString strOut;
  ::GetTempPath(1024, strOut.GetBuffer(1024));
  strOut.ReleaseBuffer();
  AddSlash(strOut);
  strOut += "RadiantClipboard.$$$";
  Map_SaveSelected(strOut.GetBuffer(0));
*/

#else
  CPtrArray holdArray;
  CleanList(&g_brClipboard);
  CleanCopyEntities();
	for (brush_s* pBrush = selected_brushes.next ; pBrush != NULL && pBrush != &selected_brushes ; pBrush=pBrush->next)
  {
		if (pBrush->owner == world_entity)
    {
      brush_s* pClone = Brush_Clone(pBrush);
      pClone->owner = NULL;
  	  Brush_AddToList (pClone, &g_brClipboard);
    }
    else
    {
      if (!OnList(pBrush->owner, &holdArray))
      {
        entity_s* e = pBrush->owner;
        holdArray.Add(reinterpret_cast<void*>(e));
        entity_s* pEClone = Entity_CopyClone(e);
			  for (brush_s* pEB = e->brushes.onext ; pEB != &e->brushes ; pEB=pEB->onext)
			  {
          brush_s* pClone = Brush_Clone(pEB);
	        //Brush_AddToList (pClone, &g_brClipboard);
          Entity_LinkBrush(pEClone, pClone);
          Brush_Build(pClone);
			  }
      }
    }
  }
#endif
}

void CXYWnd::Undo()
{
/*
  if (g_brUndo.next != &g_brUndo)
  {
    g_bScreenUpdates = false; 
    Select_Delete();
	  for (brush_s* pBrush = g_brUndo.next ; pBrush != NULL && pBrush != &g_brUndo ; pBrush=pBrush->next)
    {
      brush_s* pClone = Brush_Clone(pBrush);
    	Brush_AddToList (pClone, &active_brushes);
			Entity_LinkBrush (pBrush->pUndoOwner, pClone);
      Brush_Build(pClone);
      Select_Brush(pClone);
    }
    CleanList(&g_brUndo);
    g_bScreenUpdates = true; 
    Sys_UpdateWindows(W_ALL);
  }
  else Sys_Printf("Nothing to undo.../n");
*/
}

void CXYWnd::UndoClear()
{
/*  
  CleanList(&g_brUndo);
*/
}

void CXYWnd::UndoCopy()
{
/*
  CleanList(&g_brUndo);
	for (brush_s* pBrush = selected_brushes.next ; pBrush != NULL && pBrush != &selected_brushes ; pBrush=pBrush->next)
  {
    brush_s* pClone = Brush_Clone(pBrush);
    pClone->pUndoOwner = pBrush->owner;
	  Brush_AddToList (pClone, &g_brUndo);
  }
*/
}

bool CXYWnd::UndoAvailable()
{
  return (g_brUndo.next != &g_brUndo);
}



void CXYWnd::Paste()
{
#if 1

  CWaitCursor WaitCursor; 
  bool bPasted = false;
  UINT nClipboard = ::RegisterClipboardFormat("RadiantClippings");
  if (nClipboard > 0 && OpenClipboard() && ::IsClipboardFormatAvailable(nClipboard))
  {
    HANDLE h = ::GetClipboardData(nClipboard);
    if (h)
    {
      g_Clipboard.SetLength(0);
			unsigned char *cp = reinterpret_cast<unsigned char*>(::GlobalLock(h));
			long lSize = 0;
			memcpy(&lSize, cp, sizeof(long));
			cp += sizeof(long);
      g_Clipboard.Write(cp, lSize);
    }
    ::GlobalUnlock(h);
    ::CloseClipboard();
  }

  if (g_Clipboard.GetLength() > 0)
  {
    g_Clipboard.SeekToBegin();
    int nLen = g_Clipboard.GetLength();
    char* pBuffer = new char[nLen+1];
	  memset( pBuffer, 0, sizeof(pBuffer) );
    g_Clipboard.Read(pBuffer, nLen);
    pBuffer[nLen] = '\0';
    Map_ImportBuffer(pBuffer);
    delete []pBuffer;
  }

#if 0
  if (g_PatchClipboard.GetLength() > 0)
  {
    g_PatchClipboard.SeekToBegin();
    int nLen = g_PatchClipboard.GetLength();
    char* pBuffer = new char[nLen+1];
    g_PatchClipboard.Read(pBuffer, nLen);
    pBuffer[nLen] = '\0';
    Patch_ReadBuffer(pBuffer, true);
    delete []pBuffer;
  }
#endif

#else
  if (g_brClipboard.next != &g_brClipboard || g_enClipboard.next != &g_enClipboard)
  {
    Select_Deselect();

	  for (brush_s* pBrush = g_brClipboard.next ; pBrush != NULL && pBrush != &g_brClipboard ; pBrush=pBrush->next)
    {
      brush_s* pClone = Brush_Clone(pBrush);
	    //pClone->owner = pBrush->owner;
      if (pClone->owner == NULL)
			  Entity_LinkBrush (world_entity, pClone);
    	
      Brush_AddToList (pClone, &selected_brushes);
      Brush_Build(pClone);
    }

    for (entity_s* pEntity = g_enClipboard.next; pEntity != NULL && pEntity != &g_enClipboard; pEntity = pEntity->next)
    {
      entity_s* pEClone = Entity_Clone(pEntity);
			for (brush_s* pEB = pEntity->brushes.onext ; pEB != &pEntity->brushes ; pEB=pEB->onext)
			{
        brush_s* pClone = Brush_Clone(pEB);
	      Brush_AddToList (pClone, &selected_brushes);
        Entity_LinkBrush(pEClone, pClone);
        Brush_Build(pClone);
        if (pClone->owner && pClone->owner != world_entity)
        {
			    UpdateEntitySel(pClone->owner->eclass);
        }
			}
    }

    Sys_UpdateWindows(W_ALL);
  }
  else Sys_Printf("Nothing to paste.../n");
#endif
}


vec3_c& CXYWnd::Rotation()
{
  return g_vRotation;
}

vec3_c& CXYWnd::RotateOrigin()
{
  return g_vRotateOrigin;
}


void CXYWnd::OnTimer(UINT nIDEvent) 
{
  int nDim1 = (m_nViewType == YZ) ? 1 : 0;
  int nDim2 = (m_nViewType == XY) ? 1 : 2;
	m_vOrigin[nDim1] += m_ptDragAdj.x / m_fScale;
	m_vOrigin[nDim2] -= m_ptDragAdj.y / m_fScale;
  Sys_UpdateWindows(W_XY | W_CAMERA);
  //int nH = (m_ptDrag.y == 0) ? -1 : m_ptDrag.y;
  m_ptDrag += m_ptDragAdj;
  m_ptDragTotal += m_ptDragAdj;
  XY_MouseMoved (m_ptDrag.x, m_nHeight - 1 - m_ptDrag.y , m_nScrollFlags);
	//m_vOrigin[nDim1] -= m_ptDrag.x / m_fScale;
	//m_vOrigin[nDim1] -= m_ptDrag.x / m_fScale;
}

void CXYWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  g_pParentWnd->HandleKey(nChar, nRepCnt, nFlags, false);
  //CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CXYWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp) 
{
	CWnd::OnNcCalcSize(bCalcValidRects, lpncsp);
}

void CXYWnd::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	SendMessage(WM_NCACTIVATE, FALSE , 0 );
}

void CXYWnd::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	SendMessage(WM_NCACTIVATE, TRUE , 0 );
}

void CXYWnd::OnClose() 
{
	CWnd::OnClose();
}

// should be static as should be the rotate scale stuff
bool CXYWnd::AreaSelectOK()
{
  return RotateMode() ? false : ScaleMode() ? false : true;
}