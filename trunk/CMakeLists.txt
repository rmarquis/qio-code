#
# QIO CMakeLists.txt
#
# cmake required (http://www.cmake.org/)
#

cmake_minimum_required( VERSION 3.0 )
MESSAGE( STATUS "Using cmake version ${CMAKE_VERSION}" )

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
set( CMAKE_C_FLAGS_MINSIZEREL "-DNDEBUG -Os" CACHE STRING "" )
set( CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O3 -ffast-math -fomit-frame-pointer -fno-strict-aliasing" CACHE STRING "" )
set( CMAKE_C_FLAGS_RELWITHDEBINFO "-DNDEBUG -O3 -ffast-math -fomit-frame-pointer -fno-strict-aliasing -g" CACHE STRING "" )
set( CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "" )
set( CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} CACHE STRING "" )
set( CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL} CACHE STRING "" )
set( CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} CACHE STRING "" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} CACHE STRING "" )
set( CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration (Debug, Release, RelWithDebInfo, MinSizeRel)" )

# This line has to appear before 'PROJECT' in order to be able to disable incremental linking
SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
set( MSVC_INCREMENTAL_DEFAULT ON )

project( QIO C CXX ASM )
set( CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER} )
set( CMAKE_ASM_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT} )
set( CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" ) # Need this to get rid of -rdynamic linker flag set by default by cmake

# Options
option( BUILD_MATERIALSYSTEM  "Build QIO material system libary"               ON )
#option( BUILD_IMAGELIB        "Build QIO image library"                        OFF )
if (WIN32)
#Dushan // only on Windows platform
#option( BUILD_DX12_RENDERER   "Build DirectX 12 renderer"                      OFF )
#option( BUILD_DX11_RENDERER   "Build DirectX 11 renderer"                      OFF )
option( BUILD_DX10_RENDERER   "Build DirectX 9 renderer"                      ON )
option( BUILD_DX9_RENDERER    "Build DirectX 9 renderer"                      ON )
endif(WIN32)
option( BUILD_GL_RENDERER     "Build new renderer library"                    OFF )
option( BUILD_NULL_RENDERER   "Build new renderer library"                    OFF )

# Determine arch.
SET( BUILD_ARCH "x86" )

# Package info
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "QIO client" )
set( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README" )
set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt" )
include( CPack )

set( MOUNT_DIR ${CMAKE_SOURCE_DIR}/code/ )
set( LIB_DIR ${CMAKE_SOURCE_DIR}/code/external )
if ( WIN32 )
set( REF_DIR ${CMAKE_SOURCE_DIR}/misc/msvc )
endif( WIN32 )

find_package( PkgConfig )

if( WIN32 )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

set( CONSOLE "tty" )

# Determine platform
if( WIN32 )
  set( PLATFORM "win32" )
  set( CONSOLE "win32" )
elseif( UNIX )
  if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    add_definitions( -DMACOS_X -DMACOS_X_ACCELERATION_HACK -D USE_FILE32API -DMACOS_X_CURSOR_HACK )
  endif()
  set( PLATFORM "unix" )
endif()

############################################
# Start of the list for Shared Build Files #
############################################

set( MATHLIST_SOURCES
	${MOUNT_DIR}/math/vec3.cpp
)

set( MATHLIST_HEADERS 
	${MOUNT_DIR}/math/vec3.h
)



#####################################################
# Start of the list for Material System Build Files #
#####################################################

set( SHAREDLIST_SOURCES
	${MOUNT_DIR}/shared/ast.cpp
	${MOUNT_DIR}/shared/autoCmd.cpp
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/shared/tableList.cpp
	${MOUNT_DIR}/shared/waveForm.cpp
)

set( SHAREDLIST_HEADERS
	${MOUNT_DIR}/shared/array.h
	${MOUNT_DIR}/shared/ast.h
	${MOUNT_DIR}/shared/autoCmd.h
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/hashTableTemplate.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/shared/safePtr.h
	${MOUNT_DIR}/shared/tableList.h
	${MOUNT_DIR}/shared/waveForm.h
)

set( MATERIALSYSTEMLIST_SOURCES
	${MOUNT_DIR}/materialSystem/mat_api.cpp
	${MOUNT_DIR}/materialSystem/mat_cubeMap.cpp
	${MOUNT_DIR}/materialSystem/mat_impl.cpp
	${MOUNT_DIR}/materialSystem/mat_main.cpp
	${MOUNT_DIR}/materialSystem/mat_rgbGen.cpp
	${MOUNT_DIR}/materialSystem/mat_stageTexture.cpp
	${MOUNT_DIR}/materialSystem/mat_texMods.cpp
	${MOUNT_DIR}/materialSystem/mat_textures.cpp
	${MOUNT_DIR}/materialSystem/mat_texturesScript.cpp
)

set( MATERIALSYSTEMLIST_HEADERS
	${MOUNT_DIR}/materialSystem/mat_cubeMap.h
	${MOUNT_DIR}/materialSystem/mat_impl.h
	${MOUNT_DIR}/materialSystem/mat_local.h
	${MOUNT_DIR}/materialSystem/mat_public.h
	${MOUNT_DIR}/materialSystem/mat_rgbGen.cpp
	${MOUNT_DIR}/materialSystem/mat_stageTexture.cpp
	${MOUNT_DIR}/materialSystem/mat_texMods.cpp
	${MOUNT_DIR}/materialSystem/mat_textures.cpp
	${MOUNT_DIR}/materialSystem/mat_texturesScript.cpp
)

set( MATERIALSYSTEMLIST_HEADERS
	${MOUNT_DIR}/materialSystem/mat_cubeMap.h
	${MOUNT_DIR}/materialSystem/mat_impl.h
	${MOUNT_DIR}/materialSystem/mat_local.h
	${MOUNT_DIR}/materialSystem/mat_public.h
	${MOUNT_DIR}/materialSystem/mat_rgbGen.cpp
	${MOUNT_DIR}/materialSystem/mat_stageTexture.cpp
	${MOUNT_DIR}/materialSystem/mat_texMods.cpp
	${MOUNT_DIR}/materialSystem/mat_textures.cpp
	${MOUNT_DIR}/materialSystem/mat_texturesScript.cpp
)

set( MATERIALSYSTEMLIST_HEADERS
	${MOUNT_DIR}/materialSystem/mat_cubeMap.h
	${MOUNT_DIR}/materialSystem/mat_impl.h
	${MOUNT_DIR}/materialSystem/mat_local.h
	${MOUNT_DIR}/materialSystem/mat_public.h
	${MOUNT_DIR}/materialSystem/mat_rgbGen.cpp
	${MOUNT_DIR}/materialSystem/mat_stageTexture.cpp
	${MOUNT_DIR}/materialSystem/mat_texMods.cpp
	${MOUNT_DIR}/materialSystem/mat_textures.cpp
	${MOUNT_DIR}/materialSystem/mat_texturesScript.cpp
)

########################################################
# End of the file list for Material System Build Files #
########################################################

#####################################################
# Start of the list for Material System Build Files #
#####################################################

set( IMAGELIB_SOURCES
	${MOUNT_DIR}/imageLib/img_api.cpp
	${MOUNT_DIR}/imageLib/img_convert.cpp
	${MOUNT_DIR}/imageLib/img_devil.cpp
	${MOUNT_DIR}/imageLib/img_main.cpp
	${MOUNT_DIR}/imageLib/img_utils.cpp
	${MOUNT_DIR}/imageLib/img_write.cpp
)

set( IMAGELIB_HEADERS 
	${MOUNT_DIR}/imageLib/img_local.h
)

#############################################################
# End of the file list for Image Library System Build Files #
#############################################################

################################################
# Start of the list for DirectX 10 Build Files #
################################################

set( DIRECTX10LIB_SOURCES
	${MOUNT_DIR}/renderer/backEndDX10/backendDX10API.cpp
)

set( DIRECTXANDNULLRENDERER_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/qcommon/q_shared.h
)

set( DIRECTXANDNULLRENDERER_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
)

###################################################
# Start of the list for NULL renderer Build Files #
###################################################

set( NULLRENDERER_SOURCES
	${MOUNT_DIR}/renderer/backEndNULL/backEndNULLAPI.cpp
)

###############################################
# Start of the list for DirectX 9 Build Files #
###############################################

set( DIRECTX9LIB_HEADERS 
	${MOUNT_DIR}/renderer/backEndDX9/dx9_local.h
	${MOUNT_DIR}/renderer/backEndDX9/dx9_shader.h
)

set( DIRECTX9LIB_SOURCES
	${MOUNT_DIR}/renderer/backEndDX9/backendDX9API.cpp
	${MOUNT_DIR}/renderer/backEndDX9/dx9_shader.cpp
)

set( DIRECTX9LIB_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/calcTBN.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/renderer/rVertexBuffer.h
)

set( DIRECTX9LIB_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/calcTBN.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/renderer/rVertexBuffer.cpp
)

if( WIN32 )
  add_definitions( -DWINVER=0x501 )
  #set( OS_LIBRARIES m winmm wsock32 gdi32 ole32 ws2_32 psapi Iphlpapi advapi32 shell32 user32 )
  set( OS_LIBRARIES m winmm ws2_32 psapi libz )
  set( EXTRALIBS dinput dsound )
elseif( UNIX )
  if( CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
    set( OS_LIBRARIES m z )
  elseif( CMAKE_SYSTEM_NAME STREQUAL "Darwin" )
    set( OS_LIBRARIES dl m z )
    # FIXME This looks wrong.
    set( OS_LIBRARIES ${OS_LIBRARIES} "-framework AGL -framework OpenGL -framework Carbon -framework IOKit" )
  else()
    set( OS_LIBRARIES ${CMAKE_DL_LIBS} m z rt )
  endif()
endif()


if( BUILD_MATERIALSYSTEM )
	add_library( materialSystem 
		SHARED 
		${MATHLIST_SOURCES} 
		${MATHLIST_HEADERS} 
		${SHAREDLIST_SOURCES} 
		${SHAREDLIST_HEADERS} 
		${MATERIALSYSTEMLIST_SOURCES} 
		${MATERIALSYSTEMLIST_HEADERS} 
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
		set_target_properties( materialSystem PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/materialSystem.def" OUTPUT_NAME "materialSystem${BUILD_ARCH}" PREFIX "" )
	else()
		set_target_properties( materialSystem PROPERTIES OUTPUT_NAME "materialSystemx${BUILD_ARCH}" PREFIX "" )
	target_link_libraries( materialSystem -lm )
	endif()
endif()

if( BUILD_IMAGELIB )
	# This section finds and provides REQUIRED dependencies to imageLib target
	#find_package( libDevil REQUIRED )
	#find_package( vtflib REQUIRED )

	add_library( imageLib 
		SHARED 
		${IMAGELIB_SOURCES} 
		${IMAGELIB_HEADERS} 
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
		set_target_properties( imageLib PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/imageLib.def" OUTPUT_NAME "imageLib${BUILD_ARCH}" PREFIX "" )
        include_directories( ${LIB_DIR}/il-1.7.8-x86/Include ${LIB_DIR}/vtflib132-bin/include )
	else()
		set_target_properties( imageLib PROPERTIES OUTPUT_NAME "imageLibx${BUILD_ARCH}" PREFIX "" )
	target_link_libraries( imageLib -lm )
	endif()
endif()

if( WIN32 )
	if( BUILD_DX9_RENDERER )
		# This section finds and provides REQUIRED dependencies to imageLib target
		find_package( DirectX REQUIRED )

		add_library( backendDX9 
			SHARED 
			${MATHLIST_SOURCES} 
			${MATHLIST_HEADERS} 
			${DIRECTX9LIB_SOURCES}
			${DIRECTX9LIB_HEADERS}
			${DIRECTX9LIB_SHARED_HEADERS}
			${DIRECTX9LIB_SHARED_SOURCES}
		)
		include_directories( ${MOUNT_DIR} )
		if( WIN32 )
			set_target_properties( backendDX9 PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendDX9.def" OUTPUT_NAME "backendDX9${BUILD_ARCH}" PREFIX "" )
			include_directories( dll ${DXSDK_INCLUDE_DIR} ${LIB_DIR}/SDL12/include )
			target_link_libraries( backendDX9 ${DXSDK_D3DX9_LIBRARY} ${DXSDK_D3D9_LIBRARY} -lm )
		endif()
	endif()

	if( BUILD_DX10_RENDERER )
		# This section finds and provides REQUIRED dependencies to imageLib target
		find_package( DirectX REQUIRED )

		add_library( backendDX10
			SHARED 
			${DIRECTX10LIB_SOURCES}
			${DIRECTXANDNULLRENDERER_SHARED_HEADERS}
			${DIRECTXANDNULLRENDERER_SHARED_SOURCES}
		)
		include_directories( ${MOUNT_DIR} )
		set_target_properties( backendDX10 PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendDX10.def" OUTPUT_NAME "backendDX10${BUILD_ARCH}" PREFIX "" )
		include_directories( dll ${DXSDK_INCLUDE_DIR} ${LIB_DIR}/SDL12/include )
		target_link_libraries( backendDX10 ${DXSDK_D3DX10_LIBRARY} ${DXSDK_D3D10_LIBRARY} -lm )
	endif()
endif() ## WIN32

if( BUILD_NULL_RENDERER )

	add_library( backendNULL
		SHARED 
		${NULLRENDERER_SOURCES}
		${DIRECTXANDNULLRENDERER_SHARED_HEADERS}
		${DIRECTXANDNULLRENDERER_SHARED_SOURCES}
	)
	include_directories( ${MOUNT_DIR} )
	set_target_properties( backendNULL PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendNULL.def" OUTPUT_NAME "backendNULL${BUILD_ARCH}" PREFIX "" )
	include_directories( dll ${LIB_DIR}/SDL12/include )
	target_link_libraries( backendNULL ${DXSDK_D3DX9_LIBRARY} ${DXSDK_D3D9_LIBRARY} -lm )
endif()
