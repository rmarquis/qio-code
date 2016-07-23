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
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// common.c -- misc functions used in client and server

#include "qcommon.h"
#include <setjmp.h>
#ifndef _WIN32
#include <netinet/in.h>
#include <sys/stat.h> // umask
#else
#include <winsock.h>
#include "sys/sys_win32.h"
#endif
#include <api/iFaceMgrAPI.h>
#include <api/moduleManagerAPI.h>
#include <api/rAPI.h>
#include <api/coreAPI.h>
#include <api/declManagerAPI.h>
#include <api/materialSystemAPI.h>
#include <api/modelLoaderDLLAPI.h>
#include <api/editorAPI.h>
#include <api/tikiAPI.h>
#include <shared/colorTable.h>
#include <shared/str.h>
#include <shared/infoString.h>
#include <math/math.h>

#pragma comment(lib,"winmm.lib") 
#pragma comment(lib,"opengl32.lib") 

int demo_protocols[] =
{ 67, 66, 0 };

#define MAX_NUM_ARGVS	50

int		com_argc;
char	*com_argv[MAX_NUM_ARGVS+1];

jmp_buf abortframe;		// an ERR_DROP occured, exit the entire frame

class tmpFix_c {
public:
	tmpFix_c() {
		memset(abortframe,0,sizeof(abortframe));
	}
} fix;

FILE *debuglogfile;
static fileHandle_t pipefile;
static fileHandle_t logfile;
fileHandle_t	com_journalFile;			// events are written here
fileHandle_t	com_journalDataFile;		// config files are written here

cvar_s	*com_viewlog;
cvar_s	*com_speeds;
cvar_s	*com_developer;
cvar_s	*com_dedicated;
cvar_s	*com_timescale;
cvar_s	*com_fixedtime;
cvar_s	*com_journal;
cvar_s	*com_maxfps;
cvar_s	*com_altivec;
cvar_s	*com_timedemo;
cvar_s	*com_sv_running;
cvar_s	*com_cl_running;
cvar_s	*com_logfile;		// 1 = buffer log, 2 = flush after each print
cvar_s	*com_pipefile;
cvar_s	*com_showtrace;
cvar_s	*com_version;
cvar_s	*com_blood;
cvar_s	*com_introPlayed;
cvar_s	*cl_paused;
cvar_s	*sv_paused;
cvar_s  *cl_packetdelay;
cvar_s  *sv_packetdelay;
cvar_s	*com_cameraMode;
cvar_s	*com_ansiColor;
cvar_s	*com_unfocused;
cvar_s	*com_maxfpsUnfocused;
cvar_s	*com_minimized;
cvar_s	*com_maxfpsMinimized;
cvar_s	*com_abnormalExit;
cvar_s	*com_standalone;
cvar_s	*com_gamename;
cvar_s	*com_protocol;
cvar_s	*com_basegame;
cvar_s  *com_homepath;
cvar_s	*com_busyWait;

#if idx64
	int (*Q_VMftol)(void);
#elif id386
	long (QDECL *Q_ftol)(float f);
	int (QDECL *Q_VMftol)(void);
	void (QDECL *Q_SnapVector)(vec3_t vec);
#endif

// com_speeds times
int		time_game;
int		time_frontend;		// renderer frontend time
int		time_backend;		// renderer backend time

int			com_frameTime;
int			com_frameNumber;

bool	com_errorEntered = false;
bool	com_fullyInitialized = false;
bool	com_gameRestarting = false;
// V: -editor starts engine in editor mode
bool	com_bEditorMode = false;

char	com_errorMessage[MAXPRINTMSG];

// this is NULL if client module is not running
class materialSystemAPI_i *g_ms = 0;
class modelLoaderDLLAPI_i *g_modelLoader = 0;

void Com_WriteConfig_f();
void CIN_CloseAllVideos();

//============================================================================

static char	*rd_buffer;
static int	rd_buffersize;
static void	(*rd_flush)( char *buffer );

void Com_BeginRedirect (char *buffer, int buffersize, void (*flush)( char *) )
{
	if (!buffer || !buffersize || !flush)
		return;
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Com_EndRedirect (void)
{
	if ( rd_flush ) {
		rd_flush(rd_buffer);
	}

	rd_buffer = NULL;
	rd_buffersize = 0;
	rd_flush = NULL;
}

/*
=============
Com_Printf

Both client and server can use this, and it will output
to the apropriate place.

A raw string should NEVER be passed as fmt, because of "%f" type crashers.
=============
*/
void QDECL Com_Printf( const char *fmt, ... ) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	static bool opening_qconsole = false;

	va_start (argptr,fmt);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	if ( rd_buffer ) {
		if ((strlen (msg) + strlen(rd_buffer)) > (rd_buffersize - 1)) {
			rd_flush(rd_buffer);
			*rd_buffer = 0;
		}
		Q_strcat(rd_buffer, rd_buffersize, msg);
    // TTimo nooo .. that would defeat the purpose
		//rd_flush(rd_buffer);			
		//*rd_buffer = 0;
		return;
	}

#ifndef DEDICATED
	CL_ConsolePrint( msg );
#endif

	// echo to dedicated console and early console
	Sys_Print( msg );

	// logfile
	if ( com_logfile && com_logfile->integer ) {
    // TTimo: only open the qconsole.log if the filesystem is in an initialized state
    //   also, avoid recursing in the qconsole.log opening (i.e. if fs_debug is on)
		if ( !logfile && FS_Initialized() && !opening_qconsole) {
			struct tm *newtime;
			time_t aclock;

			opening_qconsole = true;

			time( &aclock );
			newtime = localtime( &aclock );

			logfile = FS_FOpenFileWrite( "qconsole.log" );
			
			if(logfile)
			{
				Com_Printf( "logfile opened on %s\n", asctime( newtime ) );
			
				if ( com_logfile->integer > 1 )
				{
					// force it to not buffer so we get valid
					// data even if we are crashing
					FS_ForceFlush(logfile);
				}
			}
			else
			{
				Com_Printf("Opening qconsole.log failed!\n");
				Cvar_SetValue("logfile", 0);
			}

			opening_qconsole = false;
		}

		if ( logfile && FS_Initialized()) {
			FS_Write(msg, strlen(msg), logfile);
		}
	}
}


void QDECL Com_RedWarning( const char *fmt, ... ) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);
	Com_Printf("%s%s",S_COLOR_RED,msg);
}

/*
================
Com_DPrintf

A Com_Printf that only shows up if the "developer" cvar is set
================
*/
void QDECL Com_DPrintf( const char *fmt, ...) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];
		
	if ( !com_developer || !com_developer->integer ) {
		return;			// don't confuse non-developers with techie stuff...
	}

	va_start (argptr,fmt);	
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);
	
	Com_Printf ("%s", msg);
}

/*
=============
Com_Error

Both client and server can use this, and it will
do the appropriate thing.
=============
*/
void QDECL Com_Error( int code, const char *fmt, ... ) {
	va_list		argptr;
	static int	lastErrorTime;
	static int	errorCount;
	int			currentTime;

	if(com_errorEntered)
		Sys_Error("recursive error after: %s", com_errorMessage);

	com_errorEntered = true;

	Cvar_Set("com_errorCode", va("%i", code));

	// if we are getting a solid stream of ERR_DROP, do an ERR_FATAL
	currentTime = Sys_Milliseconds();
	if ( currentTime - lastErrorTime < 100 ) {
		if ( ++errorCount > 3 ) {
			code = ERR_FATAL;
		}
	} else {
		errorCount = 0;
	}
	lastErrorTime = currentTime;

	va_start (argptr,fmt);
	Q_vsnprintf (com_errorMessage, sizeof(com_errorMessage),fmt,argptr);
	va_end (argptr);

	if (code != ERR_DISCONNECT)
		Cvar_Set("com_errorMessage", com_errorMessage);

	if (code == ERR_DISCONNECT || code == ERR_SERVERDISCONNECT) {
//		VM_Forced_Unload_Start();
		SV_Shutdown( "Server disconnected" );
		CL_Disconnect( true );
		CL_FlushMemory( );
//		VM_Forced_Unload_Done();
		// make sure we can get at our local stuff
		FS_PureServerSetLoadedPaks("", "");
		com_errorEntered = false;
		longjmp (abortframe, -1);
	} else if (code == ERR_DROP) {
		Com_Printf ("********************\nERROR: %s\n********************\n", com_errorMessage);
//		VM_Forced_Unload_Start();
		SV_Shutdown (va("Server crashed: %s",  com_errorMessage));
		CL_Disconnect( true );
		CL_FlushMemory( );
//		VM_Forced_Unload_Done();
		FS_PureServerSetLoadedPaks("", "");
		com_errorEntered = false;
		longjmp (abortframe, -1);
	} else {
//		VM_Forced_Unload_Start();
		CL_Shutdown(va("Client fatal crashed: %s", com_errorMessage), true, true);
		SV_Shutdown(va("Server fatal crashed: %s", com_errorMessage));
//		VM_Forced_Unload_Done();
	}

	Com_Shutdown ();

	Sys_Error ("%s", com_errorMessage);
}

void QDECL Com_DropError(const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	Com_Error( ERR_DROP, text );
}

/*
=============
Com_Quit_f

Both client and server can use this, and it will
do the apropriate things.
=============
*/
void Com_Quit_f() {
	// don't try to shutdown if we are in a recursive error
	char *p = Cmd_Args( );
	if ( !com_errorEntered ) {
		// Some VMs might execute "quit" command directly,
		// which would trigger an unload of active VM error.
		// Sys_Quit will kill this process anyways, so
		// a corrupt call stack makes no difference
//		VM_Forced_Unload_Start();
		SV_Shutdown(p[0] ? p : "Server quit");
		CL_Shutdown(p[0] ? p : "Client quit", true, true);
//		VM_Forced_Unload_Done();
		Com_Shutdown ();
		FS_Shutdown(true);
	}
	Sys_Quit ();
}



/*
============================================================================

COMMAND LINE FUNCTIONS

+ characters seperate the commandLine string into multiple console
command lines.

All of these are valid:

quake3 +set test blah +map test
quake3 set test blah+map test
quake3 set test blah + map test

============================================================================
*/

#define	MAX_CONSOLE_LINES	32
int		com_numConsoleLines;
char	*com_consoleLines[MAX_CONSOLE_LINES];

/*
==================
Com_ParseCommandLine

Break it up into multiple console lines
==================
*/
void Com_ParseCommandLine( char *commandLine ) {
    int inq = 0;
    com_consoleLines[0] = commandLine;
    com_numConsoleLines = 1;

    while ( *commandLine ) {
        if (*commandLine == '"') {
            inq = !inq;
        }
        // look for a + seperating character
        // if commandLine came from a file, we might have real line seperators
        if ( (*commandLine == '+' && !inq) || *commandLine == '\n'  || *commandLine == '\r' ) {
            if ( com_numConsoleLines == MAX_CONSOLE_LINES ) {
                return;
            }
            com_consoleLines[com_numConsoleLines] = commandLine + 1;
            com_numConsoleLines++;
            *commandLine = 0;
        }
        commandLine++;
    }
}


/*
===================
Com_SafeMode

Check for "safe" on the command line, which will
skip loading of q3config.cfg
===================
*/
bool Com_SafeMode() {
	int		i;

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( !stricmp( Cmd_Argv(0), "safe" )
			|| !stricmp( Cmd_Argv(0), "cvar_restart" ) ) {
			com_consoleLines[i][0] = 0;
			return true;
		}
	}
	return false;
}


/*
===============
Com_StartupVariable

Searches for command line parameters that are set commands.
If match is not NULL, only that cvar will be looked for.
That is necessary because cddir and basedir need to be set
before the filesystem is started, but all other sets should
be after execing the config and default.
===============
*/
void Com_StartupVariable( const char *match ) {
	int		i;
	const char	*s;

	for (i=0 ; i < com_numConsoleLines ; i++) {
		Cmd_TokenizeString( com_consoleLines[i] );
		if ( strcmp( Cmd_Argv(0), "set" ) ) {
			continue;
		}

		s = Cmd_Argv(1);
		
		if(!match || !strcmp(s, match))
		{
			if(Cvar_Flags(s) == CVAR_NONEXISTENT)
				Cvar_Get(s, Cmd_Argv(2), CVAR_USER_CREATED);
			else
				Cvar_Set2(s, Cmd_Argv(2), false);
		}
	}
}


/*
=================
Com_AddStartupCommands

Adds command line parameters as script statements
Commands are seperated by + signs

Returns true if any late commands were added, which
will keep the demoloop from immediately starting
=================
*/
bool Com_AddStartupCommands() {
	int		i;
	bool	added;

	added = false;
	// quote every token, so args with semicolons can work
	for (i=0 ; i < com_numConsoleLines ; i++) {
		if ( !com_consoleLines[i] || !com_consoleLines[i][0] ) {
			continue;
		}

		// set commands already added with Com_StartupVariable
		if ( !_strnicmp( com_consoleLines[i], "set", 3 ) ) {
			continue;
		}

		added = true;
		Cbuf_AddText( com_consoleLines[i] );
		Cbuf_AddText( "\n" );
	}

	return added;
}


//============================================================================

void Info_Print( const char *s ) {
	char	key[MAX_INFO_STRING];
	char	value[MAX_INFO_STRING];
	char	*o;
	int		l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20)
		{
			memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		Com_Printf ("%s ", key);

		if (!*s)
		{
			Com_Printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		Com_Printf ("%s\n", value);
	}
}

/*
============
Com_StringContains
============
*/
char *Com_StringContains(char *str1, char *str2, int casesensitive) {
	int len, i, j;

	len = strlen(str1) - strlen(str2);
	for (i = 0; i <= len; i++, str1++) {
		for (j = 0; str2[j]; j++) {
			if (casesensitive) {
				if (str1[j] != str2[j]) {
					break;
				}
			}
			else {
				if (toupper(str1[j]) != toupper(str2[j])) {
					break;
				}
			}
		}
		if (!str2[j]) {
			return str1;
		}
	}
	return NULL;
}

/*
============
Com_Filter
============
*/
int Com_Filter(const char *filter, char *name, int casesensitive)
{
	char buf[MAX_STRING_CHARS];
	char *ptr;
	int i, found;

	while(*filter) {
		if (*filter == '*') {
			filter++;
			for (i = 0; *filter; i++) {
				if (*filter == '*' || *filter == '?') break;
				buf[i] = *filter;
				filter++;
			}
			buf[i] = '\0';
			if (strlen(buf)) {
				ptr = Com_StringContains(name, buf, casesensitive);
				if (!ptr) return false;
				name = ptr + strlen(buf);
			}
		}
		else if (*filter == '?') {
			filter++;
			name++;
		}
		else if (*filter == '[' && *(filter+1) == '[') {
			filter++;
		}
		else if (*filter == '[') {
			filter++;
			found = false;
			while(*filter && !found) {
				if (*filter == ']' && *(filter+1) != ']') break;
				if (*(filter+1) == '-' && *(filter+2) && (*(filter+2) != ']' || *(filter+3) == ']')) {
					if (casesensitive) {
						if (*name >= *filter && *name <= *(filter+2)) found = true;
					}
					else {
						if (toupper(*name) >= toupper(*filter) &&
							toupper(*name) <= toupper(*(filter+2))) found = true;
					}
					filter += 3;
				}
				else {
					if (casesensitive) {
						if (*filter == *name) found = true;
					}
					else {
						if (toupper(*filter) == toupper(*name)) found = true;
					}
					filter++;
				}
			}
			if (!found) return false;
			while(*filter) {
				if (*filter == ']' && *(filter+1) != ']') break;
				filter++;
			}
			filter++;
			name++;
		}
		else {
			if (casesensitive) {
				if (*filter != *name) return false;
			}
			else {
				if (toupper(*filter) != toupper(*name)) return false;
			}
			filter++;
			name++;
		}
	}
	return true;
}

/*
============
Com_FilterPath
============
*/
int Com_FilterPath(const char *filter, char *name, int casesensitive)
{
	int i;
	char new_filter[MAX_QPATH];
	char new_name[MAX_QPATH];

	for (i = 0; i < MAX_QPATH-1 && filter[i]; i++) {
		if ( filter[i] == '\\' || filter[i] == ':' ) {
			new_filter[i] = '/';
		}
		else {
			new_filter[i] = filter[i];
		}
	}
	new_filter[i] = '\0';
	for (i = 0; i < MAX_QPATH-1 && name[i]; i++) {
		if ( name[i] == '\\' || name[i] == ':' ) {
			new_name[i] = '/';
		}
		else {
			new_name[i] = name[i];
		}
	}
	new_name[i] = '\0';
	return Com_Filter(new_filter, new_name, casesensitive);
}

/*
================
Com_RealTime
================
*/
int Com_RealTime(qtime_s *qtime) {
	time_t t;
	struct tm *tms;

	t = time(NULL);
	if (!qtime)
		return t;
	tms = localtime(&t);
	if (tms) {
		qtime->tm_sec = tms->tm_sec;
		qtime->tm_min = tms->tm_min;
		qtime->tm_hour = tms->tm_hour;
		qtime->tm_mday = tms->tm_mday;
		qtime->tm_mon = tms->tm_mon;
		qtime->tm_year = tms->tm_year;
		qtime->tm_wday = tms->tm_wday;
		qtime->tm_yday = tms->tm_yday;
		qtime->tm_isdst = tms->tm_isdst;
	}
	return t;
}

void CL_ShutdownCGame();
void CL_ShutdownGUI();
void SV_ShutdownGameProgs();

/*
=================
Hunk_Clear

The server calls this before shutting down or loading a new map
=================
*/
void Hunk_Clear() {

#ifndef DEDICATED
	CL_ShutdownCGame();
	CL_ShutdownGUI();
#endif
	SV_ShutdownGameProgs();
#ifndef DEDICATED
	CIN_CloseAllVideos();
#endif
}

/*
===================================================================

EVENTS AND JOURNALING

In addition to these events, .cfg files are also copied to the
journaled file
===================================================================
*/

#define	MAX_PUSHED_EVENTS	            1024
static int com_pushedEventsHead = 0;
static int com_pushedEventsTail = 0;
static sysEvent_s	com_pushedEvents[MAX_PUSHED_EVENTS];

/*
=================
Com_InitJournaling
=================
*/
void Com_InitJournaling() {
	Com_StartupVariable( "journal" );
	com_journal = Cvar_Get ("journal", "0", CVAR_INIT);
	if ( !com_journal->integer ) {
		return;
	}

	if ( com_journal->integer == 1 ) {
		Com_Printf( "Journaling events\n");
		com_journalFile = FS_FOpenFileWrite( "journal.dat" );
		com_journalDataFile = FS_FOpenFileWrite( "journaldata.dat" );
	} else if ( com_journal->integer == 2 ) {
		Com_Printf( "Replaying journaled events\n");
		FS_FOpenFileRead( "journal.dat", &com_journalFile, true );
		FS_FOpenFileRead( "journaldata.dat", &com_journalDataFile, true );
	}

	if ( !com_journalFile || !com_journalDataFile ) {
		Cvar_Set( "com_journal", "0" );
		com_journalFile = 0;
		com_journalDataFile = 0;
		Com_Printf( "Couldn't open journal files\n" );
	}
}

/*
========================================================================

EVENT LOOP

========================================================================
*/

#define MAX_QUEUED_EVENTS  256
#define MASK_QUEUED_EVENTS ( MAX_QUEUED_EVENTS - 1 )

static sysEvent_s  eventQueue[ MAX_QUEUED_EVENTS ];
static int         eventHead = 0;
static int         eventTail = 0;

/*
================
Com_QueueEvent

A time of 0 will get the current time
Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Com_QueueEvent( int time, sysEventType_e type, int value, int value2, int ptrLength, void *ptr )
{
	sysEvent_s  *ev;

	ev = &eventQueue[ eventHead & MASK_QUEUED_EVENTS ];

	if ( eventHead - eventTail >= MAX_QUEUED_EVENTS )
	{
		Com_Printf("Com_QueueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if ( ev->evPtr )
		{
			free( ev->evPtr );
		}
		eventTail++;
	}

	eventHead++;

	if ( time == 0 )
	{
		time = Sys_Milliseconds();
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

/*
================
Com_GetSystemEvent

================
*/
sysEvent_s Com_GetSystemEvent()
{
#if defined(_WIN32)
	MSG         msg;
#endif
	sysEvent_s  ev;
	char        *s;

	// return if we have data
	if ( eventHead > eventTail )
	{
		eventTail++;
		return eventQueue[ ( eventTail - 1 ) & MASK_QUEUED_EVENTS ];
	}

#if defined(_WIN32)
	//Dushan - push test from console to engine
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (!GetMessage(&msg, NULL, 0, 0)) {
			Com_Quit_f();
		}

		g_WinV.sysMsgTime = msg.time;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#endif

	// check for console commands
	s = Sys_ConsoleInput();
	if ( s )
	{
		char  *b;
		int   len;

		len = strlen( s ) + 1;
		b = (char*)malloc( len );
		strcpy( b, s );
		Com_QueueEvent( 0, SE_CONSOLE, 0, 0, len, b );
	}

	// return if we have data
	if ( eventHead > eventTail )
	{
		eventTail++;
		return eventQueue[ ( eventTail - 1 ) & MASK_QUEUED_EVENTS ];
	}

	// create an empty event to return
	memset( &ev, 0, sizeof( ev ) );
	ev.evTime = Sys_Milliseconds();

	return ev;
}

/*
=================
Com_GetRealEvent
=================
*/
sysEvent_s	Com_GetRealEvent() {
	int			r;
	sysEvent_s	ev;

	// either get an event from the system or the journal file
	if ( com_journal->integer == 2 ) {
		r = FS_Read( &ev, sizeof(ev), com_journalFile );
		if ( r != sizeof(ev) ) {
			Com_Error( ERR_FATAL, "Error reading from journal file" );
		}
		if ( ev.evPtrLength ) {
			ev.evPtr = malloc( ev.evPtrLength );
			r = FS_Read( ev.evPtr, ev.evPtrLength, com_journalFile );
			if ( r != ev.evPtrLength ) {
				Com_Error( ERR_FATAL, "Error reading from journal file" );
			}
		}
	} else {
		ev = Com_GetSystemEvent();

		// write the journal value out if needed
		if ( com_journal->integer == 1 ) {
			r = FS_Write( &ev, sizeof(ev), com_journalFile );
			if ( r != sizeof(ev) ) {
				Com_Error( ERR_FATAL, "Error writing to journal file" );
			}
			if ( ev.evPtrLength ) {
				r = FS_Write( ev.evPtr, ev.evPtrLength, com_journalFile );
				if ( r != ev.evPtrLength ) {
					Com_Error( ERR_FATAL, "Error writing to journal file" );
				}
			}
		}
	}

	return ev;
}


/*
=================
Com_InitPushEvent
=================
*/
void Com_InitPushEvent() {
  // clear the static buffer array
  // this requires SE_NONE to be accepted as a valid but NOP event
  memset( com_pushedEvents, 0, sizeof(com_pushedEvents) );
  // reset counters while we are at it
  // beware: GetEvent might still return an SE_NONE from the buffer
  com_pushedEventsHead = 0;
  com_pushedEventsTail = 0;
}


/*
=================
Com_PushEvent
=================
*/
void Com_PushEvent( sysEvent_s *event ) {
	sysEvent_s		*ev;
	static int printedWarning = 0;

	ev = &com_pushedEvents[ com_pushedEventsHead & (MAX_PUSHED_EVENTS-1) ];

	if ( com_pushedEventsHead - com_pushedEventsTail >= MAX_PUSHED_EVENTS ) {

		// don't print the warning constantly, or it can give time for more...
		if ( !printedWarning ) {
			printedWarning = true;
			Com_Printf( "WARNING: Com_PushEvent overflow\n" );
		}

		if ( ev->evPtr ) {
			free( ev->evPtr );
		}
		com_pushedEventsTail++;
	} else {
		printedWarning = false;
	}

	*ev = *event;
	com_pushedEventsHead++;
}

/*
=================
Com_GetEvent
=================
*/
sysEvent_s	Com_GetEvent() {
	if ( com_pushedEventsHead > com_pushedEventsTail ) {
		com_pushedEventsTail++;
		return com_pushedEvents[ (com_pushedEventsTail-1) & (MAX_PUSHED_EVENTS-1) ];
	}
	return Com_GetRealEvent();
}

/*
=================
Com_RunAndTimeServerPacket
=================
*/
void Com_RunAndTimeServerPacket( netAdr_s *evFrom, msg_c *buf ) {
	int		t1, t2, msec;

	t1 = 0;

	if ( com_speeds->integer ) {
		t1 = Sys_Milliseconds ();
	}

	SV_PacketEvent( *evFrom, buf );

	if ( com_speeds->integer ) {
		t2 = Sys_Milliseconds ();
		msec = t2 - t1;
		if ( com_speeds->integer == 3 ) {
			Com_Printf( "SV_PacketEvent time: %i\n", msec );
		}
	}
}

/*
=================
Com_EventLoop

Returns last event time
=================
*/
int Com_EventLoop() {
	sysEvent_s	ev;
	netAdr_s	evFrom;
	byte		bufData[MAX_MSGLEN];
	msg_c		buf;

	buf.init(bufData, sizeof( bufData ) );

	while ( 1 ) {
		ev = Com_GetEvent();

		// if no more events are available
		if ( ev.evType == SE_NONE ) {
			// manually send packet events for the loopback channel
			while ( NET_GetLoopPacket( NS_CLIENT, &evFrom, &buf ) ) {
				CL_PacketEvent( evFrom, &buf );
			}

			while ( NET_GetLoopPacket( NS_SERVER, &evFrom, &buf ) ) {
				// if the server just shut down, flush the events
				if ( com_sv_running->integer ) {
					Com_RunAndTimeServerPacket( &evFrom, &buf );
				}
			}

			return ev.evTime;
		}


		switch(ev.evType)
		{
			case SE_KEY:
				CL_KeyEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
			case SE_CHAR:
				CL_CharEvent( ev.evValue );
			break;
			case SE_MOUSE:
				CL_MouseEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
			case SE_JOYSTICK_AXIS:
				CL_JoystickEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
			case SE_CONSOLE:
				Cbuf_AddText( (char *)ev.evPtr );
				Cbuf_AddText( "\n" );
			break;
			default:
				Com_Error( ERR_FATAL, "Com_EventLoop: bad event type %i", ev.evType );
			break;
		}

		// free any block data
		if ( ev.evPtr ) {
			free( ev.evPtr );
		}
	}

	return 0;	// never reached
}

/*
================
Com_Milliseconds

Can be used for profiling, but will be journaled accurately
================
*/
int Com_Milliseconds (void) {
	sysEvent_s	ev;

	// get events and push them until we get a null event with the current time
	do {

		ev = Com_GetRealEvent();
		if ( ev.evType != SE_NONE ) {
			Com_PushEvent( &ev );
		}
	} while ( ev.evType != SE_NONE );
	
	return ev.evTime;
}

//============================================================================

/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
static void Com_Error_f (void) {
	if ( Cmd_Argc() > 1 ) {
		Com_Error( ERR_DROP, "Testing drop error" );
	} else {
		Com_Error( ERR_FATAL, "Testing fatal error" );
	}
}


/*
=============
Com_Freeze_f

Just freeze in place for a given number of seconds to test
error recovery
=============
*/
static void Com_Freeze_f (void) {
	float	s;
	int		start, now;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "freeze <seconds>\n" );
		return;
	}
	s = atof( Cmd_Argv(1) );

	start = Com_Milliseconds();

	while ( 1 ) {
		now = Com_Milliseconds();
		if ( ( now - start ) * 0.001 > s ) {
			break;
		}
	}
}

/*
=================
Com_Crash_f

A way to force a bus error for development reasons
=================
*/
static void Com_Crash_f() {
	* ( volatile int * ) 0 = 0x12345678;
}

/*
==================
Com_Setenv_f

For controlling environment variables
==================
*/
void Com_Setenv_f(void)
{
	int argc = Cmd_Argc();
	const char *arg1 = Cmd_Argv(1);

	if(argc > 2)
	{
		const char *arg2 = Cmd_ArgsFrom(2);
		
		Sys_SetEnv(arg1, arg2);
	}
	else if(argc == 2)
	{
		const char *env = getenv(arg1);
		
		if(env)
			Com_Printf("%s=%s\n", arg1, env);
		else
			Com_Printf("%s undefined\n", arg1);
        }
}

/*
==================
Com_ExecuteCfg

For controlling environment variables
==================
*/

void Com_ExecuteCfg(void)
{
	Cbuf_ExecuteText(EXEC_NOW, "exec default.cfg\n");
	Cbuf_Execute(); // Always execute after exec to prevent text buffer overflowing

	if(!Com_SafeMode())
	{
		// skip the q3config.cfg and autoexec.cfg if "safe" is on the command line
		Cbuf_ExecuteText(EXEC_NOW, "exec " QIOCONFIG_CFG "\n");
		Cbuf_Execute();
		Cbuf_ExecuteText(EXEC_NOW, "exec autoexec.cfg\n");
		Cbuf_Execute();
	}
}

/*
==================
Com_GameRestart

Change to a new mod properly with cleaning up cvars before switching.
==================
*/

void Com_GameRestart(int checksumFeed, bool disconnect)
{
	// make sure no recursion can be triggered
	if(!com_gameRestarting && com_fullyInitialized)
	{
		int clWasRunning;
		
		com_gameRestarting = true;
		clWasRunning = com_cl_running->integer;
		
		// Kill server if we have one
		if(com_sv_running->integer)
			SV_Shutdown("Game directory changed");

		if(clWasRunning)
		{
			if(disconnect)
				CL_Disconnect(false);
				
			CL_Shutdown("Game directory changed", disconnect, false);
		}

		FS_Restart(checksumFeed);
	
		// Clean out any user and VM created cvars
		Cvar_Restart(true);
		Com_ExecuteCfg();

		if(disconnect)
		{
			// We don't want to change any network settings if gamedir
			// change was triggered by a connect to server because the
			// new network settings might make the connection fail.
			NET_Restart_f();
		}

		if(clWasRunning)
		{
			CL_Init();
			CL_StartHunkUsers(false);
		}
		
		com_gameRestarting = false;
	}
}

/*
==================
Com_GameRestart_f

Expose possibility to change current running mod to the user
==================
*/

void Com_GameRestart_f(void)
{
	if(!FS_FilenameCompare(Cmd_Argv(1), com_basegame->string))
	{
		// This is the standard base game. Servers and clients should
		// use "" and not the standard basegame name because this messes
		// up pak file negotiation and lots of other stuff
		
		Cvar_Set("fs_game", "");
	}
	else
		Cvar_Set("fs_game", Cmd_Argv(1));

	Com_GameRestart(0, true);
}

__int64 GetMachineCycleCount()
{      
	__int64 cycles;
	_asm rdtsc
	_asm lea ebx,cycles
	_asm mov [ebx],eax
	_asm mov [ebx+4],edx
	return cycles;
}

void Com_BenchRSQRTFunctions_f() {
	int iterations;
	float f;
	float data[1000];
	int total_sqrt;
	int total_rsqrt;
	int total_rsqrt3;
	__int64 start;
	int start_msec;
	int total_sqrt_msec;
	int total_rsqrt_msec;
	int total_rsqrt3_msec;

	if(Cmd_Argc()>1) {
		iterations = atoi(Cmd_Argv(1));
	} else {
		iterations = 100000000;
	}

	for(int i = 0; i < 1000; i++) {
		data[i] = 0.01f * i;
	}
	start_msec = Sys_Milliseconds();
	start = GetMachineCycleCount();
	for(int i = 0; i < iterations; i++) {
		f = 1.f/sqrt(data[i%1000]);
	}
	total_sqrt = GetMachineCycleCount() - start;
	total_sqrt_msec = Sys_Milliseconds() - start_msec;

	start_msec = Sys_Milliseconds();
	start = GetMachineCycleCount();
	for(int i = 0; i < iterations; i++) {
		f = G_rsqrt(data[i%1000]);
	}
	total_rsqrt = GetMachineCycleCount() - start;
	total_rsqrt_msec = Sys_Milliseconds() - start_msec;

	start_msec = Sys_Milliseconds();
	start = GetMachineCycleCount();
	for(int i = 0; i < iterations; i++) {
		f = G_rsqrt3(data[i%1000]);
	}
	total_rsqrt3 = GetMachineCycleCount() - start;
	total_rsqrt3_msec = Sys_Milliseconds() - start_msec;

	Com_Printf("Total Cycles: sqrt: %i, rsqrt %i, rsqrt3 %i\n",total_sqrt,total_rsqrt,total_rsqrt3);
	Com_Printf("Total MSec: sqrt: %i, rsqrt %i, rsqrt3 %i\n",total_sqrt_msec,total_rsqrt_msec,total_rsqrt3_msec);
	float single_sqrt = total_sqrt / float(iterations);
	float single_rsqrt = total_rsqrt / float(iterations);
	float single_rsqrt3 = total_rsqrt3 / float(iterations);
	float single_sqrt_msec = total_sqrt_msec / float(iterations);
	float single_rsqrt_msec = total_rsqrt_msec / float(iterations);
	float single_rsqrt3_msec = total_rsqrt3_msec / float(iterations);
	Com_Printf("AVG Cycles: sqrt: %f, rsqrt %f, rsqrt3 %f\n",single_sqrt,single_rsqrt,single_rsqrt3);
	Com_Printf("AVG MSec: sqrt: %f, rsqrt %f, rsqrt3 %f\n",single_sqrt_msec,single_rsqrt_msec,single_rsqrt3_msec);
}

void Com_Editor_f() {
	if(g_editor) {
		Com_Printf("Editor already running.\n");
		return;
	}
	Com_Printf("Not functional now - please start engine with -editor parameter.\n");
	//Com_InitEditorDLL();
}
static void Com_DetectAltivec(void)
{
	// Only detect if user hasn't forcibly disabled it.
	if (com_altivec->integer) {
		static bool altivec = false;
		static bool detected = false;
		if (!detected) {
			altivec = ( Sys_GetProcessorFeatures( ) & CF_ALTIVEC );
			detected = true;
		}

		if (!altivec) {
			Cvar_Set( "com_altivec", "0" );  // we don't have it! Disable support!
		}
	}
}

/*
=================
Com_DetectSSE
Find out whether we have SSE support for Q_ftol function
=================
*/

#if id386 || idx64

static void Com_DetectSSE(void)
{
#if !idx64
	cpuFeatures_t feat;
	
	feat = Sys_GetProcessorFeatures();

	if(feat & CF_SSE)
	{
		if(feat & CF_SSE2)
			Q_SnapVector = qsnapvectorsse;
		else
			Q_SnapVector = qsnapvectorx87;

		Q_ftol = qftolsse;
#endif
		Q_VMftol = qvmftolsse;

		Com_Printf("Have SSE support\n");
#if !idx64
	}
	else
	{
		Q_ftol = qftolx87;
		Q_VMftol = qvmftolx87;
		Q_SnapVector = qsnapvectorx87;

		Com_Printf("No SSE support on this machine\n");
	}
#endif
}

#else

#define Com_DetectSSE()

#endif

/*
=================
Com_InitRand
Seed the random number generator, if possible with an OS supplied random seed.
=================
*/
static void Com_InitRand(void)
{
	unsigned int seed;

	if(Sys_RandomBytes((byte *) &seed, sizeof(seed)))
		srand(seed);
	else
		srand(time(NULL));
}
fontAPI_i *Com_GetDefaultFont() {
	fontAPI_i *r = rf->registerFont("camingocode/CamingoCode-Bold");
	return r;
}
void Com_EditorInitRenderer() {
	CL_InitRef();
	rf->init(true);
}
void IN_InitInputSystemAPI();
void Com_InitSysEventCasterAPI();
void SDLShared_InitSharedSDLAPI();

static coreAPI_s g_staticCoreAPI;
coreAPI_s *g_core = 0;
void Com_InitCoreAPI() {
	g_staticCoreAPI.Print = Com_Printf;
	g_staticCoreAPI.RedWarning = Com_RedWarning;
	g_staticCoreAPI.Error = Com_Error;
	g_staticCoreAPI.DropError = Com_DropError;
	g_staticCoreAPI.Milliseconds = Sys_Milliseconds;
	g_staticCoreAPI.Argc = Cmd_Argc;
	g_staticCoreAPI.Argv = Cmd_Argv;
	g_staticCoreAPI.ArgvBuffer = Cmd_ArgvBuffer;
	g_staticCoreAPI.Args = Cmd_ArgsBuffer;
	g_staticCoreAPI.ArgsFrom = Cmd_ArgsFrom;
	
	g_staticCoreAPI.Cmd_AddCommand = Cmd_AddCommand;
	g_staticCoreAPI.Cmd_RemoveCommand = Cmd_RemoveCommand;
	g_staticCoreAPI.Cbuf_ExecuteText = Cbuf_ExecuteText;
	g_staticCoreAPI.Cbuf_AddText = Cbuf_AddText;
	g_staticCoreAPI.EditorInitRenderer = Com_EditorInitRenderer;
	g_staticCoreAPI.GetDefaultFont = Com_GetDefaultFont;

	g_core = &g_staticCoreAPI;
	g_iFaceMan->registerInterface(&g_staticCoreAPI,CORE_API_IDENTSTR);
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)g_moduleMgr,MODULEMANAGER_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_ms,MATERIALSYSTEM_API_IDENTSTR);
	g_iFaceMan->registerIFaceUser(&g_modelLoader,MODELLOADERDLL_API_IDENTSTR);

	IN_InitInputSystemAPI();
	Com_InitSysEventCasterAPI();
	SDLShared_InitSharedSDLAPI();
}

// it could be in client module, but I think that server
// might need image library for loading terrain heightmaps,
// so it's shared right now.
static moduleAPI_i *com_imageLib = 0;
void Com_InitImageLib() {
	com_imageLib = g_moduleMgr->load("imageLib");
	if(com_imageLib == 0) {
		if(0) {
			Com_Error(ERR_DROP,"Cannot load imageLib DLL");
		} else {
			g_core->RedWarning("Cannot load imageLib DLL\n");
		}
	}
}
bool Com_CanStartEngineWithoutCMModule() {
	//return false;
	return true; // TODO: a developer cvar?
}
bool Com_CanStartEngineWithoutModelLoaderModule() {
	//return false;
	return true; // TODO: a developer cvar?
}
bool Com_CanStartEngineWithoutDeclManagerModule() {
	//return false;
	return true; // TODO: a developer cvar?
}
bool Com_CanStartEngineWithoutTIKIModule() {
	//return false;
	return true; // TODO: a developer cvar?
}
static moduleAPI_i *com_cmLib = 0;
void Com_InitCMDLL() {
	com_cmLib = g_moduleMgr->load("cm");
	if(com_cmLib == 0) {
		if(Com_CanStartEngineWithoutCMModule()) {
			Com_Printf("WARNING: Cannot load CM DLL\n");
		} else {
			Com_Error(ERR_DROP,"Cannot load CM DLL");
		}
	}
}
static moduleAPI_i *com_modelLoaderDLL = 0;
void Com_InitModelLoaderDLL() {
	com_modelLoaderDLL = g_moduleMgr->load("modelLoader");
	if(com_modelLoaderDLL == 0) {
		if(Com_CanStartEngineWithoutCMModule()) {
			Com_Printf("WARNING: Cannot load modelLoader DLL\n");
		} else {
			Com_Error(ERR_DROP,"Cannot load modelLoader DLL");
		}
	} else {
		g_modelLoader->initModelLoader();
	}
}


static moduleAPI_i *com_tikiLib = 0;
tikiAPI_i *tiki;
void Com_InitTIKIDLL() {
	com_tikiLib = g_moduleMgr->load("tiki");
	if(com_tikiLib == 0) {
		if(Com_CanStartEngineWithoutTIKIModule()) {
			Com_Printf("WARNING: Cannot load TIKI DLL\n");
			return;
		} else {
			Com_Error(ERR_DROP,"Cannot load TIKI DLL");
		}
	}
	g_iFaceMan->registerIFaceUser(&tiki,TIKI_API_IDENTSTR);
	tiki->init();
}
void Com_ShutdownTIKIDLL() {
	if(com_tikiLib == 0) {
		return;
	}
	if(tiki) {
		tiki->shutdown();
	}
	g_moduleMgr->unload(&com_tikiLib);
}
static moduleAPI_i *com_declMgrLib = 0;
declManagerAPI_i *g_declMgr;
void Com_InitDeclManagerDLL() {
	com_declMgrLib = g_moduleMgr->load("declManager");
	if(com_declMgrLib == 0) {
		if(Com_CanStartEngineWithoutDeclManagerModule()) {
			Com_Printf("WARNING: Cannot load declManager DLL\n");
			return;
		} else {
			Com_Error(ERR_DROP,"Cannot load declManager DLL");
		}
	}
	g_iFaceMan->registerIFaceUser(&g_declMgr,DECL_MANAGER_API_IDENTSTR);
	g_declMgr->init();
}
void Com_ShutdownDeclManagerDLL() {
	if(com_declMgrLib == 0) {
		return;
	}
	if(g_declMgr) {
		g_declMgr->shutdown();
	}
	g_moduleMgr->unload(&com_declMgrLib);
}
void Com_ShutdownModelLoaderDLL() {
	if(com_modelLoaderDLL == 0) {
		return;
	}
	if(com_modelLoaderDLL) {
		g_modelLoader->shutdownModelLoader();
	}
	g_moduleMgr->unload(&com_modelLoaderDLL);
}
void Com_ShutdownCMDLL() {
	if(com_cmLib == 0) {
		return;
	}
	//if(g_declMgr) {
	//	g_declMgr->shutdown();
	//}
	g_moduleMgr->unload(&com_cmLib);
}
static moduleAPI_i *com_editorLib = 0;
editorAPI_i *g_editor = 0;
	    HDC currentHDC;
	    HGLRC currentHGLRC;
bool Com_InitEditorDLL() {
	///    currentHDC = wglGetCurrentDC();
	///    currentHGLRC = wglGetCurrentContext();
	com_editorLib = g_moduleMgr->load("editor");
	if(com_editorLib == 0) {
		return true; // error
	}
	g_iFaceMan->registerIFaceUser(&g_editor,EDITOR_API_IDENTSTR);
	g_editor->initEditor();
	return false;
}
void Com_ShutdownEditorDLL() {
	if(com_editorLib == 0) {
		return;
	}
	g_editor->shutdownEditor();
	g_moduleMgr->unload(&com_editorLib);
	g_editor = 0;
}
/*
=================
Com_Init
=================
*/
void Com_Init( char *commandLine ) {
	const char	*s;
	int	qport;

	Com_Printf( "%s %s %s\n", Q3_VERSION, PLATFORM_STRING, __DATE__ );

	if ( setjmp (abortframe) ) {
		Sys_Error ("Error during initialization");
	}

	// let's do this first
	Com_InitCoreAPI();

	// Clear queues
	memset( &eventQueue[ 0 ], 0, MAX_QUEUED_EVENTS * sizeof( sysEvent_s ) );

	// initialize the weak pseudo-random number generator for use later.
	Com_InitRand();

	// do this before anything else decides to push events
	Com_InitPushEvent();

	Cvar_Init ();

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	Com_ParseCommandLine( commandLine );

//	Swap_Init ();
	Cbuf_Init ();

	Com_DetectSSE();

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

	Cmd_Init ();

	// get the developer cvar set as early as possible
	com_developer = Cvar_Get("developer", "0", CVAR_TEMP);

	// done early so bind command exists
	CL_InitKeyCommands();

	com_standalone = Cvar_Get("com_standalone", "0", CVAR_ROM);
	com_basegame = Cvar_Get("com_basegame", BASEGAME, CVAR_INIT);
	com_homepath = Cvar_Get("com_homepath", "", CVAR_INIT);
	
	if(!com_basegame->string[0])
		Cvar_ForceReset("com_basegame");

	FS_InitFilesystem ();

	Com_InitJournaling();

	Com_InitImageLib();

	Com_InitCMDLL();

	Com_InitModelLoaderDLL();

	Com_InitDeclManagerDLL();

	Com_InitTIKIDLL();


	// Add some commands here already so users can use them from config files
	Cmd_AddCommand ("setenv", Com_Setenv_f);
	if (com_developer && com_developer->integer)
	{
		Cmd_AddCommand ("error", Com_Error_f);
		Cmd_AddCommand ("crash", Com_Crash_f);
		Cmd_AddCommand ("freeze", Com_Freeze_f);
	}
	Cmd_AddCommand ("quit", Com_Quit_f);
	Cmd_AddCommand ("changeVectors", MSG_ReportChangeVectors_f );
	Cmd_AddCommand ("writeconfig", Com_WriteConfig_f );
	Cmd_SetCommandCompletionFunc( "writeconfig", Cmd_CompleteCfgName );
	Cmd_AddCommand("game_restart", Com_GameRestart_f);
	Cmd_AddCommand("bench_sqrt", Com_BenchRSQRTFunctions_f);
	Cmd_AddCommand("editor", Com_Editor_f);

	Com_ExecuteCfg();

	// override anything from the config files with command line args
	Com_StartupVariable( NULL );

  // get dedicated here for proper hunk megs initialization
#ifdef DEDICATED
	com_dedicated = Cvar_Get ("dedicated", "1", CVAR_INIT);
	Cvar_CheckRange( com_dedicated, 1, 2, true );
#else
	com_dedicated = Cvar_Get ("dedicated", "0", CVAR_LATCH);
	Cvar_CheckRange( com_dedicated, 0, 2, true );
#endif

	// if any archived cvars are modified after this, we will trigger a writing
	// of the config file
	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

	//
	// init commands and vars
	//
	com_altivec = Cvar_Get ("com_altivec", "1", CVAR_ARCHIVE);
	com_maxfps = Cvar_Get ("com_maxfps", "85", CVAR_ARCHIVE);
	com_blood = Cvar_Get ("com_blood", "1", CVAR_ARCHIVE);

	com_logfile = Cvar_Get ("logfile", "0", CVAR_TEMP );

	com_timescale = Cvar_Get ("timescale", "1", CVAR_CHEAT | CVAR_SYSTEMINFO );
	com_fixedtime = Cvar_Get ("fixedtime", "0", CVAR_CHEAT);
	com_showtrace = Cvar_Get ("com_showtrace", "0", CVAR_CHEAT);
	com_viewlog = Cvar_Get("viewlog", "1", CVAR_CHEAT);
	com_speeds = Cvar_Get ("com_speeds", "0", 0);
	com_timedemo = Cvar_Get ("timedemo", "0", CVAR_CHEAT);
	com_cameraMode = Cvar_Get ("com_cameraMode", "0", CVAR_CHEAT);

	cl_paused = Cvar_Get ("cl_paused", "0", CVAR_ROM);
	sv_paused = Cvar_Get ("sv_paused", "0", CVAR_ROM);
	cl_packetdelay = Cvar_Get ("cl_packetdelay", "0", CVAR_CHEAT);
	sv_packetdelay = Cvar_Get ("sv_packetdelay", "0", CVAR_CHEAT);
	com_sv_running = Cvar_Get ("sv_running", "0", CVAR_ROM);
	com_cl_running = Cvar_Get ("cl_running", "0", CVAR_ROM);
	com_ansiColor = Cvar_Get( "com_ansiColor", "0", CVAR_ARCHIVE );

	com_unfocused = Cvar_Get( "com_unfocused", "0", CVAR_ROM );
	com_maxfpsUnfocused = Cvar_Get( "com_maxfpsUnfocused", "0", CVAR_ARCHIVE );
	com_minimized = Cvar_Get( "com_minimized", "0", CVAR_ROM );
	com_maxfpsMinimized = Cvar_Get( "com_maxfpsMinimized", "0", CVAR_ARCHIVE );
	com_abnormalExit = Cvar_Get( "com_abnormalExit", "0", CVAR_ROM );
	com_busyWait = Cvar_Get("com_busyWait", "0", CVAR_ARCHIVE);
	Cvar_Get("com_errorMessage", "", CVAR_ROM | CVAR_NORESTART);

	com_introPlayed = Cvar_Get( "com_introplayed", "0", CVAR_ARCHIVE);

	if (com_dedicated->integer) {
		if (!com_viewlog->integer) {
			Cvar_Set("viewlog", "1");
		}
	}

	s = va("%s %s %s", Q3_VERSION, PLATFORM_STRING, __DATE__ );
	com_version = Cvar_Get ("version", s, CVAR_ROM | CVAR_SERVERINFO );
	com_gamename = Cvar_Get("com_gamename", GAMENAME_FOR_MASTER, CVAR_SERVERINFO | CVAR_INIT);
	com_protocol = Cvar_Get("com_protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_INIT);

	Cvar_Get("protocol", com_protocol->string, CVAR_ROM);

	Sys_Init();

	if( Sys_WritePIDFile( ) ) {
#ifndef DEDICATED
		const char *message = "The last time " CLIENT_WINDOW_TITLE " ran, "
			"it didn't exit properly. This may be due to inappropriate video "
			"settings. Would you like to start with \"safe\" video settings?";

		if( Sys_Dialog( DT_YES_NO, message, "Abnormal Exit" ) == DR_YES ) {
			Cvar_Set( "com_abnormalExit", "1" );
		}
#endif
	}

	if(com_bEditorMode == false) {
		// Pick a random port value
		Com_RandomBytes( (byte*)&qport, sizeof(int) );
		Netchan_Init( qport & 0xffff );

		SV_Init();

		com_dedicated->modified = false;
#ifndef DEDICATED
		CL_Init();
#if defined (_WIN32)
		Sys_ShowConsole(com_viewlog->integer, false);
#endif
#endif
	}

	// set com_frameTime so that if a map is started on the
	// command line it will still be able to count on com_frameTime
	// being random enough for a serverid
	com_frameTime = Com_Milliseconds();

	// add + commands from command line
	if ( !Com_AddStartupCommands() ) {
		// if the user didn't give any commands, run default action
		if ( !com_dedicated->integer ) {
			Cbuf_AddText ("cinematic idlogo.RoQ\n");
			if( !com_introPlayed->integer ) {
				Cvar_Set( com_introPlayed->name, "1" );
				Cvar_Set( "nextmap", "cinematic intro.RoQ" );
			}
		}
	}

	// start in full screen ui mode
	Cvar_Set("r_uiFullScreen", "1");

	if (!com_dedicated->integer) {
#if defined (_WIN32)
		Sys_ShowConsole(com_viewlog->integer, false);
#endif
	}
	
	if(com_bEditorMode == false) {
		CL_StartHunkUsers( false );
	}

	// make sure single player is off by default
	Cvar_Set("ui_singlePlayerActive", "0");

	com_fullyInitialized = true;

	// always set the cvar, but only print the info if it makes sense.
	Com_DetectAltivec();
#if idppc
	Com_Printf ("Altivec support is %s\n", com_altivec->integer ? "enabled" : "disabled");
#endif

	com_pipefile = Cvar_Get( "com_pipefile", "", CVAR_ARCHIVE|CVAR_LATCH );
	if( com_pipefile->string[0] )
	{
		pipefile = FS_FCreateOpenPipeFile( com_pipefile->string );
	}

	Com_Printf ("--- Common Initialization Complete ---\n");
}

/*
===============
Com_ReadFromPipe

Read whatever is in com_pipefile, if anything, and execute it
===============
*/
void Com_ReadFromPipe()
{
	static char buf[MAX_STRING_CHARS];
	static int accu = 0;
	int read;

	if( !pipefile )
		return;

	while( ( read = FS_Read( buf + accu, sizeof( buf ) - accu - 1, pipefile ) ) > 0 )
	{
		char *brk = NULL;
		int i;

		for( i = accu; i < accu + read; ++i )
		{
			if( buf[ i ] == '\0' )
				buf[ i ] = '\n';
			if( buf[ i ] == '\n' || buf[ i ] == '\r' )
				brk = &buf[ i + 1 ];
		}
		buf[ accu + read ] = '\0';

		accu += read;

		if( brk )
		{
			char tmp = *brk;
			*brk = '\0';
			Cbuf_ExecuteText( EXEC_APPEND, buf );
			*brk = tmp;

			accu -= brk - buf;
			memmove( buf, brk, accu + 1 );
		}
		else if( accu >= sizeof( buf ) - 1 ) // full
		{
			Cbuf_ExecuteText( EXEC_APPEND, buf );
			accu = 0;
		}
	}
}


//==================================================================

void Com_WriteConfigToFile( const char *filename ) {
	fileHandle_t	f;

	f = FS_FOpenFileWrite( filename );
	if ( !f ) {
		Com_Printf ("Couldn't write %s.\n", filename );
		return;
	}

	FS_Printf (f, "// generated by quake, do not modify\n");
	Key_WriteBindings (f);
	Cvar_WriteVariables (f);
	FS_FCloseFile( f );
}


/*
===============
Com_WriteConfiguration

Writes key bindings and archived cvars to config file if modified
===============
*/
void Com_WriteConfiguration() {
	// if we are quiting without fully initializing, make sure
	// we don't write out anything
	if ( !com_fullyInitialized ) {
		return;
	}

	if ( !(cvar_modifiedFlags & CVAR_ARCHIVE ) ) {
		return;
	}
	cvar_modifiedFlags &= ~CVAR_ARCHIVE;

	Com_WriteConfigToFile( QIOCONFIG_CFG );
}


/*
===============
Com_WriteConfig_f

Write the config file to a specific name
===============
*/
void Com_WriteConfig_f() {
	str	filename;

	if ( Cmd_Argc() != 2 ) {
		Com_Printf( "Usage: writeconfig <filename>\n" );
		return;
	}

	filename = Cmd_Argv(1);
	filename.defaultExtension(".cfg");
	Com_Printf( "Writing %s.\n", filename.c_str() );
	Com_WriteConfigToFile( filename );
}

/*
================
Com_ModifyMsec
================
*/
int Com_ModifyMsec( int msec ) {
	int		clampTime;

	//
	// modify time for debugging values
	//
	if ( com_fixedtime->integer ) {
		msec = com_fixedtime->integer;
	} else if ( com_timescale->value ) {
		msec *= com_timescale->value;
	} else if (com_cameraMode->integer) {
		msec *= com_timescale->value;
	}
	
	// don't let it scale below 1 msec
	if ( msec < 1 && com_timescale->value) {
		msec = 1;
	}

	if ( com_dedicated->integer ) {
		// dedicated servers don't want to clamp for a much longer
		// period, because it would mess up all the client's views
		// of time.
		if (com_sv_running->integer && msec > 500)
			Com_Printf( "Hitch warning: %i msec frame time\n", msec );

		clampTime = 5000;
	} else 
	if ( !com_sv_running->integer ) {
		// clients of remote servers do not want to clamp time, because
		// it would skew their view of the server's time temporarily
		clampTime = 5000;
	} else {
		// for local single player gaming
		// we may want to clamp the time to prevent players from
		// flying off edges when something hitches.
		clampTime = 200;
	}

	if ( msec > clampTime ) {
		msec = clampTime;
	}

	return msec;
}

/*
=================
Com_TimeVal
=================
*/

int Com_TimeVal(int minMsec)
{
	int timeVal;

	timeVal = Sys_Milliseconds() - com_frameTime;

	if(timeVal >= minMsec)
		timeVal = 0;
	else
		timeVal = minMsec - timeVal;

	return timeVal;
}

bool COM_RunEditorFrame() {
	if(g_editor->runEditor()) {
		Com_Printf("Editor closed by user...\n");
		Com_ShutdownEditorDLL();
		return true;
	}
// V: not used now, it should be used when editor is running along with game window
///	if(wglMakeCurrent(currentHDC,currentHGLRC)==FALSE) {
///		Com_RedWarning("wglMakeCurrent after editor frame failed.\n");
///	}
	return false;
}
/*
=================
Com_Frame
=================
*/
void Com_Frame() {

	int		msec, minMsec;
	int		timeVal, timeValSV;
	static int	lastTime = 0, bias = 0;
 
	int		timeBeforeFirstEvents;
	int		timeBeforeServer;
	int		timeBeforeEvents;
	int		timeBeforeClient;
	int		timeAfter;
  

	if ( setjmp (abortframe) ) {
		return;			// an ERR_DROP was thrown
	}

	timeBeforeFirstEvents =0;
	timeBeforeServer =0;
	timeBeforeEvents =0;
	timeBeforeClient = 0;
	timeAfter = 0;

	// write config file if anything changed
	Com_WriteConfiguration(); 

	// if "viewlog" has been modified, show or hide the log console
	if (com_viewlog->modified) {
#if defined (_WIN32)
		if (!com_dedicated->value) {
			Sys_ShowConsole(com_viewlog->integer, false);
		}
#endif
		com_viewlog->modified = false;
	}

	//
	// main event loop
	//
	if ( com_speeds->integer ) {
		timeBeforeFirstEvents = Sys_Milliseconds ();
	}

	// Figure out how much time we have
	if(!com_timedemo->integer)
	{
		if(com_dedicated->integer)
			minMsec = SV_FrameMsec();
		else
		{
			if(com_minimized->integer && com_maxfpsMinimized->integer > 0)
				minMsec = 1000 / com_maxfpsMinimized->integer;
			else if(com_unfocused->integer && com_maxfpsUnfocused->integer > 0)
				minMsec = 1000 / com_maxfpsUnfocused->integer;
			else if(com_maxfps->integer > 0)
				minMsec = 1000 / com_maxfps->integer;
			else
				minMsec = 1;
			
			timeVal = com_frameTime - lastTime;
			bias += timeVal - minMsec;
			
			if(bias > minMsec)
				bias = minMsec;
			
			// Adjust minMsec if previous frame took too long to render so
			// that framerate is stable at the requested value.
			minMsec -= bias;
		}
	}
	else
		minMsec = 1;

	do
	{
		if(com_sv_running->integer)
		{
			timeValSV = SV_SendQueuedPackets();
			
			timeVal = Com_TimeVal(minMsec);

			if(timeValSV < timeVal)
				timeVal = timeValSV;
		}
		else
			timeVal = Com_TimeVal(minMsec);
		
		if(com_busyWait->integer || timeVal < 1)
			NET_Sleep(0);
		else
			NET_Sleep(timeVal - 1);
	} while(Com_TimeVal(minMsec));
	
	lastTime = com_frameTime;
	com_frameTime = Com_EventLoop();
	
	msec = com_frameTime - lastTime;

	Cbuf_Execute ();

	if (com_altivec->modified)
	{
		Com_DetectAltivec();
		com_altivec->modified = false;
	}

	// mess with msec if needed
	msec = Com_ModifyMsec(msec);

	//
	// server side
	//
	if ( com_speeds->integer ) {
		timeBeforeServer = Sys_Milliseconds ();
	}

	SV_Frame( msec );

	// if "dedicated" has been modified, start up
	// or shut down the client system.
	// Do this after the server may have started,
	// but before the client tries to auto-connect
	if ( com_dedicated->modified ) {
		// get the latched value
		Cvar_Get( "dedicated", "0", 0 );
		com_dedicated->modified = false;
		if ( !com_dedicated->integer ) {
			SV_Shutdown( "dedicated set to 0" );
			CL_FlushMemory();
		}
	}

#ifndef DEDICATED
	//
	// client system
	//
	//
	// run event loop a second time to get server to client packets
	// without a frame of latency
	//
	if ( com_speeds->integer ) {
		timeBeforeEvents = Sys_Milliseconds ();
	}
	Com_EventLoop();
	Cbuf_Execute ();


	//
	// client side
	//
	if ( com_speeds->integer ) {
		timeBeforeClient = Sys_Milliseconds ();
	}

	CL_Frame( msec );

	if ( com_speeds->integer ) {
		timeAfter = Sys_Milliseconds ();
	}
#else
	if ( com_speeds->integer ) {
		timeAfter = Sys_Milliseconds ();
		timeBeforeEvents = timeAfter;
		timeBeforeClient = timeAfter;
	}
#endif
	//
	// editor system
	//	
	// V: not used not, it was used when editor was supposed to run along with game window
	//if(g_editor) {
	//	COM_RunEditorFrame();
	//}

	NET_FlushPacketQueue();

	//
	// report timing information
	//
	if ( com_speeds->integer ) {
		int			all, sv, ev, cl;

		all = timeAfter - timeBeforeServer;
		sv = timeBeforeEvents - timeBeforeServer;
		ev = timeBeforeServer - timeBeforeFirstEvents + timeBeforeClient - timeBeforeEvents;
		cl = timeAfter - timeBeforeClient;
		sv -= time_game;
		cl -= time_frontend + time_backend;

		Com_Printf ("frame:%i all:%3i sv:%3i ev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n", 
					 com_frameNumber, all, sv, ev, cl, time_game, time_frontend, time_backend );
	}	



	Com_ReadFromPipe( );

	com_frameNumber++;
}

/*
=================
Com_Shutdown
=================
*/
void Com_Shutdown (void) {
	// shutdown editor dll
	Com_ShutdownEditorDLL();
	// shutdown Doom3 decls manager
	Com_ShutdownDeclManagerDLL();
	// shutdown CollisionModel library
	Com_ShutdownCMDLL();
	// shutdown TIKI DLL
	Com_ShutdownTIKIDLL();
	// shutdown model loader DLL
	Com_ShutdownModelLoaderDLL();

	if (logfile) {
		FS_FCloseFile (logfile);
		logfile = 0;
	}

	if ( com_journalFile ) {
		FS_FCloseFile( com_journalFile );
		com_journalFile = 0;
	}

	if( pipefile ) {
		FS_FCloseFile( pipefile );
		FS_HomeRemove( com_pipefile->string );
	}

}


/*
==================
Com_RandomBytes

fills string array with len radom bytes, peferably from the OS randomizer
==================
*/
void Com_RandomBytes( byte *string, int len )
{
	int i;

	if( Sys_RandomBytes( string, len ) )
		return;

	Com_Printf( "Com_RandomBytes: using weak randomization\n" );
	for( i = 0; i < len; i++ )
		string[i] = (unsigned char)( rand() % 255 );
}


/*
==================
Com_IsVoipTarget

Returns non-zero if given clientNum is enabled in voipTargets, zero otherwise.
If clientNum is negative return if any bit is set.
==================
*/
bool Com_IsVoipTarget(uint8_t *voipTargets, int voipTargetsSize, int clientNum)
{
	int index;
	if(clientNum < 0)
	{
		for(index = 0; index < voipTargetsSize; index++)
		{
			if(voipTargets[index])
				return true;
		}
		
		return false;
	}

	index = clientNum >> 3;
	
	if(index < voipTargetsSize)
		return (voipTargets[index] & (1 << (clientNum & 0x07)));

	return false;
}





/*
===================
Com_HexStrToInt
===================
*/
int Com_HexStrToInt( const char *str )
{
	if ( !str || !str[ 0 ] )
		return -1;

	// check for hex code
	if( str[ 0 ] == '0' && str[ 1 ] == 'x' )
	{
		int i, n = 0;

		for( i = 2; i < strlen( str ); i++ )
		{
			char digit;

			n *= 16;

			digit = tolower( str[ i ] );

			if( digit >= '0' && digit <= '9' )
				digit -= '0';
			else if( digit >= 'a' && digit <= 'f' )
				digit = digit - 'a' + 10;
			else
				return -1;

			n += digit;
		}

		return n;
	}

	return -1;
}

/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/



/*
=============
Q_strncpyz
 
Safe strncpy that ensures a trailing zero
=============
*/
void Q_strncpyz( char *dest, const char *src, int destsize ) {
  if ( !dest ) {
    g_core->Error( ERR_FATAL, "Q_strncpyz: NULL dest" );
  }
	if ( !src ) {
		g_core->Error( ERR_FATAL, "Q_strncpyz: NULL src" );
	}
	if ( destsize < 1 ) {
		g_core->Error(ERR_FATAL,"Q_strncpyz: destsize < 1" ); 
	}

	strncpy( dest, src, destsize-1 );
  dest[destsize-1] = 0;
}
    
       



// never goes past bounds or leaves without a terminating 0
void Q_strcat( char *dest, int size, const char *src ) {
	int		l1;

	l1 = strlen( dest );
	if ( l1 >= size ) {
		g_core->Error( ERR_FATAL, "Q_strcat: already overflowed" );
	}
	Q_strncpyz( dest + l1, src, size - l1 );
}


int QDECL Com_sprintf(char *dest, int size, const char *fmt, ...)
{
	int		len;
	va_list		argptr;

	va_start (argptr,fmt);
	len = Q_vsnprintf(dest, size, fmt, argptr);
	va_end (argptr);

	if(len >= size)
		g_core->Print("Com_sprintf: Output length %d too short, require %d bytes.\n", size, len + 1);
	
	return len;
}


//====================================================================

/*
==================
Com_CharIsOneOfCharset
==================
*/
static bool Com_CharIsOneOfCharset( char c, char *set )
{
	int i;

	for( i = 0; i < strlen( set ); i++ )
	{
		if( set[ i ] == c )
			return true;
	}

	return false;
}

/*
==================
Com_SkipTokens
==================
*/
char *Com_SkipTokens( char *s, int numTokens, char *sep )
{
	int		sepCount = 0;
	char	*p = s;

	while( sepCount < numTokens )
	{
		if( Com_CharIsOneOfCharset( *p++, sep ) )
		{
			sepCount++;
			while( Com_CharIsOneOfCharset( *p, sep ) )
				p++;
		}
		else if( *p == '\0' )
			break;
	}

	if( sepCount == numTokens )
		return p;
	else
		return s;
}

short   ShortSwap (short l)
{
	byte    b1,b2;

	b1 = l&255;
	b2 = (l>>8)&255;

	return (b1<<8) + b2;
}


qioModule_e IFM_GetCurModule() {
	return QMD_CORE;
}
