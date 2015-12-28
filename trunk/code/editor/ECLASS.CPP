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
#include "io.h"
#include <api/vfsAPI.h>
//#include "qertypes.h"

eclass_s	*eclass = NULL;
eclass_s	*eclass_bad = NULL;
char		eclass_directory[1024];


/*

the classname, color triple, and bounding box are parsed out of comments
A ? size means take the exact brush size.

/*QUAKED <classname> (0 0 0) ?
/*QUAKED <classname> (0 0 0) (-8 -8 -8) (8 8 8)

Flag names can follow the size description:

/*QUAKED func_door (0 .5 .8) ? START_OPEN STONE_SOUND DOOR_DONT_LINK GOLD_KEY SILVER_KEY

*/

void CleanEntityList(eclass_s *&pList)
{
  while (pList)
  {
    eclass_s* pTemp = pList->next;
    
    if (pList->modelpath)
      free(pList->modelpath);
	  if (pList->skinpath)			// PGM
		  free(pList->skinpath);		// PGM
    
    free(pList->name);
    free(pList->comments);
    free(pList);
    pList = pTemp;
  }

  pList = NULL;

}


void CleanUpEntities()
{
  CleanEntityList(eclass);
  if (eclass_bad)
  {
    free(eclass_bad->name);
    free(eclass_bad->comments);
    free(eclass_bad);
    eclass_bad = NULL;
  }
}

void setSpecialLoad(eclass_s *e, const char* pWhat, char*& p)
{
  CString str = e->comments;
  int n = str.Find(pWhat);
  if (n >= 0)
  {
    char* pText = e->comments + n + strlen(pWhat);
    if (*pText == '\"')
      pText++;

    str = "";
    while (*pText != '\"' && *pText != '\0')
    {
      str += *pText;
      pText++;
    }
    if (str.GetLength() > 0)
    {         
      p = _strdup(str);
      //--LoadModel(str, e);
    }
  }
}

char	*debugname;

eclass_s *Eclass_InitFromText (const char *text)
{
	const char	*t;
	int		len;
	int		r, i;
	char	parms[256], *p;
	eclass_s	*e;
	char	color[128];

	e = (eclass_s*)qmalloc(sizeof(*e));
	memset (e, 0, sizeof(*e));
	
	text += strlen("/*QUAKED ");
	
// grab the name
	text = COM_Parse ((char*)text);
	e->name = (char*)qmalloc (strlen(com_token)+1);
	strcpy (e->name, com_token);
	debugname = e->name;
	
// grab the color, reformat as texture name
	r = sscanf (text," (%f %f %f)", &e->color[0], &e->color[1], &e->color[2]);
	if (r != 3)
		return e;
	sprintf (color, "(%f %f %f)", e->color[0], e->color[1], e->color[2]);
	//strcpy (e->texdef.getName(), color);
	e->texdef.setName(color);

	while (*text != ')')
	{
		if (!*text)
			return e;
		text++;
	}
	text++;
	
// get the size	
	text = COM_Parse ((char*)text);
	if (com_token[0] == '(')
	{	// parse the size as two vectors
		e->fixedsize = true;
		r = sscanf (text,"%f %f %f) (%f %f %f)", &e->mins[0], &e->mins[1], &e->mins[2],
			&e->maxs[0], &e->maxs[1], &e->maxs[2]);
		if (r != 6)
			return e;

		for (i=0 ; i<2 ; i++)
		{
			while (*text != ')')
			{
				if (!*text)
					return e;
				text++;
			}
			text++;
		}
	}
	else
	{	// use the brushes
	}
	
// get the flags
	

// copy to the first /n
	p = parms;
	while (*text && *text != '\n')
		*p++ = *text++;
	*p = 0;
	text++;
	
// any remaining words are parm flags
	p = parms;
	for (i=0 ; i<8 ; i++)
	{
		p = COM_Parse (p);
		if (!p)
			break;
		strcpy (e->flagnames[i], com_token);
	} 

// find the length until close comment
	for (t=text ; t[0] && !(t[0]=='*' && t[1]=='/') ; t++)
	;
	
// copy the comment block out
	len = t-text;
	e->comments = (char*)qmalloc (len+1);
	memcpy (e->comments, text, len);
#if 0
	for (i=0 ; i<len ; i++)
		if (text[i] == '\n')
			e->comments[i] = '\r';
		else
			e->comments[i] = text[i];
#endif
	e->comments[len] = 0;
	
  setSpecialLoad(e, "model=", e->modelpath);
  setSpecialLoad(e, "skin=", e->skinpath);
  char *pFrame = NULL;
  setSpecialLoad(e, "frame=", pFrame);
  if (pFrame != NULL)
  {
    e->nFrame = atoi(pFrame);
  }

  if(!e->skinpath)
	  setSpecialLoad(e, "texture=", e->skinpath);

  // setup show flags
  e->nShowFlags = 0;
  if (_strcmpi(e->name, "light") == 0 || _strcmpi(e->name, "light_static") == 0 || _strcmpi(e->name, "light_dynamic") == 0)
  {
    e->nShowFlags |= ECLASS_LIGHT;
  }

  if (  (_strnicmp(e->name, "info_player", strlen("info_player")) == 0)
      ||(_strnicmp(e->name, "path_corner", strlen("path_corner")) == 0) 
      ||(_strnicmp(e->name, "team_ctf", strlen("team_ctf")) == 0) )
  {
    e->nShowFlags |= ECLASS_ANGLE;
  }
  if (_strcmpi(e->name, "path") == 0)
  {
    e->nShowFlags |= ECLASS_PATH;
  }
  if (_strcmpi(e->name, "misc_model") == 0)
  {
    e->nShowFlags |= ECLASS_MISCMODEL;
  }


  return e;
}
void Eclass_InsertAlphabetized (eclass_s *e);
void EClass_CreateNewFromText(const char *text) {
	eclass_s *cl = Eclass_InitFromText(text);
	if(cl) {
		Eclass_InsertAlphabetized(cl);
	}
}


void EClass_InsertSortedList(eclass_s *&pList, eclass_s *e)
{
	eclass_s	*s;
	
	if (!pList)
	{
		pList = e;
		return;
	}


	s = pList;
	if (_stricmp (e->name, s->name) < 0)
	{
		e->next = s;
		pList = e;
		return;
	}

	do
	{
		if (!s->next || _stricmp (e->name, s->next->name) < 0)
		{
			e->next = s->next;
			s->next = e;
			return;
		}
		s=s->next;
	} while (1);
}

/*
=================
Eclass_InsertAlphabetized
=================
*/
void Eclass_InsertAlphabetized (eclass_s *e)
{
  EClass_InsertSortedList(eclass, e);
}


/*
=================
Eclass_ScanFile
=================
*/

bool parsing_single = false;
bool eclass_found;
eclass_s *eclass_e;
//#ifdef BUILD_LIST
extern bool g_bBuildList;
CString strDefFile;
//#endif
void Eclass_ScanFile (char *filename)
{
	int		size;
	char	*data;
	eclass_s	*e;
	int		i;
	char    temp[1024];
	
	QE_ConvertDOSToUnixName( temp, filename );
	
	Sys_Printf ("ScanFile: %s\n", temp);
	
	// BUG
	size = g_vfs->FS_ReadFile (filename, (void**)&data);
	eclass_found = false;
	for (i=0 ; i<size ; i++)
		if (!strncmp(data+i, "/*QUAKED",8))
		{
			
			//#ifdef BUILD_LIST
			if (g_bBuildList)
			{
				CString strDef = "";
				int j = i;
				while (1)
				{
					strDef += *(data+j);
					if (*(data+j) == '/' && *(data+j-1) == '*')
						break;
					j++;
				}
				strDef += "\r\n\r\n\r\n";
				strDefFile += strDef;
			}
			//#endif
			e = Eclass_InitFromText (data+i);
			if (e)
				Eclass_InsertAlphabetized (e);
			else
				printf ("Error parsing: %s in %s\n",debugname, filename);

			// single ?
			eclass_e = e;
			eclass_found = true;
			if ( parsing_single )
				break;
		}
		
		g_vfs->FS_FreeFile (data);		
}



void Eclass_InitForSourceDirectory (const char *path)
{
	struct _finddata_t fileinfo;
	int		handle;
	char	filename[1024];
	char	filebase[1024];
	char    temp[1024];
	char	*s;

	QE_ConvertDOSToUnixName( temp, path );

	Sys_Printf ("Eclass_InitForSourceDirectory: %s\n", temp );

	strcpy (filebase, path);
	s = filebase + strlen(filebase)-1;
	while (*s != '\\' && *s != '/' && s!=filebase)
		s--;
	*s = 0;

  CleanUpEntities();
	eclass = NULL;
//#ifdef BUILD_LIST
  if (g_bBuildList)
    strDefFile = "";
//#endif
	handle = _findfirst (path, &fileinfo);
	if (handle != -1)
	{
		do
		{
			sprintf (filename, "%s\\%s", filebase, fileinfo.name);
			Eclass_ScanFile (filename);
		} while (_findnext( handle, &fileinfo ) != -1);

		_findclose (handle);
	}

//#ifdef BUILD_LIST
  if (g_bBuildList)
  {
    CFile file;
    if (file.Open("c:\\entities.def", CFile::modeCreate | CFile::modeWrite)) 
    {
      file.Write(strDefFile.GetBuffer(0), strDefFile.GetLength());
      file.Close();
    }
  }
//#endif

	eclass_bad = Eclass_InitFromText ("/*QUAKED UNKNOWN_CLASS (0 0.5 0) ?");
}

eclass_s *Eclass_FindExisting (const char *name) {
	eclass_s	*e;

	if (!name)
		return 0;

	for (e=eclass ; e ; e=e->next)
		if (!strcmp (name, e->name))
			return e;

	return 0;
}
eclass_s *Eclass_ForName (const char *name, bool has_brushes)
{
	eclass_s	*e;
	char		init[1024];

#ifdef _DEBUG
  // grouping stuff, not an eclass
  if (strcmp(name, "group_info")==0)
    Sys_Printf("WARNING: unexpected group_info entity in Eclass_ForName\n");
#endif

	if (!name)
		return eclass_bad;

	for (e=eclass ; e ; e=e->next)
		if (!strcmp (name, e->name))
			return e;

	// create a new class for it
	if (has_brushes)
	{
		sprintf (init, "/*QUAKED %s (0 0.5 0) ?\nNot found in source.\n", name);
		e = Eclass_InitFromText (init);
	}
	else
	{
		sprintf (init, "/*QUAKED %s (0 0.5 0) (-8 -8 -8) (8 8 8)\nNot found in source.\n", name);
		e = Eclass_InitFromText (init);
	}

	Eclass_InsertAlphabetized (e);

	return e;
}

