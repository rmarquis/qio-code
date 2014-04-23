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


#define	MAX_POINTFILE	8192
static edVec3_c	s_pointvecs[MAX_POINTFILE];
static int		s_num_points, s_check_point;

void Pointfile_Delete (void)
{
	char	name[1024];

	strcpy (name, currentmap);
	StripExtension (name);
	strcat (name, ".lin");

	remove(name);
}

// advance camera to next point
void Pointfile_Next (void)
{
	edVec3_c	dir;

	if (s_check_point >= s_num_points-2)
	{
		Sys_Status ("End of pointfile", 0);
		return;
	}
	s_check_point++;
	g_pParentWnd->GetCamera()->Camera().origin = s_pointvecs[s_check_point];
	g_pParentWnd->GetXYWnd()->GetOrigin() = s_pointvecs[s_check_point];
	dir = s_pointvecs[s_check_point+1] - g_pParentWnd->GetCamera()->Camera().origin;
	dir.normalize();
	g_pParentWnd->GetCamera()->Camera().angles[1] = atan2 (dir[1], dir[0])*180/3.14159;
	g_pParentWnd->GetCamera()->Camera().angles[0] = asin (dir[2])*180/3.14159;

	Sys_UpdateWindows (W_ALL);
}

// advance camera to previous point
void Pointfile_Prev (void)
{
	edVec3_c	dir;

	if ( s_check_point == 0)
	{
		Sys_Status ("Start of pointfile", 0);
		return;
	}
	s_check_point--;
	g_pParentWnd->GetCamera()->Camera().origin = s_pointvecs[s_check_point];
	g_pParentWnd->GetXYWnd()->GetOrigin() = s_pointvecs[s_check_point];
	dir = s_pointvecs[s_check_point+1] - g_pParentWnd->GetCamera()->Camera().origin;
	dir.normalize();
	g_pParentWnd->GetCamera()->Camera().angles[1] = atan2 (dir[1], dir[0])*180/3.14159;
	g_pParentWnd->GetCamera()->Camera().angles[0] = asin (dir[2])*180/3.14159;

	Sys_UpdateWindows (W_ALL);
}

void WINAPI Pointfile_Check (void)
{
	char	name[1024];
	FILE	*f;
	vec3_t	v;

	strcpy (name, currentmap);
	StripExtension (name);
	strcat (name, ".lin");

	f = fopen (name, "r");
	if (!f)
		return;

	Sys_Printf ("Reading pointfile %s\n", name);

	if (!g_qeglobals.d_pointfile_display_list)
		g_qeglobals.d_pointfile_display_list = glGenLists(1);

	s_num_points = 0;
  glNewList (g_qeglobals.d_pointfile_display_list,  GL_COMPILE);
	glColor3f (1, 0, 0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glLineWidth (4);
	glBegin(GL_LINE_STRIP);
	do
	{
		if (fscanf (f, "%f %f %f\n", &v[0], &v[1], &v[2]) != 3)
			break;
		if (s_num_points < MAX_POINTFILE)
		{
			s_pointvecs[s_num_points] = v;
			s_num_points++;
		}
		glVertex3fv (v);
	} while (1);
	glEnd();
	glLineWidth (1);
	glEndList ();

	s_check_point = 0;
	fclose (f);
	//Pointfile_Next ();
}

void Pointfile_Draw( void )
{
	int i;

	glColor3f( 1.0F, 0.0F, 0.0F );
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_TEXTURE_1D);
	glLineWidth (4);
	glBegin(GL_LINE_STRIP);
	for ( i = 0; i < s_num_points; i++ )
	{
		glVertex3fv( s_pointvecs[i] );
	}
	glEnd();
	glLineWidth( 1 );
}

void Pointfile_Clear (void)
{
	if (!g_qeglobals.d_pointfile_display_list)
		return;

	glDeleteLists (g_qeglobals.d_pointfile_display_list, 1);
	g_qeglobals.d_pointfile_display_list = 0;
	Sys_UpdateWindows (W_ALL);
}

