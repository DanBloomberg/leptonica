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
#   To link and run programs using shared (dynamic linked) libraries,
#   you must do one of the following two things:
#
#     (a)  make sure your LD_LIBRARY_PATH variable points to the
#          directory in which the *.so files are placed, or
#     (b)  have the *.so files (or links to them) in this directory
#
#   On a bigendian machine (e.g., Mac), use
#      CPPFLAGS =	$(INCLUDES) -DL_BIG_ENDIAN
#
#   On cygwin, remove -fPIC from CC and remove the following targets:
#      alljpeg2ps.c      alltiff2ps.c
#      jbcorrelation.c   jbrankhaus.c
#      maketile.c        viewertest.c


# Libraries are built into a binary tree with its root in the
# parent directory
ROOT_DIR = ..

LIB_NODEBUG_DIR =	$(ROOT_DIR)/lib/nodebug
LIB_DEBUG_DIR =		$(ROOT_DIR)/lib/debug
LIB_SHARED_DIR =	$(ROOT_DIR)/lib/shared

# Include files are found within the same tree
IMAGELIB_INCL =	$(ROOT_DIR)/src


CC =		gcc -ansi -Werror -D_BSD_SOURCE -DANSI -fPIC
#CC =            g++ -Werror -D_BSD_SOURCE -fPIC


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
CPPFLAGS =      $(INCLUDES) -DL_LITTLE_ENDIAN
#CPPFLAGS =      $(INCLUDES) -DL_BIG_ENDIAN

LDFLAGS +=	-L$(LIB_DIR) -L/usr/X11R6/lib -L/usr/lib $(EXTRALIBS)
#LDFLAGS +=	-L$(LIB_DIR) -L/usr/X11R6/lib -L/usr/local/lib -L/usr/lib $(EXTRALIBS)

ifdef  SHARED
    LEPTLIB =		$(LIB_DIR)/liblept.so
else
    LEPTLIB =		$(LIB_DIR)/liblept.a
endif

ALL_LIBS =	$(LEPTLIB) -ltiff -ljpeg -lpng -lz -lm

#########################################################################

SRC =		adaptmaptest.c affinetest.c \
		alljpeg2ps.c alltiff2ps.c \
		arithtest.c \
		baselinetest.c bilineartest.c \
		bincompare.c binmazetest.c \
		blendcmaptest.c blendtest1.c buffertest.c \
		ccbordtest.c cctest.c cmapquanttest.c \
		colormaptest.c colorquanttest.c \
		colorsegtest.c colorspacetest.c \
		contrasttest.c convertformat.c converttogray.c \
		converttops.c convolvetest.c cornertest.c \
		croptext.c distancetest.c dithertest.c \
		docseg1.c docseg2.c \
		equaltest.c expandtest.c falsecolortest.c \
		fhmtautogen.c fhmtautotest.c fileinfo.c \
		findpattern1.c findpattern2.c findpattern3.c \
		flipselgen.c fliptest.c \
		fmorphautogen.c fmorphautotest.c \
		gammatest.c genfonts.c graphicstest.c graymazetest.c \
		heaptest.c histotest.c ioformats.c iotest.c \
		jbcorrelation.c jbrankhaus.c jbwords.c \
		lineremoval.c listtest.c maketile.c \
		morphgraytest.c morphseqtest.c \
		morphtest1.c morphtest2.c morphtest3.c \
		mtifftest.c \
		numaranktest.c numatest.c painttest.c \
		pixcompare.c pixatest.c plotfile.c plotprog.c \
		printimage.c printsplitimage.c printtiff.c \
		projectivetest.c reducetest.c renderfonts.c \
		rotatefastalt.c rotateorthtest1.c rotateorthtest2.c \
		rotatetest1.c rotatetest2.c runlengthtest.c \
		scaletest1.c scaletest2.c scaletest3.c \
		seedfilltest.c \
		sharptest.c sheartest.c showedges.c \
		skewtest.c snapcolortest.c \
		sorttest.c splitimage2pdf.c \
		thresholdtest.c viewertest.c \
		xvdisp.c

######################################################################

all:	$(SRC:%.c=%)

######################################################################

adaptmaptest:	adaptmaptest.o $(LEPTLIB)
	$(CC) -o adaptmaptest adaptmaptest.o $(ALL_LIBS) $(EXTRALIBS)

affinetest:	affinetest.o $(LEPTLIB)
	$(CC) -o affinetest affinetest.o $(ALL_LIBS) $(EXTRALIBS)

alljpeg2ps:	alljpeg2ps.o $(LEPTLIB)
	$(CC) -o alljpeg2ps alljpeg2ps.o $(ALL_LIBS) $(EXTRALIBS)

alltiff2ps:	alltiff2ps.o $(LEPTLIB)
	$(CC) -o alltiff2ps alltiff2ps.o $(ALL_LIBS) $(EXTRALIBS)

arithtest:	arithtest.o $(LEPTLIB)
	$(CC) -o arithtest arithtest.o $(ALL_LIBS) $(EXTRALIBS)

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

cctest:		cctest.o $(LEPTLIB)
	$(CC) -o cctest cctest.o $(ALL_LIBS) $(EXTRALIBS)

cmapquanttest:	cmapquanttest.o $(LEPTLIB)
	$(CC) -o cmapquanttest cmapquanttest.o $(ALL_LIBS) $(EXTRALIBS)

colormaptest:	colormaptest.o $(LEPTLIB)
	$(CC) -o colormaptest colormaptest.o $(ALL_LIBS) $(EXTRALIBS)

colorquanttest:	colorquanttest.o $(LEPTLIB)
	$(CC) -o colorquanttest colorquanttest.o $(ALL_LIBS) $(EXTRALIBS)

colorsegtest:	colorsegtest.o $(LEPTLIB)
	$(CC) -o colorsegtest colorsegtest.o $(ALL_LIBS) $(EXTRALIBS)

colorspacetest:	colorspacetest.o $(LEPTLIB)
	$(CC) -o colorspacetest colorspacetest.o $(ALL_LIBS) $(EXTRALIBS)

contrasttest:	contrasttest.o $(LEPTLIB)
	$(CC) -o contrasttest contrasttest.o $(ALL_LIBS) $(EXTRALIBS)

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

distancetest:	distancetest.o $(LEPTLIB)
	$(CC) -o distancetest distancetest.o $(ALL_LIBS) $(EXTRALIBS)

dithertest:	dithertest.o $(LEPTLIB)
	$(CC) -o dithertest dithertest.o $(ALL_LIBS) $(EXTRALIBS)

docseg1:	docseg1.o $(LEPTLIB)
	$(CC) -o docseg1 docseg1.o $(ALL_LIBS) $(EXTRALIBS)

docseg2:	docseg2.o $(LEPTLIB)
	$(CC) -o docseg2 docseg2.o $(ALL_LIBS) $(EXTRALIBS)

equaltest:	equaltest.o $(LEPTLIB)
	$(CC) -o equaltest equaltest.o $(ALL_LIBS) $(EXTRALIBS)

expandtest:	expandtest.o $(LEPTLIB)
	$(CC) -o expandtest expandtest.o $(ALL_LIBS) $(EXTRALIBS)

falsecolortest:	falsecolortest.o $(LEPTLIB)
	$(CC) -o falsecolortest falsecolortest.o $(ALL_LIBS) $(EXTRALIBS)

fhmtautogen:	fhmtautogen.o $(LEPTLIB)
	$(CC) -o fhmtautogen fhmtautogen.o $(ALL_LIBS) $(EXTRALIBS)

fhmtautotest:	fhmtautotest.o $(LEPTLIB)
	$(CC) -o fhmtautotest fhmtautotest.o $(ALL_LIBS) $(EXTRALIBS)

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

fliptest:	fliptest.o $(LEPTLIB)
	$(CC) -o fliptest fliptest.o $(ALL_LIBS) $(EXTRALIBS)

fmorphautogen:	fmorphautogen.o $(LEPTLIB)
	$(CC) -o fmorphautogen fmorphautogen.o $(ALL_LIBS) $(EXTRALIBS)

fmorphautotest:	fmorphautotest.o $(LEPTLIB)
	$(CC) -o fmorphautotest fmorphautotest.o $(ALL_LIBS) $(EXTRALIBS)

gammatest:	gammatest.o $(LEPTLIB)
	$(CC) -o gammatest gammatest.o $(ALL_LIBS) $(EXTRALIBS)

genfonts:	genfonts.o $(LEPTLIB)
	$(CC) -o genfonts genfonts.o $(ALL_LIBS) $(EXTRALIBS)

graphicstest:	graphicstest.o $(LEPTLIB)
	$(CC) -o graphicstest graphicstest.o $(ALL_LIBS) $(EXTRALIBS)

graymazetest:	graymazetest.o $(LEPTLIB)
	$(CC) -o graymazetest graymazetest.o $(ALL_LIBS) $(EXTRALIBS)

heaptest:	heaptest.o $(LEPTLIB)
	$(CC) -o heaptest heaptest.o $(ALL_LIBS) $(EXTRALIBS)

histotest:	histotest.o $(LEPTLIB)
	$(CC) -o histotest histotest.o $(ALL_LIBS) $(EXTRALIBS)

ioformats:	ioformats.o $(LEPTLIB)
	$(CC) -o ioformats ioformats.o $(ALL_LIBS) $(EXTRALIBS)

iotest:		iotest.o $(LEPTLIB)
	$(CC) -o iotest iotest.o $(ALL_LIBS) $(EXTRALIBS)

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

morphgraytest:	morphgraytest.o $(LEPTLIB)
	$(CC) -o morphgraytest morphgraytest.o $(ALL_LIBS) $(EXTRALIBS)

morphseqtest:	morphseqtest.o $(LEPTLIB)
	$(CC) -o morphseqtest morphseqtest.o $(ALL_LIBS) $(EXTRALIBS)

morphtest1:	morphtest1.o $(LEPTLIB)
	$(CC) -o morphtest1 morphtest1.o $(ALL_LIBS) $(EXTRALIBS)

morphtest2:	morphtest2.o $(LEPTLIB)
	$(CC) -o morphtest2 morphtest2.o $(ALL_LIBS) $(EXTRALIBS)

morphtest3:	morphtest3.o $(LEPTLIB)
	$(CC) -o morphtest3 morphtest3.o $(ALL_LIBS) $(EXTRALIBS)

mtifftest:	mtifftest.o $(LEPTLIB)
	$(CC) -o mtifftest mtifftest.o $(ALL_LIBS) $(EXTRALIBS)

numaranktest:	numaranktest.o $(LEPTLIB)
	$(CC) -o numaranktest numaranktest.o $(ALL_LIBS) $(EXTRALIBS)

numatest:	numatest.o $(LEPTLIB)
	$(CC) -o numatest numatest.o $(ALL_LIBS) $(EXTRALIBS)

painttest:	painttest.o $(LEPTLIB)
	$(CC) -o painttest painttest.o $(ALL_LIBS) $(EXTRALIBS)

pixatest:	pixatest.o $(LEPTLIB)
	$(CC) -o pixatest pixatest.o $(ALL_LIBS) $(EXTRALIBS)

pixcompare:	pixcompare.o $(LEPTLIB)
	$(CC) -o pixcompare pixcompare.o $(ALL_LIBS) $(EXTRALIBS)

plotfile:	plotfile.o $(LEPTLIB)
	$(CC) -o plotfile plotfile.o $(ALL_LIBS) $(EXTRALIBS)

plotprog:	plotprog.o $(LEPTLIB)
	$(CC) -o plotprog plotprog.o $(ALL_LIBS) $(EXTRALIBS)

printimage:	printimage.o $(LEPTLIB)
	$(CC) -o printimage printimage.o $(ALL_LIBS) $(EXTRALIBS)

printsplitimage:	printsplitimage.o $(LEPTLIB)
	$(CC) -o printsplitimage printsplitimage.o $(ALL_LIBS) $(EXTRALIBS)

printtiff:	printtiff.o $(LEPTLIB)
	$(CC) -o printtiff printtiff.o $(ALL_LIBS) $(EXTRALIBS)

projectivetest:	projectivetest.o $(LEPTLIB)
	$(CC) -o projectivetest projectivetest.o $(ALL_LIBS) $(EXTRALIBS)

reducetest:	reducetest.o $(LEPTLIB)
	$(CC) -o reducetest reducetest.o $(ALL_LIBS) $(EXTRALIBS)

renderfonts:	renderfonts.o $(LEPTLIB)
	$(CC) -o renderfonts renderfonts.o $(ALL_LIBS) $(EXTRALIBS)

rotatefastalt:	rotatefastalt.o $(LEPTLIB)
	$(CC) -o rotatefastalt rotatefastalt.o $(ALL_LIBS) $(EXTRALIBS)

rotateorthtest1:	rotateorthtest1.o $(LEPTLIB)
	$(CC) -o rotateorthtest1 rotateorthtest1.o $(ALL_LIBS) $(EXTRALIBS)

rotateorthtest2:	rotateorthtest2.o $(LEPTLIB)
	$(CC) -o rotateorthtest2 rotateorthtest2.o $(ALL_LIBS) $(EXTRALIBS)

rotatetest1:	rotatetest1.o $(LEPTLIB)
	$(CC) -o rotatetest1 rotatetest1.o $(ALL_LIBS) $(EXTRALIBS)

rotatetest2:	rotatetest2.o $(LEPTLIB)
	$(CC) -o rotatetest2 rotatetest2.o $(ALL_LIBS) $(EXTRALIBS)

runlengthtest:	runlengthtest.o $(LEPTLIB)
	$(CC) -o runlengthtest runlengthtest.o $(ALL_LIBS) $(EXTRALIBS)

scaletest1:	scaletest1.o $(LEPTLIB)
	$(CC) -o scaletest1 scaletest1.o $(ALL_LIBS) $(EXTRALIBS)

scaletest2:	scaletest2.o $(LEPTLIB)
	$(CC) -o scaletest2 scaletest2.o $(ALL_LIBS) $(EXTRALIBS)

scaletest3:	scaletest3.o $(LEPTLIB)
	$(CC) -o scaletest3 scaletest3.o $(ALL_LIBS) $(EXTRALIBS)

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

thresholdtest:	thresholdtest.o $(LEPTLIB)
	$(CC) -o thresholdtest thresholdtest.o $(ALL_LIBS) $(EXTRALIBS)

viewertest:	viewertest.o $(LEPTLIB)
	$(CC) -o viewertest viewertest.o $(ALL_LIBS) $(EXTRALIBS)

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

###########################################################

depend:
	$(BIN)/makedepend -DNO_PROTOS $(CPPFLAGS) $(SRC)

dependprotos:
	$(BIN)/makedepend $(CPPFLAGS) $(SRC)

# DO NOT DELETE THIS LINE -- make depend depends on it.
