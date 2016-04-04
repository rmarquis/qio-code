#
# QIO CMakeLists.txt
#
# cMake required (http://www.cmake.org/)
#

CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )
MESSAGE( STATUS "Using cmake version ${CMAKE_VERSION}" )

SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
SET( CMAKE_C_FLAGS_DEBUG "-DDEBUG" CACHE STRING "" )
SET( CMAKE_C_FLAGS_MINSIZEREL "-DNDEBUG -Os" CACHE STRING "" )
SET( CMAKE_C_FLAGS_RELEASE "-DNDEBUG" CACHE STRING "" )
SET( CMAKE_C_FLAGS_RELWITHDEBINFO "-DNDEBUG" CACHE STRING "" )
SET( CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS} CACHE STRING "" )
SET( CMAKE_CXX_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG} CACHE STRING "" )
SET( CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_C_FLAGS_MINSIZEREL} CACHE STRING "" )
SET( CMAKE_CXX_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE} CACHE STRING "" )
SET( CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} CACHE STRING "" )
SET( CMAKE_CXX_STACK_SIZE "8388608" )
SET( CMAKE_BUILD_TYPE Release CACHE STRING "Build configuration (Debug, Release, RelWithDebInfo, MinSizeRel)" )

# This line has to appear before 'PROJECT' in order to be able to disable incremental linking
SET( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" )
SET( MSVC_INCREMENTAL_DEFAULT ON )

PROJECT( QIO C CXX ASM )
SET( CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER} )
SET( CMAKE_ASM_COMPILE_OBJECT ${CMAKE_C_COMPILE_OBJECT} )
SET( CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" ) # Need this to get rid of -rdynamic linker flag set by default by cmake

# Options
IF( WIN32 )
#Dushan // only on Windows platform
#option( BUILD_DX12_RENDERER   "Build DirectX 12 renderer"                      OFF )
#option( BUILD_DX11_RENDERER   "Build DirectX 11 renderer"                      OFF )
OPTION( BUILD_DX10_RENDERER   "Build DirectX 10 renderer"                      ON )
OPTION( BUILD_DX9_RENDERER    "Build DirectX 9 renderer"                       ON )
ENDIF ( WIN32 )
OPTION( BUILD_GL_RENDERER     "Build new renderer library"                     ON )
OPTION( BUILD_NULL_RENDERER   "Build new renderer library"                     ON )
OPTION( BUILD_RENDERER        "Build renderer library"                         ON )
OPTION( BUILD_MATERIALSYSTEM  "Build QIO material system library"              ON )
OPTION( BUILD_IMAGELIB        "Build QIO image library"                        ON )
OPTION( BUILD_MODELLOADER     "Build QIO model loader library"                 ON )
OPTION( BUILD_CORE            "Build QIO core library"                         ON )
OPTION( BUILD_CM              "Build QIO cm library"                           ON )
OPTION( BUILD_DECLMANAGER     "Build QUI decal manager libery"                 ON )
OPTION( BUILD_GAME            "Build QIO game module library"                  ON )
OPTION( BUILD_CGAME           "Build QIO cgame module library"                 ON )
OPTION( BUILD_ENGINE          "Build QIO engine binary"                        ON )
IF( WIN32 )
OPTION( BUILD_EDITOR          "Build QIO editor module library"                ON )
ENDIF( WIN32 )
OPTION( BUILD_GPHYSICS_BULLET "Build QIO Bullet Physics module library"        ON )
OPTION( BUILD_GPHYSICS_ODE    "Build QIO ODE Physics module library"           ON )
#OPTION( BUILD_SOUND           "Build QIO sound module library"                OFF )
OPTION( BUILD_GUI             "Build QIO GUI module library"                   ON )
OPTION( BUILD_TIKI            "Build QIO TIKI module library"                  ON )
#OPTION( BUILD_RECAST          "Build QIO Recast module library"               OFF )

# Determine arch.
# Dushan - only x86 is supported at this moment
SET( BUILD_ARCH "x86" )

# Package info
SET( CPACK_PACKAGE_DESCRIPTION_SUMMARY "QIO client" )
SET( CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README" )
SET( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt" )
INCLUDE( CPack )

SET( MOUNT_DIR ${CMAKE_SOURCE_DIR}/code/ )
SET( LIB_DIR ${CMAKE_SOURCE_DIR}/code/external )
IF( WIN32 )
	SET( REF_DIR ${CMAKE_SOURCE_DIR}/misc/msvc )
ENDIF( WIN32 )
SET( ENGINEBUILD_DIR ${CMAKE_SOURCE_DIR}/game/)

#Dushan - Add library modules provided by QIO Engine
ADD_SUBDIRECTORY( ${LIB_DIR}/ )

FIND_PACKAGE( PkgConfig )

IF( WIN32 )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
ENDIF()

SET( CONSOLE "tty" )

# Determine platform
IF( WIN32 )
  SET( PLATFORM "win32" )
  SET( CONSOLE "win32" )
ELSEIF( UNIX )
  IF( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    ADD_DEFINITIONS( -DMACOS_X -DMACOS_X_ACCELERATION_HACK -D USE_FILE32API -DMACOS_X_CURSOR_HACK )
  ENDIF()
  SET( PLATFORM "unix" )
ENDIF( )

IF( WIN32 )
  #SET( OS_LIBRARIES m winmm wsock32 gdi32 ole32 ws2_32 psapi Iphlpapi advapi32 shell32 user32 )
  SET( OS_LIBRARIES m winmm ws2_32 psapi libz )
  SET( EXTRALIBS dinput dsound )
ELSEIF( UNIX )
  IF( CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" )
    SET( OS_LIBRARIES m z )
  ELSEIF( CMAKE_SYSTEM_NAME STREQUAL "Darwin" )
    SET( OS_LIBRARIES dl m z )
    # FIXME This looks wrong.
    SET( OS_LIBRARIES ${OS_LIBRARIES} "-framework AGL -framework OpenGL -framework Carbon -framework IOKit" )
  ELSE( )
    SET( OS_LIBRARIES ${CMAKE_DL_LIBS} m z rt )
  ENDIF( )
ENDIF( )



############################################
# Start of the list for Shared Build Files #
############################################

SET( MATHLIST_SOURCES
	${MOUNT_DIR}/math/vec3.cpp
)

SET( MATHLIST_HEADERS 
	${MOUNT_DIR}/math/vec3.h
)



#####################################################
# Start of the list for Material System Build Files #
#####################################################

SET( MATERIALSHAREDLIST_SOURCES
	${MOUNT_DIR}/shared/ast.cpp
	${MOUNT_DIR}/shared/autoCmd.cpp
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/tableList.cpp
	${MOUNT_DIR}/shared/waveForm.cpp
)

SET( MATERIALSHAREDLIST_HEADERS
	${MOUNT_DIR}/shared/array.h
	${MOUNT_DIR}/shared/ast.h
	${MOUNT_DIR}/shared/autoCmd.h
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/hashTableTemplate.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/safePtr.h
	${MOUNT_DIR}/shared/tableList.h
	${MOUNT_DIR}/shared/waveForm.h
)

SET( MATERIALSYSTEMLIST_SOURCES
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

SET( MATERIALSYSTEMLIST_HEADERS
	${MOUNT_DIR}/materialSystem/mat_cubeMap.h
	${MOUNT_DIR}/materialSystem/mat_impl.h
	${MOUNT_DIR}/materialSystem/mat_local.h
	${MOUNT_DIR}/materialSystem/mat_public.h
	${MOUNT_DIR}/materialSystem/mat_rgbGen.h
	${MOUNT_DIR}/materialSystem/mat_stageTexture.h
	${MOUNT_DIR}/materialSystem/mat_texMods.h
)


########################################################
# End of the file list for Material System Build Files #
########################################################

#####################################################
# Start of the list for Material System Build Files #
#####################################################

SET( IMAGELIB_SOURCES
	${MOUNT_DIR}/imageLib/img_api.cpp
	${MOUNT_DIR}/imageLib/img_convert.cpp
	${MOUNT_DIR}/imageLib/img_devil.cpp
	${MOUNT_DIR}/imageLib/img_main.cpp
	${MOUNT_DIR}/imageLib/img_utils.cpp
	${MOUNT_DIR}/imageLib/img_webp.cpp
	${MOUNT_DIR}/imageLib/img_write.cpp
)

SET( IMAGELIB_HEADERS 
	${MOUNT_DIR}/imageLib/img_local.h
)

#############################################################
# End of the file list for Image Library System Build Files #
#############################################################

################################################
# Start of the list for DirectX 10 Build Files #
################################################

SET( DIRECTX10LIB_SOURCES
	${MOUNT_DIR}/renderer/backEndDX10/backendDX10API.cpp
)

SET( DIRECTXANDNULLRENDERER_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/shared/str.h
)

SET( DIRECTXANDNULLRENDERER_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/shared/str.cpp
)

###################################################
# Start of the list for NULL renderer Build Files #
###################################################

SET( NULLRENDERER_SOURCES
	${MOUNT_DIR}/renderer/backEndNULL/backEndNULLAPI.cpp
)

###############################################
# Start of the list for DirectX 9 Build Files #
###############################################

SET( DIRECTX9LIB_HEADERS 
	${MOUNT_DIR}/renderer/backEndDX9/dx9_local.h
	${MOUNT_DIR}/renderer/backEndDX9/dx9_shader.h
)

SET( DIRECTX9LIB_SOURCES
	${MOUNT_DIR}/renderer/backEndDX9/backendDX9API.cpp
	${MOUNT_DIR}/renderer/backEndDX9/dx9_shader.cpp
)

SET( DIRECTX9LIB_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/calcTBN.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/renderer/rVertexBuffer.h
)

SET( DIRECTX9LIB_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/calcTBN.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/renderer/rVertexBuffer.cpp
)

SET ( RENDERERGL_APIS_HEADERS
	${MOUNT_DIR}/api/tableListAPI.h
)

SET ( RENDERERGL_SHARED_HEADERS
	${MOUNT_DIR}/shared/ast_input.h
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/byteRGB.h
	${MOUNT_DIR}/shared/calcTBN.h
	${MOUNT_DIR}/shared/cullType.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/renderer/rVertexBuffer.h
)

SET ( RENDERERGL_SHARED_SOURCES
	${MOUNT_DIR}/shared/ast_input.cpp
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/calcTBN.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/renderer/rVertexBuffer.cpp
)

SET ( RENDERERGL_HEADERS
	${MOUNT_DIR}/renderer/backEndGL/gl_local.h
	${MOUNT_DIR}/renderer/backEndGL/gl_shader.h
)

SET ( RENDERERGL_SOURCES
	${MOUNT_DIR}/renderer/backEndGL/backEndGLAPI.cpp
	${MOUNT_DIR}/renderer/backEndGL/gl_main.cpp
	${MOUNT_DIR}/renderer/backEndGL/gl_shader.cpp
)

SET ( IMGLIB_HEADERS
	${MOUNT_DIR}/imageLib/img_local.h
)

SET ( IMGLIB_SOURCES
	${MOUNT_DIR}/imageLib/img_api.cpp
	${MOUNT_DIR}/imageLib/img_convert.cpp
	${MOUNT_DIR}/imageLib/img_devil.cpp
	${MOUNT_DIR}/imageLib/img_main.cpp
	${MOUNT_DIR}/imageLib/img_utils.cpp
	${MOUNT_DIR}/imageLib/img_write.cpp
)

SET ( RENDERERAPI_HEADERS
	${MOUNT_DIR}/api/boxTrianglesCallback.h
	${MOUNT_DIR}/api/cubeMapAPI.h
	${MOUNT_DIR}/api/customRenderObjectAPI.h
	${MOUNT_DIR}/api/fontAPI.h
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

SET ( RENDERERFILEFORMATS_HEADERS
	${MOUNT_DIR}/fileformats/bspFileFormat.h
	${MOUNT_DIR}/fileformats/bspFileFormat_hl.h
	${MOUNT_DIR}/fileformats/bspFileFormat_hl2.h
	${MOUNT_DIR}/fileformats/bspFileFormat_q2.h
)

SET ( RENDERER_HEADERS
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

SET ( RENDERER_MATH_HEADERS
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

SET ( RENDERER_MATH_SOURCES
	${MOUNT_DIR}/math/aabb.cpp
	${MOUNT_DIR}/math/axis.cpp
	${MOUNT_DIR}/math/frustum.cpp
	${MOUNT_DIR}/math/frustumExt.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/plane.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

SET ( RENDERER_SHARED_HEADERS
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
	${MOUNT_DIR}/shared/colorTable.h
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
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/r2dVert.h
	${MOUNT_DIR}/shared/readStream.h
	${MOUNT_DIR}/renderer/rfSurfFlags.h
	${MOUNT_DIR}/shared/simpleTexturedPoly.h
	${MOUNT_DIR}/shared/simpleVert.h
	${MOUNT_DIR}/shared/singleAnimLerp.h
	${MOUNT_DIR}/shared/skelUtils.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/texCoordCalc.h
	${MOUNT_DIR}/shared/textureAllocator.h
	${MOUNT_DIR}/shared/trace.h
	${MOUNT_DIR}/shared/vec3hash.h
	${MOUNT_DIR}/shared/wolfAnimCfg.h
	${MOUNT_DIR}/shared/wolfAnimScript.h
)

SET ( RENDERER_SHARED_SOURCES
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
	${MOUNT_DIR}/shared/colorTable.cpp
	${MOUNT_DIR}/shared/displacementBuilder.cpp
	${MOUNT_DIR}/shared/entDefsList.cpp
	${MOUNT_DIR}/shared/ePairsList.cpp
	${MOUNT_DIR}/shared/etCharMgr.cpp
	${MOUNT_DIR}/shared/mapBezierPatch.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/perlinNoise.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/readStream.cpp
	${MOUNT_DIR}/shared/readStream.cpp
	${MOUNT_DIR}/renderer/rVertexBuffer.cpp
	${MOUNT_DIR}/shared/skelUtils.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/tagOr.cpp
	${MOUNT_DIR}/shared/textureAllocator.cpp
	${MOUNT_DIR}/shared/trace.cpp
	${MOUNT_DIR}/shared/wolfAnimCfg.cpp
	${MOUNT_DIR}/shared/wolfAnimScript.cpp
)

SET ( RENDERER_SOURCES
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
	${MOUNT_DIR}/renderer/frontend/rf_font.cpp
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

SET ( MODELLOADER_API
	${MOUNT_DIR}/api/kfModelAPI.h
	${MOUNT_DIR}/api/modelLoaderDLLAPI.h
	${MOUNT_DIR}/api/skelAnimAPI.h
	${MOUNT_DIR}/api/skelAnimPostProcessFuncs.h
	${MOUNT_DIR}/api/skelModelAPI.h
	${MOUNT_DIR}/api/staticModelCreatorAPI.h
)

SET ( MODELLOADERFILEFORMATS 
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
	${MOUNT_DIR}/fileformats/skd_file_format.h
	${MOUNT_DIR}/fileformats/tan_file_format.h
	${MOUNT_DIR}/fileformats/vvdFileFormat.h
)

SET ( MODELLOADER_HEADERS
	${MOUNT_DIR}/modelLoader/hl2MDLReader.h
	${MOUNT_DIR}/modelLoader/modelLoaderLocal.h
)

SET ( MODELLOADER_KEYFRAMEDMODELLOADERS_HEADERS
	${MOUNT_DIR}/modelLoader/keyFramedModelImpl.h
)
SET ( MODELLOADER_KEYFRAMEDMODELLOADERS_SOURCES
	${MOUNT_DIR}/modelLoader/keyFramedModelImpl.cpp
)

SET ( MODELLOADER_MATH_HEADERS
	${MOUNT_DIR}/math/aabb.h
	${MOUNT_DIR}/math/axis.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/math/quat.h
	${MOUNT_DIR}/math/vec2.h
	${MOUNT_DIR}/math/vec3.h
)

SET ( MODELLOADER_MATH_SOURCES
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

SET ( MODELLOADER_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/cmWinding.h
	${MOUNT_DIR}/shared/extraSurfEdgesData.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/readStream.h
	${MOUNT_DIR}/shared/skelUtils.h
	${MOUNT_DIR}/shared/stringHashTable.h
	${MOUNT_DIR}/shared/tagOr.h
	${MOUNT_DIR}/shared/textureAxisFromNormal.h
)

SET ( MODELLOADER_SHARED_SOURCES 
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/cmWinding.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/readStream.cpp
	${MOUNT_DIR}/shared/skelUtils.cpp
	${MOUNT_DIR}/shared/textureAxisFromNormal.cpp
)

SET ( MODELLOADER_SKELETALMODELLOADERS
	${MOUNT_DIR}/modelLoader/sk_local.h
	${MOUNT_DIR}/modelLoader/skelAnimImpl.cpp
	${MOUNT_DIR}/modelLoader/skelAnimImpl.h
	${MOUNT_DIR}/modelLoader/skelModelImpl.cpp
	${MOUNT_DIR}/modelLoader/skelModelImpl.h
)

SET ( MODELLOADER_SOURCES
	${MOUNT_DIR}/modelLoader/hl2MDLReader.cpp
	${MOUNT_DIR}/modelLoader/mod_postProcess.cpp
	${MOUNT_DIR}/modelLoader/modelLoaderAPI.cpp
	${MOUNT_DIR}/modelLoader/skel_animPostProcess.cpp
)

SET ( MODELLOADER_STATICMODELLOADERS_HEADERS
	${MOUNT_DIR}/modelLoader/staticModelLoaders/mtlFile.h
)

SET ( MODELLOADER_STATICMODELLOADERS_SOURCES
	${MOUNT_DIR}/modelLoader/staticModelLoaders/3dsLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/aseLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/heightmapLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/lwoLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/mapFileConverter.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/md3StaticLoader.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/mtlFile.cpp
	${MOUNT_DIR}/modelLoader/staticModelLoaders/wavefrontOBJModelLoader.cpp
)

SET ( CM_HEADERS 
	${MOUNT_DIR}/cm/cm_anims.h
	${MOUNT_DIR}/cm/cm_helper.h
	${MOUNT_DIR}/cm/cm_local.h
	${MOUNT_DIR}/cm/cm_model.h
	${MOUNT_DIR}/api/cmAPI.h
)

SET ( CM_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.h
	${MOUNT_DIR}/shared/cmBezierPatch.h
	${MOUNT_DIR}/shared/cmBrush.h
	${MOUNT_DIR}/shared/cmSurface.h
	${MOUNT_DIR}/shared/cmTriSoupOctTree.h
	${MOUNT_DIR}/shared/cmWinding.h
	${MOUNT_DIR}/shared/collisionUtils.h
	${MOUNT_DIR}/api/colMeshBuilderAPI.h
	${MOUNT_DIR}/shared/displacementBuilder.h
	${MOUNT_DIR}/shared/ePairsList.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/fileStreamHelper.h
	${MOUNT_DIR}/shared/perPlaneCallback.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/readStream.h
	${MOUNT_DIR}/shared/trace.h
	${MOUNT_DIR}/api/writeStreamAPI.h
)

SET ( CM_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.cpp
	${MOUNT_DIR}/shared/cmBezierPatch.cpp
	${MOUNT_DIR}/shared/cmBrush.cpp
	${MOUNT_DIR}/shared/cmSurface.cpp
	${MOUNT_DIR}/shared/cmTriSoupOctTree.cpp
	${MOUNT_DIR}/shared/cmWinding.cpp
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/displacementBuilder.cpp
	${MOUNT_DIR}/shared/ePairsList.cpp
	${MOUNT_DIR}/shared/mapBezierPatch.cpp
	${MOUNT_DIR}/shared/mapBezierPatch.h
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/fileStreamHelper.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/readStream.cpp
	${MOUNT_DIR}/shared/trace.cpp
)

SET ( CM_SOURCE
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

SET ( DECLMANAGER_API
	${MOUNT_DIR}/api/afDeclAPI.h
	${MOUNT_DIR}/api/declManagerAPI.h
	${MOUNT_DIR}/api/entDefAPI.h
	${MOUNT_DIR}/api/entityDeclAPI.h
	${MOUNT_DIR}/api/modelDeclAPI.h
	${MOUNT_DIR}/api/particleDeclAPI.h
	${MOUNT_DIR}/api/q3PlayerModelDeclAPI.h
)

SET ( DECLMANAGER_HEADER
	${MOUNT_DIR}/declManager/articulatedFigure.h
	${MOUNT_DIR}/declManager/declManagerIMPL.h
	${MOUNT_DIR}/declManager/declRefState.h
	${MOUNT_DIR}/declManager/particleDecl.h
	${MOUNT_DIR}/declManager/q3PlayerModelDecl.h
)

SET ( DECLMANAGER_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/entDef.h
	${MOUNT_DIR}/shared/ePairsList.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/quake3AnimationConfig.h
	${MOUNT_DIR}/shared/random.h
	${MOUNT_DIR}/shared/stringList.h
	${MOUNT_DIR}/shared/tagOr.h
)

SET ( DECLMANAGER_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/entDef.cpp
	${MOUNT_DIR}/shared/ePairsList.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/quake3AnimationConfig.cpp
	${MOUNT_DIR}/shared/random.cpp
	${MOUNT_DIR}/shared/tagOr.cpp
)

SET ( DECLMANAGER_SOURCE
	${MOUNT_DIR}/declManager/articulatedFigure.cpp
	${MOUNT_DIR}/declManager/declManagerIMPL.cpp
	${MOUNT_DIR}/declManager/particleDecl.cpp
	${MOUNT_DIR}/declManager/q3PlayerModelDecl.cpp
)

SET ( GAME_CLASSES_CSS_HEADERS
	${MOUNT_DIR}/game/classes/WeaponMP5.h
)

SET ( GAME_CLASSES_CSS_SOURCES
	${MOUNT_DIR}/game/classes/WeaponMP5.cpp
)

SET ( GAME_CLASSES_ET_HEADERS
	${MOUNT_DIR}/game/classes/Corona.h
	${MOUNT_DIR}/game/classes/MiscGameModel.h
)

SET ( GAME_CLASSES_ET_SOURCES
	${MOUNT_DIR}/game/classes/Corona.cpp
	${MOUNT_DIR}/game/classes/MiscGameModel.cpp
)

SET ( GAME_CLASSES_QUAKE3_HEADERS
	${MOUNT_DIR}/game/classes/PortalCamera.h
	${MOUNT_DIR}/game/classes/PortalSurface.h
	${MOUNT_DIR}/game/classes/Q3Weapon.h
	${MOUNT_DIR}/game/classes/TriggerPush.h
	${MOUNT_DIR}/game/classes/TriggerTeleport.h
)

SET ( GAME_CLASSES_QUAKE3_SOURCES
	${MOUNT_DIR}/game/classes/PortalCamera.cpp
	${MOUNT_DIR}/game/classes/PortalSurface.cpp
	${MOUNT_DIR}/game/classes/Q3Weapon.cpp
	${MOUNT_DIR}/game/classes/TriggerPush.cpp
	${MOUNT_DIR}/game/classes/TriggerTeleport.cpp
)

SET ( GAME_CLASSES_RTCW_HEADERS
	${MOUNT_DIR}/game/classes/AI_Loper.h
	${MOUNT_DIR}/game/classes/AI_RTCW_Base.h
	${MOUNT_DIR}/game/classes/AI_Soldier.h
	${MOUNT_DIR}/game/classes/FuncInvisibleUser.h
	${MOUNT_DIR}/game/classes/TargetScriptTrigger.h
)

SET ( GAME_CLASSES_RTCW_SOURCES
	${MOUNT_DIR}/game/classes/AI_Loper.cpp
	${MOUNT_DIR}/game/classes/AI_RTCW_Base.cpp
	${MOUNT_DIR}/game/classes/AI_Soldier.cpp
	${MOUNT_DIR}/game/classes/FuncInvisibleUser.cpp
	${MOUNT_DIR}/game/classes/TargetScriptTrigger.cpp
)

SET ( GAME_CLASSES_FAKK_HEADERS
	${MOUNT_DIR}/game/classes/Actor.h
	${MOUNT_DIR}/game/classes/Shield.h
	${MOUNT_DIR}/game/classes/Sword.h
	${MOUNT_DIR}/game/classes/SoulSucker.h
	${MOUNT_DIR}/game/classes/Flamethrower.h
	${MOUNT_DIR}/game/classes/Flashbang.h
	${MOUNT_DIR}/game/classes/FuncSpawn.h
)

SET ( GAME_CLASSES_FAKK_SOURCES
	${MOUNT_DIR}/game/classes/Actor.cpp
	${MOUNT_DIR}/game/classes/Shield.cpp
	${MOUNT_DIR}/game/classes/Sword.cpp
	${MOUNT_DIR}/game/classes/SoulSucker.cpp
	${MOUNT_DIR}/game/classes/Flamethrower.cpp
	${MOUNT_DIR}/game/classes/Flashbang.cpp
	${MOUNT_DIR}/game/classes/FuncSpawn.cpp
)

SET ( GAME_CLASSES_HEADERS
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
	${MOUNT_DIR}/game/classes/TriggerOnce.h
	${MOUNT_DIR}/game/classes/VehicleCar.h
	${MOUNT_DIR}/game/classes/TriggerSoundscape.h
	${MOUNT_DIR}/game/classes/Weapon.h
	${MOUNT_DIR}/game/classes/Weapon_PhysGun.h
	${MOUNT_DIR}/game/classes/Weapon_QioFlashLight.h
	${MOUNT_DIR}/game/classes/World.h
)

SET ( GAME_CLASSES_SOURCES
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
	${MOUNT_DIR}/game/classes/TriggerOnce.cpp
	${MOUNT_DIR}/game/classes/TriggerSoundscape.cpp
	${MOUNT_DIR}/game/classes/VehicleCar.cpp
	${MOUNT_DIR}/game/classes/Weapon.cpp
	${MOUNT_DIR}/game/classes/Weapon_PhysGun.cpp
	${MOUNT_DIR}/game/classes/Weapon_QioFlashLight.cpp
	${MOUNT_DIR}/game/classes/World.cpp
)

SET ( GAME_HEADERS
	${MOUNT_DIR}/game/bg_public.h
	${MOUNT_DIR}/game/bt_include.h
	${MOUNT_DIR}/game/edict.h
	${MOUNT_DIR}/game/explosionInfo.h
	${MOUNT_DIR}/game/g_classes.h
	${MOUNT_DIR}/game/g_local.h
	${MOUNT_DIR}/game/g_pathNodes.h
	${MOUNT_DIR}/game/g_physVehicleAPI.h
	${MOUNT_DIR}/game/g_ragdoll.h
	${MOUNT_DIR}/game/g_scriptedClasses.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/api/simplePathAPI.h
	${MOUNT_DIR}/api/stateConditionsHandlerAPI.h
	${MOUNT_DIR}/api/stateMachineAPI.h
	${MOUNT_DIR}/game/stateMachine.h
	${MOUNT_DIR}/game/stateMachineShared.h
	${MOUNT_DIR}/game/stateMachineManager.h
	${MOUNT_DIR}/qcommon/surfaceflags.h
)

SET ( GAME_LUA_HEADERS
	${MOUNT_DIR}/game/lua/g_lua.h
	${MOUNT_DIR}/game/lua/g_lua_eventHandler.h

)

SET ( GAME_LUA_SOURCES
	${MOUNT_DIR}/game/lua/g_lua.cpp
	${MOUNT_DIR}/game/lua/g_lua_entity.cpp
	${MOUNT_DIR}/game/lua/g_lua_eventHandler.cpp
	${MOUNT_DIR}/game/lua/g_lua_game.cpp
	${MOUNT_DIR}/game/lua/g_lua_qmath.cpp
	${MOUNT_DIR}/game/lua/g_lua_vector.cpp
)

SET ( GAME_LUALIBRARY
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

SET ( GAME_MATH_HEADERS
	${MOUNT_DIR}/math/aabb.h
	${MOUNT_DIR}/math/axis.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/math/vec3.h
)

SET ( GAME_MATH_SOURCES
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

SET ( GAME_PROTOCOL
	${MOUNT_DIR}/protocol/entityState.h
	${MOUNT_DIR}/protocol/gameState.h
	${MOUNT_DIR}/protocol/netLimits.h
	${MOUNT_DIR}/protocol/playerState.cpp
	${MOUNT_DIR}/protocol/playerState.h
	${MOUNT_DIR}/protocol/snapFlags.h
	${MOUNT_DIR}/protocol/userCmd.h
	${MOUNT_DIR}/protocol/voipFlags.h
)

SET ( GAME_SHARED_HEADERS
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
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/trace.h
	${MOUNT_DIR}/shared/wolfScript.h
)

SET ( GAME_SHARED_SOURCES
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
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/skelUtils.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/trace.cpp
	${MOUNT_DIR}/shared/wolfScript.cpp
)

SET ( GAME_SOURCE
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
	${MOUNT_DIR}/game/g_stateMachines.cpp
	${MOUNT_DIR}/game/g_utils.cpp
	${MOUNT_DIR}/game/g_weapons.cpp
	${MOUNT_DIR}/game/g_wolfScript.cpp
	${MOUNT_DIR}/game/stateMachine.cpp
	${MOUNT_DIR}/game/stateMachineManager.cpp
)

SET ( BUILD_GPHYSICS_BULLET_HEADERS
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

SET ( BUILD_GPHYSICS_BULLET_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.h
	${MOUNT_DIR}/shared/cmBezierPatch.h
	${MOUNT_DIR}/shared/cmSurface.h
	${MOUNT_DIR}/shared/collisionUtils.h
	${MOUNT_DIR}/shared/displacementBuilder.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/trace.h
)

SET ( BUILD_GPHYSICS_BULLET_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.cpp
	${MOUNT_DIR}/shared/cmBezierPatch.cpp
	${MOUNT_DIR}/shared/cmSurface.cpp
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/displacementBuilder.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/trace.cpp
)

SET ( BUILD_GPHYSICS_BULLET_SOURCES
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

SET ( BUILD_CGAME_HEADER
	${MOUNT_DIR}/game/bg_public.h
	${MOUNT_DIR}/cgame/cg_emitter.h
	${MOUNT_DIR}/cgame/cg_emitter_base.h
	${MOUNT_DIR}/cgame/cg_emitter_d3.h
	${MOUNT_DIR}/cgame/cg_local.h
	${MOUNT_DIR}/cgame/cg_public.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/qcommon/surfaceflags.h
)

SET ( BUILD_CGAME_MATH_HEADERS
	${MOUNT_DIR}/math/axis.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/math/vec3.h
)

SET ( BUILD_CGAME_MATH_SOURCES
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

SET ( BUILD_CGAME_PROTOCOL
	${MOUNT_DIR}/protocol/entityState.h
	${MOUNT_DIR}/protocol/netLimits.h
	${MOUNT_DIR}/protocol/playerState.cpp
	${MOUNT_DIR}/protocol/playerState.h
	${MOUNT_DIR}/protocol/snapFlags.h
	${MOUNT_DIR}/protocol/userCmd.h
	${MOUNT_DIR}/protocol/voipFlags.h
)

SET ( BUILD_CGAME_SHARED_HEADERS
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
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/trace.h
)

SET ( BUILD_CGAME_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCmd.cpp
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/colorTable.cpp
	${MOUNT_DIR}/shared/infoString.cpp
	${MOUNT_DIR}/shared/random.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/trace.cpp
)

SET ( BUILD_CGAME_SOURCE
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
	${MOUNT_DIR}/cgame/cg_tempLights.cpp
	${MOUNT_DIR}/cgame/cg_testEmitter.cpp
	${MOUNT_DIR}/cgame/cg_testMaterial.cpp
	${MOUNT_DIR}/cgame/cg_testModel.cpp
	${MOUNT_DIR}/cgame/cg_tracer.cpp
	${MOUNT_DIR}/cgame/cg_view.cpp
	${MOUNT_DIR}/cgame/cg_viewModel.cpp
	${MOUNT_DIR}/shared/str.cpp

)

SET ( GPHYSICS_ODE_HEADERS
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

SET ( GPHYSICS_ODE_SHARED_HEADERS
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.h
	${MOUNT_DIR}/shared/cmBezierPatch.h
	${MOUNT_DIR}/shared/collisionUtils.h
	${MOUNT_DIR}/shared/displacementBuilder.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/shared/trace.h
)

SET ( GPHYSICS_ODE_SHARED_SOURCES
	${MOUNT_DIR}/shared/bspPhysicsDataLoader.cpp
	${MOUNT_DIR}/shared/cmBezierPatch.cpp
	${MOUNT_DIR}/shared/collisionUtils.cpp
	${MOUNT_DIR}/shared/displacementBuilder.cpp
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/shared/trace.cpp
)

SET ( GPHYSICS_ODE_SOURCE
	${MOUNT_DIR}/gphysics/ode/ode_api.cpp
	${MOUNT_DIR}/gphysics/ode/ode_characterController.cpp
	${MOUNT_DIR}/gphysics/ode/ode_cMod2ODEGeom.cpp
	${MOUNT_DIR}/gphysics/ode/ode_constraint.cpp
	${MOUNT_DIR}/gphysics/ode/ode_rigidBody.cpp
	${MOUNT_DIR}/gphysics/ode/ode_shape.cpp
	${MOUNT_DIR}/gphysics/ode/ode_staticMapLoader.cpp
	${MOUNT_DIR}/gphysics/ode/ode_world.cpp
)

IF( WIN32 )
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

	SET ( EDITOR_MATH_HEADERS
		${MOUNT_DIR}/math/aabb.h
		${MOUNT_DIR}/math/vec3.h
	)

	SET ( EDITOR_MATH_SOURCES
		${MOUNT_DIR}/math/aabb.cpp
		${MOUNT_DIR}/math/vec3.cpp
	)

	SET ( EDITOR_SHARED_HEADERS
		${MOUNT_DIR}/shared/brush.h
		${MOUNT_DIR}/shared/brushside.h
		${MOUNT_DIR}/qcommon/q_platform.h
		${MOUNT_DIR}/shared/str.h
		${MOUNT_DIR}/shared/simpleModel.h
		${MOUNT_DIR}/shared/textureAxisFromNormal.h
		${MOUNT_DIR}/shared/texturedVertex.h
		${MOUNT_DIR}/shared/texturedWinding.h
	)

	SET ( EDITOR_SHARED_SOURCES
		${MOUNT_DIR}/shared/brush.cpp
		${MOUNT_DIR}/shared/brushside.cpp
		${MOUNT_DIR}/shared/parser.cpp
		${MOUNT_DIR}/shared/str.cpp
		${MOUNT_DIR}/shared/textureAxisFromNormal.cpp
		${MOUNT_DIR}/shared/texturedWinding.cpp
	)

	SET ( EDITOR_HEADERS
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

	SET ( EDITOR_SOURCES
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
ENDIF()

SET ( CORE_APIS
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

SET ( CORE_HEADER_FILES
	${MOUNT_DIR}/shared/array.h
	${MOUNT_DIR}/game/bg_public.h
	${MOUNT_DIR}/cgame/cg_public.h
	${MOUNT_DIR}/client/cl_curl.h
	${MOUNT_DIR}/client/client.h
	${MOUNT_DIR}/api/iFaceBase.h
	${MOUNT_DIR}/api/iFaceMgrAPI.h
	${MOUNT_DIR}/client/keys.h
	${MOUNT_DIR}/shared/lightFlags.h
	${MOUNT_DIR}/qcommon/puff.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/qcommon/qcommon.h
	${MOUNT_DIR}/server/server.h
	${MOUNT_DIR}/qcommon/surfaceflags.h
	${MOUNT_DIR}/shared/typedefs.h
	${MOUNT_DIR}/qcommon/unzip.h
	${MOUNT_DIR}/server/sv_vis.h
	${MOUNT_DIR}/sys/sys_loadlib.h
	${MOUNT_DIR}/sys/sys_local.h
	${MOUNT_DIR}/shared/str.h
)

SET ( CORE_MATH_HEADERS
	${MOUNT_DIR}/math/aabb.h
	${MOUNT_DIR}/math/axis.h
	${MOUNT_DIR}/math/math.h
	${MOUNT_DIR}/math/matrix.h
	${MOUNT_DIR}/math/plane.h
	${MOUNT_DIR}/math/quat.h
	${MOUNT_DIR}/math/vec2.h
	${MOUNT_DIR}/math/vec3.h
)

SET ( CORE_MATH_SOURCES
	${MOUNT_DIR}/math/matrix.cpp
	${MOUNT_DIR}/math/plane.cpp
	${MOUNT_DIR}/math/vec3.cpp
)

SET ( CORE_PROTOCOL
	${MOUNT_DIR}/protocol/entityState.h
	${MOUNT_DIR}/protocol/gameState.h
	${MOUNT_DIR}/protocol/netLimits.h
	${MOUNT_DIR}/protocol/playerState.h
	${MOUNT_DIR}/protocol/snapFlags.h
	${MOUNT_DIR}/protocol/userCmd.h
	${MOUNT_DIR}/protocol/voipFlags.h
)

SET ( CORE_SDL_HEADERS
	${MOUNT_DIR}/sdl/sdl_glConfig.h
)

SET ( CORE_SDL_SOURCES
	${MOUNT_DIR}/sdl/sdl_glimp.cpp
	${MOUNT_DIR}/sdl/sdl_input.cpp
)

SET ( CORE_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/bitSet.h
	${MOUNT_DIR}/shared/bspBoxDesc.h
	${MOUNT_DIR}/shared/cmWinding.h
	${MOUNT_DIR}/shared/colorTable.h
	${MOUNT_DIR}/shared/cvar.h
	${MOUNT_DIR}/shared/cvarFlags.h
	${MOUNT_DIR}/shared/cvarModificationCallback.h
	${MOUNT_DIR}/shared/doom3ProcPVSClass.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/portalizedBSPTree.h
	${MOUNT_DIR}/shared/shared.h
	${MOUNT_DIR}/shared/str.h
	${MOUNT_DIR}/shared/infoString.h
	${MOUNT_DIR}/shared/keyCatchers.h
)

SET ( CORE_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/cmWinding.cpp
	${MOUNT_DIR}/shared/colorTable.cpp
	${MOUNT_DIR}/shared/doom3ProcPVSClass.cpp
	${MOUNT_DIR}/shared/parser.cpp
	${MOUNT_DIR}/shared/portalizedBSPTree.cpp
	${MOUNT_DIR}/shared/str.cpp
	${MOUNT_DIR}/shared/infoString.cpp
)

SET ( CORE_SOURCE_FILES
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
	${MOUNT_DIR}/client/cl_gui.cpp
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
	${MOUNT_DIR}/shared/str.cpp
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

SET ( CORE_SPEEX_LIBRARY
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

SET ( CORE_ZLIB_LIBRARY
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

SET ( GUI_SHARED_HEADERS
	${MOUNT_DIR}/shared/autoCvar.h
	${MOUNT_DIR}/shared/parser.h
	${MOUNT_DIR}/shared/rect.h
)

SET ( GUI_SHARED_SOURCES
	${MOUNT_DIR}/shared/autoCvar.cpp
	${MOUNT_DIR}/shared/parser.cpp
)

SET ( GUI_HEADERS
	${MOUNT_DIR}/gui/urc_element_base.h
	${MOUNT_DIR}/gui/urc_element_bindList.h
	${MOUNT_DIR}/gui/urc_element_button.h
	${MOUNT_DIR}/gui/urc_element_checkbox.h
	${MOUNT_DIR}/gui/urc_element_field.h
	${MOUNT_DIR}/gui/urc_element_label.h
	${MOUNT_DIR}/gui/urc_element_pulldown.h
	${MOUNT_DIR}/gui/urc_element_slider.h
	${MOUNT_DIR}/gui/urc_mgr.h
	${MOUNT_DIR}/gui/urc.h
)

SET ( GUI_SOURCES
	${MOUNT_DIR}/gui/urc_element_base.cpp
	${MOUNT_DIR}/gui/urc_element_bindList.cpp
	${MOUNT_DIR}/gui/urc_element_button.cpp
	${MOUNT_DIR}/gui/urc_element_checkbox.cpp
	${MOUNT_DIR}/gui/urc_element_field.cpp
	${MOUNT_DIR}/gui/urc_element_label.cpp
	${MOUNT_DIR}/gui/urc_element_pulldown.cpp
	${MOUNT_DIR}/gui/urc_element_slider.cpp
	${MOUNT_DIR}/gui/urc_mgr.cpp
	${MOUNT_DIR}/gui/urc.cpp
	${MOUNT_DIR}/gui/gui_api.cpp
)

SET ( TIKI_SHARED_HEADERS
	${MOUNT_DIR}/shared/parser.h
)

SET ( TIKI_SHARED_SOURCES
	${MOUNT_DIR}/shared/parser.cpp
)

SET ( TIKI_HEADERS
	${MOUNT_DIR}/tiki/tiki_local.h
	${MOUNT_DIR}/api/tikiAPI.h
)

SET ( TIKI_SOURCES
	${MOUNT_DIR}/tiki/tiki_api.cpp
)


IF( BUILD_MATERIALSYSTEM )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${MATERIALSHAREDLIST_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${MATERIALSHAREDLIST_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${MATERIALSYSTEMLIST_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${MATERIALSYSTEMLIST_SOURCES} )
	ENDIF()

	ADD_LIBRARY( materialSystem 
		SHARED 
		${MATERIALSHAREDLIST_HEADERS} 
		${MATERIALSHAREDLIST_SOURCES} 
		${MATHLIST_HEADERS} 
		${MATHLIST_SOURCES} 
		${MATERIALSYSTEMLIST_SOURCES} 
		${MATERIALSYSTEMLIST_HEADERS} 
	)
	TARGET_INCLUDE_DIRECTORIES( materialSystem PRIVATE ${MOUNT_DIR} )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( materialSystem PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/materialSystem.def" OUTPUT_NAME "materialSystem${BUILD_ARCH}" PREFIX "" )
	ELSE()
		SET_TARGET_PROPERTIES( materialSystem PROPERTIES OUTPUT_NAME "materialSystemx${BUILD_ARCH}" PREFIX "" )
	TARGET_LINK_LIBRARIES( materialSystem )
	ENDIF()
	SET_TARGET_PROPERTIES( materialSystem PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF( BUILD_IMAGELIB )
	# This section finds and provides REQUIRED dependencies to imageLib target
	#find_package( libDevil REQUIRED )
	#find_package( vtflib REQUIRED )

	# Dushan - This is how Visual Studio directories (or filters) are generated
	IF( WIN32 )
		SOURCE_GROUP( Header\ Files                         FILES ${IMAGELIB_HEADERS} )
		SOURCE_GROUP( Source\ Files                         FILES ${IMAGELIB_SOURCES} )	
	ENDIF ( WIN32 )
	
	ADD_LIBRARY( imageLib 
		SHARED 
		${IMAGELIB_SOURCES}
		${IMAGELIB_HEADERS}
	)
	TARGET_INCLUDE_DIRECTORIES( imageLib PRIVATE ${MOUNT_DIR} ${LIB_DIR}/il-1.7.8-x86/include ${LIB_DIR}/vtflib132-bin/lib ${LIB_DIR}/libwebp/include )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( imageLib PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/imageLib.def" OUTPUT_NAME "imageLib${BUILD_ARCH}" PREFIX "" )
		TARGET_LINK_LIBRARIES( imageLib ${LIB_DIR}/vtflib132-bin/lib/x86/VTFLib.lib ${LIB_DIR}/il-1.7.8-x86/lib/DevIL.lib ${LIB_DIR}/libwebp/Lib/libwebp.lib )
	ELSE()
		SET_TARGET_PROPERTIES( imageLib PROPERTIES OUTPUT_NAME "imageLibx${BUILD_ARCH}" PREFIX "" )
		TARGET_LINK_LIBRARIES( imageLib )
	ENDIF()
	SET_TARGET_PROPERTIES( imageLib PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF( WIN32 )
	IF( BUILD_DX9_RENDERER )
		# This section finds and provides REQUIRED dependencies to DirectX9 target
		FIND_PACKAGE( DirectX REQUIRED )

		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${DIRECTX9LIB_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${DIRECTX9LIB_SHARED_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${DIRECTX9LIB_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${DIRECTX9LIB_SOURCES} )
		
		ADD_LIBRARY( backendDX9 
			SHARED 
			${MATHLIST_SOURCES} 
			${MATHLIST_HEADERS} 
			${DIRECTX9LIB_SOURCES}
			${DIRECTX9LIB_HEADERS}
			${DIRECTX9LIB_SHARED_HEADERS}
			${DIRECTX9LIB_SHARED_SOURCES}
		)
		TARGET_INCLUDE_DIRECTORIES( backendDX9 PRIVATE "${MOUNT_DIR}" "${DXSDK_INCLUDE_DIR}" "${LIB_DIR}/SDL12/include" )
		SET_TARGET_PROPERTIES( backendDX9 PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendDX9.def" OUTPUT_NAME "backendDX9${BUILD_ARCH}" PREFIX "" )
		TARGET_LINK_LIBRARIES( backendDX9 ${DXSDK_D3DX9_LIBRARY} ${DXSDK_D3D9_LIBRARY} SDL )
		SET_TARGET_PROPERTIES( backendDX9 PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
		)
	ENDIF()

	IF( BUILD_DX10_RENDERER )
		# This section finds and provides REQUIRED dependencies to backendDX10 target
		FIND_PACKAGE( DirectX REQUIRED )
		
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${DIRECTXANDNULLRENDERER_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${DIRECTXANDNULLRENDERER_SHARED_SOURCES} )
		SOURCE_GROUP( Source\ Files                                        FILES ${DIRECTX10LIB_SOURCES} )

		ADD_LIBRARY( backendDX10
			SHARED 
			${DIRECTX10LIB_SOURCES}
			${DIRECTXANDNULLRENDERER_SHARED_HEADERS}
			${DIRECTXANDNULLRENDERER_SHARED_SOURCES}
		)
		TARGET_INCLUDE_DIRECTORIES( backendDX10 PRIVATE ${MOUNT_DIR} ${DXSDK_INCLUDE_DIR} ${LIB_DIR}/SDL12/include )
		SET_TARGET_PROPERTIES( backendDX10 PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendDX10.def" OUTPUT_NAME "backendDX10${BUILD_ARCH}" PREFIX "" )
		TARGET_LINK_LIBRARIES( backendDX10 ${DXSDK_D3DX10_LIBRARY} ${DXSDK_D3D10_LIBRARY} SDL )
		SET_TARGET_PROPERTIES( backendDX10 PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
		)
	ENDIF()
ENDIF() ## WIN32

IF( BUILD_NULL_RENDERER )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${DIRECTXANDNULLRENDERER_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${DIRECTXANDNULLRENDERER_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${DIRECTX9LIB_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${NULLRENDERER_SOURCES} )
	ENDIF()
		
	ADD_LIBRARY( backendNULL
		SHARED 
		${NULLRENDERER_SOURCES}
		${DIRECTXANDNULLRENDERER_SHARED_HEADERS}
		${DIRECTXANDNULLRENDERER_SHARED_SOURCES}
	)
	TARGET_INCLUDE_DIRECTORIES( backendNULL PRIVATE ${MOUNT_DIR} ${LIB_DIR}/SDL12/include )
	SET_TARGET_PROPERTIES( backendNULL PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendNULL.def" OUTPUT_NAME "backendNULL${BUILD_ARCH}" PREFIX "" )
	TARGET_LINK_LIBRARIES( backendNULL SDL )
	SET_TARGET_PROPERTIES( backendNULL PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF( BUILD_GL_RENDERER )
	# This section finds and provides REQUIRED dependencies to backEndGL target
	FIND_PACKAGE( GLEW REQUIRED )
	FIND_PACKAGE( GLUT REQUIRED )

	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( API\\Header\ Files                                   FILES ${RENDERERGL_APIS_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${RENDERERGL_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${RENDERERGL_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${RENDERERGL_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${RENDERERGL_SOURCES} )
	ENDIF()
	
	ADD_LIBRARY( backendGL
		SHARED 
		${RENDERERGL_APIS_HEADERS}
		${MATHLIST_SOURCES} 
		${MATHLIST_HEADERS} 
		${RENDERERGL_SHARED_HEADERS} 
		${RENDERERGL_SHARED_SOURCES} 
		${RENDERERGL_HEADERS} 
		${RENDERERGL_SOURCES} 
	)
	TARGET_INCLUDE_DIRECTORIES( backendGL PRIVATE ${MOUNT_DIR} ${GLEW_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${LIB_DIR}/SDL12/include )
	TARGET_LINK_LIBRARIES( backendGL ${GLEW_LIBRARY} ${GLUT_glut_LIBRARY} SDL )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( backendGL PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/backendGL.def" OUTPUT_NAME "backendGL${BUILD_ARCH}" PREFIX "" )
	ELSE()
		SET_TARGET_PROPERTIES( backendGL PROPERTIES OUTPUT_NAME "backendGL${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( backendGL PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF( BUILD_RENDERER )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( API\\Header\ Files                                   FILES ${RENDERERAPI_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${RENDERER_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${RENDERER_SHARED_SOURCES} )
		SOURCE_GROUP( FileFormats\\Header\ Files                           FILES ${RENDERERFILEFORMATS_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${RENDERER_MATH_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${RENDERER_MATH_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${RENDERER_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${RENDERER_SOURCES} )
	ENDIF()

	ADD_LIBRARY( renderer
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
	TARGET_INCLUDE_DIRECTORIES( renderer PRIVATE ${MOUNT_DIR} )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( renderer PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/renderer.def" OUTPUT_NAME "renderer${BUILD_ARCH}" PREFIX "" )
		TARGET_INCLUDE_DIRECTORIES( renderer PRIVATE ${LIB_DIR}/freetype-2.3.5-1-bin/include/freetype2 )
		TARGET_LINK_LIBRARIES( renderer ${LIB_DIR}/freetype-2.3.5-1-bin/lib/freetype.lib )
	ELSE()
		SET_TARGET_PROPERTIES( renderer PROPERTIES OUTPUT_NAME "renderer${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( renderer PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF( BUILD_MODELLOADER )
	IF( WIN32 )
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
	ENDIF()
	
	ADD_LIBRARY( modelLoader
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
	TARGET_INCLUDE_DIRECTORIES( modelLoader PRIVATE ${MOUNT_DIR} )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( modelLoader PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/modelLoader.def" OUTPUT_NAME "modelLoader${BUILD_ARCH}" PREFIX "" )
	ELSE()
		SET_TARGET_PROPERTIES( modelLoader PROPERTIES OUTPUT_NAME "modelLoader${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( modelLoader PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF( BUILD_CM )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${CM_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${CM_SHARED_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Header\ Files                                        FILES ${CM_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${CM_SOURCE} )
	ENDIF()

	ADD_LIBRARY( cm
		SHARED 
		${CM_SHARED_HEADERS}
		${CM_SHARED_SOURCES}
		${MATHLIST_SOURCES}
		${MATHLIST_HEADERS}
		${CM_HEADERS}
		${CM_SOURCE}
	)
	TARGET_INCLUDE_DIRECTORIES( cm PRIVATE ${MOUNT_DIR} )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( cm PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/cm.def" OUTPUT_NAME "cm${BUILD_ARCH}" PREFIX "" )
	ELSE()
		SET_TARGET_PROPERTIES( cm PROPERTIES OUTPUT_NAME "cm${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( cm PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF( BUILD_DECLMANAGER )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( API\\Header\ Files                                   FILES ${DECLMANAGER_API} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${DECLMANAGER_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${DECLMANAGER_SHARED_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Header\ Files                                        FILES ${DECLMANAGER_HEADER} )
		SOURCE_GROUP( Source\ Files                                        FILES ${DECLMANAGER_SOURCE} )
	ENDIF()
	
	ADD_LIBRARY( declManager
		SHARED 
		${DECLMANAGER_API}
		${DECLMANAGER_HEADER}
		${MATHLIST_SOURCES}
		${MATHLIST_HEADERS}
		${DECLMANAGER_SHARED_HEADERS}
		${DECLMANAGER_SHARED_SOURCES}
		${DECLMANAGER_SOURCE}
	)
	TARGET_INCLUDE_DIRECTORIES( declManager PRIVATE ${MOUNT_DIR} )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( declManager PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/declManager.def" OUTPUT_NAME "declManager${BUILD_ARCH}" PREFIX "" )
	ELSE()
		SET_TARGET_PROPERTIES( declManager PROPERTIES OUTPUT_NAME "declManager${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( declManager PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF( BUILD_GAME )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Classes\\CSS\\Header\ Files                          FILES ${GAME_CLASSES_CSS_HEADERS} )
		SOURCE_GROUP( Classes\\CSS\\Source\ Files                          FILES ${GAME_CLASSES_CSS_SOURCES} )
		SOURCE_GROUP( Classes\\ET\\Header\ Files                           FILES ${GAME_CLASSES_ET_HEADERS} )
		SOURCE_GROUP( Classes\\ET\\Source\ Files                           FILES ${GAME_CLASSES_ET_SOURCES} )
		SOURCE_GROUP( Classes\\Quake3\\Header\ Files                       FILES ${GAME_CLASSES_QUAKE3_HEADERS} )
		SOURCE_GROUP( Classes\\Quake3\\Source\ Files                       FILES ${GAME_CLASSES_QUAKE3_SOURCES} )
		SOURCE_GROUP( Classes\\RTCW\\Header\ Files                         FILES ${GAME_CLASSES_RTCW_HEADERS} )
		SOURCE_GROUP( Classes\\RTCW\\Source\ Files                         FILES ${GAME_CLASSES_RTCW_SOURCES} )
		SOURCE_GROUP( Classes\\FAKK\\Header\ Files                         FILES ${GAME_CLASSES_FAKK_HEADERS} )
		SOURCE_GROUP( Classes\\FAKK\\Source\ Files                         FILES ${GAME_CLASSES_FAKK_SOURCES} )
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
	ENDIF()
	
	ADD_LIBRARY( qagame
		SHARED 
		${GAME_CLASSES_CSS_HEADERS}
		${GAME_CLASSES_CSS_SOURCES}
		${GAME_CLASSES_ET_HEADERS}
		${GAME_CLASSES_ET_SOURCES}
		${GAME_CLASSES_QUAKE3_HEADERS}
		${GAME_CLASSES_QUAKE3_SOURCES}
		${GAME_CLASSES_RTCW_HEADERS}
		${GAME_CLASSES_RTCW_SOURCES}
		${GAME_CLASSES_FAKK_HEADERS}
		${GAME_CLASSES_FAKK_SOURCES}
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

	IF( WIN32 )
		SET_TARGET_PROPERTIES( qagame PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/game.def" OUTPUT_NAME "qagame${BUILD_ARCH}" PREFIX "" )
		TARGET_INCLUDE_DIRECTORIES( qagame PRIVATE ${MOUNT_DIR} ${LIB_DIR}/lua-5.1.5/src )
		SET_TARGET_PROPERTIES( qagame PROPERTIES COMPILE_FLAGS -DG_ENABLE_LUA_SCRIPTING )
		TARGET_LINK_LIBRARIES( qagame ${LIB_DIR}/lua-5.1.5/lib/lua5.1.lib )
	ELSE()
		TARGET_INCLUDE_DIRECTORIES( qagame PRIVATE ${MOUNT_DIR} )
		SET_TARGET_PROPERTIES( qagame PROPERTIES OUTPUT_NAME "qagame${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( qagame PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF ( BUILD_GPHYSICS_BULLET )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${MATHLIST_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${MATHLIST_SOURCES} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${BUILD_GPHYSICS_BULLET_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${BUILD_GPHYSICS_BULLET_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${BUILD_GPHYSICS_BULLET_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${BUILD_GPHYSICS_BULLET_SOURCES} )
	ENDIF()

	ADD_LIBRARY( gphysics_bullet
		SHARED
		${MATHLIST_HEADERS}
		${MATHLIST_SOURCES}
		${BUILD_GPHYSICS_BULLET_SHARED_HEADERS}
		${BUILD_GPHYSICS_BULLET_SHARED_SOURCES}
		${BUILD_GPHYSICS_BULLET_HEADERS}
		${BUILD_GPHYSICS_BULLET_SOURCES}
	)
	IF( WIN32 )
		SET_TARGET_PROPERTIES( gphysics_bullet PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/game.def" OUTPUT_NAME "gphysics_bullet${BUILD_ARCH}" PREFIX "" )
		TARGET_INCLUDE_DIRECTORIES( gphysics_bullet PRIVATE ${MOUNT_DIR} ${LIB_DIR}/bullet-2.80-rev2531/src )
		TARGET_LINK_LIBRARIES( gphysics_bullet BulletDynamics BulletCollision LinearMath)
	ELSE()
		TARGET_INCLUDE_DIRECTORIES( gphysics_bullet PRIVATE ${MOUNT_DIR} )
		SET_TARGET_PROPERTIES( gphysics_bullet PROPERTIES OUTPUT_NAME "gphysics_bullet${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( gphysics_bullet PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF ( BUILD_CGAME )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Math\ Files\\Header\ Files                           FILES ${BUILD_CGAME_MATH_HEADERS} )
		SOURCE_GROUP( Math\ Files\\Source\ Files                           FILES ${BUILD_CGAME_MATH_SOURCES} )
		SOURCE_GROUP( Protocol\\Header\ Files                              FILES ${BUILD_CGAME_PROTOCOL} )
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${BUILD_CGAME_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${BUILD_CGAME_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${BUILD_CGAME_HEADER} )
		SOURCE_GROUP( Source\ Files                                        FILES ${BUILD_CGAME_SOURCE} )
	ENDIF()

	ADD_LIBRARY( cgame
		SHARED
		${BUILD_CGAME_MATH_HEADERS}
		${BUILD_CGAME_MATH_SOURCES}
		${BUILD_CGAME_PROTOCOL}
		${BUILD_CGAME_SHARED_HEADERS}
		${BUILD_CGAME_SHARED_SOURCES}
		${BUILD_CGAME_HEADER}
		${BUILD_CGAME_SOURCE}
	)
	TARGET_INCLUDE_DIRECTORIES( cgame PRIVATE ${MOUNT_DIR} )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( cgame PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/cgame.def" OUTPUT_NAME "cgame${BUILD_ARCH}" PREFIX "" )
	ELSE()
		SET_TARGET_PROPERTIES( cgame PROPERTIES OUTPUT_NAME "cgame${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( cgame PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF ( BUILD_GPHYSICS_ODE )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${GPHYSICS_ODE_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${GPHYSICS_ODE_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${GPHYSICS_ODE_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${GPHYSICS_ODE_SOURCE} )
	ENDIF()
	ADD_LIBRARY( gphysics_ode
		SHARED
		${GPHYSICS_ODE_SHARED_HEADERS}
		${GPHYSICS_ODE_SHARED_SOURCES}
		${GPHYSICS_ODE_HEADERS}
		${GPHYSICS_ODE_SOURCE}
	)
	IF( WIN32 )
		SET_TARGET_PROPERTIES( gphysics_ode PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/gphysics_ode.def" OUTPUT_NAME "gphysics_ode${BUILD_ARCH}" PREFIX "" )
		TARGET_INCLUDE_DIRECTORIES( gphysics_ode PRIVATE ${MOUNT_DIR} ${LIB_DIR}/ode-r1939/include )
	ELSE()
		TARGET_INCLUDE_DIRECTORIES( gphysics_ode PRIVATE ${MOUNT_DIR} )
		SET_TARGET_PROPERTIES( gphysics_ode PROPERTIES OUTPUT_NAME "gphysics_ode${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	TARGET_LINK_LIBRARIES( gphysics_ode ode )
	SET_TARGET_PROPERTIES( gphysics_ode PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)
ENDIF()

IF( WIN32 )
	IF ( BUILD_EDITOR )
		FIND_PACKAGE( GLUT REQUIRED )
		
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

		ADD_LIBRARY( editor
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
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4996")
		SET_TARGET_PROPERTIES( editor PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/editor.def" OUTPUT_NAME "editor${BUILD_ARCH}" PREFIX "" )
		TARGET_INCLUDE_DIRECTORIES( editor PRIVATE ${MOUNT_DIR} ${GLUT_INCLUDE_DIR} ${MOUNT_DIR}/editor/ ${MOUNT_DIR}/editor/gui/mfc ${LIB_DIR}/il-1.7.8-x86/include ${MOUNT_DIR}/tools/radiantAFXInclude/MFC/Include )
		ADD_DEFINITIONS( -DWIN32 -D_AFXDLL -D_WINDOWS -DQUAKE3 -DQ3RADIANT -D_WIN32 -D_CRT_SECURE_NO_WARNINGS -DUSE_DEVIL_LIBRARY )
		TARGET_LINK_LIBRARIES( editor opengl32.lib ${LIB_DIR}/il-1.7.8-x86/lib/DevIL.lib winmm.lib ${GLUT_glut_LIBRARY} )
		SET_TARGET_PROPERTIES( editor PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
			RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
		)
	ENDIF()
ENDIF()

IF( BUILD_CORE )
	IF( WIN32 )
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
	ENDIF()

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

	IF( WIN32 )
		TARGET_INCLUDE_DIRECTORIES( qio PRIVATE ${MOUNT_DIR} ${LIB_DIR}/SDL12/include ${LIB_DIR}/libcurl/ ${LIB_DIR}/libspeex/include ${LIB_DIR}/zlib )
		SET_TARGET_PROPERTIES( qio PROPERTIES OUTPUT_NAME "qio${BUILD_ARCH}" PREFIX "" )
		ADD_DEFINITIONS( -D_WIN32 -DWIN32 -D_WINDOWS -D_CRT_SECURE_NO_WARNINGS -DBOTLIB -DUSE_ICON -DUSE_CURL -DUSE_CURL_DLOPEN -DUSE_OPENAL -DUSE_OPENAL_DLOPEN -DUSE_VOIP -DHAVE_CONFIG_H -DUSE_INTERNAL_JPEG)
		TARGET_LINK_LIBRARIES( qio SDL SDLmain user32.lib advapi32.lib winmm.lib wsock32.lib ws2_32.lib msvcrt.lib psapi.lib Gdi32.lib )

		SET_TARGET_PROPERTIES( qio PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:windows" )
		SET_TARGET_PROPERTIES( qio PROPERTIES COMPILE_DEFINITIONS_DEBUG "windows" )
		SET_TARGET_PROPERTIES( qio PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:windows" )
		SET_TARGET_PROPERTIES( qio PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO "windows" )
		SET_TARGET_PROPERTIES( qio PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:windows" )
		SET_TARGET_PROPERTIES( qio PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:windows" )
	ELSE()
		TARGET_INCLUDE_DIRECTORIES( qio PRIVATE ${MOUNT_DIR} )
		SET_TARGET_PROPERTIES( qio PROPERTIES OUTPUT_NAME "qio${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( qio PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game
	)
ENDIF()

IF( BUILD_TIKI )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${TIKI_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${TIKI_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${TIKI_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${TIKI_SOURCES} )
	ENDIF()

	ADD_LIBRARY( tiki
		SHARED 
		${TIKI_SHARED_HEADERS}
		${TIKI_SHARED_SOURCES}
		${TIKI_HEADERS}
		${TIKI_SOURCES}
	)
	TARGET_INCLUDE_DIRECTORIES( tiki PRIVATE ${MOUNT_DIR} )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( tiki PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/tiki.def" OUTPUT_NAME "tiki${BUILD_ARCH}" PREFIX "" )
	ELSE()
		SET_TARGET_PROPERTIES( tiki PROPERTIES OUTPUT_NAME "tiki${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( tiki PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)	
ENDIF()

IF( BUILD_GUI )
	IF( WIN32 )
		# Dushan - This is how Visual Studio directories (or filters) are generated
		SOURCE_GROUP( Shared\ Files\\Header\ Files                         FILES ${GUI_SHARED_HEADERS} )
		SOURCE_GROUP( Shared\ Files\\Source\ Files                         FILES ${GUI_SHARED_SOURCES} )
		SOURCE_GROUP( Header\ Files                                        FILES ${GUI_HEADERS} )
		SOURCE_GROUP( Source\ Files                                        FILES ${GUI_SOURCES} )
	ENDIF()

	ADD_LIBRARY( gui
		SHARED 
		${GUI_SHARED_HEADERS}
		${GUI_SHARED_SOURCES}
		${GUI_HEADERS}
		${GUI_SOURCES}
	)
	TARGET_INCLUDE_DIRECTORIES( gui PRIVATE ${MOUNT_DIR} )
	IF( WIN32 )
		SET_TARGET_PROPERTIES( gui PROPERTIES LINK_FLAGS "/DEF:\"${REF_DIR}/gui.def" OUTPUT_NAME "gui${BUILD_ARCH}" PREFIX "" )
	ELSE()
		SET_TARGET_PROPERTIES( gui PROPERTIES OUTPUT_NAME "gui${BUILD_ARCH}" PREFIX "" )
	ENDIF()
	SET_TARGET_PROPERTIES( gui PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY_DEBUG   ${CMAKE_SOURCE_DIR}/game/system
		RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/game/system
	)	
ENDIF()