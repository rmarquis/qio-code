SET( SDL_SOURCES
    src/SDL.c
    src/events/SDL_active.c
    src/audio/SDL_audio.c
    src/audio/SDL_audiocvt.c
    src/video/SDL_blit.c
    src/video/SDL_blit_0.c
    src/video/SDL_blit_1.c
    src/video/SDL_blit_A.c
    src/video/SDL_blit_N.c
    src/video/SDL_bmp.c
    src/cdrom/SDL_cdrom.c
    src/cpuinfo/SDL_cpuinfo.c
    src/video/SDL_cursor.c
    src/audio/windib/SDL_dibaudio.c
    src/video/windib/SDL_dibevents.c
    src/video/windib/SDL_dibvideo.c
    src/audio/disk/SDL_diskaudio.c
    src/audio/dummy/SDL_dummyaudio.c
    src/audio/windx5/SDL_dx5audio.c
    src/video/Windx5/SDL_dx5events.c
    src/video/Windx5/SDL_dx5video.c
    src/video/windx5/SDL_dx5yuv.c
    src/SDL_error.c
    src/events/SDL_events.c
    src/events/SDL_expose.c
    src/SDL_fatal.c
    src/video/SDL_gamma.c
    src/stdlib/SDL_getenv.c
    src/stdlib/SDL_iconv.c
    src/joystick/SDL_joystick.c
    src/events/SDL_keyboard.c
    src/stdlib/SDL_malloc.c
    src/audio/SDL_mixer.c
    src/audio/SDL_mixer_MMX_VC.c
    src/joystick/win32/SDL_mmjoystick.c
    src/events/SDL_mouse.c
    src/video/dummy/SDL_nullevents.c
    src/video/dummy/SDL_nullmouse.c
    src/video/dummy/SDL_nullvideo.c
    src/video/SDL_pixels.c
    src/stdlib/SDL_qsort.c
    src/events/SDL_quit.c
    src/events/SDL_resize.c
    src/video/SDL_RLEaccel.c
    src/file/SDL_rwops.c
    src/stdlib/SDL_stdlib.c
    src/video/SDL_stretch.c
    src/stdlib/SDL_string.c
    src/video/SDL_surface.c
    src/cdrom/win32/SDL_syscdrom.c
    src/thread/generic/SDL_syscond.c
    src/video/wincommon/SDL_sysevents.c
    src/loadso/win32/SDL_sysloadso.c
    src/video/wincommon/SDL_sysmouse.c
    src/thread/win32/SDL_sysmutex.c
    src/thread/win32/SDL_syssem.c
    src/thread/win32/SDL_systhread.c
    src/timer/win32/SDL_systimer.c
    src/video/wincommon/SDL_syswm.c
    src/thread/SDL_thread.c
    src/timer/SDL_timer.c
    src/video/SDL_video.c
    src/audio/SDL_wave.c
    src/video/wincommon/SDL_wingl.c
    src/video/SDL_yuv.c
    src/video/SDL_yuv_sw.c
    src/audio/SDL_audio_c.h
    src/audio/SDL_audiomem.h
    src/video/SDL_blit.h
    src/video/SDL_cursor_c.h
    src/audio/windib/SDL_dibaudio.h
    src/video/windib/SDL_dibevents_c.h
    src/video/windib/SDL_dibvideo.h
    src/audio/disk/SDL_diskaudio.h
    src/audio/dummy/SDL_dummyaudio.h
    src/audio/windx5/SDL_dx5audio.h
    src/video/Windx5/SDL_dx5events_c.h
    src/video/Windx5/SDL_dx5video.h
    src/video/windx5/SDL_dx5yuv_c.h
    src/SDL_error_c.h
    src/events/SDL_events_c.h
    src/SDL_fatal.h
    src/joystick/SDL_joystick_c.h
    src/video/SDL_leaks.h
    src/video/wincommon/SDL_lowvideo.h
    src/video/dummy/SDL_nullevents_c.h
    src/video/dummy/SDL_nullmouse_c.h
    src/video/dummy/SDL_nullvideo.h
    src/video/SDL_pixels_c.h
    src/video/SDL_RLEaccel_c.h
    src/video/SDL_stretch_c.h
    src/audio/SDL_sysaudio.h
    src/cdrom/SDL_syscdrom.h
    src/events/SDL_sysevents.h
    src/joystick/SDL_sysjoystick.h
    src/video/wincommon/SDL_sysmouse_c.h
    src/thread/SDL_systhread.h
    src/thread/win32/SDL_systhread_c.h
    src/timer/SDL_systimer.h
    src/video/SDL_sysvideo.h
    src/video/wincommon/SDL_syswm_c.h
    src/thread/SDL_thread_c.h
    src/timer/SDL_timer_c.h
    src/video/windib/SDL_vkeys.h
    src/audio/SDL_wave.h
    src/video/wincommon/SDL_wingl_c.h
    src/video/SDL_yuv_sw_c.h
    src/video/SDL_yuvfuncs.h
    src/video/wincommon/Wmmsg.h
)

ADD_DEFINITIONS( -D_CRT_SECURE_NO_DEPRECATE -D_WINDOWS )
ADD_LIBRARY( SDL 
	SHARED
		${SDL_SOURCES} 
	)
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/code/external/SDL12/include)
	target_link_libraries( SDL user32.lib advapi32.lib winmm.lib wsock32.lib ws2_32.lib msvcrt.lib psapi.lib Gdi32.lib ${DXSDK_GUID_LIBRARY} )
	set_target_properties( SDL PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game
	)

if( WIN32 )
	set ( SDL_MAIN_SOURCES
		src/main/win32/SDL_win32_main.c
	)

	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/code/external/SDL12/include ${CMAKE_SOURCE_DIR}/code/external/SDL12/src )
	ADD_DEFINITIONS( -D_CRT_SECURE_NO_DEPRECATE -DWIN32 -D_WINDOWS )
	ADD_LIBRARY( SDLmain 
		${SDL_MAIN_SOURCES} 
	)
endif()