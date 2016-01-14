SUBDIRS =

# The endianness is computed before compilation by the Makefile
# in the root directory.  To do this, it compiles and runs a
# small testing program.
MAKE = make -f Makefile
AM_CFLAGS = -D_BSD_SOURCE -DANSI -fPIC
AM_CPPFLAGS = $(ENDIANNESS)
CFLAGS = -O2

lib_LIBRARIES = liblept.a
liblept_a_SOURCES = adaptmap.c affine.c \
 arithlow.c arrayaccess.c \
 bardecode.c \
 baseline.c bbuffer.c \
 bilinear.c binexpand.c \
 binexpandlow.c binreduce.c \
 binreducelow.c blend.c bmf.c \
 bmpio.c bmpiostub.c boxbasic.c \
 boxfunc.c ccbord.c ccthin.c \
 classapp.c colorcontent.c \
 colormap.c colormorph.c \
 colorquant1.c colorquant2.c \
 colorseg.c \
 compare.c conncomp.c \
 convolve.c convolvelow.c \
 correlscore.c dwacomb.2.c \
 dwacomblow.2.c edge.c \
 endiantest.c \
 enhance.c fhmtauto.c \
 fhmtgen.1.c fhmtgenlow.1.c \
 flipdetect.c fliphmtgen.c \
 fmorphauto.c fmorphgen.1.c \
 fmorphgenlow.1.c fpix1.c fpix2.c \
 gifio.c gifiostub.c \
 gplot.c graphics.c graymorph.c \
 graymorphlow.c grayquant.c \
 grayquantlow.c jbclass.c \
 jpegio.c jpegiostub.c \
 kernel.c list.c \
 maze.c morphapp.c \
 morph.c morphdwa.c \
 morphseq.c numabasic.c \
 numafunc1.c numafunc2.c pageseg.c \
 paintcmap.c parseprotos.c \
 partition.c pheap.c pix1.c \
 pix2.c pix3.c pix4.c \
 pixabasic.c pixacc.c \
 pixafunc.c pixarith.c \
 pixconv.c pixtiling.c \
 pngio.c pngiostub.c \
 pnmio.c pnmiostub.c pqueue.c \
 projective.c psio.c psiostub.c pts.c \
 rank.c readbarcode.c readfile.c \
 rop.c ropiplow.c \
 roplow.c rotateam.c \
 rotateamlow.c rotate.c \
 rotateorth.c rotateorthlow.c \
 rotateshear.c runlength.c \
 sarray.c scale.c \
 scalelow.c seedfill.c \
 seedfilllow.c sel1.c \
 sel2.c selgen.c \
 shear.c skew.c \
 stack.c textops.c \
 tiffio.c tiffiostub.c \
 utils.c viewfiles.c \
 writefile.c zlibmem.c zlibmemstub.c

include_HEADERS = allheaders.h alltypes.h arrayaccess.h \
 array.h bbuffer.h bmf.h bmp.h ccbord.h environ.h \
 gplot.h imageio.h jbclass.h jmorecfg.h jpeglib.h leptprotos.h list.h \
 morph.h pheap.h pix.h pqueue.h readbarcode.h stack.h

EXTRA_DIST = Makefile.mingw arrayaccess.h.vc arrayaccess.h.vc60 \
 hmttemplate1.txt hmttemplate2.txt leptonica-license.txt \
 leptonlib.sln leptonlib.vcproj morphtemplate1.txt morphtemplate2.txt \
 xtractprotos.c

