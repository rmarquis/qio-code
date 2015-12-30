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
#include "stdafx.h"
#include "qe3.h"

/*

  drag either multiple brushes, or select plane points from
  a single brush.

*/

bool	drag_ok;
vec3_c	drag_xvec;
vec3_c	drag_yvec;

static	int	buttonstate;
int	pressx, pressy;
static	vec3_c pressdelta;
static	vec3_c vPressStart;
static	int	buttonx, buttony;

int		lastx, lasty;

bool	drag_first;





/*
===========
Drag_Setup
===========
*/
void Drag_Setup (int x, int y, int buttons,
		   vec3_t xaxis, vec3_t yaxis,
		   vec3_t origin, vec3_t dir)
{
	trace_t	t;
	face_s	*f;

	drag_first = true;
	
	pressdelta.clear();
	pressx = x;
	pressy = y;

	drag_xvec = xaxis;
	drag_xvec.axializeVector();
	drag_yvec = yaxis;
	drag_yvec.axializeVector();



	extern void SelectCurvePointByRay (const vec3_c &org, vec3_t dir, int buttons);
	if (g_qeglobals.d_select_mode == sel_curvepoint)
	{
		//if ((buttons == MK_LBUTTON))
		//  g_qeglobals.d_num_move_points = 0;

		SelectCurvePointByRay (origin, dir, buttons);	
		
		if (g_qeglobals.d_num_move_points || g_qeglobals.d_select_mode == sel_area)
		{
			drag_ok = true;
		}
    
		Sys_UpdateWindows(W_ALL);

		Undo_Start("drag curve point");
		Undo_AddBrushList(&selected_brushes);

		return;
	}
	else
	{
		g_qeglobals.d_num_move_points = 0;
	}

	if (selected_brushes.next == &selected_brushes)
	{
		//in this case a new brush is created when the dragging
		//takes place in the XYWnd, An useless undo is created
		//when the dragging takes place in the CamWnd
		Undo_Start("create brush");

		Sys_Status("No selection to drag\n", 0);
		return;
	}


	if (g_qeglobals.d_select_mode == sel_vertex)
	{
		SelectVertexByRay (origin, dir);	
		if (g_qeglobals.d_num_move_points)
		{
			drag_ok = true;
			Undo_Start("drag vertex");
			Undo_AddBrushList(&selected_brushes);
			return;
		}
	}

	if (g_qeglobals.d_select_mode == sel_edge)
	{
		SelectEdgeByRay (origin, dir);	
		if (g_qeglobals.d_num_move_points)
		{
			drag_ok = true;
			Undo_Start("drag edge");
			Undo_AddBrushList(&selected_brushes);
			return;
		}
	}


	//
	// check for direct hit first
	//
	t = Test_Ray (origin, dir, true);
	if (t.selected)
	{
		drag_ok = true;

		Undo_Start("drag selection");
		Undo_AddBrushList(&selected_brushes);

		if (buttons == (MK_LBUTTON|MK_CONTROL) )
		{
			Sys_Printf ("Shear dragging face\n");
			Brush_SelectFaceForDragging (t.brush, t.face, true);
		}
		else if (buttons == (MK_LBUTTON|MK_CONTROL|MK_SHIFT) )
		{
			Sys_Printf ("Sticky dragging brush\n");
			for (f=t.brush->brush_faces ; f ; f=f->next)
				Brush_SelectFaceForDragging (t.brush, f, false);
		}
		else
			Sys_Printf ("Dragging entire selection\n");
		
		return;
	}

	if (g_qeglobals.d_select_mode == sel_vertex || g_qeglobals.d_select_mode == sel_edge)
		return;

	//
	// check for side hit
	//
	// multiple brushes selected?
	if (selected_brushes.next && selected_brushes.next->next != &selected_brushes)
	{
		// yes, special handling
		bool bOK = (g_PrefsDlg.m_bALTEdge) ? (static_cast<bool>(::GetAsyncKeyState(VK_MENU))) : true;
		if (bOK)
		{
			for (brush_s* pBrush = selected_brushes.next ; pBrush != &selected_brushes ; pBrush = pBrush->next)
			{
				if (buttons & MK_CONTROL)
					Brush_SideSelect (pBrush, origin, dir, true);
				else
					Brush_SideSelect (pBrush, origin, dir, false);
			}
		}
		else
		{
			Sys_Printf ("press ALT to drag multiple edges\n");
			return;
		}
	}
	else if(selected_brushes.next)
	{
		// single select.. trying to drag fixed entities handle themselves and just move
		if (buttons & MK_CONTROL)
			Brush_SideSelect (selected_brushes.next, origin, dir, true);
		else
			Brush_SideSelect (selected_brushes.next, origin, dir, false);
	}

	Sys_Printf ("Side stretch\n");
	drag_ok = true;

	Undo_Start("side stretch");
	Undo_AddBrushList(&selected_brushes);
}

entity_s *peLink;

void UpdateTarget(vec3_t origin, vec3_t dir)
{
	trace_t	t;
	entity_s *pe;
	int i;
	char sz[128];

	t = Test_Ray (origin, dir, 0);

	if (!t.brush)
		return;

	pe = t.brush->owner;

	if (pe == NULL)
		return;

	// is this the first?
	if (peLink != NULL)
	{

		// Get the target id from out current target
		// if there is no id, make one

		i = pe->getKeyInt("target");
		if (i <= 0)
		{
			i = GetUniqueTargetId(1);
			sprintf(sz, "%d", i);

			pe->setKeyValue( "target", sz);
		}

		// set the target # into our src

		sprintf(sz, "%d", i);
		peLink->setKeyValue("targetname", sz);

		Sys_UpdateWindows(W_ENTITY);

	}

	// promote the target to the src

	peLink = pe;
	
}

/*
===========
Drag_Begin
//++timo test three button mouse and three button emulation here ?
===========
*/
void Drag_Begin (int x, int y, int buttons,
		   vec3_t xaxis, vec3_t yaxis,
		   vec3_t origin, vec3_t dir)
{
	trace_t	t;
	bool altdown;

	drag_ok = false;
	pressdelta.clear();
	vPressStart.clear();
	drag_first = true;
	peLink = NULL;

	altdown = static_cast<bool>(::GetAsyncKeyState(VK_MENU));

	// shift-LBUTTON = select entire brush
	if (buttons == (MK_LBUTTON | MK_SHIFT) && g_qeglobals.d_select_mode != sel_curvepoint)
	{
		int nFlag = altdown ? SF_CYCLE : 0;
		if (dir[0] == 0 || dir[1] == 0 || dir[2] == 0)  // extremely low chance of this happening from camera
			Select_Ray (origin, dir, nFlag | SF_ENTITIES_FIRST);	// hack for XY
		else
			Select_Ray (origin, dir, nFlag);
		return;
	}

	// ctrl-alt-LBUTTON = multiple brush select without selecting whole entities
	if (buttons == (MK_LBUTTON | MK_CONTROL) && altdown && g_qeglobals.d_select_mode != sel_curvepoint)
	{
		if (dir[0] == 0 || dir[1] == 0 || dir[2] == 0)  // extremely low chance of this happening from camera
			Select_Ray (origin, dir, SF_ENTITIES_FIRST);	// hack for XY
		else
			Select_Ray (origin, dir, 0);
		return;
	}

	// ctrl-shift-LBUTTON = select single face
	if (buttons == (MK_LBUTTON | MK_CONTROL | MK_SHIFT) && g_qeglobals.d_select_mode != sel_curvepoint)
	{
		Select_Deselect (!static_cast<bool>(::GetAsyncKeyState(VK_MENU)));
		Select_Ray (origin, dir, SF_SINGLEFACE);
		return;
	}


	// LBUTTON + all other modifiers = manipulate selection
	if (buttons & MK_LBUTTON)
	{
		//
		Drag_Setup (x, y, buttons, xaxis, yaxis, origin, dir);
		return;
	}

	int nMouseButton = g_PrefsDlg.m_nMouseButtons == 2 ? MK_RBUTTON : MK_MBUTTON;
	// middle button = grab texture
	if (buttons == nMouseButton)
	{
		t = Test_Ray (origin, dir, false);
		if (t.face)
		{
			g_qeglobals.d_new_brush_bottom_z = t.brush->getMins()[2];
			g_qeglobals.d_new_brush_top_z = t.brush->getMaxs()[2];
			// use a local brushprimit_texdef fitted to a default 2x2 texture
			brushprimit_texdef_s bp_local;
			ConvertTexMatWithQTexture( &t.face->brushprimit_texdef, t.face->d_texture, &bp_local, NULL );
			Texture_SetTexture ( &t.face->texdef, &bp_local, false);
			UpdateSurfaceDialog();
			UpdatePatchInspector();
		}
		else
			Sys_Printf ("Did not select a texture\n");
		return;
	}

	// ctrl-middle button = set entire brush to texture
	if (buttons == (nMouseButton|MK_CONTROL) )
	{
		t = Test_Ray (origin, dir, false);
		if (t.brush)
		{
			if (t.brush->brush_faces->texdef.getName()[0] == '(')
				Sys_Printf ("Can't change an entity texture\n");
			else
			{
				Brush_SetTexture (t.brush, &g_qeglobals.d_texturewin.texdef, &g_qeglobals.d_texturewin.brushprimit_texdef, false );
				Sys_UpdateWindows (W_ALL);
			}
		}
		else
			Sys_Printf ("Didn't hit a btrush\n");
		return;
	}

	// ctrl-shift-middle button = set single face to texture
	if (buttons == (nMouseButton|MK_SHIFT|MK_CONTROL) )
	{
		t = Test_Ray (origin, dir, false);
		if (t.brush)
		{
			if (t.brush->brush_faces->texdef.getName()[0] == '(')
				Sys_Printf ("Can't change an entity texture\n");
			else
			{
				SetFaceTexdef (t.brush, t.face, &g_qeglobals.d_texturewin.texdef, &g_qeglobals.d_texturewin.brushprimit_texdef);
				Brush_Build( t.brush );
				Sys_UpdateWindows (W_ALL);
			}
		}
		else
			Sys_Printf ("Didn't hit a btrush\n");
		return;
	}

	if (buttons == (nMouseButton | MK_SHIFT))
	{
		Sys_Printf("Set brush face texture info\n");
		t = Test_Ray (origin, dir, false);
		if (t.brush)
		{
			if (t.brush->brush_faces->texdef.getName()[0] == '(')
      {
        if (t.brush->owner->eclass->hasEditorFlagLight())
        {
          CString strBuff;
          mtrAPI_i* pTex = QERApp_TryTextureForName(g_qeglobals.d_texturewin.texdef.getName());
          if (pTex)
          {
            vec3_c vColor = pTex->getEditorColor();

            float fLargest = 0.0f;
            for (int i = 0; i < 3; i++)
            {
		          if (vColor[i] > fLargest)
			          fLargest = vColor[i];
            }
		        
		        if (fLargest == 0.0f)
		        {
              vColor[0] = vColor[1] = vColor[2] = 1.0f;
            }
		        else
		        {
			        float fScale = 1.0f / fLargest;
              for (int i = 0; i < 3; i++)
              {
                vColor[i] *= fScale;
              }
            }
            strBuff.Format("%f %f %f",pTex->getEditorColor()[0], pTex->getEditorColor()[1], pTex->getEditorColor()[2]);
			t.brush->owner->setKeyValue("_color", strBuff.GetBuffer(0));
				    Sys_UpdateWindows (W_ALL);
          }
        }
        else
        {
				  Sys_Printf ("Can't select an entity brush face\n");
        }
      }
			else
			{
      	//strcpy(t.face->texdef.getName(),g_qeglobals.d_texturewin.texdef.getName());
      	t.face->texdef.setName(g_qeglobals.d_texturewin.texdef.getName());
				Brush_Build(t.brush);
				Sys_UpdateWindows (W_ALL);
			}
		}
		else
			Sys_Printf ("Didn't hit a brush\n");
		return;
	}

}


//
//===========
//MoveSelection
//===========
//
void MoveSelection (vec3_t move)
{
	int		i, success;
	brush_s	*b;
	CString strStatus;
	vec3_c vTemp, vTemp2, end;

	if (!move[0] && !move[1] && !move[2])
		return;

	move[0] = (g_nScaleHow & SCALE_X) ? 0 : move[0];
	move[1] = (g_nScaleHow & SCALE_Y) ? 0 : move[1];
  move[2] = (g_nScaleHow & SCALE_Z) ? 0 : move[2];

	if (g_pParentWnd->ActiveXY()->RotateMode() || g_bPatchBendMode)
	{
		float fDeg = -move[2];
		float fAdj = move[2];
		int nAxis = 0;
		if (g_pParentWnd->ActiveXY()->GetViewType() == XY)
		{
			fDeg = -move[1];
			fAdj = move[1];
			nAxis = 2;
		}
		else 
			if (g_pParentWnd->ActiveXY()->GetViewType() == XZ)
		{
			fDeg = move[2];
			fAdj = move[2];
			nAxis = 1;
		}
		else
			nAxis = 0;

		g_pParentWnd->ActiveXY()->Rotation()[nAxis] += fAdj;
		strStatus.Format("%s x:: %.1f  y:: %.1f  z:: %.1f", (g_bPatchBendMode) ? "Bend angle" : "Rotation", g_pParentWnd->ActiveXY()->Rotation()[0], g_pParentWnd->ActiveXY()->Rotation()[1], g_pParentWnd->ActiveXY()->Rotation()[2]);
		g_pParentWnd->SetStatusText(2, strStatus);

		if (g_bPatchBendMode)
		{
			Patch_SelectBendNormal();
			Select_RotateAxis(nAxis, fDeg*2, false, true);
			Patch_SelectBendAxis();
			Select_RotateAxis(nAxis, fDeg, false, true);
		}
		else
		{
			Select_RotateAxis(nAxis, fDeg, false, true);
		}
		return;
	}

	if (g_pParentWnd->ActiveXY()->ScaleMode())
	{
		vec3_t v;
		v[0] = v[1] = v[2] = 1.0;
		if (move[1] > 0)
		{
			v[0] = 1.1;
			v[1] = 1.1;
			v[2] = 1.1;
		}
		else 
			if (move[1] < 0)
		{
			v[0] = 0.9;
			v[1] = 0.9;
			v[2] = 0.9;
		}

			Select_Scale((g_nScaleHow & SCALE_X) ? 1.0 : v[0],
									 (g_nScaleHow & SCALE_Y) ? 1.0 : v[1],
  								 (g_nScaleHow & SCALE_Z) ? 1.0 : v[2]);
		Sys_UpdateWindows (W_ALL);
		return;
	}


	vec3_c vDistance = pressdelta - vPressStart;
	strStatus.Format("Distance x: %.1f  y: %.1f  z: %.1f", vDistance[0], vDistance[1], vDistance[2]);
	g_pParentWnd->SetStatusText(3, strStatus);

	//
	// dragging only a part of the selection
	//


	// this is fairly crappy way to deal with curvepoint and area selection
	// but it touches the smallest amount of code this way
	// 
	if (g_qeglobals.d_num_move_points || g_qeglobals.d_select_mode == sel_area)
	{
		//area selection
		if (g_qeglobals.d_select_mode == sel_area)
		{
			g_qeglobals.d_vAreaBR += move;
			return;
		}
		//curve point selection
		if (g_qeglobals.d_select_mode == sel_curvepoint)
		{
			Patch_UpdateSelected(move);
			return;
		}

		//vertex selection
		if (g_qeglobals.d_select_mode == sel_vertex)
		{
			success = true;
			for (b = selected_brushes.next; b != &selected_brushes; b = b->next)
			{
				success &= Brush_MoveVertex(b, g_qeglobals.d_move_points[0], move, end, true);
			}
			if (success)
				*((vec3_c*)g_qeglobals.d_move_points[0]) = end;
			return;
		}
		//all other selection types
		for (i=0 ; i<g_qeglobals.d_num_move_points ; i++)
			*((vec3_c*)g_qeglobals.d_move_points[i]) += move;
		//VectorScale(move, .5, move);
		//for (i=0 ; i<g_qeglobals.d_num_move_points2 ; i++)
		//	VectorAdd (g_qeglobals.d_move_points2[i], move, g_qeglobals.d_move_points2[i]);
		for (b = selected_brushes.next; b != &selected_brushes; b = b->next)
		{
			vTemp = b->getMaxs() - b->getMins();
			Brush_Build( b );
			for (i=0 ; i<3 ; i++)
				if (b->getMins()[i] > b->getMaxs()[i]
				|| b->getMaxs()[i] - b->getMins()[i] > MAX_BRUSH_SIZE)
					break;	// dragged backwards or fucked up
			if (i != 3)
				break;
			if (b->patchBrush)
			{
				vTemp2 = b->getMaxs();
				vTemp2 = vTemp2 - b->getMins();
				vTemp2 = vTemp2 - vTemp;
				//if (!Patch_DragScale(b->nPatchID, vTemp2, move))
				if (!Patch_DragScale(b->pPatch, vTemp2, move))
				{
					b = NULL;
					break;
				}
			}
		}
		// if any of the brushes were crushed out of existance
		// calcel the entire move
		if (b != &selected_brushes)
		{
			Sys_Printf ("Brush dragged backwards, move canceled\n");
			for (i=0 ; i<g_qeglobals.d_num_move_points ; i++) {
				*((vec3_c*)g_qeglobals.d_move_points[i]) -= move;
			}

			for (b=selected_brushes.next ; b != &selected_brushes ; b=b->next)
				Brush_Build( b );
		}

	}
	else
	{
		// reset face originals from vertex edit mode
		// this is dirty, but unfortunately necessary because Brush_Build
		// can remove windings
		for (b = selected_brushes.next; b != 0 && b != &selected_brushes; b = b->next)
		{
			Brush_ResetFaceOriginals(b);
		}
		//
		// if there are lots of brushes selected, just translate instead
		// of rebuilding the brushes
		//
		if (drag_yvec[2] == 0 && selected_brushes.next != 0 && selected_brushes.next->next != &selected_brushes)
		{
			Select_Move (move);
			//VectorAdd (g_qeglobals.d_select_translate, move, g_qeglobals.d_select_translate);
		}
		else
		{
			Select_Move (move);
		}
	}
}

/*
===========
Drag_MouseMoved
===========
*/
void Drag_MouseMoved (int x, int y, int buttons)
{
	vec3_c	move, delta;
	int		i;

	if (!buttons)
	{
		drag_ok = false;
		return;
	}
	if (!drag_ok)
		return;

	// clear along one axis
	if (buttons & MK_SHIFT)
	{
		drag_first = false;
		if (abs(x-pressx) > abs(y-pressy))
			y = pressy;
		else
			x = pressx;
	}


	for (i=0 ; i<3 ; i++)
	{
		move[i] = drag_xvec[i]*(x - pressx)	+ drag_yvec[i]*(y - pressy);
		if (!g_PrefsDlg.m_bNoClamp)
		{
			  move[i] = floor(move[i]/g_qeglobals.d_gridsize+0.5)*g_qeglobals.d_gridsize;
		}
	}

	delta = move - pressdelta;
	pressdelta = move;

	MoveSelection (delta);
}

/*
===========
Drag_MouseUp
===========
*/
void Drag_MouseUp (int nButtons)
{
	Sys_Status ("drag completed.", 0);

	if (g_qeglobals.d_select_mode == sel_area)
	{
		Patch_SelectAreaPoints();
		g_qeglobals.d_select_mode = sel_curvepoint;
		Sys_UpdateWindows (W_ALL);
	}
	
	if (g_qeglobals.d_select_translate[0] || g_qeglobals.d_select_translate[1] || g_qeglobals.d_select_translate[2])
	{
		Select_Move (g_qeglobals.d_select_translate);
		g_qeglobals.d_select_translate.clear();
		Sys_UpdateWindows (W_CAMERA);
	}
  
	g_pParentWnd->SetStatusText(3, "");

	//
	Undo_EndBrushList(&selected_brushes);
	Undo_End();
}
