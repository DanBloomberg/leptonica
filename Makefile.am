ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign
EXTRA_DIST = config README.html leptonica-license.txt moller52.jpg version-notes.html make-for-auto make-for-local autogen.sh Doxyfile

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = lept.pc

# Cmake configs:
lept_cmakeconfigdir = $(libdir)/cmake/
lept_cmakeconfig_DATA = \
	cmake/templates/LeptonicaConfig.cmake \
	cmake/templates/LeptonicaConfig-version.cmake

SUBDIRS = src prog

# The fuzzing tests are run by OSS-Fuzz https://oss-fuzz.com/,
# but can also be run locally.
# Note: -fsanitize=fuzzer currently requires the clang++ compiler.

# LIB_FUZZING_ENGINE can be overridden by the caller.
# This is used by OSS-Fuzz.
LIB_FUZZING_ENGINE ?= -fsanitize=fuzzer

FUZZERS :=
FUZZERS += barcode_fuzzer
FUZZERS += blend_fuzzer
FUZZERS += colorquant_fuzzer
FUZZERS += dewarp_fuzzer
FUZZERS += enhance_fuzzer
FUZZERS += pixa_recog_fuzzer
FUZZERS += pixconv_fuzzer
FUZZERS += pix_orient_fuzzer
FUZZERS += pix_rotate_shear_fuzzer

fuzzers: $(FUZZERS)

$(FUZZERS): all

%_fuzzer: $(top_srcdir)/prog/fuzzing/%_fuzzer.cc
	$(CXX) $(CXXFLAGS) -g $(LIB_FUZZING_ENGINE) \
          -I $(top_srcdir)/src \
          -I $(builddir)/src \
          $< \
          $(builddir)/src/.libs/liblept.a \
          $(GIFLIB_LIBS) \
          $(JPEG_LIBS) \
          $(LIBJP2K_LIBS) \
          $(LIBPNG_LIBS) \
          $(LIBTIFF_LIBS) \
          $(LIBWEBP_LIBS) \
          -o $@
