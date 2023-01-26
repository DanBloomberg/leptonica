#
# leptonica
#

# ##############################################################################
#
# cmake settings
#
# ##############################################################################

cmake_minimum_required(VERSION 3.1.3)
cmake_policy(SET CMP0054 NEW)

# In-source builds are disabled.
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(
    FATAL_ERROR
      "CMake generation is not possible within the source directory!"
      "\n Remove the CMakeCache.txt file and try again from another folder"
      ", e.g.:\n "
      "\n rm CMakeCache.txt"
      "\n mkdir build"
      "\n cd build"
      "\n cmake ..")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake Targets")

# ##############################################################################
#
# project settings
#
# ##############################################################################

project(leptonica C CXX)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 83)
set(VERSION_PATCH 1)
set(VERSION_PLAIN ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
set(MINIMUM_WEBPMUX_VERSION 0.5.0)

option(BUILD_PROG "Build utility programs" OFF)
if(WIN32)
  option(SW_BUILD "Build with sw" ON)
else()
  option(SW_BUILD "Build with sw" OFF)
endif()
if(UNIX OR MINGW)
  option(SYM_LINK "Create symlink leptonica -> lept on UNIX" OFF)
endif()
option(LIBWEBP_SUPPORT "Build with WebP support" ON)
option(OPENJPEG_SUPPORT "Build with OpenJPEG support" ON)

set(leptonica_INSTALL_CMAKE_DIR
    "lib/cmake/leptonica"
    CACHE STRING "Install destination for CMake package files")

if(NOT SW_BUILD)
  find_package(GIF)
  if(GIF_FOUND)
    set(libs_private "${libs_private} -lgif")
  endif()
  find_package(JPEG)
  if(JPEG_FOUND)
    set(libs_private "${libs_private} -ljpeg")
  endif()
  find_package(PNG)
  if(PNG_FOUND)
    set(libs_private "${libs_private} -lpng")
  endif()
  find_package(TIFF)
  if(TIFF_FOUND)
    set(libs_private "${libs_private} -ltiff")
  endif()
  find_package(ZLIB)
  if(ZLIB_FOUND)
    set(libs_private "${libs_private} -lz")
  endif()
  find_package(PkgConfig QUIET)
  if(LIBWEBP_SUPPORT)
    if(PKG_CONFIG_FOUND)
      pkg_check_modules(WEBP libwebp QUIET)
      pkg_check_modules(WEBPMUX libwebpmux>=${MINIMUM_WEBPMUX_VERSION} QUIET)
    endif(PKG_CONFIG_FOUND)
    if(NOT WEBP)
      find_path(WEBP_INCLUDE_DIR /webp/decode.h)
      find_library(WEBP_LIBRARY NAMES webp)
      if(WEBP_INCLUDE_DIR AND WEBP_LIBRARY)
        set(WEBP 1)
        set(WEBP_FOUND TRUE)
        set(WEBP_LIBRARIES ${WEBP_LIBRARY})
        set(WEBP_INCLUDE_DIRS ${WEBP_INCLUDE_DIR})
      endif()
    endif()
    if(NOT WEBPMUX)
      find_path(WEBPMUX_INCLUDE_DIR /webp/mux.h)
      # TODO:  check minimal required version
      if(NOT WEBPMUX_INCLUDE_DIR)
        message(STATUS "Can not find: /webp/mux.h")
      endif()
      if(NOT "${WEBPMUX_INCLUDE_DIR}" STREQUAL "${WEBP_INCLUDE_DIR}")
        set(WEBP_INCLUDE_DIRS ${WEBP_INCLUDE_DIRS} ${WEBPMUX_INCLUDE_DIR})
      endif()
      find_library(WEBPMUX_LIBRARY NAMES webpmux)
      if(WEBPMUX_INCLUDE_DIR AND WEBPMUX_LIBRARY)
        set(WEBPMUX 1)
        set(HAVE_LIBWEBP_ANIM 1)
        set(WEBPMUX_FOUND TRUE)
        set(WEBP_LIBRARIES ${WEBP_LIBRARIES} ${WEBPMUX_LIBRARY})
      endif()
    endif()
    if(WEBP)
      set(libs_private "${libs_private} -lwebp")
    endif(WEBP)
    if(WEBPMUX)
      set(libs_private "${libs_private} -lwebpmux")
    endif(WEBPMUX)
  endif(LIBWEBP_SUPPORT)
  if(OPENJPEG_SUPPORT)
    find_package(OpenJPEG)
    # we need absolute path in some cases on Windows
    find_library(JP2K_LIBRARY NAMES openjp2)
    if(OpenJPEG_FOUND AND JP2K_LIBRARY)
      set(JP2K_FOUND TRUE)
      set(JP2K_INCLUDE_DIRS ${OPENJPEG_INCLUDE_DIRS})
      get_filename_component(JP2K_HEADER_DIR "${JP2K_INCLUDE_DIRS}" NAME)
    else()
      if(PKG_CONFIG_FOUND)
        pkg_check_modules(JP2K libopenjp2>=2.0 QUIET)
      endif(PKG_CONFIG_FOUND)
    endif()
    if(JP2K_FOUND)
      set(JP2K_LIBRARIES ${JP2K_LIBRARY})
      set(libs_private "${libs_private} -lopenjp2")
      message(STATUS "FOUND OpenJPEG: ${JP2K_LIBRARIES}")
    endif()
  endif(OPENJPEG_SUPPORT)
else() # SW_BUILD=ON
  find_package(SW REQUIRED)
  sw_add_package(
    org.sw.demo.gif org.sw.demo.jpeg org.sw.demo.glennrp.png org.sw.demo.tiff
    org.sw.demo.webmproject.webp org.sw.demo.uclouvain.openjpeg.openjp2)
  sw_execute()

  add_definitions(
    -DHAVE_LIBGIF=1
    -DHAVE_LIBJPEG=1
    -DHAVE_LIBPNG=1
    -DHAVE_LIBTIFF=1
    -DHAVE_LIBWEBP=1
    -DHAVE_LIBWEBP_ANIM=1
    -DHAVE_LIBZ=1
    -DHAVE_LIBJP2K=1
    -DLIBJP2K_HEADER="openjpeg.h")
endif()

set(libs_private "${libs_private} -lm")

# ##############################################################################
#
# compiler and linker
#
# ##############################################################################

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CLANG 1)
endif()

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
else()
  message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
endif()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

# Check for valid compiler flags
macro(CHECK_CCXX_COMPILER_FLAG flag_ result_)
  check_c_compiler_flag(${flag_} C_HAS${result_})
  check_cxx_compiler_flag(${flag_} CXX_HAS${result_})
endmacro()

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_compile_options("$<$<CONFIG:Release>:/W1;/Ox>")
  add_compile_options("$<$<CONFIG:Debug>:/W4;/DDEBUG>")
  if(NOT CLANG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif()
else()
  if(CMAKE_BUILD_TYPE MATCHES "[Dd][Ee][Bb][Uu][Gg]")
    # Enable warnings and Optimize for Debugging
    foreach(
      flag_
      -Wall
      -Wextra
      -Werror=format-security
      -pedantic
      -Og
      -g3
      "-fdebug-prefix-map=${CMAKE_SOURCE_DIR}=.")
      string(REGEX REPLACE "[^A-Za-z0-9]" "_" flag_var "${flag_}")
      check_ccxx_compiler_flag(${flag_} ${flag_var})
      if(C_HAS${flag_var})
        set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${flag_}")
      endif()
      if(CXX_HAS${flag_var})
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${flag_}")
      endif()
    endforeach()
    add_definitions(-DDEBUG)
  endif()
endif()

# ##############################################################################
#
# configure
#
# ##############################################################################

set(AUTOCONFIG_SRC ${CMAKE_CURRENT_BINARY_DIR}/config_auto.h.in)
set(AUTOCONFIG ${CMAKE_CURRENT_BINARY_DIR}/src/config_auto.h)
if(HAVE_LIBJP2K)
  set(ADDITIONAL_INCLUDE_DIRS "${JP2K_INCLUDE_DIR}")
endif()
set(CMAKE_REQUIRED_INCLUDES
    ${CMAKE_REQUIRED_INCLUDES} "${CMAKE_PREFIX_PATH}/include"
    "${CMAKE_INSTALL_PREFIX}/include" "${ADDITIONAL_INCLUDE_DIRS}")

include(Configure)

configure_file(${AUTOCONFIG_SRC} ${AUTOCONFIG} @ONLY)

set(INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include"
                "${CMAKE_INSTALL_PREFIX}/include/leptonica")

# ##############################################################################
#
# Show summary
#
# ##############################################################################
message(STATUS)
message(STATUS "General configuration for Leptonica ${VERSION_PLAIN}")
message(STATUS "--------------------------------------------------------")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "C compiler options:
  ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE_UP}} ${CMAKE_C_FLAGS}")
message(STATUS "Linker options: ${CMAKE_EXE_LINKER_FLAGS}
  ${CMAKE_EXE_LINKER_FLAGS_${CMAKE_BUILD_TYPE_UP}}")
message(STATUS "Install directory: ${CMAKE_INSTALL_PREFIX}")
message(STATUS)
message(STATUS "Build with sw [SW_BUILD]: ${SW_BUILD}")
message(STATUS "Build utility programs [BUILD_PROG]: ${BUILD_PROG}")
if(NOT SW_BUILD)
  message(STATUS "Used ZLIB library: ${ZLIB_LIBRARIES}")
  message(STATUS "Used PNG library:  ${PNG_LIBRARIES}")
  message(STATUS "Used JPEG library: ${JPEG_LIBRARIES}")
  message(STATUS "Used JP2K library: ${JP2K_LIBRARIES}")
  message(STATUS "Used TIFF library: ${TIFF_LIBRARIES}")
  message(STATUS "Used GIF library:  ${GIF_LIBRARIES}")
  message(STATUS "Used WEBP library: ${WEBP_LIBRARIES}")
endif()
message(STATUS "--------------------------------------------------------")
message(STATUS)

# ##############################################################################
#
# build
#
# ##############################################################################

add_definitions(-DHAVE_CONFIG_H)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)

if(BUILD_PROG)
  add_subdirectory(prog)
endif()

add_subdirectory(src)

get_target_property(leptonica_NAME leptonica NAME)
set(leptonica_VERSION ${VERSION_PLAIN})
get_target_property(leptonica_OUTPUT_NAME leptonica OUTPUT_NAME)

configure_file(lept.pc.cmake ${CMAKE_CURRENT_BINARY_DIR}/lept.pc @ONLY)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/LeptonicaConfig-version.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/LeptonicaConfig-version.cmake @ONLY)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/LeptonicaConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/LeptonicaConfig.cmake @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lept.pc DESTINATION lib/pkgconfig)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/LeptonicaConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/LeptonicaConfig-version.cmake
        DESTINATION ${leptonica_INSTALL_CMAKE_DIR})

# ##############################################################################
# uninstall target
# ##############################################################################
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/templates/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake" IMMEDIATE @ONLY)

  add_custom_target(
    uninstall COMMAND ${CMAKE_COMMAND} -P
                      ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

# ##############################################################################
