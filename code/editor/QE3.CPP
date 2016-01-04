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
#include "PrefsDlg.h"
#include <direct.h>  
#include <sys\stat.h> 
#include <api/vfsAPI.h>
#include <math/math.h>

QEGlobals_t  g_qeglobals;
int g_allocatedCounter_brush;
int g_allocatedCounter_entity;

class allocatedCounterWatcher_c {

public:
	~allocatedCounterWatcher_c() {
		FILE *d = fopen("editorAllocatedCounterDump.txt","wb");
		if(d == 0)
			return;
		fprintf(d,"Unfried faces: %i\n",g_allocatedCounter_face);
		fprintf(d,"Unfried brushes: %i\n",g_allocatedCounter_brush);
		fprintf(d,"Unfried entities: %i\n",g_allocatedCounter_entity);
		fclose(d);
	}
} acw;

void WINAPI QE_CheckOpenGLForErrors()
{
  CString strMsg;
  int i = glGetError();
  if (i != GL_NO_ERROR)
  {
    if (i == GL_OUT_OF_MEMORY)
    {
      //strMsg.Format("OpenGL out of memory error %s\nDo you wish to save before exiting?", gluErrorString((GLenum)i));
      if (MessageBox(g_qeglobals.d_hwndMain, strMsg, "Q3Radiant Error", MB_YESNO) == IDYES)
      {
        Map_SaveFile(NULL, false);
      }
		  exit(1);
    }
    else
    {
      //strMsg.Format("Warning: OpenGL Error %s\n ", gluErrorString((GLenum)i));
		  Sys_Printf (strMsg.GetBuffer(0));
    }
  }
}


bool DoesFileExist(const char* pBuff, long& lSize)
{
  CFile file;
  if (file.Open(pBuff, CFile::modeRead | CFile::shareDenyNone))
  {
    lSize += file.GetLength();
    file.Close();
    return true;
  }
  return false;
}


void Map_Snapshot()
{
  CString strMsg;
  // we need to do the following
  // 1. make sure the snapshot directory exists (create it if it doesn't)
  // 2. find out what the lastest save is based on number
  // 3. inc that and save the map
  CString strOrgPath, strOrgFile;
  ExtractPath_and_Filename(currentmap, strOrgPath, strOrgFile);
  AddSlash(strOrgPath);
  strOrgPath += "snapshots";
  bool bGo = true;
  struct _stat Stat;
  if (_stat(strOrgPath, &Stat) == -1)
  {
    bGo = (_mkdir(strOrgPath) != -1);
  }
  AddSlash(strOrgPath);
  if (bGo)
  {
    int nCount = 0;
    long lSize = 0;
    CString strNewPath = strOrgPath;
    strNewPath += strOrgFile;
    CString strFile;
    while (bGo)
    {
      strFile.Format("%s.%i", strNewPath, nCount);
      bGo = DoesFileExist(strFile, lSize);
      nCount++;
    }
    // strFile has the next available slot
    Map_SaveFile(strFile.GetBuffer(0), false);
		Sys_SetTitle (currentmap);
    if (lSize > 12 * 1024 * 1024) // total size of saves > 4 mb
    {
      Sys_Printf("The snapshot files in the [%s] directory total more than 4 megabytes. You might consider cleaning the directory up.", strOrgPath);
    }
  }
  else
  {
    strMsg.Format("Snapshot save failed.. unabled to create directory\n%s", strOrgPath);
    g_pParentWnd->MessageBox(strMsg);
  }
}
/*
If five minutes have passed since making a change
and the map hasn't been saved, save it out.
*/
void QE_CheckAutoSave( void )
{
	static clock_t s_start;
	clock_t        now;

	now = clock();

	if ( modified != 1 || !s_start)
	{
		s_start = now;
		return;
	}

	if ( now - s_start > ( CLOCKS_PER_SEC * 60 * g_PrefsDlg.m_nAutoSave))
	{

    if (g_PrefsDlg.m_bAutoSave)
    {
      CString strMsg = g_PrefsDlg.m_bSnapShots ? "Autosaving snapshot..." : "Autosaving...";
		  Sys_Printf(strMsg.GetBuffer(0));
      Sys_Printf("\n");
		  Sys_Status (strMsg.GetBuffer(0),0);

      // only snapshot if not working on a default map
      if (g_PrefsDlg.m_bSnapShots && _stricmp(currentmap, "unnamed.map") != 0)
      {
        Map_Snapshot();
      }
      else
      {
		///  Map_SaveFile (g_qeglobals.d_project_entity->getKeyValue("autosave"), false);
		  // TODO - VALID PATH
		  Map_SaveFile ("C:/radiantAutoSave.map", false);
      }

		  Sys_Status ("Autosaving...Saved.", 0 );
		  modified = 2;
    }
    else
    {
		  Sys_Printf ("Autosave skipped...\n");
		  Sys_Status ("Autosave skipped...", 0 );
    }
		s_start = now;
	}
}


int BuildShortPathName(const char* pPath, char* pBuffer, int nBufferLen)
{
  char *pFile = NULL;
  int nResult = GetFullPathName(pPath, nBufferLen, pBuffer, &pFile); 
  nResult = GetShortPathName(pPath, pBuffer, nBufferLen);
  if (nResult == 0)
    strcpy(pBuffer, pPath);                     // Use long filename
  return nResult;
}



float Q_rint (float in)
{
  if (g_PrefsDlg.m_bNoClamp)
    return in;
  else
	  return (float)floor (in + 0.5);
}




#define	SPEED_MOVE	32
#define	SPEED_TURN	22.5


/*
Sets target / targetname on the two entities selected
from the first selected to the secon
*/
void ConnectEntities ()
{
	entity_s	*e1, *e2, *e;
	const char		*target, *tn;
	int			maxtarg, targetnum;
	char		newtarg[32];

	if (g_qeglobals.d_select_count != 2)
	{
		Sys_Status ("Must have two brushes selected.", 0);
		Sys_Beep ();
		return;
	}

	e1 = g_qeglobals.d_select_order[0]->owner;
	e2 = g_qeglobals.d_select_order[1]->owner;

	if (e1 == world_entity || e2 == world_entity)
	{
		Sys_Status ("Can't connect to the world.", 0);
		Sys_Beep ();
		return;
	}

	if (e1 == e2)
	{
		Sys_Status ("Brushes are from same entity.", 0);
		Sys_Beep ();
		return;
	}

	target = e1->getKeyValue("target");
	if (target && target[0])
		strcpy (newtarg, target);
	else
	{
		target = e2->getKeyValue("targetname");
		if (target && target[0])
			strcpy (newtarg, target);
		else
		{
			// make a unique target value
			maxtarg = 0;
			for (e=entities.next ; e != &entities ; e=e->next)
			{
				tn = e->getKeyValue("targetname");
				if (tn && tn[0])
				{
					targetnum = atoi(tn+1);
					if (targetnum > maxtarg)
						maxtarg = targetnum;
				}
			}
			sprintf (newtarg, "t%i", maxtarg+1);
		}
	}

	e1->setKeyValue("target", newtarg);
	e2->setKeyValue("targetname", newtarg);
	Sys_UpdateWindows (W_XY | W_CAMERA);

	Select_Deselect();
	Select_Brush (g_qeglobals.d_select_order[1]);
}

bool QE_SingleBrush (bool bQuiet)
{
	if ( (selected_brushes.next == &selected_brushes)
		|| (selected_brushes.next->next != &selected_brushes) )
	{
		if (!bQuiet)
		{
			Sys_Printf ("Error: you must have a single brush selected\n");
		}
		return false;
	}
	if (selected_brushes.next->owner->getEntityClass()->isFixedSize())
	{
		if (!bQuiet)
		{
			Sys_Printf ("Error: you cannot manipulate fixed size entities\n");
		}
		return false;
	}

	return true;
}



void WINAPI QE_ConvertDOSToUnixName( char *dst, const char *src )
{
	while ( *src )
	{
		if ( *src == '\\' )
			*dst = '/';
		else
			*dst = *src;
		dst++; src++;
	}
	*dst = 0;
}

int g_numbrushes, g_numentities;

void QE_CountBrushesAndUpdateStatusBar( void )
{
	static int      s_lastbrushcount, s_lastentitycount;
	static bool s_didonce;
	
	//entity_s   *e;
	brush_s	   *b, *next;

	g_numbrushes = 0;
	g_numentities = 0;
	
	if ( active_brushes.next != NULL )
	{
		for ( b = active_brushes.next ; b != NULL && b != &active_brushes ; b=next)
		{
			next = b->next;
			if (b->getFirstFace() )
			{
				if ( !b->owner->getEntityClass()->isFixedSize())
					g_numbrushes++;
				else
					g_numentities++;
			}
		}
	}

	if ( ( ( g_numbrushes != s_lastbrushcount ) || ( g_numentities != s_lastentitycount ) ) || ( !s_didonce ) )
	{
		Sys_UpdateStatusBar();

		s_lastbrushcount = g_numbrushes;
		s_lastentitycount = g_numentities;
		s_didonce = true;
	}
}

double I_FloatTime ()
{
	time_t	t;
	
	time (&t);
	
	return t;
}

