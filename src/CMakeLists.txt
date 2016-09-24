#
# leptonica
#

################################################################################

########################################
# SHARED LIBRARY leptonica
########################################

file(GLOB hdr "*.h")
file(GLOB src "*.c")
if (MSVC)
    set_source_files_properties(${src} PROPERTIES LANGUAGE CXX)
endif()

add_library                     (leptonica ${LIBRARY_TYPE} ${src} ${hdr})
set_target_properties           (leptonica PROPERTIES VERSION   ${VERSION_MAJOR}.${VERSION_MINOR})
set_target_properties           (leptonica PROPERTIES SOVERSION ${VERSION_MAJOR}.${VERSION_MINOR})
if (WIN32)
set_target_properties           (leptonica PROPERTIES OUTPUT_NAME       lept${VERSION_MAJOR}${VERSION_MINOR})
set_target_properties           (leptonica PROPERTIES DEBUG_OUTPUT_NAME lept${VERSION_MAJOR}${VERSION_MINOR}d)
endif()

if (NOT STATIC)
    target_compile_definitions  (leptonica PRIVATE -DLIBLEPT_EXPORTS)
endif()

if (GIF_LIBRARY)
    target_link_libraries       (leptonica ${GIF_LIBRARY})
endif()
if (JPEG_LIBRARY)
    target_link_libraries       (leptonica ${JPEG_LIBRARY})
endif()
if (JP2K_FOUND)
    target_link_libraries       (leptonica ${JP2K_LIBRARIES})
endif()
if (PNG_LIBRARY)
    target_link_libraries       (leptonica ${PNG_LIBRARY})
endif()
if (TIFF_LIBRARY)
    target_link_libraries       (leptonica ${TIFF_LIBRARY})
endif()
if (WEBP_FOUND)
    target_link_libraries       (leptonica ${WEBP_LIBRARIES})
endif()
if (ZLIB_LIBRARY)
    target_link_libraries       (leptonica ${ZLIB_LIBRARY})
endif()

if (UNIX)
    target_link_libraries       (leptonica m)
endif()

if (NOT CPPAN_BUILD)
    export(TARGETS leptonica FILE ${CMAKE_BINARY_DIR}/LeptonicaTargets.cmake)
else()
    target_link_libraries       (leptonica cppan)
    file(WRITE ${CMAKE_BINARY_DIR}/LeptonicaTargets.cmake "include(${CMAKE_BINARY_DIR}/cppan.cmake)\n")
    export(TARGETS leptonica APPEND FILE ${CMAKE_BINARY_DIR}/LeptonicaTargets.cmake)
endif()

################################################################################

