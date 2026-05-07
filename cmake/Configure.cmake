################################################################################
#
# configure
#
################################################################################

########################################
# FUNCTION check_includes
########################################
function(check_includes files)
    foreach(F ${${files}})
        set(name ${F})
        string(REPLACE "-" "_" name ${name})
        string(REPLACE "." "_" name ${name})
        string(REPLACE "/" "_" name ${name})
        string(TOUPPER ${name} name)
        check_include_files(${F} HAVE_${name})
        file(APPEND ${AUTOCONFIG_SRC} "/* Define to 1 if you have the <${F}> header file. */\n")
        file(APPEND ${AUTOCONFIG_SRC} "#cmakedefine HAVE_${name} 1\n")
        file(APPEND ${AUTOCONFIG_SRC} "\n")
    endforeach()
endfunction(check_includes)

########################################
# FUNCTION check_functions
########################################
function(check_functions functions)
    foreach(F ${${functions}})
        set(name ${F})
        string(TOUPPER ${name} name)
        check_function_exists(${F} HAVE_${name})
        file(APPEND ${AUTOCONFIG_SRC} "/* Define to 1 if you have the `${F}' function. */\n")
        file(APPEND ${AUTOCONFIG_SRC} "#cmakedefine HAVE_${name} 1\n")
        file(APPEND ${AUTOCONFIG_SRC} "\n")
    endforeach()
endfunction(check_functions)

########################################

file(WRITE ${AUTOCONFIG_SRC})

include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(CheckCXXSourceCompiles)
include(CheckCXXSourceRuns)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckPrototypeDefinition)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(CheckTypeSize)
include(TestBigEndian)

set(include_files_list
    dlfcn.h
    inttypes.h
    memory.h
    stdint.h
    stdlib.h
    strings.h
    string.h
    sys/stat.h
    sys/types.h
    unistd.h
)
check_includes(include_files_list)

set(functions_list
    fmemopen
    fstatat
)
check_functions(functions_list)

test_big_endian(BIG_ENDIAN)

if(BIG_ENDIAN)
  set(ENDIANNESS L_BIG_ENDIAN)
else()
  set(ENDIANNESS L_LITTLE_ENDIAN)
endif()

set(APPLE_UNIVERSAL_BUILD "defined (__APPLE_CC__)")
configure_file(
    ${PROJECT_SOURCE_DIR}/src/endianness.h.in
    ${PROJECT_BINARY_DIR}/src/endianness.h
    @ONLY)

if(NOT SW_BUILD)
  if(ENABLE_GIF AND GIF_FOUND)
    set(HAVE_LIBGIF 1)
  endif()

  if(ENABLE_JPEG AND JPEG_FOUND)
    set(HAVE_LIBJPEG 1)
  endif()

  if(ENABLE_OPENJPEG AND HAVE_LIBJP2K)
    set(HAVE_LIBJP2K 1)
  endif()

  if(ENABLE_PNG AND PNG_FOUND)
    set(HAVE_LIBPNG 1)
  endif()

  if(ENABLE_TIFF AND TIFF_FOUND)
    set(HAVE_LIBTIFF 1)
  endif()

  if(ENABLE_WEBP AND WebP_FOUND)
    set(HAVE_LIBWEBP 1)
    set(HAVE_LIBWEBP_ANIM 1)
  endif()

  if(ENABLE_ZLIB AND ZLIB_FOUND)
    set(HAVE_LIBZ 1)
  endif()
endif()

file(APPEND ${AUTOCONFIG_SRC} "
/* Define to 1 if you have giflib. */
#cmakedefine HAVE_LIBGIF 1

/* Define to 1 if you have libopenjp2. */
#cmakedefine HAVE_LIBJP2K 1

/* Define to 1 if you have jpeg. */
#cmakedefine HAVE_LIBJPEG 1

/* Define to 1 if you have libpng. */
#cmakedefine HAVE_LIBPNG 1

/* Define to 1 if you have libtiff. */
#cmakedefine HAVE_LIBTIFF 1

/* Define to 1 if you have libwebp. */
#cmakedefine HAVE_LIBWEBP 1

/* Define to 1 if you have libwebpmux. */
#cmakedefine HAVE_LIBWEBP_ANIM 1

/* Define to 1 if you have zlib. */
#cmakedefine HAVE_LIBZ 1
")

########################################

################################################################################
