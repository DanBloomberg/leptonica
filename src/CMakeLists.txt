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

string(REPLACE "-O3" "-O2" CMAKE_C_FLAGS_RELEASE ${CMAKE_C_FLAGS_RELEASE})

add_library                     (leptonica ${src} ${hdr})
set_target_properties           (leptonica PROPERTIES VERSION   ${VERSION_PLAIN})
set_target_properties           (leptonica PROPERTIES SOVERSION 5.4.0)
if (WIN32)
set_target_properties           (leptonica PROPERTIES OUTPUT_NAME       leptonica-${VERSION_PLAIN})
set_target_properties           (leptonica PROPERTIES DEBUG_OUTPUT_NAME leptonica-${VERSION_PLAIN}d)
else()
set_target_properties           (leptonica PROPERTIES OUTPUT_NAME       leptonica)
set_target_properties           (leptonica PROPERTIES DEBUG_OUTPUT_NAME leptonica)
endif()

if (BUILD_SHARED_LIBS)
    target_compile_definitions  (leptonica PRIVATE -DLIBLEPT_EXPORTS)
endif()

if (GIF_LIBRARIES)
    target_include_directories  (leptonica PUBLIC ${GIF_INCLUDE_DIR})
    target_link_libraries       (leptonica ${GIF_LIBRARIES})
endif()
if (JPEG_LIBRARIES)
    target_include_directories  (leptonica PUBLIC ${JPEG_INCLUDE_DIR})
    target_link_libraries       (leptonica ${JPEG_LIBRARIES})
endif()
if (JP2K_FOUND)
    target_include_directories  (leptonica PUBLIC ${JP2K_INCLUDE_DIRS})
    target_link_libraries       (leptonica ${JP2K_LIBRARIES})
endif()
if (PNG_LIBRARIES)
    target_include_directories  (leptonica PUBLIC ${PNG_INCLUDE_DIRS})
    target_link_libraries       (leptonica ${PNG_LIBRARIES})
endif()
if (TIFF_LIBRARIES)
    target_include_directories  (leptonica PUBLIC ${TIFF_INCLUDE_DIR})
    target_link_libraries       (leptonica ${TIFF_LIBRARIES})
endif()
if (WEBP_FOUND)
    target_include_directories  (leptonica PUBLIC ${WEBP_INCLUDE_DIRS})
    target_link_libraries       (leptonica ${WEBP_LIBRARIES})
endif()
if (ZLIB_LIBRARIES)
    target_include_directories  (leptonica PUBLIC ${ZLIB_INCLUDE_DIRS})
    target_link_libraries       (leptonica ${ZLIB_LIBRARIES})
endif()

if (UNIX)
    target_link_libraries       (leptonica m)
    set(lib ${CMAKE_SHARED_LIBRARY_PREFIX})
    set(dll ${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(old_target ${lib}lept${dll})
    if (SYM_LINK)
        add_custom_target(lept ALL
            ${CMAKE_COMMAND} -E create_symlink "$<TARGET_FILE:leptonica>" ${old_target}
            DEPENDS ${PROJECT_NAME}
            COMMENT "Creating lept symlink")
        install(FILES ${CMAKE_BINARY_DIR}/src/${old_target} DESTINATION lib)
    endif(SYM_LINK)
endif()

if (NOT SW_BUILD)
    export(TARGETS leptonica FILE ${CMAKE_BINARY_DIR}/LeptonicaTargets.cmake)
else()
    target_link_libraries       (leptonica
        org.sw.demo.gif
        org.sw.demo.jpeg
        org.sw.demo.glennrp.png
        org.sw.demo.tiff
        org.sw.demo.webmproject.webp
        org.sw.demo.uclouvain.openjpeg.openjp2
    )
    file(WRITE ${CMAKE_BINARY_DIR}/LeptonicaTargets.cmake "include(${CMAKE_BINARY_DIR}/cppan.cmake)\n")
    export(TARGETS leptonica APPEND FILE ${CMAKE_BINARY_DIR}/LeptonicaTargets.cmake)
endif()

install(TARGETS leptonica EXPORT LeptonicaTargets RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install(EXPORT LeptonicaTargets DESTINATION cmake)
install(FILES
    ${hdr}
    ${CMAKE_BINARY_DIR}/src/endianness.h
    DESTINATION include/leptonica)

################################################################################
