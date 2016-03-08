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

// TODO: make routines from this file shared between DX9 and GL backends

#ifdef USE_LOCAL_HEADERS
#	include "SDL.h"
#else
#	include <SDL.h>
#endif

#ifdef SMP
#	ifdef USE_LOCAL_HEADERS
#		include "SDL_thread.h"
#	else
#		include <SDL_thread.h>
#	endif
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../client/client.h"
#include "../sys/sys_local.h"
#include <api/coreAPI.h>
#include <api/cvarAPI.h>
#include <api/inputSystemAPI.h>
#include "sdl_icon.h"

/* Just hack it for now. */
#ifdef MACOS_X
#include <OpenGL/OpenGL.h>
typedef CGLContextObj QGLContext;
#define GLimp_GetCurrentContext() CGLGetCurrentContext()
#define GLimp_SetCurrentContext(ctx) CGLSetCurrentContext(ctx)
#else
typedef void *QGLContext;
#define GLimp_GetCurrentContext() (NULL)
#define GLimp_SetCurrentContext(ctx)
#endif

static QGLContext opengl_context;
static float displayAspect;

#include "sdl_glConfig.h"
struct glConfig_s glConfig;


typedef enum
{
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;

static SDL_Surface *screen = NULL;
static const SDL_VideoInfo *videoInfo = NULL;

cvar_s *r_allowResize; // make window resizable
cvar_s *r_centerWindow;
cvar_s *r_sdlDriver;
cvar_s *r_fullscreen;
cvar_s *r_mode;



/*
** R_GetModeInfo
*/
typedef struct vidmode_s
{
	const char *description;
	int width, height;
	float pixelAspect;		// pixel width / height
} vidmode_t;

vidmode_t r_vidModes[] =
{
	{ "Mode  0: 320x240",		320,	240,	1 },
	{ "Mode  1: 400x300",		400,	300,	1 },
	{ "Mode  2: 512x384",		512,	384,	1 },
	{ "Mode  3: 640x480",		640,	480,	1 },
	{ "Mode  4: 800x600",		800,	600,	1 },
	{ "Mode  5: 960x720",		960,	720,	1 },
	{ "Mode  6: 1024x768",		1024,	768,	1 },
	{ "Mode  7: 1152x864",		1152,	864,	1 },
	{ "Mode  8: 1280x1024",		1280,	1024,	1 },
	{ "Mode  9: 1600x1200",		1600,	1200,	1 },
	{ "Mode 10: 2048x1536",		2048,	1536,	1 },
	{ "Mode 11: 856x480 (wide)",856,	480,	1 }
};
static int	s_numVidModes = ARRAY_LEN( r_vidModes );

bool R_GetModeInfo( int *width, int *height, float *windowAspect, int mode ) {
	vidmode_t	*vm;
	float			pixelAspect;

	if ( mode < -1 ) {
		return false;
	}
	if ( mode >= s_numVidModes ) {
		return false;
	}

	if ( mode == -1 ) {
#if 0
		*width = r_customwidth->integer;
		*height = r_customheight->integer;
		pixelAspect = r_customPixelAspect->value;
#else
		mode = 3;
		return R_GetModeInfo(width,height,windowAspect,mode);
#endif
	} else {
		vm = &r_vidModes[mode];

		*width  = vm->width;
		*height = vm->height;
		pixelAspect = vm->pixelAspect;
	}

	*windowAspect = (float)*width / ( *height * pixelAspect );

	return true;
}

/*
** R_ModeList_f
*/
static void R_ModeList_f()
{
	int i;

	g_core->Print("\n");
	for ( i = 0; i < s_numVidModes; i++ )
	{
		g_core->Print("%s\n", r_vidModes[i].description);
	}
	g_core->Print("\n");
}

/*
===============
GLimp_Shutdown
===============
*/
void GLimp_Shutdown()
{
	g_inputSystem->IN_Shutdown();

	SDL_QuitSubSystem( SDL_INIT_VIDEO );
	screen = NULL;

	//memset( &glConfig, 0, sizeof( glConfig ) );
	//memset( &glState, 0, sizeof( glState ) );
}

/*
===============
GLimp_CompareModes
===============
*/
static int GLimp_CompareModes( const void *a, const void *b )
{
	const float ASPECT_EPSILON = 0.001f;
	SDL_Rect *modeA = *(SDL_Rect **)a;
	SDL_Rect *modeB = *(SDL_Rect **)b;
	float aspectA = (float)modeA->w / (float)modeA->h;
	float aspectB = (float)modeB->w / (float)modeB->h;
	int areaA = modeA->w * modeA->h;
	int areaB = modeB->w * modeB->h;
	float aspectDiffA = fabs( aspectA - displayAspect );
	float aspectDiffB = fabs( aspectB - displayAspect );
	float aspectDiffsDiff = aspectDiffA - aspectDiffB;

	if( aspectDiffsDiff > ASPECT_EPSILON )
		return 1;
	else if( aspectDiffsDiff < -ASPECT_EPSILON )
		return -1;
	else
		return areaA - areaB;
}


/*
===============
GLimp_DetectAvailableModes
===============
*/
static void GLimp_DetectAvailableModes(void)
{
	char buf[ MAX_STRING_CHARS ] = { 0 };
	SDL_Rect **modes;
	int numModes;
	int i;

	modes = SDL_ListModes( videoInfo->vfmt, SDL_OPENGL | SDL_FULLSCREEN );

	if( !modes )
	{
		g_core->RedWarning( "Can't get list of available modes\n" );
		return;
	}

	if( modes == (SDL_Rect **)-1 )
	{
		g_core->Print( "Display supports any resolution\n" );
		return; // can set any resolution
	}

	for( numModes = 0; modes[ numModes ]; numModes++ );

	if( numModes > 1 )
		qsort( modes, numModes, sizeof( SDL_Rect* ), GLimp_CompareModes );

	for( i = 0; i < numModes; i++ )
	{
		const char *newModeString = va( "%ux%u ", modes[ i ]->w, modes[ i ]->h );

		if( strlen( newModeString ) < (int)sizeof( buf ) - strlen( buf ) )
			Q_strcat( buf, sizeof( buf ), newModeString );
		else
			g_core->RedWarning( "Skipping mode %ux%x, buffer too small\n", modes[i]->w, modes[i]->h );
	}

	if( *buf )
	{
		buf[ strlen( buf ) - 1 ] = 0;
		g_core->Print( "Available modes: '%s'\n", buf );
		g_cvars->Cvar_Set( "r_availableModes", buf );
	}
}

/*
=================
GLimp_SetGamma
=================
*/
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] )
{
	Uint16 table[3][256];
	int i, j;

	if( !glConfig.deviceSupportsGamma /*|| r_ignorehwgamma->integer > 0*/ )
		return;

	for (i = 0; i < 256; i++)
	{
		table[0][i] = ( ( ( Uint16 ) red[i] ) << 8 ) | red[i];
		table[1][i] = ( ( ( Uint16 ) green[i] ) << 8 ) | green[i];
		table[2][i] = ( ( ( Uint16 ) blue[i] ) << 8 ) | blue[i];
	}

#ifdef _WIN32
#include <windows.h>

	// Win2K and newer put this odd restriction on gamma ramps...
	{
		OSVERSIONINFO	vinfo;

		vinfo.dwOSVersionInfoSize = sizeof( vinfo );
		GetVersionEx( &vinfo );
		if( vinfo.dwMajorVersion >= 5 && vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
			g_core->Print("performing gamma clamp.\n" );
			for( j = 0 ; j < 3 ; j++ )
			{
				for( i = 0 ; i < 128 ; i++ )
				{
					if( table[ j ] [ i] > ( ( 128 + i ) << 8 ) )
						table[ j ][ i ] = ( 128 + i ) << 8;
				}

				if( table[ j ] [127 ] > 254 << 8 )
					table[ j ][ 127 ] = 254 << 8;
			}
		}
	}
#endif

	// enforce constantly increasing
	for (j = 0; j < 3; j++)
	{
		for (i = 1; i < 256; i++)
		{
			if (table[j][i] < table[j][i-1])
				table[j][i] = table[j][i-1];
		}
	}

	SDL_SetGammaRamp(table[0], table[1], table[2]);
}


/*
===============
GLimp_SetMode
===============
*/
static int GLimp_SetMode(int mode, bool fullscreen, bool noborder)
{
	int sdlcolorbits;
	int colorbits, depthbits, stencilbits;
	int tcolorbits, tdepthbits, tstencilbits;
	int samples;
	int i = 0;
	SDL_Surface *vidscreen = NULL;

	g_core->Print( "Initializing OpenGL display\n");


	if( videoInfo == NULL )
	{
		static SDL_VideoInfo sVideoInfo;
		static SDL_PixelFormat sPixelFormat;

		videoInfo = SDL_GetVideoInfo( );

		// Take a copy of the videoInfo
		memcpy( &sPixelFormat, videoInfo->vfmt, sizeof( SDL_PixelFormat ) );
		sPixelFormat.palette = NULL; // Should already be the case
		memcpy( &sVideoInfo, videoInfo, sizeof( SDL_VideoInfo ) );
		sVideoInfo.vfmt = &sPixelFormat;
		videoInfo = &sVideoInfo;

		if( videoInfo->current_h > 0 )
		{
			// Guess the display aspect ratio through the desktop resolution
			// by assuming (relatively safely) that it is set at or close to
			// the display's native aspect ratio
			displayAspect = (float)videoInfo->current_w / (float)videoInfo->current_h;

			g_core->Print( "Estimated display aspect: %.3f\n", displayAspect );
		}
		else
		{
			g_core->Print( 
					"Cannot estimate display aspect, assuming 1.333\n" );
		}
	}

	g_core->Print( "...setting mode %d:", mode );

	if (mode == -2)
	{
		// use desktop video resolution
		if( videoInfo->current_h > 0 )
		{
			glConfig.vidWidth = videoInfo->current_w;
			glConfig.vidHeight = videoInfo->current_h;
		}
		else
		{
			glConfig.vidWidth = 640;
			glConfig.vidHeight = 480;
			g_core->Print( 
					"Cannot determine display resolution, assuming 640x480\n" );
		}

		glConfig.windowAspect = (float)glConfig.vidWidth / (float)glConfig.vidHeight;
	}
	else if ( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode ) )
	{
		g_core->Print( " invalid mode\n" );
		return RSERR_INVALID_MODE;
	}
	g_core->Print( " %d %d\n", glConfig.vidWidth, glConfig.vidHeight);

	Uint32 flags = SDL_OPENGL;
	if ( r_allowResize->integer )
		flags |= SDL_RESIZABLE;

	if (fullscreen)
	{
		flags |= SDL_FULLSCREEN;
		glConfig.isFullscreen = true;
	}
	else
	{
		if (noborder)
			flags |= SDL_NOFRAME;

		glConfig.isFullscreen = false;
	}

	colorbits = 32; // r_colorbits->value;
	if ((!colorbits) || (colorbits >= 32))
		colorbits = 24;

	//if (!r_depthbits->value)
		depthbits = 24;
	//else
	//	depthbits = r_depthbits->value;
		// stencil bits are needed for stencil shadows
	stencilbits = 32;//r_stencilbits->value;
	samples = 0;//r_ext_multisample->value;

	for (i = 0; i < 16; i++)
	{
		// 0 - default
		// 1 - minus colorbits
		// 2 - minus depthbits
		// 3 - minus stencil
		if ((i % 4) == 0 && i)
		{
			// one pass, reduce
			switch (i / 4)
			{
				case 2 :
					if (colorbits == 24)
						colorbits = 16;
					break;
				case 1 :
					if (depthbits == 24)
						depthbits = 16;
					else if (depthbits == 16)
						depthbits = 8;
				case 3 :
					if (stencilbits == 24)
						stencilbits = 16;
					else if (stencilbits == 16)
						stencilbits = 8;
			}
		}

		tcolorbits = colorbits;
		tdepthbits = depthbits;
		tstencilbits = stencilbits;

		if ((i % 4) == 3)
		{ // reduce colorbits
			if (tcolorbits == 24)
				tcolorbits = 16;
		}

		if ((i % 4) == 2)
		{ // reduce depthbits
			if (tdepthbits == 24)
				tdepthbits = 16;
			else if (tdepthbits == 16)
				tdepthbits = 8;
		}

		if ((i % 4) == 1)
		{ // reduce stencilbits
			if (tstencilbits == 24)
				tstencilbits = 16;
			else if (tstencilbits == 16)
				tstencilbits = 8;
			else
				tstencilbits = 0;
		}

		sdlcolorbits = 4;
		if (tcolorbits == 24)
			sdlcolorbits = 8;

#ifdef __sgi /* Fix for SGIs grabbing too many bits of color */
		if (sdlcolorbits == 4)
			sdlcolorbits = 0; /* Use minimum size for 16-bit color */

		/* Need alpha or else SGIs choose 36+ bit RGB mode */
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 1);
#endif

		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, sdlcolorbits );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, sdlcolorbits );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, sdlcolorbits );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, tdepthbits );
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, tstencilbits );

		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, samples ? 1 : 0 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, samples );

		//if(r_stereoEnabled->integer)
		//{
		//	glConfig.stereoEnabled = true;
		//	SDL_GL_SetAttribute(SDL_GL_STEREO, 1);
		//}
		//else
		//{
		//	glConfig.stereoEnabled = false;
			SDL_GL_SetAttribute(SDL_GL_STEREO, 0);
		//}
		
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

#if 0 // See http://bugzilla.icculus.org/show_bug.cgi?id=3526
		// If not allowing software GL, demand accelerated
		if( !r_allowSoftwareGL->integer )
		{
			if( SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 ) < 0 )
			{
				g_core->Print( "Unable to guarantee accelerated "
						"visual with libSDL < 1.2.10\n" );
			}
		}
#endif

		if( SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 0/*r_swapInterval->integer*/ ) < 0 )
			g_core->Print( "r_swapInterval requires libSDL >= 1.2.10\n" );

//#ifdef USE_ICON
//		{
//			SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(
//					(void *)CLIENT_WINDOW_ICON.pixel_data,
//					CLIENT_WINDOW_ICON.width,
//					CLIENT_WINDOW_ICON.height,
//					CLIENT_WINDOW_ICON.bytes_per_pixel * 8,
//					CLIENT_WINDOW_ICON.bytes_per_pixel * CLIENT_WINDOW_ICON.width,
//#ifdef Q3_LITTLE_ENDIAN
//					0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
//#else
//					0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
//#endif
//					);
//
//			SDL_WM_SetIcon( icon, NULL );
//			SDL_FreeSurface( icon );
//		}
//#endif

		SDL_WM_SetCaption(CLIENT_WINDOW_TITLE, CLIENT_WINDOW_MIN_TITLE);
		SDL_ShowCursor(0);

		if (!(vidscreen = SDL_SetVideoMode(glConfig.vidWidth, glConfig.vidHeight, colorbits, flags)))
		{
			g_core->Print("SDL_SetVideoMode failed: %s\n", SDL_GetError( ) );
			continue;
		}

		opengl_context = GLimp_GetCurrentContext();

		g_core->Print( "Using %d/%d/%d Color bits, %d depth, %d stencil display.\n",
				sdlcolorbits, sdlcolorbits, sdlcolorbits, tdepthbits, tstencilbits);

		glConfig.colorBits = tcolorbits;
		glConfig.depthBits = tdepthbits;
		glConfig.stencilBits = tstencilbits;
		break;
	}

	GLimp_DetectAvailableModes();

	if (!vidscreen)
	{
		g_core->Print( "Couldn't get a visual\n" );
		return RSERR_INVALID_MODE;
	}

	screen = vidscreen;
	
//	const char *glstring = (char *) qglGetString (GL_RENDERER);
	//g_core->Print( "GL_RENDERER: %s\n", glstring );

	return RSERR_OK;
}

/*
===============
GLimp_StartDriverAndSetMode
===============
*/
static bool GLimp_StartDriverAndSetMode(int mode, bool fullscreen, bool noborder)
{
	int err;

	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		char driverName[ 64 ];

		if (SDL_Init(SDL_INIT_VIDEO) == -1)
		{
			g_core->Print( "SDL_Init( SDL_INIT_VIDEO ) FAILED (%s)\n",
					SDL_GetError());
			return false;
		}

		SDL_VideoDriverName( driverName, sizeof( driverName ) - 1 );
		g_core->Print( "SDL using driver \"%s\"\n", driverName );
		g_cvars->Cvar_Set( "r_sdlDriver", driverName );
	}

	if (fullscreen && g_cvars->Cvar_VariableIntegerValue( "in_nograb" ) )
	{
		g_core->Print( "Fullscreen not allowed with in_nograb 1\n");
		g_cvars->Cvar_Set( "r_fullscreen", "0" );
		r_fullscreen->modified = false;
		fullscreen = false;
	}
	
	err = GLimp_SetMode(mode, fullscreen, noborder);

	switch ( err )
	{
		case RSERR_INVALID_FULLSCREEN:
			g_core->Print( "...WARNING: fullscreen unavailable in this mode\n" );
			return false;
		case RSERR_INVALID_MODE:
			g_core->Print( "...WARNING: could not set the given mode (%d)\n", mode );
			return false;
		default:
			break;
	}

	return true;
}

#define R_MODE_FALLBACK 3 // 640 * 480

/*
===============
GLimp_Init

This routine is responsible for initializing the OS specific portions
of OpenGL
===============
*/
void GLimp_Init()
{
	r_sdlDriver = g_cvars->Cvar_Get( "r_sdlDriver", "", CVAR_ROM );
	r_allowResize = g_cvars->Cvar_Get( "r_allowResize", "0", CVAR_ARCHIVE );
	r_centerWindow = g_cvars->Cvar_Get( "r_centerWindow", "0", CVAR_ARCHIVE );
	r_fullscreen = g_cvars->Cvar_Get( "r_fullscreen", "0", CVAR_ARCHIVE );
	r_mode = g_cvars->Cvar_Get( "r_mode", "0", CVAR_ARCHIVE );

	if( g_cvars->Cvar_VariableIntegerValue( "com_abnormalExit" ) )
	{
		g_cvars->Cvar_Set( "r_mode", va( "%d", R_MODE_FALLBACK ) );
		g_cvars->Cvar_Set( "r_fullscreen", "0" );
		g_cvars->Cvar_Set( "r_centerWindow", "0" );
		g_cvars->Cvar_Set( "com_abnormalExit", "0" );
	}

// TODO?
#if 0
	Sys_SetEnv( "SDL_VIDEO_CENTERED", r_centerWindow->integer ? "1" : "" );
#endif

	//Sys_GLimpInit( ); // TODO

	// Create the window and set up the context
	if(GLimp_StartDriverAndSetMode(r_mode->integer, r_fullscreen->integer, /*r_noborder->integer*/0))
		goto success;

	// Try again, this time in a platform specific "safe mode"
	//Sys_GLimpSafeInit( ); // TODO

	if(GLimp_StartDriverAndSetMode(r_mode->integer, r_fullscreen->integer, false))
		goto success;

	// Finally, try the default screen resolution
	if( r_mode->integer != R_MODE_FALLBACK )
	{
		g_core->Print( "Setting r_mode %d failed, falling back on r_mode %d\n",
				r_mode->integer, R_MODE_FALLBACK );

		if(GLimp_StartDriverAndSetMode(R_MODE_FALLBACK, false, false))
			goto success;
	}

	// Nothing worked, give up
	g_core->Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem" );

success:
	// This values force the UI to disable driver selection
	glConfig.deviceSupportsGamma = SDL_SetGamma( 1.0f, 1.0f, 1.0f ) >= 0;

	// Mysteriously, if you use an NVidia graphics card and multiple monitors,
	// SDL_SetGamma will incorrectly return false... the first time; ask
	// again and you get the correct answer. This is a suspected driver bug, see
	// http://bugzilla.icculus.org/show_bug.cgi?id=4316
	glConfig.deviceSupportsGamma = SDL_SetGamma( 1.0f, 1.0f, 1.0f ) >= 0;

	// get our config strings
	//Q_strncpyz( glConfig.vendor_string, (char *) qglGetString (GL_VENDOR), sizeof( glConfig.vendor_string ) );
	//Q_strncpyz( glConfig.renderer_string, (char *) qglGetString (GL_RENDERER), sizeof( glConfig.renderer_string ) );
	//if (*glConfig.renderer_string && glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] == '\n')
	//	glConfig.renderer_string[strlen(glConfig.renderer_string) - 1] = 0;
	//Q_strncpyz( glConfig.version_string, (char *) qglGetString (GL_VERSION), sizeof( glConfig.version_string ) );
	//Q_strncpyz( glConfig.extensions_string, (char *) qglGetString (GL_EXTENSIONS), sizeof( glConfig.extensions_string ) );

	g_cvars->Cvar_Get( "r_availableModes", "", CVAR_ROM );

	// This depends on SDL_INIT_VIDEO, hence having it here
	g_inputSystem->IN_Init( );
}


/*
===============
GLimp_EndFrame

Responsible for doing a swapbuffers
===============
*/
void GLimp_EndFrame()
{
	// don't flip if drawing to front buffer
	//if ( stricmp( r_drawBuffer->string, "GL_FRONT" ) != 0 )
	//{
		SDL_GL_SwapBuffers();
	//}

	if( r_fullscreen->modified )
	{
		bool    fullscreen;
		bool    needToToggle = true;
		bool    sdlToggled = false;
		SDL_Surface *s = SDL_GetVideoSurface( );

		if( s )
		{
			// Find out the current state
			fullscreen = !!( s->flags & SDL_FULLSCREEN );
				
			if( r_fullscreen->integer && g_cvars->Cvar_VariableIntegerValue( "in_nograb" ) )
			{
				g_core->Print( "Fullscreen not allowed with in_nograb 1\n");
				g_cvars->Cvar_Set( "r_fullscreen", "0" );
				r_fullscreen->modified = false;
			}

			// Is the state we want different from the current state?
			needToToggle = !!r_fullscreen->integer != fullscreen;

			if( needToToggle )
				sdlToggled = SDL_WM_ToggleFullScreen( s );
		}

		if( needToToggle )
		{
			// SDL_WM_ToggleFullScreen didn't work, so do it the slow way
			if( !sdlToggled )
				g_core->Cbuf_ExecuteText(EXEC_APPEND, "vid_restart");

			g_inputSystem->IN_Restart( );
		}

		r_fullscreen->modified = false;
	}
}
#include <api/sdlSharedAPI.h>
class sdlSharedIMPL_c : public sdlSharedAPI_i {
public:
	virtual u32 getWinWidth() const {
		return glConfig.vidWidth;
	}
	virtual u32 getWinHeigth() const {
		return glConfig.vidHeight;
	}
	virtual void endFrame() {
		GLimp_EndFrame();
	}
	virtual void init() {
		GLimp_Init();
	}
	virtual void shutdown() {
		GLimp_Shutdown();
	}
};
static sdlSharedIMPL_c sharedSDLImpl;
sdlSharedAPI_i *g_sharedSDLAPI = &sharedSDLImpl;

#include <api/iFaceMgrAPI.h>
void SDLShared_InitSharedSDLAPI() {
	g_iFaceMan->registerInterface((iFaceBase_i *)(void*)g_sharedSDLAPI,SHARED_SDL_API_IDENTSTRING);
}