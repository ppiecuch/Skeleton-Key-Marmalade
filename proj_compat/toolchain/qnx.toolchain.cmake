# ------------------------------------------------------------------------------
#  Minimum QNX/Blackberry CMake toolchain file, for use with the Playbook SDK
#  and Blackberry SDK.
#  Requires cmake 2.6.3 or newer (2.8.3 or newer is recommended).
#
#  Version 1.0
#   Initial toolchain version.
# ------------------------------------------------------------------------------

cmake_minimum_required( VERSION 2.6.3 )

if( DEFINED CMAKE_CROSSCOMPILING )
 #subsequent toolchain loading is not really needed
 return()
endif()

get_property(_CMAKE_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE)
if( _CMAKE_IN_TRY_COMPILE )
 include( "${CMAKE_CURRENT_SOURCE_DIR}/qnx.toolchain.config.cmake" OPTIONAL )
 #subsequent toolchain loading is not really needed and failing builing
 return()
endif()

if( "$ENV{QNX_HOST}" AND NOT "${QNX_HOST}" )
    set( ${QNX_HOST} ENV{QNX_HOST} )
endif()
if( "$ENV{QNX_TARGET}" AND NOT "${QNX_TARGET}" )
    set( ${QNX_TARGET} ENV{QNX_TARGET} )
endif()

if( ".${QNX_TARGET}" STREQUAL "." AND ".${QNX_HOST}" STREQUAL ".")
  message( FATAL_ERROR "*** No Blackberry SDK specified. Stop." )
endif()

set( BLACKBERRY_TOOLCHAIN_ROOT "${QNX_HOST}" )
set( BLACKBERRY_TARGET_ROOT "${QNX_TARGET}" )

# STL version: by default gnustl_static will be used
set( BLACKBERRY_USE_STLPORT FALSE CACHE BOOL "Experimental: use stlport_static instead of gnustl_static")
mark_as_advanced( BLACKBERRY_USE_STLPORT )

# Detect host platform
set( TOOL_OS_SUFFIX "" )
if( CMAKE_HOST_APPLE )
 set( BLACKBERRY_NDK_HOST_SYSTEM_NAME "darwin-x86" )
elseif( CMAKE_HOST_WIN32 )
 set( BLACKBERRY_NDK_HOST_SYSTEM_NAME "windows" )
 set( TOOL_OS_SUFFIX ".exe" )
elseif( CMAKE_HOST_UNIX )
 set(BLACKBERRY_NDK_HOST_SYSTEM_NAME "linux-x86" )
else()
 message( FATAL_ERROR "Cross-compilation on your platform is not supported by this cmake toolchain" )
endif()

# toolchain info
SET(CMAKE_SYSTEM_NAME QNX)
SET(QNX 1)
SET(QNXNTO 1)
SET(BLACKBERRY 1)
SET(CMAKE_SYSTEM_VERSION 1.0)

if( NOT QNX_COMPILER_VERSION )
  execute_process( COMMAND ${QCC_VERSION_CHECK} "" OUTPUT_VARIABLE QNX_COMPILER_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE )
  string( REGEX MATCH "[0-9]+.[0-9]+.[0-9]+" QNX_COMPILER_VERSION "${QNX_COMPILER_VERSION}" )
endif()

SET( STAGE_LIB "${STAGE_DIR}/${CMAKE_SYSTEM_PROCESSOR}/lib" )
SET( STAGE_USR_LIB "${STAGE_DIR}/${CMAKE_SYSTEM_PROCESSOR}/usr/lib" )
SET( STAGE_INC "${STAGE_DIR}/usr/include" )
SET( TARGET_LIB "${QNX_TARGET}/lib" )
SET( TARGET_USR_LIB "${QNX_TARGET}/usr/lib" )
SET( TARGET_INC "${QNX_TARGET}/usr/include" )

SET( COMP_PATHS "-Wl,-rpath-link,${STAGE_LIB} -Wl,-rpath-link,${STAGE_USR_LIB} -L${STAGE_LIB} -L${STAGE_USR_LIB} -L${TARGET_LIB} -L${TARGET_USR_LIB} -I${STAGE_INC} -I${STAGE_INC}/freetype2 -I${TARGET_INC}" )

# Flags and preprocessor definitions
set( BLACKBERRY_CC_FLAGS  " -Vgcc_nto${CPU} -D__QNX__ -D__QNXNTO__ ${COMP_PATHS} ${PROFILER_FLAGS}" )
set( BLACKBERRY_CXX_FLAGS " -Vgcc_nto${CPU} -Y_cpp -D__QNX__ -D__QNXNTO__ ${COMP_PATHS} ${PROFILER_FLAGS}" )

SET( ENV{QNX_HOST} ${QNX_HOST} )
SET( ENV{QNX_TARGET} ${QNX_TARGET} )

# NDK flags
set( CMAKE_CXX_FLAGS "${BLACKBERRY_CXX_FLAGS}" )
set( CMAKE_C_FLAGS "${BLACKBERRY_CC_FLAGS}" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions" )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fexceptions" )

# Release and Debug flags
if( ${CPU} MATCHES "arm.*" )
 set( CMAKE_CXX_FLAGS_RELEASE "-mthumb -O3" )
 set( CMAKE_C_FLAGS_RELEASE   "-mthumb -O3" )
 set( CMAKE_CXX_FLAGS_DEBUG   "-g -DDEBUG -marm -Os -finline-limit=64" )
 set( CMAKE_C_FLAGS_DEBUG     "-g -DDEBUG -marm -Os -finline-limit=64" )
else()
 set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=i486" )
 set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=i486" )
endif()

# specify cross compiler
SET(CMAKE_C_COMPILER "${CC_COMMAND}" CACHE PATH "gcc")
SET(CMAKE_CXX_COMPILER "${CC_COMMAND}" CACHE PATH "g++")
SET(CMAKE_ASM_COMPILER "${CC_COMMAND}" CACHE PATH "assembler")
if( CMAKE_VERSION VERSION_LESS 2.8.5 )
 set( CMAKE_ASM_COMPILER_ARG1 "-c" )
endif()
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "c compiler settings")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "c++ compiler settings")
SET(CMAKE_INCLUDE_PATH "${STAGE_INC};${STAGE_INC}/skia" CACHE STRING "include paths")
SET(CMAKE_LIBRARY_PATH "${STAGE_LIB};${STAGE_USR_LIB}" CACHE STRING "library paths")
SET(CMAKE_AR "${QNX_HOST}/usr/bin/nto${BUSUFFIX}-ar${TOOL_OS_SUFFIX}" CACHE PATH "archive")
SET(CMAKE_RANLIB "${QNX_HOST}/usr/bin/nto${BUSUFFIX}-ranlib${TOOL_OS_SUFFIX}" CACHE PATH "ranlib")
SET(CMAKE_LD "{$QNX_HOST}/usr/bin/nto${BUSUFFIX}-ld${TOOL_OS_SUFFIX}" CACHE PATH "ld")
SET(CMAKE_LINKER "{$QNX_HOST}/usr/bin/nto${BUSUFFIX}-ld${TOOL_OS_SUFFIX}" CACHE PATH "linker")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-undefined -lm -lz -lbps -lscreen -lcpp" CACHE STRING "linker flags")

SET(THREADS_PTHREAD_ARG 0)

IF(APPLE)
 FIND_PROGRAM( CMAKE_INSTALL_NAME_TOOL NAMES install_name_tool )
 IF( NOT CMAKE_INSTALL_NAME_TOOL )
  MESSAGE( FATAL_ERROR "Could not find install_name_tool, please check your installation." )
 ENDIF()
 MARK_AS_ADVANCED( CMAKE_INSTALL_NAME_TOOL )
ENDIF()

SET(CMAKE_C_COMPILER_ENV_VAR "${CMAKE_C_COMPILER_ENV_VAR}")
SET(CMAKE_CXX_COMPILER_ENV_VAR "${CMAKE_CXX_COMPILER_ENV_VAR}")

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH "${CMAKE_SOURCE_DIR}" 
			 "${QNX_TARGET}" 
			 "${CMAKE_INSTALL_PREFIX}" 
			 "${CMAKE_INSTALL_PREFIX}/share")

# only search for libraries and includes in the ndk toolchain
set( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )
set( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )

# update cache
SET( QNX_HOST "${QNX_HOST}" CACHE INTERNAL "QNX host" FORCE)
SET( QNX_TARGET "${QNX_TARGET}" CACHE INTERNAL "QNX target" FORCE)
SET( QCC_VERSION_CHECK "${QCC_VERSION_CHECK}" CACHE INTERNAL "qcc version checker" )
SET( QNX_COMPILER_VERSION "${QNX_COMPILER_VERSION}" CACHE INTERNAL "compiler version from selected toolchain" )

# We are doing cross compiling, reset the OS information of the Building system
UNSET( APPLE )
UNSET( WIN32 )
UNSET( UNIX )

# Packaging utilities
SET(DEBUG_TOKEN "" CACHE FILEPATH "Path to the debug token to sign with")
SET(DEVICE_PASSWORD "" CACHE STRING "Password to the device")
SET(DEVICE_IP "169.254.0.1" CACHE STRING "IP address to the device")
SET(SIGNING_PASSWORD "" CACHE STRING "Password for the signing keys")

FUNCTION(COMPILE target build_ver dist_files)
	GET_FILENAME_COMPONENT(OUT_PATH ${dist_files} PATH)
	IF (build_ver)
	   SET(BARFILE "${target}-${build_ver}")
   	ELSE()
	   SET(BARFILE "${target}")
   	ENDIF (build_ver)
	
	IF (NOT DEBUG_TOKEN)
	   MESSAGE( FATAL_ERROR "Missing DEBUG_TOKEN - cannot create an installation package, CMake will exit." )
	ENDIF (NOT DEBUG_TOKEN)

	ADD_CUSTOM_TARGET(run
		blackberry-nativepackager -package ${BARFILE}-debug.bar -devMode -debugToken ${DEBUG_TOKEN} -installApp -launchApp -device ${DEVICE_IP} -password ${DEVICE_PASSWORD} ${dist_files}
		WORKING_DIRECTORY ${OUT_PATH}
		DEPENDS ${target}
		COMMENT "Package and deploy ${BARFILE}-debug.bar file"
	)
	ADD_CUSTOM_TARGET(release
		COMMAND blackberry-nativepackager -package ${BARFILE}-release.bar ${dist_files}
		COMMAND blackberry-signer -storepass ${SIGNING_PASSWORD} ${target}-release.bar
		WORKING_DIRECTORY ${OUT_PATH}
		DEPENDS ${target}
		COMMENT "Package and sign ${BARFILE}-release.bar file"
	)
ENDFUNCTION()
