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
