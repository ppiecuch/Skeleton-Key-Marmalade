######################################################################
# SkeletonKey CROSSPLATFORM BUILD SYSTEM
# Welcome to the CMake build system for SkeletonKey.
# This is the main file where we prepare the general build environment
# and provide build configuration options.
######################################################################

function(JOIN VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

cmake_minimum_required(VERSION 2.6.2)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
if(COMMAND CMAKE_POLICY)
	   cmake_policy(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)

project(SkeletonKey)

set(SK_DIST_DIRECTORY ${CMAKE_BINARY_DIR}/build_output)
set(SK_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build_output)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${SK_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${SK_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${SK_OUTPUT_DIRECTORY})

#Parse version information
#Using system commands: grep, cut, sed, date, bash
MACRO(BuildInfo_Get_Version file defname variable)
    EXECUTE_PROCESS(COMMAND grep ${defname} ${file} 
    			    COMMAND cut -d\  -f 3 
			    OUTPUT_VARIABLE ${variable} OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDMACRO(BuildInfo_Get_Version)
 
set( BUILD_INFO "build_info.h" )
BuildInfo_Get_Version(${BUILD_INFO} SK_MAJOR PROJECT_VER_MAJOR)
BuildInfo_Get_Version(${BUILD_INFO} SK_MINOR PROJECT_VER_MINOR)
BuildInfo_Get_Version(${BUILD_INFO} SK_REVISION PROJECT_VER_REVISION)
if (NOT (PROJECT_VER_MAJOR LESS 0) AND NOT (PROJECT_VER_MINOR LESS 0) AND NOT (PROJECT_VER_REVISION LESS 0))
   EXECUTE_PROCESS(COMMAND date +"%y/%m/%Y %H:%M" OUTPUT_VARIABLE TT OUTPUT_STRIP_TRAILING_WHITESPACE)
   #Simple escape string
   STRING(REPLACE "/" "\\/" TT_ ${TT})
   EXECUTE_PROCESS(COMMAND sed -i "" "s/SK_BUILDTIME \".*\"/SK_BUILDTIME ${TT_}/" ${BUILD_INFO})
   EXECUTE_PROCESS(COMMAND bash -c "echo $((`date +%y%m`+`date +%H%M|sed s/^0*//`))" OUTPUT_VARIABLE BUILD_ID OUTPUT_STRIP_TRAILING_WHITESPACE)
   set(BUILD_VER "${PROJECT_VER_MAJOR}.${PROJECT_VER_MINOR}.${PROJECT_VER_REVISION}")
   EXECUTE_PROCESS(COMMAND sed -i "" "s/SK_BUILDVERSION \".*\"/SK_BUILDVERSION \"${BUILD_VER}.${BUILD_ID}\"/" ${BUILD_INFO})
   message("** Project build version: ${BUILD_VER} (${BUILD_ID})")
   message("** Project build timestmap: ${TT}")
endif ()

FIND_PROGRAM(SVN_EXECUTABLE svn
  HINTS /opt/local/bin /sw/bin
  DOC "subversion command line client")

FIND_PROGRAM(GIT_EXECUTABLE git
  HINTS /opt/local/bin /sw/bin
  DOC "git command line client")
 
MACRO(Subversion_GET_REVISION dir variable)
    EXECUTE_PROCESS(COMMAND ${SVN_EXECUTABLE} info ${dir} OUTPUT_VARIABLE ${variable} OUTPUT_STRIP_TRAILING_WHITESPACE)
    STRING(REGEX REPLACE "^(.*\n)?Revision: ([^\n]+).*" "\\2" ${variable} "${${variable}}")
ENDMACRO(Subversion_GET_REVISION)

MACRO(Git_GET_REVISION dir variable)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} log -n 1 --pretty=%h ${dir} OUTPUT_VARIABLE ${variable} OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDMACRO(Git_GET_REVISION)

MACRO(Git_IS_WORKTREE var)
    EXECUTE_PROCESS( COMMAND ${GIT_EXECUTABLE} rev-parse --is-inside-work-tree 
    OUTPUT_VARIABLE _IS_GIT
    OUTPUT_STRIP_TRAILING_WHITESPACE )
    IF(${_IS_GIT} STREQUAL "true")
    	      SET(${var} TRUE)
    ENDIF()
ENDMACRO(Git_IS_WORKTREE)

Git_IS_WORKTREE(IS_GIT)
IF( IS_GIT )
  IF ( GIT_EXECUTABLE )
     Git_GET_REVISION( ../source ProjectRevision )
     IF ( ProjectRevision )
     	EXECUTE_PROCESS(COMMAND sed -i "" "s/SK_SOURCE_REV [x0-9a-f]*/SK_SOURCE_REV 0x${ProjectRevision}/" ${BUILD_INFO})
   	MESSAGE("** Git source revision is ${ProjectRevision}")
     ENDIF ( ProjectRevision )
  ENDIF( GIT_EXECUTABLE )
ELSEIF ( SVN_EXECUTABLE )
 Subversion_GET_REVISION( ../source ProjectRevision )
 IF ( ProjectRevision )
   EXECUTE_PROCESS(COMMAND sed -i "" "s/SK_SOURCE_REV [0-9]*/SK_SOURCE_REV ${ProjectRevision}/" ${BUILD_INFO})
   MESSAGE("** SVN source revision is ${ProjectRevision}")
 ENDIF ( ProjectRevision )
ENDIF( IS_GIT )

#Wav converting
FIND_PROGRAM(AFC_EXECUTABLE afconvert
  HINTS /opt/local/bin /sw/bin
  DOC "afconvert command line sound converter")
IF(AFC_EXECUTABLE)
	SET(WAV_CONVERT ${AFC_EXECUTABLE} -f WAVE -d LEI16@11025 -c 1)
ELSE()
	MESSAGE("## Cannot find afconvert executable. Copying source wav files.")
	SET(WAV_CONVERT ${CMAKE_COMMAND} -E copy)
ENDIF()

#===================================
# Environment tests
#===================================

include(TestForANSIForScope)
include(TestForANSIStreamHeaders)
include(TestForSTDNamespace)


#===================================
# Build options
#===================================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

# CMake 2.8.2 has a bug that creates unusable Xcode projects when using ARCHS_STANDARD_32_BIT
# to specify both armv6 and armv7.
if(NC_BUILD_PLATFORM_APPLE_IOS AND (CMAKE_VERSION VERSION_EQUAL 2.8.2) AND (CMAKE_GENERATOR STREQUAL "Xcode"))
	message(FATAL_ERROR "CMake 2.8.2 cannot create compatible Xcode projects for iOS, please download the latest version or an older release from http://www.cmake.org/files/")
endif()

# Use relative paths
# This is mostly to reduce path size for command-line limits on windows
if(WIN32)
  # This seems to break Xcode projects so definitely don't enable on Apple builds
  set(CMAKE_USE_RELATIVE_PATHS true)
  set(CMAKE_SUPPRESS_REGENERATION true)
  #on windows, check for VC10 and fix the multiple compile target issue.
  if(MSVC)
    if(${MSVC_VERSION} STREQUAL 1600 OR ${MSVC_VERSION} STRGREATER 1600)
      message("Visual Studio 2010 (${MSVC_VERSION}) build fix at play (/FORCE:MULTIPLE)")
      set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /FORCE:MULTIPLE")
    endif()
  endif() 
endif()

if (QNX_TARGET AND QNX_HOST)
   set (NC_BUILD_PLATFORM_QNX 1)
   # bbndk.h is 10.1 header
   EXECUTE_PROCESS(COMMAND grep "BPS_VERSION " ${STAGE_INC}/bps/bps.h 
   			    COMMAND tr -s " "
			    COMMAND tr -d "()"
    			    COMMAND cut -d\  -f 3 
			    OUTPUT_VARIABLE QNX_SDK_VER OUTPUT_STRIP_TRAILING_WHITESPACE)
   if (NOT QNX_SDK_VER)
      set(REQ_PLATF_VER "1.0.0.0")
   elseif (QNX_SDK_VER EQUAL 3000000)
   	set(REQ_PLATF_VER "2.0.0.0")
	add_definitions("-D__PLAYBOOK__")
   else ()
   	set(REQ_PLATF_VER "3.0.0.0")
	add_definitions("-D__BB10__")
   endif()
   message( "** QNX SDK version: ${QNX_SDK_VER}, version required: ${REQ_PLATF_VER}" )
elseif (APPLE)
  include(CMakeForceCompiler)
  CMAKE_FORCE_C_COMPILER(clang GNU)
  CMAKE_FORCE_CXX_COMPILER(clang++ GNU)
  SET(CMAKE_SIZEOF_VOID_P 4)
  set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
endif ()

 if (NC_BUILD_PLATFORM_IOS)
   set_xcode_property (hello-lib GCC_GENERATE_DEBUGGING_SYMBOLS YES)
endif ()

if(SDLDIR)
	set(ENV{SDLDIR} ${SDLDIR})
 endif()

# Find SDL library
include(FindSDL)

if(NOT SDL_FOUND AND NOT SDLDIR)
       message( FATAL_ERROR "** Required SDL library not found!\n*** Specify SDLDIR for library root directory." )
endif()

if(NOT SDL_FOUND)
       set( SDL_CONFIG ${SDLDIR}/bin/sdl-config )
       if(EXISTS ${SDLDIR}/${REQ_PLATF_VER}/bin/sdl-config)
       		 # SDL exists in the subdirectory
       		 set( SDL_CONFIG 
		        ${SDLDIR}/${REQ_PLATF_VER}/bin/sdl-config )
		 set( SDLDIR ${SDLDIR}/${REQ_PLATF_VER} )
       endif()
       if( EXISTS ${SDL_CONFIG} )
       		 EXECUTE_PROCESS(COMMAND ${SDL_CONFIG} --version
			OUTPUT_VARIABLE SDL_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
       		 EXECUTE_PROCESS(COMMAND ${SDL_CONFIG} --cflags
			OUTPUT_VARIABLE SDL_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
       		 EXECUTE_PROCESS(COMMAND ${SDL_CONFIG} --libs 
       	      	 	OUTPUT_VARIABLE SDL_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
		 message( "** Manual configuration for SDL library found: ${SDL_VERSION}." )
       else()
		 message( "** Configuration for SDL library not found. Forcing ${SDLDIR} for root directory." )
       endif()

       set(SDL_INCLUDE_DIR ${SDLDIR}/include)
       set(SDL_LIB_DIR ${SDLDIR}/lib)
       set(SDL_LIBRARY SDL)
 endif()

include(CMakeDependentOption)
include(CheckCXXCompilerFlag)

# game files:
set( LOCAL_SOURCE_ROOT ${PROJECT_SOURCE_DIR}/source )
set( DATA_ROOT ${PROJECT_SOURCE_DIR}/../data )
set( SOURCE_ROOT ${PROJECT_SOURCE_DIR}/../source )
set( SkeletonKey_Src_Files
	## (data files)
	${DATA_ROOT}/fonts/font_algerian_16.h
	${DATA_ROOT}/fonts/font_algerian_24.h
	${DATA_ROOT}/fonts/font_gabriola_14.h
	${DATA_ROOT}/fonts/font_gabriola_22b.h
	${DATA_ROOT}/fonts/font_algerian_20.h
	${DATA_ROOT}/fonts/font_deutsch_26.h
	${DATA_ROOT}/fonts/font_gabriola_16b.h
	## (source)
	${SOURCE_ROOT}/main.cpp
	${SOURCE_ROOT}/config.h
	${SOURCE_ROOT}/scene_menu.h
	${SOURCE_ROOT}/scene_menu.cpp
	${SOURCE_ROOT}/scene_map.h
	${SOURCE_ROOT}/scene_map.cpp
	${SOURCE_ROOT}/scene_select_level.h
	${SOURCE_ROOT}/scene_select_level.cpp
	${SOURCE_ROOT}/scene_game.h
	${SOURCE_ROOT}/scene_game.cpp
	${SOURCE_ROOT}/scene_game_menu.h
	${SOURCE_ROOT}/scene_game_menu.cpp
	${SOURCE_ROOT}/scene_instructions.h
	${SOURCE_ROOT}/scene_instructions.cpp
	${SOURCE_ROOT}/scene_nag.h
	${SOURCE_ROOT}/scene_nag.cpp
	${SOURCE_ROOT}/scene_options.h
	${SOURCE_ROOT}/scene_options.cpp
	${SOURCE_ROOT}/scene_achievements.h
	${SOURCE_ROOT}/scene_achievements.cpp
	${SOURCE_ROOT}/settings.h
	${SOURCE_ROOT}/settings.cpp
	${SOURCE_ROOT}/sqlite3_wrapper.h
	${SOURCE_ROOT}/sqlite3_wrapper.cpp
	${SOURCE_ROOT}/sounds.h
	${SOURCE_ROOT}/sounds.cpp
	${SOURCE_ROOT}/game_data.h
	${SOURCE_ROOT}/game_data.cpp
	${SOURCE_ROOT}/achievements.h
	${SOURCE_ROOT}/achievements.cpp
	## (source/ig2d)
	${SOURCE_ROOT}/ig2d/ig.h
	${SOURCE_ROOT}/ig2d/ig_animation.h
	${SOURCE_ROOT}/ig2d/ig_animation.cpp
	${SOURCE_ROOT}/ig2d/ig_button.h
	${SOURCE_ROOT}/ig2d/ig_button.cpp
	${SOURCE_ROOT}/ig2d/ig_director.h
	${SOURCE_ROOT}/ig2d/ig_director.cpp
	${SOURCE_ROOT}/ig2d/ig_distorter.h
	${SOURCE_ROOT}/ig2d/ig_distorter.cpp
	${SOURCE_ROOT}/ig2d/ig_global.h
	${SOURCE_ROOT}/ig2d/ig_global.cpp
	${SOURCE_ROOT}/ig2d/ig_label.h
	${SOURCE_ROOT}/ig2d/ig_label.cpp
	${SOURCE_ROOT}/ig2d/ig_node.h
	${SOURCE_ROOT}/ig2d/ig_node.cpp
	${SOURCE_ROOT}/ig2d/ig_resource_manager.h
	${SOURCE_ROOT}/ig2d/ig_resource_manager.cpp
	${SOURCE_ROOT}/ig2d/ig_scene.h
	${SOURCE_ROOT}/ig2d/ig_scene.cpp
	${SOURCE_ROOT}/ig2d/ig_sprite.h
	${SOURCE_ROOT}/ig2d/ig_sprite.cpp
	${SOURCE_ROOT}/ig2d/ig_touches.h
	${SOURCE_ROOT}/ig2d/ig_touches.cpp
	## (source/dgreedutils)
	${LOCAL_SOURCE_ROOT}/dgreed/system.h
	${LOCAL_SOURCE_ROOT}/dgreed/system.c
	${LOCAL_SOURCE_ROOT}/dgreed/darray.h
	${LOCAL_SOURCE_ROOT}/dgreed/darray.c
	${LOCAL_SOURCE_ROOT}/dgreed/memory.h
	${LOCAL_SOURCE_ROOT}/dgreed/memory.c
	${LOCAL_SOURCE_ROOT}/dgreed/mempool.c
	${LOCAL_SOURCE_ROOT}/dgreed/mempool.h
	${LOCAL_SOURCE_ROOT}/dgreed/miniz.c
	${LOCAL_SOURCE_ROOT}/dgreed/sophist.h
	${LOCAL_SOURCE_ROOT}/dgreed/async.c
	${LOCAL_SOURCE_ROOT}/dgreed/async.h
	${LOCAL_SOURCE_ROOT}/dgreed/utils.h
	${LOCAL_SOURCE_ROOT}/dgreed/utils.c
	${LOCAL_SOURCE_ROOT}/dgreed/datastruct.h
	${LOCAL_SOURCE_ROOT}/dgreed/datastruct.c
	${LOCAL_SOURCE_ROOT}/dgreed/mml.h
	${LOCAL_SOURCE_ROOT}/dgreed/mml.c
	${LOCAL_SOURCE_ROOT}/dgreed/localization.h
	${LOCAL_SOURCE_ROOT}/dgreed/localization.c
	${LOCAL_SOURCE_ROOT}/dgreed/wav.h
	${LOCAL_SOURCE_ROOT}/dgreed/wav.c
	${LOCAL_SOURCE_ROOT}/dgreed/system.h
	## (source/sqlite3)
    	${SOURCE_ROOT}/sqlite3/sqlite3.h
    	${SOURCE_ROOT}/sqlite3/sqlite3ext.h
    	${SOURCE_ROOT}/sqlite3/sqlite3.c
	## (compat)
    	${LOCAL_SOURCE_ROOT}/compat/AffineTransform.h
    	${LOCAL_SOURCE_ROOT}/compat/IwGx.h
    	${LOCAL_SOURCE_ROOT}/compat/Iw2D.h
    	${LOCAL_SOURCE_ROOT}/compat/IwResManager.h
    	${LOCAL_SOURCE_ROOT}/compat/s3e.h
    	${LOCAL_SOURCE_ROOT}/compat/s3e.cpp
	${LOCAL_SOURCE_ROOT}/stb/image_DXT.h
	${LOCAL_SOURCE_ROOT}/stb/image_DXT.c
	${LOCAL_SOURCE_ROOT}/stb/image_helper.h
	${LOCAL_SOURCE_ROOT}/stb/image_helper.c
	${LOCAL_SOURCE_ROOT}/stb/stb_image.h
	${LOCAL_SOURCE_ROOT}/stb/stb_image_aug.c
	${LOCAL_SOURCE_ROOT}/stb/stbi_DDS.h
	${LOCAL_SOURCE_ROOT}/stb/stbi_DDS_aug_c.h
	${LOCAL_SOURCE_ROOT}/stb/SOIL.h
	${LOCAL_SOURCE_ROOT}/stb/SOIL.cpp
	${LOCAL_SOURCE_ROOT}/audio_engine/audio.cpp
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_descriptor.cpp
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_effects.cpp
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_input.cpp
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_stream.cpp
	${LOCAL_SOURCE_ROOT}/audio_engine/audio.h
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_descriptor.h
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_effects.h
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_input.h
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_stream.h
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_utils.cpp
	${LOCAL_SOURCE_ROOT}/audio_engine/audio_utils.h
	${LOCAL_SOURCE_ROOT}/AngelcodeFont.cpp
	${LOCAL_SOURCE_ROOT}/FPSLimit.cpp
	${LOCAL_SOURCE_ROOT}/SDLGLFont.cpp
	${LOCAL_SOURCE_ROOT}/toolkit.cpp
	${LOCAL_SOURCE_ROOT}/BMF_font.cpp
	${LOCAL_SOURCE_ROOT}/MersenneTwister.cpp
	${LOCAL_SOURCE_ROOT}/SDLGLTexture.cpp
)

# platform dependant game files:
if (NC_BUILD_PLATFORM_QNX)
   set(SkeletonKey_Src_PlatfFiles
	${LOCAL_SOURCE_ROOT}/qnx/_bb_simple_audio_engine.cpp
	${LOCAL_SOURCE_ROOT}/qnx/_bb_simple_audio_engine.h
   )
endif(NC_BUILD_PLATFORM_QNX)

if (NC_BUILD_PLATFORM_IOS)
   set(SkeletonKey_Src_PlatfFiles
   )
endif(NC_BUILD_PLATFORM_IOS)
 
if (NC_BUILD_PLATFORM_QNX)
   set_source_files_properties(${SkeletonKey_Src_PlatfFiles} ${SkeletonKey_Src_Files}
       PROPERTIES
       COMPILE_FLAGS  "${SDL_CFLAGS} -Wno-psabi -Duint8=uint8_t -Dint8=int8_t -Duint16=uint16_t -Duint32=uint32_t -DBOOL=int -DYES=1 -DNO=0")
   set(PLATF_LIBS
		vorbisfileS vorbisS oggS
   		freetype
		GLES_bc EGL GLESv2
		OpenAL alut bps pps screen asound scoreloopcore
		curl crypto mmrndclient icuuc icui18n strm)
endif (NC_BUILD_PLATFORM_QNX)

if (NC_BUILD_PLATFORM_IOS)
   set(IOS_FRAMEWORKS
	Foundation
    	AudioToolbox
   	CoreGraphics
    	QuartzCore
    	UIKit
    	OpenGLES2 )

   set(CMAKE_OSX_SYSROOT iphonesimulator4.0)
   set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD_32_BIT)")
   foreach(FW ${IOS_FRAMEWORKS})
   	      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework ${FW}")
   endforeach()
   
   set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.komsoft.\${PRODUCT_NAME:identifier}")

endif (NC_BUILD_PLATFORM_IOS)

set( THIRD_PARTY ./third_party/ )

include_directories(
	${SDL_INCLUDE_DIR}
	${LOCAL_SOURCE_ROOT}/compat/
	${LOCAL_SOURCE_ROOT}/dgreed/
	${LOCAL_SOURCE_ROOT}/stb/
	${LOCAL_SOURCE_ROOT}/qnx
	${LOCAL_SOURCE_ROOT}
	${SOURCE_ROOT}
	${DATA_ROOT}
)
link_directories(
	${SDL_LIB_DIR}
)

#Main resource files:
SET( DATA_ROOT ../data )
SET(SkeletonKey_Data_Files
	graphics/achievements/achievements_back.png
	graphics/achievements/achievements_back2.png
	graphics/achievements/achievements_header.png
	graphics/achievements/achievements_locked_trophy.png
	graphics/achievements/achievements_menu.png
	graphics/achievements/achievements_menu2.png
	graphics/achievements/achievements_next.png
	graphics/achievements/achievements_next2.png
	graphics/achievements/achievements_row_background.png
	graphics/achievements/achievements_trophy.png
	#
	graphics/leadersboard/leadersboard_header.png
	graphics/leadersboard/leadersboard_sl_badge256.png
	graphics/leadersboard/leadersboard_number1.png
	graphics/leadersboard/leadersboard_top10.png
	#
	graphics/backgrounds/background_black.png
	graphics/backgrounds/background_forest_dark.png
	graphics/backgrounds/background_forest_light.png
	graphics/backgrounds/background_wood.png
	graphics/backgrounds/playbook_blur_background_forest_dark.png
	graphics/backgrounds/playbook_blur_background_wood.png
	graphics/backgrounds/playbook_blur_background_forest_light.png
	graphics/backgrounds/playbook/background_forest_dark.png
	graphics/backgrounds/playbook/background_forest_light.png
	graphics/backgrounds/playbook/background_wood.png
	#
	graphics/game/playbook/game_background_beach.png
	graphics/game/playbook/game_background_caves.png
	graphics/game/playbook/game_background_forest.png
	graphics/game/playbook/game_background_ship.png
	graphics/game/playbook/game_wood.png
	graphics/game/game_achievement.png
	graphics/game/game_background_beach.png
	graphics/game/game_background_caves.png
	graphics/game/game_background_forest.png
	graphics/game/game_background_ship.png
	graphics/game/game_beach_0s.png
	graphics/game/game_beach_1s_b.png
	graphics/game/game_beach_1s_l.png
	graphics/game/game_beach_1s_r.png
	graphics/game/game_beach_1s_t.png
	graphics/game/game_beach_2s_bl.png
	graphics/game/game_beach_2s_rb.png
	graphics/game/game_beach_2s_rl.png
	graphics/game/game_beach_2s_tb.png
	graphics/game/game_beach_2s_tl.png
	graphics/game/game_beach_2s_tr.png
	graphics/game/game_beach_3s_rbl.png
	graphics/game/game_beach_3s_tlb.png
	graphics/game/game_beach_3s_trb.png
	graphics/game/game_beach_3s_trl.png
	graphics/game/game_beach_4s.png
	graphics/game/game_caves_0s.png
	graphics/game/game_caves_1s_b.png
	graphics/game/game_caves_1s_l.png
	graphics/game/game_caves_1s_r.png
	graphics/game/game_caves_1s_t.png
	graphics/game/game_caves_2s_bl.png
	graphics/game/game_caves_2s_rb.png
	graphics/game/game_caves_2s_rl.png
	graphics/game/game_caves_2s_tb.png
	graphics/game/game_caves_2s_tl.png
	graphics/game/game_caves_2s_tr.png
	graphics/game/game_caves_3s_rbl.png
	graphics/game/game_caves_3s_tlb.png
	graphics/game/game_caves_3s_trb.png
	graphics/game/game_caves_3s_trl.png
	graphics/game/game_caves_4s.png
	graphics/game/game_forest_0s.png
	graphics/game/game_forest_1s_b.png
	graphics/game/game_forest_1s_l.png
	graphics/game/game_forest_1s_r.png
	graphics/game/game_forest_1s_t.png
	graphics/game/game_forest_2s_bl.png
	graphics/game/game_forest_2s_rb.png
	graphics/game/game_forest_2s_rl.png
	graphics/game/game_forest_2s_tb.png
	graphics/game/game_forest_2s_tl.png
	graphics/game/game_forest_2s_tr.png
	graphics/game/game_forest_3s_rbl.png
	graphics/game/game_forest_3s_tlb.png
	graphics/game/game_forest_3s_trb.png
	graphics/game/game_forest_3s_trl.png
	graphics/game/game_forest_4s.png
	graphics/game/game_level_complete.png
	graphics/game/game_menu.png
	graphics/game/game_menu2.png
	graphics/game/game_message_no_moves.png
	graphics/game/game_message_no_moves_menu.png
	graphics/game/game_message_perfect.png
	graphics/game/game_message_start.png
	graphics/game/game_ship_0s.png
	graphics/game/game_ship_1s_b.png
	graphics/game/game_ship_1s_l.png
	graphics/game/game_ship_1s_r.png
	graphics/game/game_ship_1s_t.png
	graphics/game/game_ship_2s_bl.png
	graphics/game/game_ship_2s_rb.png
	graphics/game/game_ship_2s_rl.png
	graphics/game/game_ship_2s_tb.png
	graphics/game/game_ship_2s_tl.png
	graphics/game/game_ship_2s_tr.png
	graphics/game/game_ship_3s_rbl.png
	graphics/game/game_ship_3s_tlb.png
	graphics/game/game_ship_3s_trb.png
	graphics/game/game_ship_3s_trl.png
	graphics/game/game_ship_4s.png
	graphics/game/game_sprite_chest_closed.png
	graphics/game/game_sprite_chest_open.png
	graphics/game/game_sprite_door_lr_closed.png
	graphics/game/game_sprite_door_lr_open.png
	graphics/game/game_sprite_door_tb_closed.png
	graphics/game/game_sprite_door_tb_open.png
	graphics/game/game_sprite_key.png
	graphics/game/game_sprite_switch.png
	graphics/game/game_wood.png
	#
	graphics/game_menu/game_menu_abandon.png
	graphics/game_menu/game_menu_abandon2.png
	graphics/game_menu/game_menu_error_background.png
	graphics/game_menu/game_menu_error_continue.png
	graphics/game_menu/game_menu_error_continue2.png
	graphics/game_menu/game_menu_error_message_last_level.png
	graphics/game_menu/game_menu_error_message_stage_locked.png
	graphics/game_menu/game_menu_header.png
	graphics/game_menu/game_menu_info_background.png
	graphics/game_menu/game_menu_next.png
	graphics/game_menu/game_menu_next2.png
	graphics/game_menu/game_menu_options.png
	graphics/game_menu/game_menu_options2.png
	graphics/game_menu/game_menu_restart.png
	graphics/game_menu/game_menu_restart2.png
	graphics/game_menu/game_menu_resume.png
	graphics/game_menu/game_menu_resume2.png
	#
	graphics/instructions/instructions_back.png
	graphics/instructions/instructions_back2.png
	graphics/instructions/instructions_header.png
	graphics/instructions/instructions_menu.png
	graphics/instructions/instructions_menu2.png
	graphics/instructions/instructions_next.png
	graphics/instructions/instructions_next2.png
	graphics/instructions/instructions_page1.png
	graphics/instructions/instructions_page2.png
	graphics/instructions/instructions_page3.png
	graphics/instructions/instructions_page4.png
	graphics/instructions/instructions_page4.psd
	#
	graphics/map/map_background.png
	graphics/map/map_key1.png
	graphics/map/map_key2.png
	graphics/map/map_key3.png
	graphics/map/map_key4.png
	graphics/map/map_key5.png
	graphics/map/map_lock1.png
	graphics/map/map_lock2.png
	graphics/map/map_lock3.png
	graphics/map/map_lock4.png
	graphics/map/map_lock5.png
	graphics/map/map_select_destination.png
	graphics/map/playbook/map_background.png
	#
	graphics/menu/menu_achievements.png
	graphics/menu/menu_achievements2.png
	graphics/menu/menu_header.png
	graphics/menu/menu_instructions.png
	graphics/menu/menu_instructions2.png
	graphics/menu/menu_options.png
	graphics/menu/menu_options2.png
	graphics/menu/menu_other_games.png
	graphics/menu/menu_other_games2.png
	graphics/menu/menu_play_game.png
	graphics/menu/menu_play_game2.png
	#
	graphics/nag/nag_header.png
	graphics/nag/nag_no.png
	graphics/nag/nag_no2.png
	graphics/nag/nag_yes.png
	graphics/nag/nag_yes2.png
	#
	graphics/options/options_back_game.png
	graphics/options/options_back_game2.png
	graphics/options/options_back_menu.png
	graphics/options/options_back_menu2.png
	graphics/options/options_background.png
	graphics/options/options_music_off.png
	graphics/options/options_music_on.png
	graphics/options/options_reset.png
	graphics/options/options_reset2.png
	graphics/options/options_reset_confirm_background.png
	graphics/options/options_reset_confirm_cancel.png
	graphics/options/options_reset_confirm_cancel2.png
	graphics/options/options_reset_confirm_yes.png
	graphics/options/options_reset_confirm_yes2.png
	graphics/options/options_shake_off.png
	graphics/options/options_shake_on.png
	graphics/options/options_sound_off.png
	graphics/options/options_sound_on.png
	#
	graphics/select_level/select_level_back.png
	graphics/select_level/select_level_back2.png
	graphics/select_level/select_level_beach.png
	graphics/select_level/select_level_caves.png
	graphics/select_level/select_level_diff_easy.png
	graphics/select_level/select_level_diff_hard.png
	graphics/select_level/select_level_diff_medium.png
	graphics/select_level/select_level_forest.png
	graphics/select_level/select_level_header.png
	graphics/select_level/select_level_perfect.png
	graphics/select_level/select_level_ship.png
	#
	fonts/font_algerian_16.tga
	fonts/font_algerian_24.tga
	fonts/font_gabriola_14.tga
	fonts/font_gabriola_22b.tga
	fonts/font_algerian_20.tga
	fonts/font_deutsch_26.tga
	fonts/font_gabriola_16b.tga
	#
	click.raw
	key_move.raw
	open_chest.raw
	unlock_achievement.raw
	door.raw
	map_locked.raw
	restart_level.raw
	menu.mp3
	#
	achievements.db
	levels.db
	saved_game.db
	settings.db
)
SET_SOURCE_FILES_PROPERTIES(
	${SkeletonKey_Data_Files}
	PROPERTIES TARGET_PATH "./data"
)
SET_SOURCE_FILES_PROPERTIES(
	${SkeletonKey_Data_Files}
	PROPERTIES ROOT_PATH "../data"
)
#SDL fonts:
SET(Sdl_Font_Files
	../third_party/sdl/fonts/10x20.fnt
	../third_party/sdl/fonts/6x13O.fnt
	../third_party/sdl/fonts/8x13.fnt
	../third_party/sdl/fonts/5x7.fnt
	../third_party/sdl/fonts/6x9.fnt
	../third_party/sdl/fonts/8x13B.fnt
	../third_party/sdl/fonts/5x8.fnt
	../third_party/sdl/fonts/7x13.fnt
	../third_party/sdl/fonts/8x13O.fnt
	../third_party/sdl/fonts/6x10.fnt
	../third_party/sdl/fonts/7x13B.fnt
	../third_party/sdl/fonts/9x15.fnt
	../third_party/sdl/fonts/6x12.fnt
	../third_party/sdl/fonts/7x13O.fnt
	../third_party/sdl/fonts/9x15B.fnt
	../third_party/sdl/fonts/6x13.fnt
	../third_party/sdl/fonts/7x14.fnt
	../third_party/sdl/fonts/9x18.fnt
	../third_party/sdl/fonts/6x13B.fnt
	../third_party/sdl/fonts/7x14B.fnt
	../third_party/sdl/fonts/9x18B.fnt
)
SET_SOURCE_FILES_PROPERTIES(
	${Sdl_Font_Files}
	PROPERTIES TARGET_PATH "./data/sdlfonts"
)
if (NC_BUILD_PLATFORM_QNX)
   # update bar file:
   MACRO(Update_Bar_File file)
   		FILE(READ ${file} FILE_CONTENT)
   		STRING(REGEX REPLACE "%VER_NUMBER%" "${BUILD_VER}" MODIFIED_FILE_CONTENT "${FILE_CONTENT}")
   		STRING(REGEX REPLACE "%VER_LABEL%" "${BUILD_VER_LABEL}" FILE_CONTENT "${MODIFIED_FILE_CONTENT}")
   		STRING(REGEX REPLACE "%REQ_PLATF_VER%" "${REQ_PLATF_VER}" MODIFIED_FILE_CONTENT "${FILE_CONTENT}")
   		FILE(WRITE ${file} "${MODIFIED_FILE_CONTENT}")
   ENDMACRO(Update_Bar_File)
   SET(BB10_INFO_FILES
	./toolchain/bar-descriptor.xml
	../data/splash_playbook.jpg
	../data/splash_bb10.jpg
	../data/icons/icon_114_p.png
	../data/icons/icon_300_p.png
	../data/icons/icon_512_p.png
	../data/icons/icon_86_p.png
	../data/icons/icon_90_p.png
   )
   #Get the full path:
   FOREACH(file ${BB10_INFO_FILES})
	GET_FILENAME_COMPONENT(fullfile ${file} REALPATH)
   	LIST( APPEND BB10_FILES ${fullfile} )
   	SET_SOURCE_FILES_PROPERTIES(
		${fullfile}
		PROPERTIES TARGET_PATH "."
   	)
   ENDFOREACH()
   SET_SOURCE_FILES_PROPERTIES(
	${BB10_FILES}
	PROPERTIES TARGET_PATH "."
   )
   SET(BB10_SO_FILES
	${STAGE_LIB}/libTouchControlOverlay.so
	${STAGE_LIB}/libGLES_bc.so
	${SDL_LIB_DIR}/libSDL.so
   )
   #Rebuild project if any of the libraries has changed:
   JOIN("${BB10_SO_FILES}" ";" BB10_SO_FILES_S)
   SET_SOURCE_FILES_PROPERTIES(
	${SkeletonKey_Src_Files}
	PROPERTIES OBJECT_DEPENDS "${BB10_SO_FILES_S}"
   )
   SET_SOURCE_FILES_PROPERTIES(
	${SkeletonKey_Src_PlatfFiles}
	PROPERTIES OBJECT_DEPENDS "${BB10_SO_FILES_S}"
   )
   #Get the full .so name:
   FOREACH(file ${BB10_SO_FILES})
	GET_FILENAME_COMPONENT(fullfile ${file} REALPATH)
   	LIST( APPEND BB10_FILES ${fullfile} )
   	SET_SOURCE_FILES_PROPERTIES(
		${fullfile}
		PROPERTIES TARGET_PATH "./lib"
   	)
   ENDFOREACH()
   LIST( APPEND BB10_FILES ${SkeletonKey_Data_Files} )
   LIST( APPEND BB10_FILES ${SkeletonKey_DataGfx_Files} )
   LIST( APPEND BB10_FILES ${Sdl_Fonts_Files} )
   FOREACH(file ${BB10_FILES})
	GET_FILENAME_COMPONENT(filename ${file} NAME)
	GET_FILENAME_COMPONENT(ext ${file} EXT)
	GET_SOURCE_FILE_PROPERTY(path ${file} TARGET_PATH)
	IF(EXISTS ${file})
		IF(IS_ABSOLUTE ${file})
				SET( root "" )
		ELSE()
				SET( root "." )
		ENDIF()
	       	SET( dest ${filename} )
	ELSE()
		GET_SOURCE_FILE_PROPERTY(root ${file} ROOT_PATH)
		IF(NOT root)
	       	       IF(IS_ABSOLUTE ${file})
				SET( root "" )
				SET( dest ${filename} )
	       	       ELSE()
				SET( root "." )
				SET( dest ${file} )
	       	       ENDIF()
		ELSE()
	       	       IF(IS_ABSOLUTE ${file})
				SET( dest ${filename} )
	       	       ELSE()
				SET( dest ${file} )
	       	       ENDIF()
		ENDIF()
	ENDIF()
	if (${filename} STREQUAL "bar-descriptor.xml")
	   ADD_CUSTOM_COMMAND(OUTPUT ${SK_DIST_DIRECTORY}/${path}/${filename}
		COMMAND ${CMAKE_COMMAND} -E copy "${root}/${file}" "${SK_DIST_DIRECTORY}/${path}/${filename}"
		COMMAND sed -i \"\" \"s/%VER_NUMBER%/${BUILD_VER}/\" \"${SK_DIST_DIRECTORY}/${path}/${filename}\"
		COMMAND sed -i \"\" \"s/%VER_LABEL%/${BUILD_VER}/\" \"${SK_DIST_DIRECTORY}/${path}/${filename}\"
		COMMAND sed -i \"\" \"s/%REQ_PLATF_VER%/${REQ_PLATF_VER}/\" \"${SK_DIST_DIRECTORY}/${path}/${filename}\"
		COMMAND sed -i \"\" \"s/%BUILD_ID%/${BUILD_ID}/\" \"${SK_DIST_DIRECTORY}/${path}/${filename}\"
		MAIN_DEPENDENCY ${root}/${file}
		COMMENT "Copying ${filename}"
	   )
	elseif(${ext} STREQUAL ".wav")
	   ADD_CUSTOM_COMMAND(OUTPUT ${SK_DIST_DIRECTORY}/${path}/${dest}
		COMMAND ${WAV_CONVERT} \"${root}/${file}\" \"${SK_DIST_DIRECTORY}/${path}/${dest}\"
		MAIN_DEPENDENCY ${root}/${file}
		COMMENT "Converting wav file ${filename}"
	   )
	else()
	   ADD_CUSTOM_COMMAND(OUTPUT ${SK_DIST_DIRECTORY}/${path}/${dest}
		COMMAND ${CMAKE_COMMAND} -E copy "${root}/${file}" "${SK_DIST_DIRECTORY}/${path}/${dest}"
		MAIN_DEPENDENCY ${root}/${file}
		COMMENT "Copying ${filename}"
	   )
	endif()
	LIST( APPEND SkeletonKey_Src_PlatfFiles ${SK_DIST_DIRECTORY}/${path}/${dest} )
	LIST( APPEND DIST_FILES ${SK_DIST_DIRECTORY}/${path}/${dest} )
   ENDFOREACH()
 endif (NC_BUILD_PLATFORM_QNX)


# Building executable:
add_definitions("-DROCKET_WITH_FREETYPE")

if (NC_BUILD_PLATFORM_QNX)
    	add_executable( SkeletonKey
			${SkeletonKey_Src_Files}
			${SkeletonKey_Src_PlatfFiles}
			${SkeletonKey_Hdr_Files}
			${libRocket_Src_Files} )
elseif (WIN32)
    	add_executable( SkeletonKey WIN32 
			${SkeletonKey_Src_Files}
			${SkeletonKey_Src_PlatfFiles}
			${SkeletonKey_Hdr_Files} 
			${libRocket_Src_Files} )
elseif(APPLE)
	add_executable( SkeletonKey MACOSX_BUNDLE
			${SkeletonKey_Src_Files}
    			${SkeletonKey_Src_PlatfFiles}
			${SkeletonKey_Hdr_Files}
			${libRocket_Src_Files} )
endif(NC_BUILD_PLATFORM_QNX)
    
target_link_libraries( SkeletonKey ${SDL_LIBRARY} ${PLATF_LIBS})

# Target properties:

if (APPLE)
       	set_target_properties(SkeletonKey PROPERTIES OSX_ARCHITECTURES "i386;" )
endif()

# Optional packaging:

if (COMMAND COMPILE)
   COMPILE( SkeletonKey ${BUILD_VER} ${DIST_FILES} )
endif(COMMAND COMPILE)
