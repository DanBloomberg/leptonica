prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}/bin
libdir=${prefix}/lib
includedir=${prefix}/include

Name: @leptonica_NAME@
Description: An open source C library for efficient image processing and image analysis operations
Version: @leptonica_VERSION@
Libs: -L${libdir} -l@leptonica_OUTPUT_NAME@
Libs.private:
Cflags: -I${includedir} -I${includedir}/leptonica
