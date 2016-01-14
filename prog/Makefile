#/*====================================================================*
# -  Copyright (C) 2001 Leptonica.  All rights reserved.
# -  This software is distributed in the hope that it will be
# -  useful, but with NO WARRANTY OF ANY KIND.
# -  No author or distributor accepts responsibility to anyone for the
# -  consequences of using this software, or for whether it serves any
# -  particular purpose or works at all, unless he or she says so in
# -  writing.  Everyone is granted permission to copy, modify and
# -  redistribute this source code, for commercial or non-commercial
# -  purposes, with the following restrictions: (1) the origin of this
# -  source code must not be misrepresented; (2) modified versions must
# -  be plainly marked as such; and (3) this notice may not be removed
# -  or altered from any source or modified source distribution.
# *====================================================================*/

#   Makefile  (for linux)
#
#   For a nodebug version:             make
#   For a debug version:               make DEBUG=yes
#   For a shared library version:      make SHARED=yes
#   With nonstandard library directories:
#                    make EXTRALIBS="-L<nonstandard-lib-dir>"
#   With nonstandard header directories
#                    make EXTRAINCLUDES="-I<nonstandard-incl-dir>"
#
#   To remove all executables:  make clean
#
#   ========================================================================
#   IMPORTANT: This Makefile is set up to link with liblept.a and liblept.so
#   that were built using using src/makefile (i.e., with 'make' in src).
#   If instead you built the library with configure/make/makeinstall,
#   you must do this here:
#
#     (1) Change LIB_NODEBUG_DIR:
#            LIB_NODEBUG_DIR = /usr/local/lib  [or wherever you installed
#                                               liblept.a]
#     (2) Edit ALL_LIBS to include the imaging libraries on your system,
#         as found in config_auto.h.  For example, if you have the
#         jpeg, png, tiff and gif libraries, set
#            ALL_LIBS = $(LEPTLIB) -ltiff -ljpeg -lpng -lgif -lz -lm
#   ========================================================================
#
#   To link and run programs using shared (dynamic linked) libraries,
#   you must do one of the following two things:
#
#     (a)  make sure your LD_LIBRARY_PATH variable points to the
#          directory in which the *.so files are placed; e.g.
#            export LD_LIBRARY_PATH=../lib/shared:$LD_LIBRARY_PATH
#          or
#     (b)  have the *.so files (or links to them) in this directory
#
#   Using gnu make, this should compile properly on a bigendian
#   machine (e.g., Mac).  Otherwise, you can explicitly set the endianness,
#   using either of these
#      CPPFLAGS =    $(INCLUDES) -DL_LITTLE_ENDIAN -D_STANDARD_C_
#      CPPFLAGS =    $(INCLUDES) -DL_BIG_ENDIAN -D_STANDARD_C_
#
#   To use the non-POSIX-compliant GNU functions fmemopen() and open_memstream(),
#   remove -D_STANDARD_C_ from CPPFLAGS:
#      CPPFLAGS =    $(INCLUDES) -D$(ENDIANNESS)
#
#   On cygwin, remove -fPIC from CC.  I believe that these files --
#   convertfilestops.c, jbcorrelation.c, jbrankhaus.c, maketile.c,
#   and viewertest.c -- which use posix directory functions, will
#   now work properly.
#
#   See below for enabling gif read/write.


# Libraries are built into a binary tree with its root in the
# parent directory
ROOT_DIR = ..

LIB_NODEBUG_DIR =	$(ROOT_DIR)/lib/nodebug
LIB_DEBUG_DIR =		$(ROOT_DIR)/lib/debug
LIB_SHARED_DIR =	$(ROOT_DIR)/lib/shared

# Include files are found within the same tree
IMAGELIB_INCL =	$(ROOT_DIR)/src


CC =		gcc -ansi -Werror -DANSI -fPIC
#CC =            g++ -Werror -fPIC


ifdef  SHARED
    LIB_DIR =	$(LIB_SHARED_DIR)
    OPTIMIZE =	-O2
else
    ifdef  DEBUG
	LIB_DIR =	$(LIB_DEBUG_DIR)
	OPTIMIZE =	-g
    else
	LIB_DIR =	$(LIB_NODEBUG_DIR)
	OPTIMIZE =	-O2
    endif
endif

OPTIONS =

INCLUDES =	-I$(IMAGELIB_INCL) -I/usr/X11R6/include $(EXTRAINCLUDES)
#INCLUDES =	-I$(IMAGELIB_INCL) -I/usr/X11R6/include -I/usr/local/include $(EXTRAINCLUDES)

CFLAGS =	$(OPTIMIZE) $(OPTIONS)

# Test for processor endianness (valid with gnu make only)
ENDIANNESS := $(shell $(CC) -o endiantest endiantest.c; ./endiantest; rm -f endiantest)

# Conditional compilation (depending on processor endian-ness)
# Use -D_STANDARD_C to stub out the non-posix GNU functions fmemopen()
# and open_memstream()
CPPFLAGS =      $(INCLUDES) -D$(ENDIANNESS)
#CPPFLAGS =      $(INCLUDES) -DL_LITTLE_ENDIAN
#CPPFLAGS =      $(INCLUDES) -DL_BIG_ENDIAN

LDFLAGS +=	-L$(LIB_DIR) -L/usr/X11R6/lib -L/usr/lib $(EXTRALIBS)
#LDFLAGS +=	-L$(LIB_DIR) -L/usr/X11R6/lib -L/usr/local/lib -L/usr/lib $(EXTRALIBS)

ifdef  SHARED
    LEPTLIB =		$(LIB_DIR)/liblept.so
else
    LEPTLIB =		$(LIB_DIR)/liblept.a
endif

# To enable gif read/write:
#    (1) In src/environ.h:  #define  HAVE_LIBGIF
#    (2) Install giflib-4.1.4, and be sure your LD_LIBRARY_PATH includes
#        the directory in which libgif.so.4 is installed.
#    (3) Use the second line below for ALL_LIBS
ALL_LIBS =	$(LEPTLIB) -ltiff -ljpeg -lpng -lz -lm
#ALL_LIBS =	$(LEPTLIB) -ltiff -ljpeg -lpng -lgif -lz -lm

#########################################################################

SRC =		binmorph1_reg.c binmorph2_reg.c binmorph3_reg.c \
		blend_reg.c ccthin1_reg.c ccthin2_reg.c \
		cmapquant_reg.c colorquant_reg.c \
		compfilter_reg.c \
		conncomp_reg.c conversion_reg.c \
		distance_reg.c dwamorph1_reg.c enhance_reg.c \
		equal_reg.c expand_reg.c \
		fhmtauto_reg.c flipdetect_reg.c \
		fmorphauto_reg.c fpix_reg.c gifio_reg.c \
		graymorph_reg.c grayquant_reg.c \
		hardlight_reg.c ioformats_reg.c \
		kernel_reg.c locminmax_reg.c logicops_reg.c \
		morphseq_reg.c numa_reg.c \
		paint_reg.c paintmask_reg.c \
		pixa_reg.c pixadisp_reg.c pixtile_reg.c \
		rank_reg.c rasterop_reg.c rasteropip_reg.c \
		rotate_reg.c rotateorth_reg.c \
		scale_reg.c selio_reg.c \
		string_reg.c threshnorm_reg.c \
		xformbox_reg.c \
		adaptmaptest.c affinetest.c \
		arithtest.c barcodetest.c \
		baselinetest.c bilineartest.c \
		bincompare.c binmazetest.c \
		blendcmaptest.c blendtest1.c buffertest.c \
		ccbordtest.c cctest1.c \
		colormorphtest.c \
		colorquanttest.c colorsegtest.c \
		colorspacetest.c comparetest.c contrasttest.c \
		convertfilestops.c convertformat.c converttogray.c \
		converttops.c convolvetest.c cornertest.c \
		croptext.c digitprep1.c dithertest.c \
		edgetest.c extrematest.c falsecolortest.c \
		fcombautogen.c fhmtautogen.c fileinfo.c \
		findpattern1.c findpattern2.c findpattern3.c \
		flipselgen.c \
		fmorphautogen.c gammatest.c \
		genfonts.c graphicstest.c \
		graymazetest.c graymorphtest.c \
		heaptest.c histotest.c iotest.c inserttest.c \
		jbcorrelation.c jbrankhaus.c jbwords.c \
		lineremoval.c listtest.c \
		maketile.c modifyhuesat.c \
		morphtest1.c mtifftest.c \
		numaranktest.c otsutest.c \
		pagesegtest1.c pagesegtest2.c pagesegtest3.c \
		partitiontest.c pixaatest.c \
		plottest.c \
		printimage.c printsplitimage.c printtiff.c \
		projectivetest.c ranktest.c reducetest.c \
		removecmap.c renderfonts.c \
		rotatefastalt.c rotateorthtest1.c \
		rotatetest1.c runlengthtest.c \
		scaletest1.c scaletest2.c \
		seedfilltest.c sharptest.c \
		sheartest.c showedges.c \
		skewtest.c snapcolortest.c \
		sorttest.c splitimage2pdf.c \
		textlinemask.c \
		viewertest.c wordsinorder.c \
		xvdisp.c

#  Source files that are compiled after running program(s) from SRC
SRC2 =		dwamorph2_reg.c dwamorph3_reg.c

#  Remove these files and their .o's on clean
SRC3 =		dwalinear.3.c dwalinearlow.3.c

######################################################################

all:	$(SRC:%.c=%)

debian:	binmazetest buffertest comparetest \
	cctest1 ccthin1_reg colormorphtest colorquant_reg \
	colorspacetest conversion_reg \
	convertfilestops convertformat \
	converttops distance_reg \
	dithertest edgetest equal_reg extrematest \
	fhmtauto_reg fhmtautogen fileinfo \
	flipdetect_reg flipselgen fmorphauto_reg fmorphautogen \
	gammatest graphicstest graymazetest graymorph_reg \
	grayquant_reg heaptest histotest ioformats_reg \
	jbcorrelation jbrankhaus jbwords \
	kernel_reg lineremoval numaranktest pagesegtest1 \
	pagesegtest2 pagesegtest3 paint_reg paintmask_reg \
	partitiontest plottest \
	printimage printsplitimage printtiff \
	ranktest rank_reg removecmap scale_reg \
	sharptest splitimage2pdf viewertest

######################################################################

binmorph1_reg:	binmorph1_reg.o $(LEPTLIB)
	$(CC) -o binmorph1_reg binmorph1_reg.o $(ALL_LIBS) $(EXTRALIBS)

binmorph2_reg:	binmorph2_reg.o $(LEPTLIB)
	$(CC) -o binmorph2_reg binmorph2_reg.o $(ALL_LIBS) $(EXTRALIBS)

binmorph3_reg:	binmorph3_reg.o $(LEPTLIB)
	$(CC) -o binmorph3_reg binmorph3_reg.o $(ALL_LIBS) $(EXTRALIBS)

blend_reg:	blend_reg.o $(LEPTLIB)
	$(CC) -o blend_reg blend_reg.o $(ALL_LIBS) $(EXTRALIBS)

ccthin1_reg:	ccthin1_reg.o $(LEPTLIB)
	$(CC) -o ccthin1_reg ccthin1_reg.o $(ALL_LIBS) $(EXTRALIBS)

ccthin2_reg:	ccthin2_reg.o $(LEPTLIB)
	$(CC) -o ccthin2_reg ccthin2_reg.o $(ALL_LIBS) $(EXTRALIBS)

cmapquant_reg:	cmapquant_reg.o $(LEPTLIB)
	$(CC) -o cmapquant_reg cmapquant_reg.o $(ALL_LIBS) $(EXTRALIBS)

colorquant_reg:	colorquant_reg.o $(LEPTLIB)
	$(CC) -o colorquant_reg colorquant_reg.o $(ALL_LIBS) $(EXTRALIBS)

compfilter_reg: compfilter_reg.o $(LEPTLIB)
	$(CC) -o compfilter_reg compfilter_reg.o $(ALL_LIBS) $(EXTRALIBS)

conncomp_reg: conncomp_reg.o $(LEPTLIB)
	$(CC) -o conncomp_reg conncomp_reg.o $(ALL_LIBS) $(EXTRALIBS)

conversion_reg: conversion_reg.o $(LEPTLIB)
	$(CC) -o conversion_reg conversion_reg.o $(ALL_LIBS) $(EXTRALIBS)

distance_reg:	distance_reg.o $(LEPTLIB)
	$(CC) -o distance_reg distance_reg.o $(ALL_LIBS) $(EXTRALIBS)

dwamorph1_reg: dwamorph1_reg.o $(LEPTLIB)
	$(CC) -o dwamorph1_reg dwamorph1_reg.o $(ALL_LIBS) $(EXTRALIBS)

dwamorph2_reg:  dwamorph2_reg.o dwalinear.3.o dwalinearlow.3.o $(LEPTLIB)
	$(CC) -o dwamorph2_reg dwamorph2_reg.o dwalinear.3.o dwalinearlow.3.o $(ALL_LIBS) $(EXTRALIBS)

dwamorph3_reg:  dwamorph3_reg.o dwalinear.3.o dwalinearlow.3.o $(LEPTLIB)
	$(CC) -o dwamorph3_reg dwamorph3_reg.o dwalinear.3.o dwalinearlow.3.o $(ALL_LIBS) $(EXTRALIBS)

enhance_reg:	enhance_reg.o $(LEPTLIB)
	$(CC) -o enhance_reg enhance_reg.o $(ALL_LIBS) $(EXTRALIBS)

equal_reg:	equal_reg.o $(LEPTLIB)
	$(CC) -o equal_reg equal_reg.o $(ALL_LIBS) $(EXTRALIBS)

expand_reg:	expand_reg.o $(LEPTLIB)
	$(CC) -o expand_reg expand_reg.o $(ALL_LIBS) $(EXTRALIBS)

fhmtauto_reg:	fhmtauto_reg.o $(LEPTLIB)
	$(CC) -o fhmtauto_reg fhmtauto_reg.o $(ALL_LIBS) $(EXTRALIBS)

flipdetect_reg:	flipdetect_reg.o $(LEPTLIB)
	$(CC) -o flipdetect_reg flipdetect_reg.o $(ALL_LIBS) $(EXTRALIBS)

fmorphauto_reg:	fmorphauto_reg.o $(LEPTLIB)
	$(CC) -o fmorphauto_reg fmorphauto_reg.o $(ALL_LIBS) $(EXTRALIBS)

fpix_reg:	fpix_reg.o $(LEPTLIB)
	$(CC) -o fpix_reg fpix_reg.o $(ALL_LIBS) $(EXTRALIBS)

gifio_reg:	gifio_reg.o $(LEPTLIB)
	$(CC) -o gifio_reg gifio_reg.o $(ALL_LIBS) $(EXTRALIBS)

graymorph_reg:	graymorph_reg.o $(LEPTLIB)
	$(CC) -o graymorph_reg graymorph_reg.o $(ALL_LIBS) $(EXTRALIBS)

grayquant_reg:	grayquant_reg.o $(LEPTLIB)
	$(CC) -o grayquant_reg grayquant_reg.o $(ALL_LIBS) $(EXTRALIBS)

hardlight_reg:	hardlight_reg.o $(LEPTLIB)
	$(CC) -o hardlight_reg hardlight_reg.o $(ALL_LIBS) $(EXTRALIBS)

ioformats_reg:	ioformats_reg.o $(LEPTLIB)
	$(CC) -o ioformats_reg ioformats_reg.o $(ALL_LIBS) $(EXTRALIBS)

kernel_reg:	kernel_reg.o $(LEPTLIB)
	$(CC) -o kernel_reg kernel_reg.o $(ALL_LIBS) $(EXTRALIBS)

locminmax_reg:	locminmax_reg.o $(LEPTLIB)
	$(CC) -o locminmax_reg locminmax_reg.o $(ALL_LIBS) $(EXTRALIBS)

logicops_reg:	logicops_reg.o $(LEPTLIB)
	$(CC) -o logicops_reg logicops_reg.o $(ALL_LIBS) $(EXTRALIBS)

morphseq_reg:	morphseq_reg.o $(LEPTLIB)
	$(CC) -o morphseq_reg morphseq_reg.o $(ALL_LIBS) $(EXTRALIBS)

numa_reg:	numa_reg.o $(LEPTLIB)
	$(CC) -o numa_reg numa_reg.o $(ALL_LIBS) $(EXTRALIBS)

paint_reg:	paint_reg.o $(LEPTLIB)
	$(CC) -o paint_reg paint_reg.o $(ALL_LIBS) $(EXTRALIBS)

paintmask_reg:	paintmask_reg.o $(LEPTLIB)
	$(CC) -o paintmask_reg paintmask_reg.o $(ALL_LIBS) $(EXTRALIBS)

pixa_reg:	pixa_reg.o $(LEPTLIB)
	$(CC) -o pixa_reg pixa_reg.o $(ALL_LIBS) $(EXTRALIBS)

pixadisp_reg:	pixadisp_reg.o $(LEPTLIB)
	$(CC) -o pixadisp_reg pixadisp_reg.o $(ALL_LIBS) $(EXTRALIBS)

pixtile_reg:	pixtile_reg.o $(LEPTLIB)
	$(CC) -o pixtile_reg pixtile_reg.o $(ALL_LIBS) $(EXTRALIBS)

rank_reg:	rank_reg.o $(LEPTLIB)
	$(CC) -o rank_reg rank_reg.o $(ALL_LIBS) $(EXTRALIBS)

rasterop_reg:	rasterop_reg.o $(LEPTLIB)
	$(CC) -o rasterop_reg rasterop_reg.o $(ALL_LIBS) $(EXTRALIBS)

rasteropip_reg:	rasteropip_reg.o $(LEPTLIB)
	$(CC) -o rasteropip_reg rasteropip_reg.o $(ALL_LIBS) $(EXTRALIBS)

rotate_reg:	rotate_reg.o $(LEPTLIB)
	$(CC) -o rotate_reg rotate_reg.o $(ALL_LIBS) $(EXTRALIBS)

rotateorth_reg:	rotateorth_reg.o $(LEPTLIB)
	$(CC) -o rotateorth_reg rotateorth_reg.o $(ALL_LIBS) $(EXTRALIBS)

scale_reg:	scale_reg.o $(LEPTLIB)
	$(CC) -o scale_reg scale_reg.o $(ALL_LIBS) $(EXTRALIBS)

selio_reg:	selio_reg.o $(LEPTLIB)
	$(CC) -o selio_reg selio_reg.o $(ALL_LIBS) $(EXTRALIBS)

string_reg:	string_reg.o $(LEPTLIB)
	$(CC) -o string_reg string_reg.o $(ALL_LIBS) $(EXTRALIBS)

threshnorm_reg:	threshnorm_reg.o $(LEPTLIB)
	$(CC) -o threshnorm_reg threshnorm_reg.o $(ALL_LIBS) $(EXTRALIBS)

xformbox_reg:	xformbox_reg.o $(LEPTLIB)
	$(CC) -o xformbox_reg xformbox_reg.o $(ALL_LIBS) $(EXTRALIBS)

adaptmaptest:	adaptmaptest.o $(LEPTLIB)
	$(CC) -o adaptmaptest adaptmaptest.o $(ALL_LIBS) $(EXTRALIBS)

affinetest:	affinetest.o $(LEPTLIB)
	$(CC) -o affinetest affinetest.o $(ALL_LIBS) $(EXTRALIBS)

arithtest:	arithtest.o $(LEPTLIB)
	$(CC) -o arithtest arithtest.o $(ALL_LIBS) $(EXTRALIBS)

barcodetest:	barcodetest.o $(LEPTLIB)
	$(CC) -o barcodetest barcodetest.o $(ALL_LIBS) $(EXTRALIBS)

baselinetest:	baselinetest.o $(LEPTLIB)
	$(CC) -o baselinetest baselinetest.o $(ALL_LIBS) $(EXTRALIBS)

bilineartest:	bilineartest.o $(LEPTLIB)
	$(CC) -o bilineartest bilineartest.o $(ALL_LIBS) $(EXTRALIBS)

bincompare:	bincompare.o $(LEPTLIB)
	$(CC) -o bincompare bincompare.o $(ALL_LIBS) $(EXTRALIBS)

binmazetest:	binmazetest.o $(LEPTLIB)
	$(CC) -o binmazetest binmazetest.o $(ALL_LIBS) $(EXTRALIBS)

blendcmaptest:	blendcmaptest.o $(LEPTLIB)
	$(CC) -o blendcmaptest blendcmaptest.o $(ALL_LIBS) $(EXTRALIBS)

blendtest1:	blendtest1.o $(LEPTLIB)
	$(CC) -o blendtest1 blendtest1.o $(ALL_LIBS) $(EXTRALIBS)

buffertest:	buffertest.o $(LEPTLIB)
	$(CC) -o buffertest buffertest.o $(ALL_LIBS) $(EXTRALIBS)

ccbordtest:	ccbordtest.o $(LEPTLIB)
	$(CC) -o ccbordtest ccbordtest.o $(ALL_LIBS) $(EXTRALIBS)

cctest1:	cctest1.o $(LEPTLIB)
	$(CC) -o cctest1 cctest1.o $(ALL_LIBS) $(EXTRALIBS)

colormorphtest:	colormorphtest.o $(LEPTLIB)
	$(CC) -o colormorphtest colormorphtest.o $(ALL_LIBS) $(EXTRALIBS)

colorquanttest:	colorquanttest.o $(LEPTLIB)
	$(CC) -o colorquanttest colorquanttest.o $(ALL_LIBS) $(EXTRALIBS)

colorsegtest:	colorsegtest.o $(LEPTLIB)
	$(CC) -o colorsegtest colorsegtest.o $(ALL_LIBS) $(EXTRALIBS)

colorspacetest:	colorspacetest.o $(LEPTLIB)
	$(CC) -o colorspacetest colorspacetest.o $(ALL_LIBS) $(EXTRALIBS)

comparetest:	comparetest.o $(LEPTLIB)
	$(CC) -o comparetest comparetest.o $(ALL_LIBS) $(EXTRALIBS)

contrasttest:	contrasttest.o $(LEPTLIB)
	$(CC) -o contrasttest contrasttest.o $(ALL_LIBS) $(EXTRALIBS)

convertfilestops:	convertfilestops.o $(LEPTLIB)
	$(CC) -o convertfilestops convertfilestops.o $(ALL_LIBS) $(EXTRALIBS)

convertformat:	convertformat.o $(LEPTLIB)
	$(CC) -o convertformat convertformat.o $(ALL_LIBS) $(EXTRALIBS)

converttogray:	converttogray.o $(LEPTLIB)
	$(CC) -o converttogray converttogray.o $(ALL_LIBS) $(EXTRALIBS)

converttops:	converttops.o $(LEPTLIB)
	$(CC) -o converttops converttops.o $(ALL_LIBS) $(EXTRALIBS)

convolvetest:	convolvetest.o $(LEPTLIB)
	$(CC) -o convolvetest convolvetest.o $(ALL_LIBS) $(EXTRALIBS)

cornertest:	cornertest.o $(LEPTLIB)
	$(CC) -o cornertest cornertest.o $(ALL_LIBS) $(EXTRALIBS)

croptext:	croptext.o $(LEPTLIB)
	$(CC) -o croptext croptext.o $(ALL_LIBS) $(EXTRALIBS)

digitprep1:	digitprep1.o $(LEPTLIB)
	$(CC) -o digitprep1 digitprep1.o $(ALL_LIBS) $(EXTRALIBS)

dithertest:	dithertest.o $(LEPTLIB)
	$(CC) -o dithertest dithertest.o $(ALL_LIBS) $(EXTRALIBS)

edgetest:	edgetest.o $(LEPTLIB)
	$(CC) -o edgetest edgetest.o $(ALL_LIBS) $(EXTRALIBS)

extrematest:	extrematest.o $(LEPTLIB)
	$(CC) -o extrematest extrematest.o $(ALL_LIBS) $(EXTRALIBS)

falsecolortest:	falsecolortest.o $(LEPTLIB)
	$(CC) -o falsecolortest falsecolortest.o $(ALL_LIBS) $(EXTRALIBS)

fhmtautogen:	fhmtautogen.o $(LEPTLIB)
	$(CC) -o fhmtautogen fhmtautogen.o $(ALL_LIBS) $(EXTRALIBS)

fileinfo:	fileinfo.o $(LEPTLIB)
	$(CC) -o fileinfo fileinfo.o $(ALL_LIBS) $(EXTRALIBS)

findpattern1:	findpattern1.o $(LEPTLIB)
	$(CC) -o findpattern1 findpattern1.o $(ALL_LIBS) $(EXTRALIBS)

findpattern2:	findpattern2.o $(LEPTLIB)
	$(CC) -o findpattern2 findpattern2.o $(ALL_LIBS) $(EXTRALIBS)

findpattern3:	findpattern3.o $(LEPTLIB)
	$(CC) -o findpattern3 findpattern3.o $(ALL_LIBS) $(EXTRALIBS)

flipselgen:	flipselgen.o $(LEPTLIB)
	$(CC) -o flipselgen flipselgen.o $(ALL_LIBS) $(EXTRALIBS)

fcombautogen:	fcombautogen.o $(LEPTLIB)
	$(CC) -o fcombautogen fcombautogen.o $(ALL_LIBS) $(EXTRALIBS)

fmorphautogen:	fmorphautogen.o $(LEPTLIB)
	$(CC) -o fmorphautogen fmorphautogen.o $(ALL_LIBS) $(EXTRALIBS)

gammatest:	gammatest.o $(LEPTLIB)
	$(CC) -o gammatest gammatest.o $(ALL_LIBS) $(EXTRALIBS)

genfonts:	genfonts.o $(LEPTLIB)
	$(CC) -o genfonts genfonts.o $(ALL_LIBS) $(EXTRALIBS)

graphicstest:	graphicstest.o $(LEPTLIB)
	$(CC) -o graphicstest graphicstest.o $(ALL_LIBS) $(EXTRALIBS)

graymazetest:	graymazetest.o $(LEPTLIB)
	$(CC) -o graymazetest graymazetest.o $(ALL_LIBS) $(EXTRALIBS)

graymorphtest:	graymorphtest.o $(LEPTLIB)
	$(CC) -o graymorphtest graymorphtest.o $(ALL_LIBS) $(EXTRALIBS)

heaptest:	heaptest.o $(LEPTLIB)
	$(CC) -o heaptest heaptest.o $(ALL_LIBS) $(EXTRALIBS)

histotest:	histotest.o $(LEPTLIB)
	$(CC) -o histotest histotest.o $(ALL_LIBS) $(EXTRALIBS)

iotest:		iotest.o $(LEPTLIB)
	$(CC) -o iotest iotest.o $(ALL_LIBS) $(EXTRALIBS)

inserttest:	inserttest.o $(LEPTLIB)
	$(CC) -o inserttest inserttest.o $(ALL_LIBS) $(EXTRALIBS)

jbcorrelation:	jbcorrelation.o $(LEPTLIB)
	$(CC) -o jbcorrelation jbcorrelation.o $(ALL_LIBS) $(EXTRALIBS)

jbrankhaus:	jbrankhaus.o $(LEPTLIB)
	$(CC) -o jbrankhaus jbrankhaus.o $(ALL_LIBS) $(EXTRALIBS)

jbwords:	jbwords.o $(LEPTLIB)
	$(CC) -o jbwords jbwords.o $(ALL_LIBS) $(EXTRALIBS)

lineremoval:	lineremoval.o $(LEPTLIB)
	$(CC) -o lineremoval lineremoval.o $(ALL_LIBS) $(EXTRALIBS)

listtest:	listtest.o $(LEPTLIB)
	$(CC) -o listtest listtest.o $(ALL_LIBS) $(EXTRALIBS)

maketile:	maketile.o $(LEPTLIB)
	$(CC) -o maketile maketile.o $(ALL_LIBS) $(EXTRALIBS)

modifyhuesat:	modifyhuesat.o $(LEPTLIB)
	$(CC) -o modifyhuesat modifyhuesat.o $(ALL_LIBS) $(EXTRALIBS)

morphtest1:	morphtest1.o $(LEPTLIB)
	$(CC) -o morphtest1 morphtest1.o $(ALL_LIBS) $(EXTRALIBS)

mtifftest:	mtifftest.o $(LEPTLIB)
	$(CC) -o mtifftest mtifftest.o $(ALL_LIBS) $(EXTRALIBS)

numaranktest:	numaranktest.o $(LEPTLIB)
	$(CC) -o numaranktest numaranktest.o $(ALL_LIBS) $(EXTRALIBS)

otsutest:	otsutest.o $(LEPTLIB)
	$(CC) -o otsutest otsutest.o $(ALL_LIBS) $(EXTRALIBS)

pagesegtest1:	pagesegtest1.o $(LEPTLIB)
	$(CC) -o pagesegtest1 pagesegtest1.o $(ALL_LIBS) $(EXTRALIBS)

pagesegtest2:	pagesegtest2.o $(LEPTLIB)
	$(CC) -o pagesegtest2 pagesegtest2.o $(ALL_LIBS) $(EXTRALIBS)

pagesegtest3:	pagesegtest3.o $(LEPTLIB)
	$(CC) -o pagesegtest3 pagesegtest3.o $(ALL_LIBS) $(EXTRALIBS)

partitiontest:	partitiontest.o $(LEPTLIB)
	$(CC) -o partitiontest partitiontest.o $(ALL_LIBS) $(EXTRALIBS)

pixaatest:	pixaatest.o $(LEPTLIB)
	$(CC) -o pixaatest pixaatest.o $(ALL_LIBS) $(EXTRALIBS)

plottest:	plottest.o $(LEPTLIB)
	$(CC) -o plottest plottest.o $(ALL_LIBS) $(EXTRALIBS)

printimage:	printimage.o $(LEPTLIB)
	$(CC) -o printimage printimage.o $(ALL_LIBS) $(EXTRALIBS)

printsplitimage:	printsplitimage.o $(LEPTLIB)
	$(CC) -o printsplitimage printsplitimage.o $(ALL_LIBS) $(EXTRALIBS)

printtiff:	printtiff.o $(LEPTLIB)
	$(CC) -o printtiff printtiff.o $(ALL_LIBS) $(EXTRALIBS)

projectivetest:	projectivetest.o $(LEPTLIB)
	$(CC) -o projectivetest projectivetest.o $(ALL_LIBS) $(EXTRALIBS)

ranktest:	ranktest.o $(LEPTLIB)
	$(CC) -o ranktest ranktest.o $(ALL_LIBS) $(EXTRALIBS)

reducetest:	reducetest.o $(LEPTLIB)
	$(CC) -o reducetest reducetest.o $(ALL_LIBS) $(EXTRALIBS)

removecmap:	removecmap.o $(LEPTLIB)
	$(CC) -o removecmap removecmap.o $(ALL_LIBS) $(EXTRALIBS)

renderfonts:	renderfonts.o $(LEPTLIB)
	$(CC) -o renderfonts renderfonts.o $(ALL_LIBS) $(EXTRALIBS)

rotatefastalt:	rotatefastalt.o $(LEPTLIB)
	$(CC) -o rotatefastalt rotatefastalt.o $(ALL_LIBS) $(EXTRALIBS)

rotateorthtest1:	rotateorthtest1.o $(LEPTLIB)
	$(CC) -o rotateorthtest1 rotateorthtest1.o $(ALL_LIBS) $(EXTRALIBS)

rotatetest1:	rotatetest1.o $(LEPTLIB)
	$(CC) -o rotatetest1 rotatetest1.o $(ALL_LIBS) $(EXTRALIBS)

runlengthtest:	runlengthtest.o $(LEPTLIB)
	$(CC) -o runlengthtest runlengthtest.o $(ALL_LIBS) $(EXTRALIBS)

scaletest1:	scaletest1.o $(LEPTLIB)
	$(CC) -o scaletest1 scaletest1.o $(ALL_LIBS) $(EXTRALIBS)

scaletest2:	scaletest2.o $(LEPTLIB)
	$(CC) -o scaletest2 scaletest2.o $(ALL_LIBS) $(EXTRALIBS)

seedfilltest:	seedfilltest.o $(LEPTLIB)
	$(CC) -o seedfilltest seedfilltest.o $(ALL_LIBS) $(EXTRALIBS)

sharptest:	sharptest.o $(LEPTLIB)
	$(CC) -o sharptest sharptest.o $(ALL_LIBS) $(EXTRALIBS)

sheartest:	sheartest.o $(LEPTLIB)
	$(CC) -o sheartest sheartest.o $(ALL_LIBS) $(EXTRALIBS)

showedges:	showedges.o $(LEPTLIB)
	$(CC) -o showedges showedges.o $(ALL_LIBS) $(EXTRALIBS)

skewtest:	skewtest.o $(LEPTLIB)
	$(CC) -o skewtest skewtest.o $(ALL_LIBS) $(EXTRALIBS)

snapcolortest:	snapcolortest.o $(LEPTLIB)
	$(CC) -o snapcolortest snapcolortest.o $(ALL_LIBS) $(EXTRALIBS)

sorttest:	sorttest.o $(LEPTLIB)
	$(CC) -o sorttest sorttest.o $(ALL_LIBS) $(EXTRALIBS)

splitimage2pdf:	splitimage2pdf.o $(LEPTLIB)
	$(CC) -o splitimage2pdf splitimage2pdf.o $(ALL_LIBS) $(EXTRALIBS)

textlinemask:	textlinemask.o $(LEPTLIB)
	$(CC) -o textlinemask textlinemask.o $(ALL_LIBS) $(EXTRALIBS)

viewertest:	viewertest.o $(LEPTLIB)
	$(CC) -o viewertest viewertest.o $(ALL_LIBS) $(EXTRALIBS)

wordsinorder:	wordsinorder.o $(LEPTLIB)
	$(CC) -o wordsinorder wordsinorder.o $(ALL_LIBS) $(EXTRALIBS)

xvdisp:		xvdisp.o $(LEPTLIB)
	$(CC) -o xvdisp xvdisp.o $(ALL_LIBS) $(EXTRALIBS)

###########################################################

clean:
	-@ for file in $(SRC:%.c=%) ; do \
		rm -f $$file; \
	done ;
	-@ for file in $(SRC:%.c=%.o) ; do \
		rm -f $$file; \
	done ;
	-@ for file in $(SRC2:%.c=%) ; do \
		rm -f $$file; \
	done ;
	-@ for file in $(SRC2:%.c=%.o) ; do \
		rm -f $$file; \
	done ;
	-@ for file in $(SRC3) ; do \
		rm -f $$file; \
	done ;
	-@ for file in $(SRC3:%.c=%.o) ; do \
		rm -f $$file; \
	done ;

###########################################################
 
install:
	mkdir -p $(DESTDIR)/usr/bin/leptonica
	-@ for file in $(SRC:%.c=%) ; do \
		install $$file $(DESTDIR)/usr/bin/leptonica; \
	done ;

###########################################################

depend:
	$(BIN)/makedepend -DNO_PROTOS $(CPPFLAGS) $(SRC)

dependprotos:
	$(BIN)/makedepend $(CPPFLAGS) $(SRC)

# DO NOT DELETE THIS LINE -- make depend depends on it.
