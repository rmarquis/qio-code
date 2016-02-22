#
# QIO CMakeLists.txt
#
# cMake required (http://www.cmake.org/)
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
set( CMAKE_CXX_STACK_SIZE "8388608" )
set( CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration (Debug, Release, RelWithDebInfo, MinSizeRel)" )

# This line has to appear before 'PROJECT' in order to be able to disable incremental linking
SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
set( MSVC_INCREMENTAL_DEFAULT ON )

project( QIO C CXX ASM )
set( CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER} )
set( CMAKE_ASM_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT} )
set( CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" ) # Need this to get rid of -rdynamic linker flag set by default by cmake

# Options
if (WIN32)
#Dushan // only on Windows platform
#option( BUILD_DX12_RENDERER   "Build DirectX 12 renderer"                      OFF )
#option( BUILD_DX11_RENDERER   "Build DirectX 11 renderer"                      OFF )
option( BUILD_DX10_RENDERER   "Build DirectX 10 renderer"                      ON )
option( BUILD_DX9_RENDERER    "Build DirectX 9 renderer"                       ON )
endif(WIN32)
option( BUILD_GL_RENDERER     "Build new renderer library"                     ON )
option( BUILD_NULL_RENDERER   "Build new renderer library"                     ON )
option( BUILD_RENDERER        "Build renderer library"                         ON )
option( BUILD_MATERIALSYSTEM  "Build QIO material system library"              ON )
option( BUILD_IMAGELIB        "Build QIO image library"                        ON )
option( BUILD_MODELLOADER     "Build QIO model loader library"                 ON )
option( BUILD_CORE            "Build QIO core library"                         ON )
option( BUILD_CM              "Build QIO cm library"                           ON )
option( BUILD_DECLMANAGER     "Build QUI decal manager libery"                 ON )
option( BUILD_GAME            "Build QIO game module library"                  ON )
option( BUILD_CGAME           "Build QIO cgame module library"                 ON )
option( BUILD_ENGINE          "Build QIO engine binary"                        ON )
if (WIN32)
#Dushan - only on Windows at this moment
#there are compiling problems with VS2015
#option( BUILD_EDITOR          "Build QIO editor module library"                ON )
endif(WIN32)
option( BUILD_GPHYSICS_BULLET "Build QIO Bullet Physics module library"        ON )
option( BUILD_GPHYSICS_ODE    "Build QIO ODE Physics module library"           ON )
#option( BUILD_SOUND           "Build QIO sound module library"                OFF )
#option( BUILD_GUI             "Build QIO GUI module library"                  OFF )
#option( BUILD_RECAST          "Build QIO Recast module library"               OFF )

# Determine arch.
# Dushan - only x86 is supported at this moment
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
set( ENGINEBUILD_DIR ${CMAKE_SOURCE_DIR}/game/)

#Dushan - Add library modules provided by QIO Engine
ADD_SUBDIRECTORY( ${LIB_DIR}/ )

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
	${MOUNT_DIR}/imageLib/img_webp.cpp
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

set ( RENDERERGL_APIS_HEADERS
	${MOUNT_DIR}/api/tableListAPI.h
)

set ( RENDERERGL_SHARED_HEADERS
	${MOUNT_DIR}/shared/ast_input.h
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/byteRGB.h
	${MOUNT_DIR}/shared/calcTBN.h
	${MOUNT_DIR}/shared/cullType.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/renderer/rVertexBuffer.h
)

set ( RENDERERGL_SHARED_SOURCES
	${MOUNT_DIR}/shared/ast_input.cpp
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/calcTBN.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/renderer/rVertexBuffer.cpp
)

set ( RENDERERGL_HEADERS
	${MOUNT_DIR}/renderer/backEndGL/gl_local.h
	${MOUNT_DIR}/renderer/backEndGL/gl_shader.h
)

set ( RENDERERGL_SOURCES
	${MOUNT_DIR}/renderer/backEndGL/backEndGLAPI.cpp
	${MOUNT_DIR}/renderer/backEndGL/gl_main.cpp
	${MOUNT_DIR}/renderer/backEndGL/gl_shader.cpp
)

set ( IMGLIB_HEADERS
	${MOUNT_DIR}/imageLib/img_local.h
)

set ( IMGLIB_SOURCES
	${MOUNT_DIR}/imageLib/img_api.cpp
	${MOUNT_DIR}/imageLib/img_convert.cpp
	${MOUNT_DIR}/imageLib/img_devil.cpp
	${MOUNT_DIR}/imageLib/img_main.cpp
	${MOUNT_DIR}/imageLib/img_utils.cpp
	${MOUNT_DIR}/imageLib/img_write.cpp
)

set ( RENDERERAPI_HEADERS
	${MOUNT_DIR}/api/boxTrianglesCallback.h
	${MOUNT_DIR}/api/cubeMapAPI.h
	${MOUNT_DIR}/api/customRenderObjectAPI.h
	${MOUNT_DIR}/api/mtrAPI.h
	${MOUNT_DIR}/api/mtrStageAPI.h
	${MOUNT_DIR}/api/noise2DAPI.h
	${MOUNT_DIR}/api/occlusionQueryAPI.h
	${MOUNT_DIR}/api/rAPI.h
	${MOUNT_DIR}/api/rbAPI.h
	${MOUNT_DIR}/api/readStreamAPI.h
	${MOUNT_DIR}/api/rEntityAPI.h
	${MOUNT_DIR}/api/rLightAPI.h
)

set ( RENDERERFILEFORMATS_HEADERS
	${MOUNT_DIR}/fileformats/bspFileFormat.h
	${MOUNT_DIR}/fileformats/bspFileFormat_hl.h
	${MOUNT_DIR}/fileformats/bspFileFormat_hl2.h
	${MOUNT_DIR}/fileformats/bspFileFormat_q2.h
)

set ( RENDERER_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/renderer/cameraDef.h
	${MOUNT_DIR}/renderer/drawCallSort.h
	${MOUNT_DIR}/renderer/pointLightSample.h
	${MOUNT_DIR}/shared/rendererSurfaceRef.h
	${MOUNT_DIR}/renderer/frontend/rf_2d.h
	${MOUNT_DIR}/renderer/frontend/rf_anims.h
	${MOUNT_DIR}/renderer/frontend/rf_bezier.h
	${MOUNT_DIR}/renderer/frontend/rf_bsp.h
	${MOUNT_DIR}/renderer/frontend/rf_decalProjector.h
	${MOUNT_DIR}/renderer/frontend/rf_decals.h
	${MOUNT_DIR}/renderer/frontend/rf_drawCall.h
	${MOUNT_DIR}/renderer/frontend/rf_entities.h
	${MOUNT_DIR}/renderer/frontend/rf_lights.h
	${MOUNT_DIR}/renderer/frontend/rf_lightGrid.h
	${MOUNT_DIR}/renderer/frontend/rf_local.h
	${MOUNT_DIR}/renderer/frontend/rf_model.h
	${MOUNT_DIR}/renderer/frontend/rf_proc.h
	${MOUNT_DIR}/renderer/frontend/rf_shadowVolume.h
	${MOUNT_DIR}/renderer/frontend/rf_skin.h
	${MOUNT_DIR}/renderer/frontend/rf_stencilShadowCaster.h
	${MOUNT_DIR}/renderer/frontend/rf_sunLight.h
	${MOUNT_DIR}/renderer/frontend/rf_skin.h
	${MOUNT_DIR}/renderer/frontend/rf_stencilShadowCaster.h
	${MOUNT_DIR}/renderer/frontend/rf_sunLight.h
	${MOUNT_DIR}/renderer/frontend/rf_world.h
	${MOUNT_DIR}/renderer/rfSurfsFlagsArray.h
	${MOUNT_DIR}/renderer/rIndexBuffer.h
	${MOUNT_DIR}/renderer/rModelAPI.h
	${MOUNT_DIR}/renderer/rPointBuffer.h
	${MOUNT_DIR}/renderer/rVertex.h
	${MOUNT_DIR}/renderer/rVertexBuffer.h
)

set ( RENDERER_MATH_HEADERS
	${MOUNT_DIR}/math/aabb.h
	${MOUNT_DIR}/math/axis.h
	${MOUNT_DIR}/math/frustum.h
	${MOUNT_DIR}/math/frustumExt.h
	${MOUNT_DIR}/shared/fcolor4.h
	${MOUNT_DIR}/math/math.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/math/plane.h
	${MOUNT_DIR}/math/vec2.h
	${MOUNT_DIR}/math/vec3.h
)

set ( RENDERER_MATH_SOURCES
	${MOUNT_DIR}/math/aabb.cpp
	${MOUNT_DIR}/math/axis.cpp
	${MOUNT_DIR}/math/frustum.cpp
	${MOUNT_DIR}/math/frustumExt.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/plane.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

set ( RENDERER_SHARED_HEADERS
	${MOUNT_DIR}/shared/afRagdollHelper.h
	${MOUNT_DIR}/shared/animationController.h
	${MOUNT_DIR}/shared/autoCmd.h
	${MOUNT_DIR}/shared/bitSet.h
	${MOUNT_DIR}/shared/boxAreasCacher.h
	${MOUNT_DIR}/shared/calcTBN.h
	${MOUNT_DIR}/shared/charFile.h
	${MOUNT_DIR}/shared/cmSurface.h
	${MOUNT_DIR}/shared/cmTriSoupOctTree.h
	${MOUNT_DIR}/shared/cmWinding.h
	${MOUNT_DIR}/shared/collisionUtils.h
	${MOUNT_DIR}/shared/cullResult.h
	${MOUNT_DIR}/shared/displacementBuilder.h
	${MOUNT_DIR}/shared/entDefsList.h
	${MOUNT_DIR}/shared/ePairsList.h
	${MOUNT_DIR}/shared/etCharMgr.h
	${MOUNT_DIR}/shared/flags32.h
	${MOUNT_DIR}/shared/kfAnimCtrl.h
	${MOUNT_DIR}/shared/mapBezierPatch.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/perlinNoise.h
	${MOUNT_DIR}/shared/perStringCallback.h
	${MOUNT_DIR}/shared/planeArray.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/shared/r2dVert.h
	${MOUNT_DIR}/shared/readStream.h
	${MOUNT_DIR}/renderer/rfSurfFlags.h
	${MOUNT_DIR}/shared/simpleTexturedPoly.h
	${MOUNT_DIR}/shared/simpleVert.h
	${MOUNT_DIR}/shared/singleAnimLerp.h
	${MOUNT_DIR}/shared/skelUtils.h
	${MOUNT_DIR}/shared/texCoordCalc.h
	${MOUNT_DIR}/shared/trace.h
	${MOUNT_DIR}/shared/vec3hash.h
	${MOUNT_DIR}/shared/wolfAnimCfg.h
	${MOUNT_DIR}/shared/wolfAnimScript.h
)

set ( RENDERER_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/afRagdollHelper.cpp
	${MOUNT_DIR}/shared/animationController.cpp
	${MOUNT_DIR}/shared/autoCmd.cpp
	${MOUNT_DIR}/shared/boxAreasCacher.cpp
	${MOUNT_DIR}/shared/calcTBN.cpp
	${MOUNT_DIR}/shared/charFile.cpp
	${MOUNT_DIR}/shared/cmSurface.cpp
	${MOUNT_DIR}/shared/cmTriSoupOctTree.cpp
	${MOUNT_DIR}/shared/cmWinding.cpp
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/displacementBuilder.cpp
	${MOUNT_DIR}/shared/entDefsList.cpp
	${MOUNT_DIR}/shared/ePairsList.cpp
	${MOUNT_DIR}/shared/etCharMgr.cpp
	${MOUNT_DIR}/shared/mapBezierPatch.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/perlinNoise.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/shared/readStream.cpp
	${MOUNT_DIR}/shared/readStream.cpp
	${MOUNT_DIR}/renderer/rVertexBuffer.cpp
	${MOUNT_DIR}/shared/skelUtils.cpp
	${MOUNT_DIR}/shared/tagOr.cpp
	${MOUNT_DIR}/shared/trace.cpp
	${MOUNT_DIR}/shared/wolfAnimCfg.cpp
	${MOUNT_DIR}/shared/wolfAnimScript.cpp
)

set ( RENDERER_SOURCES
	${MOUNT_DIR}/renderer/frontend/rf_2d.cpp
	${MOUNT_DIR}/renderer/frontend/rf_anims.cpp
	${MOUNT_DIR}/renderer/frontend/rf_api.cpp
	${MOUNT_DIR}/renderer/frontend/rf_bezier.cpp
	${MOUNT_DIR}/renderer/frontend/rf_bsp.cpp
	${MOUNT_DIR}/renderer/frontend/rf_cubeMap.cpp
	${MOUNT_DIR}/renderer/frontend/rf_custom.cpp
	${MOUNT_DIR}/renderer/frontend/rf_debugDrawIMPL.cpp
	${MOUNT_DIR}/renderer/frontend/rf_debugDrawing.cpp
	${MOUNT_DIR}/renderer/frontend/rf_decalProjector.cpp
	${MOUNT_DIR}/renderer/frontend/rf_decals.cpp
	${MOUNT_DIR}/renderer/frontend/rf_drawCall.cpp
	${MOUNT_DIR}/renderer/frontend/rf_entities.cpp
	${MOUNT_DIR}/renderer/frontend/rf_explosions.cpp
	${MOUNT_DIR}/renderer/frontend/rf_lightGrid.cpp
	${MOUNT_DIR}/renderer/frontend/rf_lightGrid_debug.cpp
	${MOUNT_DIR}/renderer/frontend/rf_lights.cpp
	${MOUNT_DIR}/renderer/frontend/rf_main.cpp
	${MOUNT_DIR}/renderer/frontend/rf_map.cpp
	${MOUNT_DIR}/renderer/frontend/rf_model.cpp
	${MOUNT_DIR}/renderer/frontend/rf_proc.cpp
	${MOUNT_DIR}/renderer/frontend/rf_shadowVolume.cpp
	${MOUNT_DIR}/renderer/frontend/rf_skin.cpp
	${MOUNT_DIR}/renderer/frontend/rf_sky.cpp
	${MOUNT_DIR}/renderer/frontend/rf_staticModel.cpp
	${MOUNT_DIR}/renderer/frontend/rf_stencilShadowCaster.cpp
	${MOUNT_DIR}/renderer/frontend/rf_sunLight.cpp
	${MOUNT_DIR}/renderer/frontend/rf_surface.cpp
	${MOUNT_DIR}/renderer/frontend/rf_terrain.cpp
	${MOUNT_DIR}/renderer/frontend/rf_water.cpp
	${MOUNT_DIR}/renderer/frontend/rf_world.cpp
)

set ( MODELLOADER_API
	${MOUNT_DIR}/api/kfModelAPI.h
	${MOUNT_DIR}/api/modelLoaderDLLAPI.h
	${MOUNT_DIR}/api/skelAnimAPI.h
	${MOUNT_DIR}/api/skelAnimPostProcessFuncs.h
	${MOUNT_DIR}/api/skelModelAPI.h
	${MOUNT_DIR}/api/staticModelCreatorAPI.h
)

set ( MODELLOADERFILEFORMATS 
	${MOUNT_DIR}/fileformats/3ds_file_format.h
	${MOUNT_DIR}/fileformats/actorX_file_format.h
	${MOUNT_DIR}/fileformats/md2_file_format.h
	${MOUNT_DIR}/fileformats/md3_file_format.h
	${MOUNT_DIR}/fileformats/md5r_file_format.h
	${MOUNT_DIR}/fileformats/mdc_file_format.h
	${MOUNT_DIR}/fileformats/mdm_file_format.h
	${MOUNT_DIR}/fileformats/mds_file_format.h
	${MOUNT_DIR}/fileformats/mdx_file_format.h
	${MOUNT_DIR}/fileformats/skab_file_format.h
	${MOUNT_DIR}/fileformats/vvdFileFormat.h
)

set ( MODELLOADER_HEADERS
	${MOUNT_DIR}/modelLoader/hl2MDLReader.h
	${MOUNT_DIR}/modelLoader/modelLoaderLocal.h
)

set ( MODELLOADER_KEYFRAMEDMODELLOADERS_HEADERS
	${MOUNT_DIR}/modelLoader/keyFramedModelImpl.h
)
set ( MODELLOADER_KEYFRAMEDMODELLOADERS_SOURCES
	${MOUNT_DIR}/modelLoader/keyFramedModelImpl.cpp
)

set ( MODELLOADER_MATH_HEADERS
	${MOUNT_DIR}/math/aabb.h
	${MOUNT_DIR}/math/axis.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/math/quat.h
	${MOUNT_DIR}/math/vec2.h
	${MOUNT_DIR}/math/vec3.h
)

set ( MODELLOADER_MATH_SOURCES
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

set ( MODELLOADER_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/cmWinding.h
	${MOUNT_DIR}/shared/extraSurfEdgesData.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/shared/readStream.h
	${MOUNT_DIR}/shared/skelUtils.h
	${MOUNT_DIR}/shared/stringHashTable.h
	${MOUNT_DIR}/shared/tagOr.h
	${MOUNT_DIR}/shared/textureAxisFromNormal.h
)

set ( MODELLOADER_SHARED_SOURCES 
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/cmWinding.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/shared/readStream.cpp
	${MOUNT_DIR}/shared/skelUtils.cpp
	${MOUNT_DIR}/shared/textureAxisFromNormal.cpp
)

set ( MODELLOADER_SKELETALMODELLOADERS
	${MOUNT_DIR}/modelLoader/sk_local.h
	${MOUNT_DIR}/modelLoader/skelAnimImpl.cpp
	${MOUNT_DIR}/modelLoader/skelAnimImpl.h
	${MOUNT_DIR}/modelLoader/skelModelImpl.cpp
	${MOUNT_DIR}/modelLoader/skelModelImpl.h
)

set ( MODELLOADER_SOURCES
	${MOUNT_DIR}/modelLoader/hl2MDLReader.cpp
	${MOUNT_DIR}/modelLoader/mod_postProcess.cpp
	${MOUNT_DIR}/modelLoader/modelLoaderAPI.cpp
	${MOUNT_DIR}/modelLoader/skel_animPostProcess.cpp
)

set ( MODELLOADER_STATICMODELLOADERS_HEADERS
	${MOUNT_DIR}/modelLoader/staticModelLoaders/mtlFile.h
)

set ( MODELLOADER_STATICMODELLOADERS_SOURCES
	${MOUNT_DIR}/modelLoader/staticModelLoaders/3dsLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/aseLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/heightmapLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/lwoLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/mapFileConverter.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/md3StaticLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/mtlFile.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/wavefrontOBJModelLoader.cpp
)

set ( CM_HEADERS 
	${MOUNT_DIR}/cm/cm_anims.h
	${MOUNT_DIR}/cm/cm_helper.h
	${MOUNT_DIR}/cm/cm_local.h
	${MOUNT_DIR}/cm/cm_model.h
	${MOUNT_DIR}/api/cmAPI.h
)

set ( CM_SHARED
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.cpp
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.h
	${MOUNT_DIR}/shared/cmBezierPatch.cpp
	${MOUNT_DIR}/shared/cmBezierPatch.h
	${MOUNT_DIR}/shared/cmBrush.cpp
	${MOUNT_DIR}/shared/cmBrush.h
	${MOUNT_DIR}/shared/cmSurface.cpp
	${MOUNT_DIR}/shared/cmSurface.h
	${MOUNT_DIR}/shared/cmTriSoupOctTree.cpp
	${MOUNT_DIR}/shared/cmTriSoupOctTree.h
	${MOUNT_DIR}/shared/cmWinding.cpp
	${MOUNT_DIR}/shared/cmWinding.h
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/collisionUtils.h
	${MOUNT_DIR}/api/colMeshBuilderAPI.h
	${MOUNT_DIR}/shared/displacementBuilder.cpp
	${MOUNT_DIR}/shared/displacementBuilder.h
	${MOUNT_DIR}/shared/ePairsList.cpp
	${MOUNT_DIR}/shared/ePairsList.h
	${MOUNT_DIR}/shared/mapBezierPatch.cpp
	${MOUNT_DIR}/shared/mapBezierPatch.h
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/fileStreamHelper.cpp
	${MOUNT_DIR}/shared/fileStreamHelper.h
	${MOUNT_DIR}/shared/perPlaneCallback.h
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/shared/readStream.cpp
	${MOUNT_DIR}/shared/readStream.h
	${MOUNT_DIR}/shared/trace.cpp
	${MOUNT_DIR}/shared/trace.h
	${MOUNT_DIR}/api/writeStreamAPI.h
)

set ( CM_SOURCE
	${MOUNT_DIR}/cm/cm_anims.cpp
	${MOUNT_DIR}/cm/cm_api.cpp
	${MOUNT_DIR}/cm/cm_cmds.cpp
	${MOUNT_DIR}/cm/cm_helper.cpp
	${MOUNT_DIR}/cm/cm_inlineBSPModel.cpp
	${MOUNT_DIR}/cm/cm_model.cpp
	${MOUNT_DIR}/cm/cm_model_impl.cpp
	${MOUNT_DIR}/cm/cm_modelLoaderWrapper.cpp
	${MOUNT_DIR}/cm/cm_phy.cpp
	${MOUNT_DIR}/cm/cm_proc.cpp
	${MOUNT_DIR}/cm/cm_world.cpp
)

set ( DECLMANAGER_API
	${MOUNT_DIR}/api/afDeclAPI.h
	${MOUNT_DIR}/api/declManagerAPI.h
	${MOUNT_DIR}/api/entDefAPI.h
	${MOUNT_DIR}/api/entityDeclAPI.h
	${MOUNT_DIR}/api/modelDeclAPI.h
	${MOUNT_DIR}/api/particleDeclAPI.h
	${MOUNT_DIR}/api/q3PlayerModelDeclAPI.h
)

set ( DECLMANAGER_HEADER
	${MOUNT_DIR}/declManager/articulatedFigure.h
	${MOUNT_DIR}/declManager/declManagerIMPL.h
	${MOUNT_DIR}/declManager/declRefState.h
	${MOUNT_DIR}/declManager/particleDecl.h
	${MOUNT_DIR}/declManager/q3PlayerModelDecl.h
)

set ( DECLMANAGER_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/entDef.h
	${MOUNT_DIR}/shared/ePairsList.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/shared/quake3AnimationConfig.h
	${MOUNT_DIR}/shared/random.h
	${MOUNT_DIR}/shared/stringList.h
	${MOUNT_DIR}/shared/tagOr.h
)

set ( DECLMANAGER_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/entDef.cpp
	${MOUNT_DIR}/shared/ePairsList.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/shared/quake3AnimationConfig.cpp
	${MOUNT_DIR}/shared/random.cpp
	${MOUNT_DIR}/shared/tagOr.cpp
)

set ( DECLMANAGER_SOURCE
	${MOUNT_DIR}/declManager/articulatedFigure.cpp
	${MOUNT_DIR}/declManager/declManagerIMPL.cpp
	${MOUNT_DIR}/declManager/particleDecl.cpp
	${MOUNT_DIR}/declManager/q3PlayerModelDecl.cpp
)

set ( GAME_CLASSES_CSS_HEADERS
	${MOUNT_DIR}/game/classes/WeaponMP5.h
)

set ( GAME_CLASSES_CSS_SOURCES
	${MOUNT_DIR}/game/classes/WeaponMP5.cpp
)

set ( GAME_CLASSES_ET_HEADERS
	${MOUNT_DIR}/game/classes/Corona.h
)

set ( GAME_CLASSES_ET_SOURCES
	${MOUNT_DIR}/game/classes/Corona.cpp
)

set ( GAME_CLASSES_QUAKE3_HEADERS
	${MOUNT_DIR}/game/classes/PortalCamera.h
	${MOUNT_DIR}/game/classes/PortalSurface.h
	${MOUNT_DIR}/game/classes/Q3Weapon.h
	${MOUNT_DIR}/game/classes/TriggerPush.h
	${MOUNT_DIR}/game/classes/TriggerTeleport.h
)

set ( GAME_CLASSES_QUAKE3_SOURCES
	${MOUNT_DIR}/game/classes/PortalCamera.cpp
	${MOUNT_DIR}/game/classes/PortalSurface.cpp
	${MOUNT_DIR}/game/classes/Q3Weapon.cpp
	${MOUNT_DIR}/game/classes/TriggerPush.cpp
	${MOUNT_DIR}/game/classes/TriggerTeleport.cpp
)

set ( GAME_CLASSES_RTCW_HEADERS
	${MOUNT_DIR}/game/classes/AI_Loper.h
	${MOUNT_DIR}/game/classes/AI_RTCW_Base.h
	${MOUNT_DIR}/game/classes/AI_Soldier.h
)

set ( GAME_CLASSES_RTCW_SOURCES
	${MOUNT_DIR}/game/classes/AI_Loper.cpp
	${MOUNT_DIR}/game/classes/AI_RTCW_Base.cpp
	${MOUNT_DIR}/game/classes/AI_Soldier.cpp
)

set ( GAME_CLASSES_HEADERS
	${MOUNT_DIR}/game/classes/Actor.h
	${MOUNT_DIR}/game/classes/BaseEntity.h
	${MOUNT_DIR}/game/classes/Button.h
	${MOUNT_DIR}/game/classes/Constraint.h
	${MOUNT_DIR}/game/classes/Door.h
	${MOUNT_DIR}/game/classes/FakePlayer.h
	${MOUNT_DIR}/game/classes/FuncBombTarget.h
	${MOUNT_DIR}/game/classes/FuncBuyZone.h
	${MOUNT_DIR}/game/classes/FuncDustmotes.h
	${MOUNT_DIR}/game/classes/FuncRotating.h
	${MOUNT_DIR}/game/classes/FuncSmokeVolume.h
	${MOUNT_DIR}/game/classes/FuncStatic.h
	${MOUNT_DIR}/game/classes/InfoPathNode.h
	${MOUNT_DIR}/game/classes/Light.h
	${MOUNT_DIR}/game/classes/Location.h
	${MOUNT_DIR}/game/classes/Logic_Blinker.h
	${MOUNT_DIR}/game/classes/Mover.h
	${MOUNT_DIR}/game/classes/ModelEntity.h
	${MOUNT_DIR}/game/classes/Player.h
	${MOUNT_DIR}/game/classes/Projectile.h
	${MOUNT_DIR}/game/classes/Prop_Physics.h
	${MOUNT_DIR}/game/classes/QioSpawner.h
	${MOUNT_DIR}/game/classes/RotatingDoor.h
	${MOUNT_DIR}/game/classes/Terrain.h
	${MOUNT_DIR}/game/classes/Trigger.h
	${MOUNT_DIR}/game/classes/VehicleCar.h
	${MOUNT_DIR}/game/classes/TriggerSoundscape.h
	${MOUNT_DIR}/game/classes/Weapon.h
	${MOUNT_DIR}/game/classes/Weapon_PhysGun.h
	${MOUNT_DIR}/game/classes/Weapon_QioFlashLight.h
	${MOUNT_DIR}/game/classes/World.h
)

set ( GAME_CLASSES_SOURCES
	${MOUNT_DIR}/game/classes/Actor.cpp
	${MOUNT_DIR}/game/classes/BaseEntity.cpp
	${MOUNT_DIR}/game/classes/Button.cpp
	${MOUNT_DIR}/game/classes/Constraint.cpp
	${MOUNT_DIR}/game/classes/Door.cpp
	${MOUNT_DIR}/game/classes/FakePlayer.cpp
	${MOUNT_DIR}/game/classes/FuncBombTarget.cpp
	${MOUNT_DIR}/game/classes/FuncBuyZone.cpp
	${MOUNT_DIR}/game/classes/FuncDustmotes.cpp
	${MOUNT_DIR}/game/classes/FuncRotating.cpp
	${MOUNT_DIR}/game/classes/FuncSmokeVolume.cpp
	${MOUNT_DIR}/game/classes/FuncStatic.cpp
	${MOUNT_DIR}/game/classes/InfoPathNode.cpp
	${MOUNT_DIR}/game/classes/Light.cpp
	${MOUNT_DIR}/game/classes/Location.cpp
	${MOUNT_DIR}/game/classes/Logic_Blinker.cpp
	${MOUNT_DIR}/game/classes/Mover.cpp
	${MOUNT_DIR}/game/classes/ModelEntity.cpp
	${MOUNT_DIR}/game/classes/Player.cpp
	${MOUNT_DIR}/game/classes/Projectile.cpp
	${MOUNT_DIR}/game/classes/Prop_Physics.cpp
	${MOUNT_DIR}/game/classes/QioSpawner.cpp
	${MOUNT_DIR}/game/classes/RotatingDoor.cpp
	${MOUNT_DIR}/game/classes/Terrain.cpp
	${MOUNT_DIR}/game/classes/Trigger.cpp
	${MOUNT_DIR}/game/classes/TriggerSoundscape.cpp
	${MOUNT_DIR}/game/classes/VehicleCar.cpp
	${MOUNT_DIR}/game/classes/Weapon.cpp
	${MOUNT_DIR}/game/classes/Weapon_PhysGun.cpp
	${MOUNT_DIR}/game/classes/Weapon_QioFlashLight.cpp
	${MOUNT_DIR}/game/classes/World.cpp
)

set ( GAME_HEADERS
	${MOUNT_DIR}/game/bg_public.h
	${MOUNT_DIR}/game/bt_include.h
	${MOUNT_DIR}/game/explosionInfo.h
	${MOUNT_DIR}/game/g_classes.h
	${MOUNT_DIR}/game/g_local.h
	${MOUNT_DIR}/game/g_pathNodes.h
	${MOUNT_DIR}/game/g_physVehicleAPI.h
	${MOUNT_DIR}/game/g_public.h
	${MOUNT_DIR}/game/g_ragdoll.h
	${MOUNT_DIR}/game/g_scriptedClasses.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/api/simplePathAPI.h
	${MOUNT_DIR}/qcommon/surfaceflags.h
)

set ( GAME_LUA_HEADERS
	${MOUNT_DIR}/game/lua/g_lua.h
	${MOUNT_DIR}/game/lua/g_lua_eventHandler.h

)

set ( GAME_LUA_SOURCES
	${MOUNT_DIR}/game/lua/g_lua.cpp
	${MOUNT_DIR}/game/lua/g_lua_entity.cpp
	${MOUNT_DIR}/game/lua/g_lua_eventHandler.cpp
	${MOUNT_DIR}/game/lua/g_lua_game.cpp
	${MOUNT_DIR}/game/lua/g_lua_qmath.cpp
	${MOUNT_DIR}/game/lua/g_lua_vector.cpp
)

set ( GAME_LUALIBRARY
	${LIB_DIR}/lua-5.1.5/src/lapi.c
	${LIB_DIR}/lua-5.1.5/src/lapi.h
	${LIB_DIR}/lua-5.1.5/src/lauxlib.c
	${LIB_DIR}/lua-5.1.5/src/lauxlib.h
	${LIB_DIR}/lua-5.1.5/src/lbaselib.c
	${LIB_DIR}/lua-5.1.5/src/lcode.c
	${LIB_DIR}/lua-5.1.5/src/lcode.h
	${LIB_DIR}/lua-5.1.5/src/ldblib.c
	${LIB_DIR}/lua-5.1.5/src/ldebug.c
	${LIB_DIR}/lua-5.1.5/src/ldebug.h
	${LIB_DIR}/lua-5.1.5/src/ldo.c
	${LIB_DIR}/lua-5.1.5/src/ldo.h
	${LIB_DIR}/lua-5.1.5/src/ldump.c
	${LIB_DIR}/lua-5.1.5/src/lfunc.c
	${LIB_DIR}/lua-5.1.5/src/lfunc.h
	${LIB_DIR}/lua-5.1.5/src/lgc.c
	${LIB_DIR}/lua-5.1.5/src/lgc.h
	${LIB_DIR}/lua-5.1.5/src/linit.c
	${LIB_DIR}/lua-5.1.5/src/liolib.c
	${LIB_DIR}/lua-5.1.5/src/llex.c
	${LIB_DIR}/lua-5.1.5/src/llex.h
	${LIB_DIR}/lua-5.1.5/src/llimits.h
	${LIB_DIR}/lua-5.1.5/src/lmathlib.c
	${LIB_DIR}/lua-5.1.5/src/lmem.c
	${LIB_DIR}/lua-5.1.5/src/lmem.h
	${LIB_DIR}/lua-5.1.5/src/loadlib.c
	${LIB_DIR}/lua-5.1.5/src/lobject.c
	${LIB_DIR}/lua-5.1.5/src/lobject.h
	${LIB_DIR}/lua-5.1.5/src/lopcodes.c
	${LIB_DIR}/lua-5.1.5/src/lopcodes.h
	${LIB_DIR}/lua-5.1.5/src/loslib.c
	${LIB_DIR}/lua-5.1.5/src/lparser.c
	${LIB_DIR}/lua-5.1.5/src/lparser.h
	${LIB_DIR}/lua-5.1.5/src/lstate.c
	${LIB_DIR}/lua-5.1.5/src/lstate.h
	${LIB_DIR}/lua-5.1.5/src/lstring.c
	${LIB_DIR}/lua-5.1.5/src/lstring.h
	${LIB_DIR}/lua-5.1.5/src/lstrlib.c
	${LIB_DIR}/lua-5.1.5/src/ltable.c
	${LIB_DIR}/lua-5.1.5/src/ltable.h
	${LIB_DIR}/lua-5.1.5/src/ltablib.c
	${LIB_DIR}/lua-5.1.5/src/ltm.c
	${LIB_DIR}/lua-5.1.5/src/ltm.h
	${LIB_DIR}/lua-5.1.5/src/lua.h
	${LIB_DIR}/lua-5.1.5/src/luaconf.h
	${LIB_DIR}/lua-5.1.5/src/lualib.h
	${LIB_DIR}/lua-5.1.5/src/lundump.c
	${LIB_DIR}/lua-5.1.5/src/lundump.h
	${LIB_DIR}/lua-5.1.5/src/lvm.c
	${LIB_DIR}/lua-5.1.5/src/lvm.h
	${LIB_DIR}/lua-5.1.5/src/lzio.c
	${LIB_DIR}/lua-5.1.5/src/lzio.h
	${LIB_DIR}/lua-5.1.5/src/print.c
)

set ( GAME_MATH_HEADERS
	${MOUNT_DIR}/math/aabb.h
	${MOUNT_DIR}/math/axis.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/math/vec3.h
)

set ( GAME_MATH_SOURCES
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

set ( GAME_PROTOCOL
	${MOUNT_DIR}/protocol/entityState.h
	${MOUNT_DIR}/protocol/gameState.h
	${MOUNT_DIR}/protocol/netLimits.h
	${MOUNT_DIR}/protocol/playerState.h
	${MOUNT_DIR}/protocol/snapFlags.h
	${MOUNT_DIR}/protocol/userCmd.h
	${MOUNT_DIR}/protocol/voipFlags.h
)

set ( GAME_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCmd.h
	${MOUNT_DIR}/shared/afRagdollHelper.h
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.h
	${MOUNT_DIR}/shared/cmBezierPatch.h
	${MOUNT_DIR}/shared/cmSurface.h
	${MOUNT_DIR}/shared/collisionUtils.h
	${MOUNT_DIR}/shared/cStylePreprocessor.h
	${MOUNT_DIR}/shared/displacementBuilder.h
	${MOUNT_DIR}/shared/ePairsList.h
	${MOUNT_DIR}/shared/entDef.h
	${MOUNT_DIR}/shared/entDefsList.h
	${MOUNT_DIR}/shared/eventBaseAPI.h
	${MOUNT_DIR}/shared/eventReceiverAPI.h
	${MOUNT_DIR}/shared/eventSystem.h
	${MOUNT_DIR}/shared/fileStreamHelper.h
	${MOUNT_DIR}/shared/infoString.h
	${MOUNT_DIR}/shared/keyValuesListener.h
	${MOUNT_DIR}/shared/mapFileWriter.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/quake3Anims.h
	${MOUNT_DIR}/shared/skelUtils.h
	${MOUNT_DIR}/shared/trace.h
	${MOUNT_DIR}/shared/wolfScript.h
)

set ( GAME_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCmd.cpp
	${MOUNT_DIR}/shared/afRagdollHelper.cpp
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.cpp
	${MOUNT_DIR}/shared/cmBezierPatch.cpp
	${MOUNT_DIR}/shared/cmSurface.cpp
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/cStylePreprocessor.cpp
	${MOUNT_DIR}/shared/displacementBuilder.cpp
	${MOUNT_DIR}/shared/ePairsList.cpp
	${MOUNT_DIR}/shared/entDef.cpp
	${MOUNT_DIR}/shared/entDefsList.cpp
	${MOUNT_DIR}/shared/eventSystem.cpp
	${MOUNT_DIR}/shared/fileStreamHelper.cpp
	${MOUNT_DIR}/shared/infoString.cpp
	${MOUNT_DIR}/shared/mapFileWriter.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/qcommon/q_math.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/shared/skelUtils.cpp
	${MOUNT_DIR}/shared/trace.cpp
	${MOUNT_DIR}/shared/wolfScript.cpp
)

set ( GAME_SOURCE
	${MOUNT_DIR}/game/bg_misc.cpp
	${MOUNT_DIR}/game/bt_vehicle.cpp
	${MOUNT_DIR}/game/g_active.cpp
	${MOUNT_DIR}/game/g_ammo.cpp
	${MOUNT_DIR}/game/g_api.cpp
	${MOUNT_DIR}/game/g_bot.cpp
	${MOUNT_DIR}/game/g_bullet.cpp
	${MOUNT_DIR}/game/g_bullet_debugDraw.cpp
	${MOUNT_DIR}/game/g_classes.cpp
	${MOUNT_DIR}/game/g_client.cpp
	${MOUNT_DIR}/game/g_collision.cpp
	${MOUNT_DIR}/game/g_debugDraw.cpp
	${MOUNT_DIR}/game/g_main.cpp
	${MOUNT_DIR}/game/g_pathNodes.cpp
	${MOUNT_DIR}/game/g_physDLL.cpp
	${MOUNT_DIR}/game/g_playerModels.cpp
	${MOUNT_DIR}/game/g_ragdoll.cpp
	${MOUNT_DIR}/game/g_saveMapFile.cpp
	${MOUNT_DIR}/game/g_scriptedClasses.cpp
	${MOUNT_DIR}/game/g_spawn.cpp
	${MOUNT_DIR}/game/g_utils.cpp
	${MOUNT_DIR}/game/g_weapons.cpp
)

set ( BUILD_GPHYSICS_BULLET_HEADERS
	${MOUNT_DIR}/gphysics/bullet/btp_characterController.h
	${MOUNT_DIR}/gphysics/bullet/btp_cMod2BulletShape.h
	${MOUNT_DIR}/gphysics/bullet/btp_constraint.h
	${MOUNT_DIR}/gphysics/bullet/btp_convert.h
	${MOUNT_DIR}/gphysics/bullet/btp_headers.h
	${MOUNT_DIR}/gphysics/bullet/btp_rigidBody.h
	${MOUNT_DIR}/gphysics/bullet/btp_shape.h
	${MOUNT_DIR}/gphysics/bullet/btp_staticMapLoader.h
	${MOUNT_DIR}/gphysics/bullet/btp_vehicle.h
	${MOUNT_DIR}/gphysics/bullet/btp_world.h
	${MOUNT_DIR}/api/physAPI.h
	${MOUNT_DIR}/api/physCharacterControllerAPI.h
	${MOUNT_DIR}/api/physConstraintAPI.h
	${MOUNT_DIR}/api/physObjectAPI.h
	${MOUNT_DIR}/shared/physObjectDef.h
)

set ( BUILD_GPHYSICS_BULLET_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.h
	${MOUNT_DIR}/shared/cmBezierPatch.h
	${MOUNT_DIR}/shared/cmSurface.h
	${MOUNT_DIR}/shared/collisionUtils.h
	${MOUNT_DIR}/shared/displacementBuilder.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/shared/trace.h
)

set ( BUILD_GPHYSICS_BULLET_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.cpp
	${MOUNT_DIR}/shared/cmBezierPatch.cpp
	${MOUNT_DIR}/shared/cmSurface.cpp
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/displacementBuilder.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/shared/trace.cpp
)

set ( BUILD_GPHYSICS_BULLET_SOURCES
	${MOUNT_DIR}/gphysics/bullet/btp_api.cpp
	${MOUNT_DIR}/gphysics/bullet/btp_characterController.cpp
	${MOUNT_DIR}/gphysics/bullet/btp_cMod2BulletShape.cpp
	${MOUNT_DIR}/gphysics/bullet/btp_constraint.cpp
	${MOUNT_DIR}/gphysics/bullet/btp_rigidBody.cpp
	${MOUNT_DIR}/gphysics/bullet/btp_shape.cpp
	${MOUNT_DIR}/gphysics/bullet/btp_staticMapLoader.cpp
	${MOUNT_DIR}/gphysics/bullet/btp_vehicle.cpp
	${MOUNT_DIR}/gphysics/bullet/btp_world.cpp
)

set ( BUILD_CGAME_HEADER
	${MOUNT_DIR}/game/bg_public.h
	${MOUNT_DIR}/cgame/cg_emitter.h
	${MOUNT_DIR}/cgame/cg_emitter_base.h
	${MOUNT_DIR}/cgame/cg_emitter_d3.h
	${MOUNT_DIR}/cgame/cg_local.h
	${MOUNT_DIR}/cgame/cg_public.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/qcommon/surfaceflags.h
)

set ( BUILD_CGAME_MATH_HEADERS
	${MOUNT_DIR}/math/axis.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/math/vec3.h
)

set ( BUILD_CGAME_MATH_SOURCES
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

set ( BUILD_CGAME_PROTOCOL
	${MOUNT_DIR}/protocol/entityState.h
	${MOUNT_DIR}/protocol/netLimits.h
	${MOUNT_DIR}/protocol/playerState.h
	${MOUNT_DIR}/protocol/snapFlags.h
	${MOUNT_DIR}/protocol/userCmd.h
	${MOUNT_DIR}/protocol/voipFlags.h
)

set ( BUILD_CGAME_SHARED_HEADERS
	${MOUNT_DIR}/shared/animationFlags.h
	${MOUNT_DIR}/shared/autoCmd.h
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/boneOrQP.h
	${MOUNT_DIR}/shared/collisionUtils.h
	${MOUNT_DIR}/shared/colorTable.h
	${MOUNT_DIR}/shared/entityType.h
	${MOUNT_DIR}/protocol/gameState.h
	${MOUNT_DIR}/shared/infoString.h
	${MOUNT_DIR}/shared/random.h
	${MOUNT_DIR}/shared/trace.h
)

set ( BUILD_CGAME_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCmd.cpp
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/colorTable.cpp
	${MOUNT_DIR}/shared/infoString.cpp
	${MOUNT_DIR}/shared/random.cpp
	${MOUNT_DIR}/shared/trace.cpp
)

set ( BUILD_CGAME_SOURCE
	${MOUNT_DIR}/game/bg_misc.cpp
	${MOUNT_DIR}/cgame/cg_api.cpp
	${MOUNT_DIR}/cgame/cg_chat.cpp
	${MOUNT_DIR}/cgame/cg_collision.cpp
	${MOUNT_DIR}/cgame/cg_consoleCmds.cpp
	${MOUNT_DIR}/cgame/cg_draw.cpp
	${MOUNT_DIR}/cgame/cg_drawtools.cpp
	${MOUNT_DIR}/cgame/cg_emitter.cpp
	${MOUNT_DIR}/cgame/cg_emitter_d3.cpp
	${MOUNT_DIR}/cgame/cg_ents.cpp
	${MOUNT_DIR}/cgame/cg_main.cpp
	${MOUNT_DIR}/cgame/cg_playerstate.cpp
	${MOUNT_DIR}/cgame/cg_predict.cpp
	${MOUNT_DIR}/cgame/cg_serverCmds.cpp
	${MOUNT_DIR}/cgame/cg_snapshot.cpp
	${MOUNT_DIR}/cgame/cg_syscalls.cpp
	${MOUNT_DIR}/cgame/cg_tempLights.cpp
	${MOUNT_DIR}/cgame/cg_testEmitter.cpp
	${MOUNT_DIR}/cgame/cg_testMaterial.cpp
	${MOUNT_DIR}/cgame/cg_testModel.cpp
	${MOUNT_DIR}/cgame/cg_tracer.cpp
	${MOUNT_DIR}/cgame/cg_view.cpp
	${MOUNT_DIR}/cgame/cg_viewModel.cpp
	${MOUNT_DIR}/qcommon/q_math.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp

)

set ( GPHYSICS_ODE_HEADERS
	${MOUNT_DIR}/gphysics/ode/ode_characterController.h
	${MOUNT_DIR}/gphysics/ode/ode_cMod2ODEGeom.h
	${MOUNT_DIR}/gphysics/ode/ode_constraint.h
	${MOUNT_DIR}/gphysics/ode/ode_convert.h
	${MOUNT_DIR}/gphysics/ode/ode_headers.h
	${MOUNT_DIR}/gphysics/ode/ode_rigidBody.h
	${MOUNT_DIR}/gphysics/ode/ode_shape.h
	${MOUNT_DIR}/gphysics/ode/ode_staticMapLoader.h
	${MOUNT_DIR}/gphysics/ode/ode_world.h
	${MOUNT_DIR}/api/physAPI.h
	${MOUNT_DIR}/api/physCharacterControllerAPI.h
	${MOUNT_DIR}/api/physConstraintAPI.h
	${MOUNT_DIR}/api/physObjectAPI.h
	${MOUNT_DIR}/shared/physObjectDef.h
)

set ( GPHYSICS_ODE_SHARED_HEADERS
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.h
	${MOUNT_DIR}/shared/cmBezierPatch.h
	${MOUNT_DIR}/shared/collisionUtils.h
	${MOUNT_DIR}/shared/displacementBuilder.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/shared/trace.h
)

set ( GPHYSICS_ODE_SHARED_SOURCES
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.cpp
	${MOUNT_DIR}/shared/cmBezierPatch.cpp
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/displacementBuilder.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/shared/trace.cpp
)

set ( GPHYSICS_ODE_SOURCE
	${MOUNT_DIR}/gphysics/ode/ode_api.cpp
	${MOUNT_DIR}/gphysics/ode/ode_characterController.cpp
	${MOUNT_DIR}/gphysics/ode/ode_cMod2ODEGeom.cpp
	${MOUNT_DIR}/gphysics/ode/ode_constraint.cpp
	${MOUNT_DIR}/gphysics/ode/ode_rigidBody.cpp
	${MOUNT_DIR}/gphysics/ode/ode_shape.cpp
	${MOUNT_DIR}/gphysics/ode/ode_staticMapLoader.cpp
	${MOUNT_DIR}/gphysics/ode/ode_world.cpp
)

SET ( EDITOR_API 
	${MOUNT_DIR}/api/rStaticModelAPI.h
)

SET ( EDITOR_GUI_MFC_HEADERS
	${MOUNT_DIR}/editor/gui/mfc/CamWnd.h
	${MOUNT_DIR}/editor/gui/mfc/CapDialog.h
	${MOUNT_DIR}/editor/gui/mfc/CommandsDlg.h
	${MOUNT_DIR}/editor/gui/mfc/DialogInfo.h
	${MOUNT_DIR}/editor/gui/mfc/DialogTextures.h
	${MOUNT_DIR}/editor/gui/mfc/DialogThick.h
	${MOUNT_DIR}/editor/gui/mfc/DlgEvent.h
	${MOUNT_DIR}/editor/gui/mfc/EditWnd.h
	${MOUNT_DIR}/editor/gui/mfc/EntityListDlg.h
	${MOUNT_DIR}/editor/gui/mfc/ENTITYW.H
	${MOUNT_DIR}/editor/gui/mfc/FindTextureDlg.h
	${MOUNT_DIR}/editor/gui/mfc/LstToolBar.h
	${MOUNT_DIR}/editor/gui/mfc/MainFrm.h
	${MOUNT_DIR}/editor/gui/mfc/MapInfo.h
	${MOUNT_DIR}/editor/gui/mfc/MRU.H
	${MOUNT_DIR}/editor/gui/mfc/NewProjDlg.h
	${MOUNT_DIR}/editor/gui/mfc/PatchDensityDlg.h
	${MOUNT_DIR}/editor/gui/mfc/PatchDialog.h
	${MOUNT_DIR}/editor/gui/mfc/PrefsDlg.h
	${MOUNT_DIR}/editor/gui/mfc/QE3.H
	${MOUNT_DIR}/editor/gui/mfc/QEDEFS.H
	${MOUNT_DIR}/editor/gui/mfc/RADEditView.h
	${MOUNT_DIR}/editor/gui/mfc/RADEditWnd.h
	${MOUNT_DIR}/editor/gui/mfc/Radiant.h
	${MOUNT_DIR}/editor/gui/mfc/RotateDlg.h
	${MOUNT_DIR}/editor/gui/mfc/ScaleDialog.h
	${MOUNT_DIR}/editor/gui/mfc/SurfaceDlg.h
	${MOUNT_DIR}/editor/gui/mfc/TexEdit.h
	${MOUNT_DIR}/editor/gui/mfc/TextureBar.h
	${MOUNT_DIR}/editor/gui/mfc/TextureLayout.h
	${MOUNT_DIR}/editor/gui/mfc/TextureLoad.h
	${MOUNT_DIR}/editor/gui/mfc/TexWnd.h
	${MOUNT_DIR}/editor/gui/mfc/ToolWnd.h
	${MOUNT_DIR}/editor/gui/mfc/WaveOpen.h
	${MOUNT_DIR}/editor/gui/mfc/XYWnd.h
	${MOUNT_DIR}/editor/gui/mfc/ZView.h
	${MOUNT_DIR}/editor/gui/mfc/ZWnd.h
)

SET ( EDITOR_GUI_MFC_SOURCES
	${MOUNT_DIR}/editor/gui/mfc/CamWnd.cpp
	${MOUNT_DIR}/editor/gui/mfc/CapDialog.cpp
	${MOUNT_DIR}/editor/gui/mfc/CommandsDlg.cpp
	${MOUNT_DIR}/editor/gui/mfc/DialogInfo.cpp
	${MOUNT_DIR}/editor/gui/mfc/DialogTextures.cpp
	${MOUNT_DIR}/editor/gui/mfc/DialogThick.cpp
	${MOUNT_DIR}/editor/gui/mfc/DlgEvent.cpp
	${MOUNT_DIR}/editor/gui/mfc/EditWnd.cpp
	${MOUNT_DIR}/editor/gui/mfc/EntityListDlg.cpp
	${MOUNT_DIR}/editor/gui/mfc/FindTextureDlg.cpp
	${MOUNT_DIR}/editor/gui/mfc/LstToolBar.cpp
	${MOUNT_DIR}/editor/gui/mfc/MainFrm.cpp
	${MOUNT_DIR}/editor/gui/mfc/MapInfo.cpp
	${MOUNT_DIR}/editor/gui/mfc/MRU.CPP
	${MOUNT_DIR}/editor/gui/mfc/NewProjDlg.cpp
	${MOUNT_DIR}/editor/gui/mfc/PatchDensityDlg.cpp
	${MOUNT_DIR}/editor/gui/mfc/PatchDialog.cpp
	${MOUNT_DIR}/editor/gui/mfc/PrefsDlg.cpp
	${MOUNT_DIR}/editor/gui/mfc/RADEditView.cpp
	${MOUNT_DIR}/editor/gui/mfc/RADEditWnd.cpp
	${MOUNT_DIR}/editor/gui/mfc/Radiant.cpp
	${MOUNT_DIR}/editor/gui/mfc/RotateDlg.cpp
	${MOUNT_DIR}/editor/gui/mfc/ScaleDialog.cpp
	${MOUNT_DIR}/editor/gui/mfc/SurfaceDlg.cpp
	${MOUNT_DIR}/editor/gui/mfc/TexEdit.cpp
	${MOUNT_DIR}/editor/gui/mfc/TextureBar.cpp
	${MOUNT_DIR}/editor/gui/mfc/TextureLayout.cpp
	${MOUNT_DIR}/editor/gui/mfc/TextureLoad.cpp
	${MOUNT_DIR}/editor/gui/mfc/TexWnd.cpp
	${MOUNT_DIR}/editor/gui/mfc/ToolWnd.cpp
	${MOUNT_DIR}/editor/gui/mfc/WaveOpen.cpp
	${MOUNT_DIR}/editor/gui/mfc/WIN_DLG.CPP
	${MOUNT_DIR}/editor/gui/mfc/Win_ent.cpp
	${MOUNT_DIR}/editor/gui/mfc/Win_main.cpp
	${MOUNT_DIR}/editor/gui/mfc/WIN_QE3.CPP
	${MOUNT_DIR}/editor/gui/mfc/XYWnd.cpp
	${MOUNT_DIR}/editor/gui/mfc/ZView.cpp
	${MOUNT_DIR}/editor/gui/mfc/ZWnd.cpp
)

set ( EDITOR_MATH_HEADERS
	${MOUNT_DIR}/math/aabb.h
	${MOUNT_DIR}/math/vec3.h
)

set ( EDITOR_MATH_SOURCES
	${MOUNT_DIR}/math/aabb.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

set ( EDITOR_SHARED_HEADERS
	${MOUNT_DIR}/shared/brush.h
	${MOUNT_DIR}/shared/brushside.h
	${MOUNT_DIR}/qcommon/q_platform.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/shared/simpleModel.h
	${MOUNT_DIR}/shared/textureAxisFromNormal.h
	${MOUNT_DIR}/shared/texturedVertex.h
	${MOUNT_DIR}/shared/texturedWinding.h
)

set ( EDITOR_SHARED_SOURCES
	${MOUNT_DIR}/shared/brush.cpp
	${MOUNT_DIR}/shared/brushside.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/shared/textureAxisFromNormal.cpp
	${MOUNT_DIR}/shared/texturedWinding.cpp
)

set ( EDITOR_HEADERS
	${MOUNT_DIR}/editor/ed_bezierPatch.h
	${MOUNT_DIR}/editor/ed_brush.h
	${MOUNT_DIR}/editor/ed_entity.h
	${MOUNT_DIR}/editor/ed_map.h
	${MOUNT_DIR}/editor/ed_select.h
	${MOUNT_DIR}/editor/ed_textures.h
	${MOUNT_DIR}/editor/ed_undo.h
	${MOUNT_DIR}/editor/ed_z.h
	${MOUNT_DIR}/editor/StdAfx.h
)

set ( EDITOR_SOURCES
	${MOUNT_DIR}/editor/ed_api.cpp
	${MOUNT_DIR}/editor/ed_bezierPatch.cpp
	${MOUNT_DIR}/editor/ed_brush.cpp
	${MOUNT_DIR}/editor/ed_csg.cpp
	${MOUNT_DIR}/editor/ed_drag.cpp
	${MOUNT_DIR}/editor/ed_entity.cpp
	${MOUNT_DIR}/editor/ed_main.cpp
	${MOUNT_DIR}/editor/ed_map.cpp
	${MOUNT_DIR}/editor/ed_select.cpp
	${MOUNT_DIR}/editor/ed_undo.cpp
	${MOUNT_DIR}/editor/ed_verticesSelection.cpp
	${MOUNT_DIR}/editor/ed_z.cpp
	${MOUNT_DIR}/editor/StdAfx.cpp
)

set ( CORE_APIS
	${MOUNT_DIR}/api/cgameAPI.h
	${MOUNT_DIR}/api/clientAPI.h
	${MOUNT_DIR}/api/cmAPI.h
	${MOUNT_DIR}/api/coreAPI.h
	${MOUNT_DIR}/api/cvarAPI.h
	${MOUNT_DIR}/api/ddAPI.h
	${MOUNT_DIR}/api/editorAPI.h
	${MOUNT_DIR}/api/gameAPI.h
	${MOUNT_DIR}/api/imgAPI.h
	${MOUNT_DIR}/api/inputSystemAPI.h
	${MOUNT_DIR}/api/loadingScreenMgrAPI.h
	${MOUNT_DIR}/api/materialSystemAPI.h
	${MOUNT_DIR}/api/modelLoaderDLLAPI.h
	${MOUNT_DIR}/api/modelPostProcessFuncs.h
	${MOUNT_DIR}/api/moduleManagerAPI.h
	${MOUNT_DIR}/api/mtrAPI.h
	${MOUNT_DIR}/api/mtrStageAPI.h
	${MOUNT_DIR}/api/portalizedWorldAPI.h
	${MOUNT_DIR}/api/rAPI.h
	${MOUNT_DIR}/api/rbAPI.h
	${MOUNT_DIR}/api/rEntityAPI.h
	${MOUNT_DIR}/api/sdlSharedAPI.h
	${MOUNT_DIR}/api/serverAPI.h
	${MOUNT_DIR}/api/staticModelCreatorAPI.h
	${MOUNT_DIR}/api/sysEventCasterAPI.h
	${MOUNT_DIR}/api/textureAPI.h
	${MOUNT_DIR}/api/vfsAPI.h
)

set ( CORE_HEADER_FILES
	${MOUNT_DIR}/shared/array.h
	${MOUNT_DIR}/game/bg_public.h
	${MOUNT_DIR}/cgame/cg_public.h
	${MOUNT_DIR}/client/cl_curl.h
	${MOUNT_DIR}/client/client.h
	${MOUNT_DIR}/game/g_public.h
	${MOUNT_DIR}/api/iFaceBase.h
	${MOUNT_DIR}/api/iFaceMgrAPI.h
	${MOUNT_DIR}/client/keys.h
	${MOUNT_DIR}/shared/lightFlags.h
	${MOUNT_DIR}/qcommon/puff.h
	${MOUNT_DIR}/qcommon/q_shared.h
	${MOUNT_DIR}/qcommon/qcommon.h
	${MOUNT_DIR}/server/server.h
	${MOUNT_DIR}/qcommon/surfaceflags.h
	${MOUNT_DIR}/shared/typedefs.h
	${MOUNT_DIR}/qcommon/unzip.h
	${MOUNT_DIR}/server/sv_vis.h
	${MOUNT_DIR}/sys/sys_loadlib.h
	${MOUNT_DIR}/sys/sys_local.h
	${MOUNT_DIR}/qcommon/q_shared.h
)

set ( CORE_MATH_HEADERS
	${MOUNT_DIR}/math/aabb.h
	${MOUNT_DIR}/math/axis.h
	${MOUNT_DIR}/math/math.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/math/plane.h
	${MOUNT_DIR}/math/quat.h
	${MOUNT_DIR}/math/vec2.h
	${MOUNT_DIR}/math/vec3.h
)

set ( CORE_MATH_SOURCES
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/plane.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

set ( CORE_PROTOCOL
	${MOUNT_DIR}/protocol/entityState.h
	${MOUNT_DIR}/protocol/gameState.h
	${MOUNT_DIR}/protocol/netLimits.h
	${MOUNT_DIR}/protocol/playerState.h
	${MOUNT_DIR}/protocol/snapFlags.h
	${MOUNT_DIR}/protocol/userCmd.h
	${MOUNT_DIR}/protocol/voipFlags.h
)

set ( CORE_SDL_HEADERS
	${MOUNT_DIR}/sdl/sdl_glConfig.h
)

set ( CORE_SDL_SOURCES
	${MOUNT_DIR}/sdl/sdl_glimp.cpp
	${MOUNT_DIR}/sdl/sdl_input.cpp
)

set ( CORE_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/bitSet.h
	${MOUNT_DIR}/shared/bspBoxDesc.h
	${MOUNT_DIR}/shared/cmWinding.h
	${MOUNT_DIR}/shared/colorTable.h
	${MOUNT_DIR}/shared/cvarModificationCallback.h
	${MOUNT_DIR}/shared/doom3ProcPVSClass.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/portalizedBSPTree.h
	${MOUNT_DIR}/shared/shared.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/infoString.h
	${MOUNT_DIR}/shared/keyCatchers.h
)

set ( CORE_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/cmWinding.cpp
	${MOUNT_DIR}/shared/colorTable.cpp
	${MOUNT_DIR}/shared/doom3ProcPVSClass.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/portalizedBSPTree.cpp
	${MOUNT_DIR}/shared/infoString.cpp
)

set ( CORE_SOURCE_FILES
	${MOUNT_DIR}/client/cl_avi.cpp
	${MOUNT_DIR}/client/cl_cgame.cpp
	${MOUNT_DIR}/client/cl_cin.cpp
	${MOUNT_DIR}/client/cl_console.cpp
	${MOUNT_DIR}/client/cl_curl.cpp
	${MOUNT_DIR}/client/cl_input.cpp
	${MOUNT_DIR}/client/cl_keys.cpp
	${MOUNT_DIR}/client/cl_loadingScreenMgr.cpp
	${MOUNT_DIR}/client/cl_main.cpp
	${MOUNT_DIR}/client/cl_net_chan.cpp
	${MOUNT_DIR}/client/cl_parse.cpp
	${MOUNT_DIR}/client/cl_renderer.cpp
	${MOUNT_DIR}/client/cl_scrn.cpp
	${MOUNT_DIR}/client/cl_ui.cpp
	${MOUNT_DIR}/qcommon/cmd.cpp
	${MOUNT_DIR}/qcommon/com_systemEventCaster.cpp
	${MOUNT_DIR}/qcommon/common.cpp
	${MOUNT_DIR}/sys/con_log.cpp
	${MOUNT_DIR}/sys/con_passive.cpp
	${MOUNT_DIR}/qcommon/cvar.cpp
	${MOUNT_DIR}/qcommon/files.cpp
	${MOUNT_DIR}/qcommon/huffman.cpp
	${MOUNT_DIR}/qcommon/iFaceMgrIMPL.cpp
	${MOUNT_DIR}/qcommon/ioapi.c
	${MOUNT_DIR}/qcommon/md4.cpp
	${MOUNT_DIR}/qcommon/md5.cpp
	${MOUNT_DIR}/qcommon/moduleManagerIMPL.cpp
	${MOUNT_DIR}/qcommon/msg.cpp
	${MOUNT_DIR}/qcommon/net_chan.cpp
	${MOUNT_DIR}/qcommon/net_ip.cpp
	${MOUNT_DIR}/qcommon/puff.c
	${MOUNT_DIR}/qcommon/q_math.cpp
	${MOUNT_DIR}/qcommon/q_shared.cpp
	${MOUNT_DIR}/server/sv_ccmds.cpp
	${MOUNT_DIR}/server/sv_client.cpp
	${MOUNT_DIR}/server/sv_game.cpp
	${MOUNT_DIR}/server/sv_init.cpp
	${MOUNT_DIR}/server/sv_main.cpp
	${MOUNT_DIR}/server/sv_net_chan.cpp
	${MOUNT_DIR}/server/sv_snapshot.cpp
	${MOUNT_DIR}/server/sv_vis.cpp
	${MOUNT_DIR}/sys/sys_main.cpp
	${MOUNT_DIR}/sys/sys_win32.cpp
	${MOUNT_DIR}/qcommon/unzip.c
)

set ( CORE_SPEEX_LIBRARY
	${LIB_DIR}/libspeex/bits.c
	${LIB_DIR}/libspeex/buffer.c
	${LIB_DIR}/libspeex/cb_search.c
	${LIB_DIR}/libspeex/config.h
	${LIB_DIR}/libspeex/exc_10_16_table.c
	${LIB_DIR}/libspeex/exc_10_32_table.c
	${LIB_DIR}/libspeex/exc_20_32_table.c
	${LIB_DIR}/libspeex/exc_5_256_table.c
	${LIB_DIR}/libspeex/exc_5_64_table.c
	${LIB_DIR}/libspeex/exc_8_128_table.c
	${LIB_DIR}/libspeex/fftwrap.c
	${LIB_DIR}/libspeex/filterbank.c
	${LIB_DIR}/libspeex/filters.c
	${LIB_DIR}/libspeex/gain_table.c
	${LIB_DIR}/libspeex/gain_table_lbr.c
	${LIB_DIR}/libspeex/hexc_10_32_table.c
	${LIB_DIR}/libspeex/hexc_table.c
	${LIB_DIR}/libspeex/high_lsp_tables.c
	${LIB_DIR}/libspeex/jitter.c
	${LIB_DIR}/libspeex/kiss_fft.c
	${LIB_DIR}/libspeex/kiss_fftr.c
	${LIB_DIR}/libspeex/lpc.c
	${LIB_DIR}/libspeex/lsp.c
	${LIB_DIR}/libspeex/lsp_tables_nb.c
	${LIB_DIR}/libspeex/ltp.c
	${LIB_DIR}/libspeex/mdf.c
	${LIB_DIR}/libspeex/modes.c
	${LIB_DIR}/libspeex/modes_wb.c
	${LIB_DIR}/libspeex/nb_celp.c
	${LIB_DIR}/libspeex/preprocess.c
	${LIB_DIR}/libspeex/quant_lsp.c
	${LIB_DIR}/libspeex/resample.c
	${LIB_DIR}/libspeex/sb_celp.c
	${LIB_DIR}/libspeex/smallft.c
	${LIB_DIR}/libspeex/speex.c
	${LIB_DIR}/libspeex/speex_callbacks.c
	${LIB_DIR}/libspeex/speex_header.c
	${LIB_DIR}/libspeex/stereo.c
	${LIB_DIR}/libspeex/vbr.c
	${LIB_DIR}/libspeex/vq.c
	${LIB_DIR}/libspeex/window.c
)

set ( CORE_ZLIB_LIBRARY
	${LIB_DIR}/zlib/adler32.c
	${LIB_DIR}/zlib/compress.c
	${LIB_DIR}/zlib/crc32.c
	${LIB_DIR}/zlib/crc32.h
	${LIB_DIR}/zlib/deflate.c
	${LIB_DIR}/zlib/deflate.h
	${LIB_DIR}/zlib/gzclose.c
	${LIB_DIR}/zlib/gzguts.h
	${LIB_DIR}/zlib/gzlib.c
	${LIB_DIR}/zlib/gzread.c
	${LIB_DIR}/zlib/gzwrite.c
	${LIB_DIR}/zlib/infback.c
	${LIB_DIR}/zlib/inffast.c
	${LIB_DIR}/zlib/inffast.h
	${LIB_DIR}/zlib/inffixed.h
	${LIB_DIR}/zlib/inflate.c
	${LIB_DIR}/zlib/inflate.h
	${LIB_DIR}/zlib/inftrees.c
	${LIB_DIR}/zlib/inftrees.h
	${LIB_DIR}/zlib/trees.c
	${LIB_DIR}/zlib/trees.h
	${LIB_DIR}/zlib/uncompr.c
	${LIB_DIR}/zlib/zconf.h
	${LIB_DIR}/zlib/zlib.h
	${LIB_DIR}/zlib/zutil.c
	${LIB_DIR}/zlib/zutil.h
)

if( BUILD_MATERIALSYSTEM )
	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${SHAREDLIST_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${SHAREDLIST_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${MATERIALSYSTEMLIST_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${MATERIALSYSTEMLIST_SOURCES} )
	endif()

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
	target_link_libraries( materialSystem )
	endif()
	set_target_properties( materialSystem PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if( BUILD_IMAGELIB )
	# This section finds and provides REQUIRED dependencies to imageLib target
	#find_package( libDevil REQUIRED )
	#find_package( vtflib REQUIRED )

	# Dushan - This is how Visual Studio directories (or filters) are generated
	if( WIN32 )
		SOURCE_GROUP( Header\ Files                         FILES ${IMAGELIB_HEADERS} )
		SOURCE_GROUP( Source\ Files                         FILES ${IMAGELIB_SOURCES} )	
	endif ( WIN32 )
	
	add_library( imageLib 
		SHARED 
		${IMAGELIB_SOURCES}
		${IMAGELIB_HEADERS}
	)
	include_directories( ${MOUNT_DIR} ${LIB_DIR}/il-1.7.8-x86/include ${LIB_DIR}/vtflib132-bin/lib ${LIB_DIR}/libwebp/include )
	if( WIN32 )
		set_target_properties( imageLib PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/imageLib.def" OUTPUT_NAME "imageLib${BUILD_ARCH}" PREFIX "" )
		#link_directories( ${LIB_DIR}/vtflib132-bin/lib/x86/ )
        #include_directories( ${LIB_DIR}/il-1.7.8-x86/include ${LIB_DIR}/vtflib132-bin/lib ${LIB_DIR}/libwebp/include )
		target_link_libraries( imageLib ${LIB_DIR}/vtflib132-bin/lib/x86/VTFLib.lib ${LIB_DIR}/il-1.7.8-x86/lib/DevIL.lib ${LIB_DIR}/libwebp/Lib/libwebp.lib )
	else()
		set_target_properties( imageLib PROPERTIES OUTPUT_NAME "imageLibx${BUILD_ARCH}" PREFIX "" )
		target_link_libraries( imageLib )
	endif()
	set_target_properties( imageLib PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if( WIN32 )
	if( BUILD_DX9_RENDERER )
		# This section finds and provides REQUIRED dependencies to DirectX9 target
		find_package( DirectX REQUIRED )

		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${DIRECTX9LIB_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${DIRECTX9LIB_SHARED_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${DIRECTX9LIB_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${DIRECTX9LIB_SOURCES} )
		
		add_library( backendDX9 
			SHARED 
			${MATHLIST_SOURCES} 
			${MATHLIST_HEADERS} 
			${DIRECTX9LIB_SOURCES}
			${DIRECTX9LIB_HEADERS}
			${DIRECTX9LIB_SHARED_HEADERS}
			${DIRECTX9LIB_SHARED_SOURCES}
		)
		include_directories( ${MOUNT_DIR} ${DXSDK_INCLUDE_DIR} ${LIB_DIR}/SDL12/include )
		set_target_properties( backendDX9 PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendDX9.def" OUTPUT_NAME "backendDX9${BUILD_ARCH}" PREFIX "" )
		target_link_libraries( backendDX9 ${DXSDK_D3DX9_LIBRARY} ${DXSDK_D3D9_LIBRARY} SDL )
		set_target_properties( backendDX9 PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
		)
	endif()

	if( BUILD_DX10_RENDERER )
		# This section finds and provides REQUIRED dependencies to backendDX10 target
		find_package( DirectX REQUIRED )
		
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${DIRECTXANDNULLRENDERER_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${DIRECTXANDNULLRENDERER_SHARED_SOURCES} )
		SOURCE_GROUP( Source\ Files                                        FILES ${DIRECTX10LIB_SOURCES} )

		add_library( backendDX10
			SHARED 
			${DIRECTX10LIB_SOURCES}
			${DIRECTXANDNULLRENDERER_SHARED_HEADERS}
			${DIRECTXANDNULLRENDERER_SHARED_SOURCES}
		)
		include_directories( ${MOUNT_DIR} ${DXSDK_INCLUDE_DIR} ${LIB_DIR}/SDL12/include )
		set_target_properties( backendDX10 PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendDX10.def" OUTPUT_NAME "backendDX10${BUILD_ARCH}" PREFIX "" )
		target_link_libraries( backendDX10 ${DXSDK_D3DX10_LIBRARY} ${DXSDK_D3D10_LIBRARY} SDL )
		set_target_properties( backendDX10 PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
		)
	endif()
endif() ## WIN32

if( BUILD_NULL_RENDERER )
	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${DIRECTXANDNULLRENDERER_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${DIRECTXANDNULLRENDERER_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${DIRECTX9LIB_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${NULLRENDERER_SOURCES} )
	endif()
		
	add_library( backendNULL
		SHARED 
		${NULLRENDERER_SOURCES}
		${DIRECTXANDNULLRENDERER_SHARED_HEADERS}
		${DIRECTXANDNULLRENDERER_SHARED_SOURCES}
	)
	include_directories( ${MOUNT_DIR} ${LIB_DIR}/SDL12/include )
	set_target_properties( backendNULL PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendNULL.def" OUTPUT_NAME "backendNULL${BUILD_ARCH}" PREFIX "" )
	target_link_libraries( backendNULL ${DXSDK_D3DX9_LIBRARY} ${DXSDK_D3D9_LIBRARY} SDL )
	set_target_properties( backendNULL PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if( BUILD_GL_RENDERER )
	# This section finds and provides REQUIRED dependencies to backEndGL target
	find_package( GLEW REQUIRED )
	find_package( GLUT REQUIRED )

	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( API\\Header\ Files                                   FILES ${RENDERERGL_APIS_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${RENDERERGL_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${RENDERERGL_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${RENDERERGL_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${RENDERERGL_SOURCES} )
	endif()
	
	add_library( backendGL
		SHARED 
		${RENDERERGL_APIS_HEADERS}
		${MATHLIST_SOURCES} 
		${MATHLIST_HEADERS} 
		${RENDERERGL_SHARED_HEADERS} 
		${RENDERERGL_SHARED_SOURCES} 
		${RENDERERGL_HEADERS} 
		${RENDERERGL_SOURCES} 
	)
	include_directories( ${MOUNT_DIR} ${GLEW_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${LIB_DIR}/SDL12/include )
	target_link_libraries( backendGL ${GLEW_LIBRARY} ${GLUT_glut_LIBRARY} SDL )
	if( WIN32 )
		set_target_properties( backendGL PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendGL.def" OUTPUT_NAME "backendGL${BUILD_ARCH}" PREFIX "" )
	else()
		set_target_properties( backendGL PROPERTIES OUTPUT_NAME "backendGL${BUILD_ARCH}" PREFIX "" )
	endif()
	set_target_properties( backendGL PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if( BUILD_RENDERER )
	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( API\\Header\ Files                                   FILES ${RENDERERAPI_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${RENDERER_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${RENDERER_SHARED_SOURCES} )
		SOURCE_GROUP( FileFormats\\Header\ Files                           FILES ${RENDERERFILEFORMATS_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${RENDERER_MATH_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${RENDERER_MATH_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${RENDERER_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${RENDERER_SOURCES} )
	endif()

	add_library( renderer
		SHARED 
		${RENDERERAPI_HEADERS} 
		${RENDERERFILEFORMATS_HEADERS} 
		${RENDERER_HEADERS}
		${RENDERER_MATH_SOURCES}
		${RENDERER_MATH_HEADERS}
		${RENDERER_SHARED_HEADERS}
		${RENDERER_SHARED_SOURCES}
		${RENDERER_SOURCES}
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
		set_target_properties( renderer PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/renderer.def" OUTPUT_NAME "renderer${BUILD_ARCH}" PREFIX "" )
	else()
		set_target_properties( renderer PROPERTIES OUTPUT_NAME "backendGL${BUILD_ARCH}" PREFIX "" )
	endif()
	set_target_properties( renderer PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if( BUILD_MODELLOADER )

	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( API\\Header\ Files                                   FILES ${MODELLOADER_API} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${MODELLOADER_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${MODELLOADER_SHARED_SOURCES} )
		SOURCE_GROUP( FileFormats\\Header\ Files                           FILES ${MODELLOADERFILEFORMATS} )
		SOURCE_GROUP( KeyframedModelLoaders\\Header\ Files                 FILES ${MODELLOADER_KEYFRAMEDMODELLOADERS_HEADERS} )
		SOURCE_GROUP( KeyframedModelLoaders\\Source\ Files                 FILES ${MODELLOADER_KEYFRAMEDMODELLOADERS_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MODELLOADER_MATH_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MODELLOADER_MATH_SOURCES} )
		SOURCE_GROUP( StaticModelLoader\\Header\ Files                     FILES ${MODELLOADER_STATICMODELLOADERS_HEADERS} )
		SOURCE_GROUP( StaticModelLoader\\Source\ Files                     FILES ${MODELLOADER_STATICMODELLOADERS_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${MODELLOADER_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${MODELLOADER_SOURCES} )
	endif()
	
	add_library( modelLoader
		SHARED 
		${MODELLOADER_API}
		${MODELLOADERFILEFORMATS}
		${MODELLOADER_HEADERS}
		${MODELLOADER_KEYFRAMEDMODELLOADERS_SOURCES}
		${MODELLOADER_KEYFRAMEDMODELLOADERS_HEADERS}
		${MODELLOADER_MATH_HEADERS}
		${MODELLOADER_MATH_SOURCES}
		${MODELLOADER_SHARED_SOURCES}
		${MODELLOADER_SHARED_HEADERS}
		${MODELLOADER_SKELETALMODELLOADERS}
		${MODELLOADER_SOURCES}
		${MODELLOADER_STATICMODELLOADERS_HEADERS}
		${MODELLOADER_STATICMODELLOADERS_SOURCES}
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
		set_target_properties( modelLoader PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/modelLoader.def" OUTPUT_NAME "modelLoader${BUILD_ARCH}" PREFIX "" )
	else()
		set_target_properties( modelLoader PROPERTIES OUTPUT_NAME "modelLoader${BUILD_ARCH}" PREFIX "" )
	endif()
	set_target_properties( modelLoader PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if( BUILD_CM )
	add_library( cm
		SHARED 
		${CM_HEADERS}
		${MATHLIST_SOURCES}
		${MATHLIST_HEADERS}
		${CM_SHARED}
		${CM_SOURCE}
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
		set_target_properties( cm PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/cm.def" OUTPUT_NAME "cm${BUILD_ARCH}" PREFIX "" )
	else()
		set_target_properties( cm PROPERTIES OUTPUT_NAME "cm${BUILD_ARCH}" PREFIX "" )
	endif()
	set_target_properties( cm PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if( BUILD_DECLMANAGER )
	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( API\\Header\ Files                                   FILES ${DECLMANAGER_API} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${DECLMANAGER_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${DECLMANAGER_SHARED_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Header\ Files                                        FILES ${DECLMANAGER_HEADER} )
		SOURCE_GROUP( Source\ Files                                        FILES ${DECLMANAGER_SOURCE} )
	endif()
	
	add_library( declManager
		SHARED 
		${DECLMANAGER_API}
		${DECLMANAGER_HEADER}
		${MATHLIST_SOURCES}
		${MATHLIST_HEADERS}
		${DECLMANAGER_SHARED_HEADERS}
		${DECLMANAGER_SHARED_SOURCES}
		${DECLMANAGER_SOURCE}
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
set_target_properties( declManager PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/declManager.def" OUTPUT_NAME "declManager${BUILD_ARCH}" PREFIX "" )
	else()
		set_target_properties( declManager PROPERTIES OUTPUT_NAME "declManager${BUILD_ARCH}" PREFIX "" )
	endif()
	set_target_properties( declManager PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

IF( BUILD_GAME )
	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Classes\\CSS\\Header\ Files                          FILES ${GAME_CLASSES_CSS_HEADERS} )
		SOURCE_GROUP( Classes\\CSS\\Source\ Files                          FILES ${GAME_CLASSES_CSS_SOURCES} )
		SOURCE_GROUP( Classes\\ET\\Header\ Files                           FILES ${GAME_CLASSES_ET_HEADERS} )
		SOURCE_GROUP( Classes\\ET\\Source\ Files                           FILES ${GAME_CLASSES_ET_SOURCES} )
		SOURCE_GROUP( Classes\\Quake3\\Header\ Files                       FILES ${GAME_CLASSES_QUAKE3_HEADERS} )
		SOURCE_GROUP( Classes\\Quake3\\Source\ Files                       FILES ${GAME_CLASSES_QUAKE3_SOURCES} )
		SOURCE_GROUP( Classes\\RTCW\\Header\ Files                         FILES ${GAME_CLASSES_RTCW_HEADERS} )
		SOURCE_GROUP( Classes\\RTCW\\Source\ Files                         FILES ${GAME_CLASSES_RTCW_SOURCES} )
		SOURCE_GROUP( Classes\\Header\ Files                               FILES ${GAME_CLASSES_HEADERS} )
		SOURCE_GROUP( Classes\\Source\ Files                               FILES ${GAME_CLASSES_SOURCES} )
		SOURCE_GROUP( LUA\\Header\ Files                                   FILES ${GAME_LUA_HEADERS} )
		SOURCE_GROUP( LUA\\Source\ Files                                   FILES ${GAME_LUA_SOURCES} )
		SOURCE_GROUP( LUA\ Library\\Files                                  FILES ${GAME_LUALIBRARY} )
		SOURCE_GROUP( Protocol\\Header\ Files                              FILES ${GAME_PROTOCOL} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${GAME_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${GAME_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${GAME_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${GAME_SOURCE} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${GAME_MATH_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${GAME_MATH_SOURCES} )
	endif()
	
	add_library( qagame
		SHARED 
		${GAME_CLASSES_CSS_HEADERS}
		${GAME_CLASSES_CSS_SOURCES}
		${GAME_CLASSES_ET_HEADERS}
		${GAME_CLASSES_ET_SOURCES}
		${GAME_CLASSES_QUAKE3_HEADERS}
		${GAME_CLASSES_QUAKE3_SOURCES}
		${GAME_CLASSES_RTCW_HEADERS}
		${GAME_CLASSES_RTCW_SOURCES}
		${GAME_CLASSES_HEADERS}
		${GAME_CLASSES_SOURCES}
		${GAME_LUA_HEADERS}
		${GAME_LUA_SOURCES}
		${GAME_LUALIBRARY}
		${GAME_MATH_HEADERS}
		${GAME_MATH_SOURCES}
		${GAME_PROTOCOL}
		${GAME_SHARED_HEADERS}
		${GAME_SHARED_SOURCES}
		${GAME_HEADERS}
		${GAME_SOURCE}
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
		set_target_properties( qagame PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/game.def" OUTPUT_NAME "qagame${BUILD_ARCH}" PREFIX "" )
		include_directories( ${MOUNT_DIR} ${LIB_DIR}/lua-5.1.5/src )
		SET_TARGET_PROPERTIES( qagame PROPERTIES COMPILE_FLAGS -DG_ENABLE_LUA_SCRIPTING )
		target_link_libraries( qagame ${LIB_DIR}/lua-5.1.5/lib/lua5.1.lib )
	else()
		set_target_properties( qagame PROPERTIES OUTPUT_NAME "qagame${BUILD_ARCH}" PREFIX "" )
	endif()
	set_target_properties( qagame PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if ( BUILD_GPHYSICS_BULLET )
	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${BUILD_GPHYSICS_BULLET_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${BUILD_GPHYSICS_BULLET_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${BUILD_GPHYSICS_BULLET_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${BUILD_GPHYSICS_BULLET_SOURCES} )
	endif()

	add_library( gphysics_bullet
		SHARED
		${MATHLIST_HEADERS}
		${MATHLIST_SOURCES}
		${BUILD_GPHYSICS_BULLET_SHARED_HEADERS}
		${BUILD_GPHYSICS_BULLET_SHARED_SOURCES}
		${BUILD_GPHYSICS_BULLET_HEADERS}
		${BUILD_GPHYSICS_BULLET_SOURCES}
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
		SET_TARGET_PROPERTIES( gphysics_bullet PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/game.def" OUTPUT_NAME "gphysics_bullet${BUILD_ARCH}" PREFIX "" )
		INCLUDE_DIRECTORIES( ${MOUNT_DIR} ${LIB_DIR}/bullet-2.80-rev2531/src )
		TARGET_LINK_LIBRARIES( gphysics_bullet BulletDynamics BulletCollision LinearMath)
	else()
		set_target_properties( gphysics_bullet PROPERTIES OUTPUT_NAME "gphysics_bullet${BUILD_ARCH}" PREFIX "" )
	endif()
	set_target_properties( gphysics_bullet PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if ( BUILD_CGAME )
	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${BUILD_CGAME_MATH_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${BUILD_CGAME_MATH_SOURCES} )
		SOURCE_GROUP( Protocol\\Header\ Files                              FILES ${BUILD_CGAME_PROTOCOL} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${BUILD_CGAME_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${BUILD_CGAME_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${BUILD_CGAME_HEADER} )
		SOURCE_GROUP( Source\ Files                                        FILES ${BUILD_CGAME_SOURCE} )
	endif()

	add_library( cgame
		SHARED
		${BUILD_CGAME_MATH_HEADERS}
		${BUILD_CGAME_MATH_SOURCES}
		${BUILD_CGAME_PROTOCOL}
		${BUILD_CGAME_SHARED_HEADERS}
		${BUILD_CGAME_SHARED_SOURCES}
		${BUILD_CGAME_HEADER}
		${BUILD_CGAME_SOURCE}
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
		set_target_properties( cgame PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/cgame.def" OUTPUT_NAME "cgame${BUILD_ARCH}" PREFIX "" )
	else()
		set_target_properties( cgame PROPERTIES OUTPUT_NAME "cgame${BUILD_ARCH}" PREFIX "" )
	endif()
	set_target_properties( cgame PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if ( BUILD_GPHYSICS_ODE )
	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${GPHYSICS_ODE_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${GPHYSICS_ODE_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${GPHYSICS_ODE_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${GPHYSICS_ODE_SOURCE} )
	endif()
	add_library( gphysics_ode
		SHARED
		${GPHYSICS_ODE_SHARED_HEADERS}
		${GPHYSICS_ODE_SHARED_SOURCES}
		${GPHYSICS_ODE_HEADERS}
		${GPHYSICS_ODE_SOURCE}
	)
	include_directories( ${MOUNT_DIR} )
	if( WIN32 )
		set_target_properties( gphysics_ode PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/gphysics_ode.def" OUTPUT_NAME "gphysics_ode${BUILD_ARCH}" PREFIX "" )
		INCLUDE_DIRECTORIES( ${MOUNT_DIR} ${LIB_DIR}/ode-r1939/include )
	else()
		set_target_properties( gphysics_ode PROPERTIES OUTPUT_NAME "gphysics_ode${BUILD_ARCH}" PREFIX "" )
	endif()
	TARGET_LINK_LIBRARIES( gphysics_ode ode )
	set_target_properties( gphysics_ode PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if ( BUILD_EDITOR )
	if( WIN32 )
		
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( API\\Header\ Files                                   FILES ${EDITOR_API} )
		SOURCE_GROUP( GUI\ MFC\\Header\ Files                              FILES ${EDITOR_GUI_MFC_HEADERS} )
		SOURCE_GROUP( GUI\ MFC\\Source\ Files                              FILES ${EDITOR_GUI_MFC_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${EDITOR_MATH_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${EDITOR_MATH_SOURCES} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${EDITOR_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${EDITOR_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${EDITOR_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${EDITOR_SOURCES} )
	endif()

	add_library( editor
		SHARED
		${EDITOR_API}
		${EDITOR_GUI_MFC_HEADERS}
		${EDITOR_GUI_MFC_SOURCES}
		${EDITOR_MATH_HEADERS}
		${EDITOR_MATH_SOURCES}
		${EDITOR_SHARED_HEADERS}
		${EDITOR_SHARED_SOURCES}
		${EDITOR_HEADERS}
		${EDITOR_SOURCES}
	)
	set_target_properties( editor PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/editor.def" OUTPUT_NAME "editor${BUILD_ARCH}" PREFIX "" )
	INCLUDE_DIRECTORIES( ${MOUNT_DIR} ${MOUNT_DIR}/editor/ ${MOUNT_DIR}/editor/gui/mfc ${LIB_DIR}/il-1.7.8-x86/include )
	ADD_DEFINITIONS( -DWIN32 -D_AFXDLL -D_WINDOWS -DQUAKE3 -DQ3RADIANT -D_WIN32 -D_CRT_SECURE_NO_WARNINGS -DWINVER=0x0600 -DUSE_DEVIL_LIBRARY )
	target_link_libraries( editor 	opengl32.lib ${LIB_DIR}/gl/lib/glu32.lib ${LIB_DIR}/il-1.7.8-x86/lib/DevIL.lib )
	set_target_properties( editor PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
endif()

if ( BUILD_CORE )
	if( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( API\\Header\ Files                                   FILES ${CORE_APIS} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${CORE_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${CORE_SHARED_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${CORE_MATH_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${CORE_MATH_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${CORE_HEADER_FILES} )
		SOURCE_GROUP( Source\ Files                                        FILES ${CORE_SOURCE_FILES} )
		SOURCE_GROUP( SDL\\Header\ Files                                   FILES ${CORE_SDL_HEADERS} )
		SOURCE_GROUP( SDL\\Source\ Files                                   FILES ${CORE_SDL_SOURCES} )
		SOURCE_GROUP( Library\\Speex\ Files                                FILES ${CORE_SPEEX_LIBRARY} )
		SOURCE_GROUP( Library\\zLib\ Files                                 FILES ${CORE_ZLIB_LIBRARY} )
	endif()

	ADD_EXECUTABLE( qio  
		${CORE_APIS} 
		${CORE_MATH_HEADERS} 
		${CORE_MATH_SOURCES} 
		${CORE_HEADER_FILES} 
		${CORE_SOURCE_FILES}
		${CORE_SDL_HEADERS}
		${CORE_SDL_SOURCES}
		${CORE_SHARED_HEADERS}
		${CORE_SHARED_SOURCES}
		${CORE_SPEEX_LIBRARY}
		${CORE_ZLIB_LIBRARY}
	)

	if( WIN32 )
		include_directories( ${MOUNT_DIR} ${LIB_DIR}/SDL12/include ${LIB_DIR}/libcurl/ ${LIB_DIR}/libspeex/include ${LIB_DIR}/zlib )
		set_target_properties( qio PROPERTIES OUTPUT_NAME "qio${BUILD_ARCH}" PREFIX "" )
		ADD_DEFINITIONS( -D_WIN32 -DWIN32 -D_WINDOWS -D_CRT_SECURE_NO_WARNINGS -DBOTLIB -DUSE_ICON -DUSE_CURL -DUSE_CURL_DLOPEN -DUSE_OPENAL -DUSE_OPENAL_DLOPEN -DUSE_VOIP -DHAVE_CONFIG_H -DUSE_INTERNAL_JPEG)
		target_link_libraries( qio SDL SDLmain user32.lib advapi32.lib winmm.lib wsock32.lib ws2_32.lib msvcrt.lib psapi.lib Gdi32.lib )

		set_target_properties(qio PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:windows")
		set_target_properties(qio PROPERTIES COMPILE_DEFINITIONS_DEBUG "windows")
		set_target_properties(qio PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:windows")
		set_target_properties(qio PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "windows")
		set_target_properties(qio PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:windows")
		set_target_properties(qio PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows")
	else()
		include_directories( ${MOUNT_DIR} )
		set_target_properties( qio PROPERTIES OUTPUT_NAME "qio${BUILD_ARCH}" PREFIX "" )
	endif()
	set_target_properties( qio PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game
	)
endif()