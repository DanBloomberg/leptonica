/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/

#ifndef  PIX_H_INCLUDED
#define  PIX_H_INCLUDED

/*
 *   pix.h
 *
 *   Contains the following structures:
 *       struct Pix
 *       struct PixColormap
 *       struct RGBA_Quad
 *       struct Pixa
 *       struct Pixaa
 *       struct Box
 *       struct Boxa
 *       struct Boxaa
 *       struct Pta
 *       struct Ptaa
 *       struct Pixacc
 *
 *   Contains definitions for:
 *       colors for RGB
 *       colormap conversion flags
 *       rasterop bit flags
 *       structure access flags (for insert, copy, clone, copy-clone)
 *       sorting flags (by type and direction)
 *       blending flags
 *       graphics pixel setting flags
 *       size filtering flags
 *       rotation pixel flags
 *       dithering flags
 *       distance flags
 */


/*-------------------------------------------------------------------------*
 *                              Basic Pix                                  *
 *-------------------------------------------------------------------------*/
struct Pix
{
    l_uint32             w;           /* width in pixels                   */
    l_uint32             h;           /* height in pixels                  */
    l_uint32             d;           /* depth in bits                     */
    l_uint32             wpl;         /* 32-bit words/line                 */
    l_uint32             refcount;    /* reference count (1 if no clones)  */
    l_uint32             xres;        /* image res (ppi) in x direction    */
				      /* (use 0 if unknown)                */
    l_uint32             yres;        /* image res (ppi) in y direction    */
				      /* (use 0 if unknown)                */
    l_int32              informat;    /* input file format, IFF_*          */
    char                *text;        /* text string associated with pix   */
    struct PixColormap  *colormap;    /* colormap (may be null)            */
    l_uint32            *data;        /* the image data                    */
};
typedef struct Pix PIX;


struct PixColormap
{
        void        *array;     /* colormap table (array of RGBA_QUAD)     */
	l_int32      depth;     /* of pix (1, 2, 4 or 8 bpp)               */
	l_int32      nalloc;    /* number of color entries allocated       */
	l_int32      n;         /* number of color entries used            */
};
typedef struct PixColormap  PIXCMAP;


    /* Colormap table entry (after the BMP version).
     * Note that the BMP format stores the colormap table exactly
     * as it appears here, with color samples being stored sequentially,
     * in the order (b,g,r,a). */
struct RGBA_Quad
{
    l_uint8     blue;
    l_uint8     green;
    l_uint8     red;
    l_uint8     reserved;
};
typedef struct RGBA_Quad  RGBA_QUAD;



/*-------------------------------------------------------------------------*
 *                             Colors for 32 bpp                           *
 *-------------------------------------------------------------------------*/
/* Note: colors are used in 32 bpp images.  The 4th byte, typically
 *       known as the "alpha channel", can be used for blending.
 *       It's not explicitly used in leptonica.  */
enum {
    COLOR_RED = 0,
    COLOR_GREEN = 1,
    COLOR_BLUE = 2,
    L_ALPHA_CHANNEL = 3
};


/*-------------------------------------------------------------------------*
 *                        Flags for colormap conversion                    *
 *-------------------------------------------------------------------------*/
enum {
    REMOVE_CMAP_TO_BINARY = 0,
    REMOVE_CMAP_TO_GRAYSCALE = 1,
    REMOVE_CMAP_TO_FULL_COLOR = 2,
    REMOVE_CMAP_BASED_ON_SRC = 3
};


/*-------------------------------------------------------------------------*
 *
 * The following operation bit flags have been modified from
 * Sun's pixrect.h.
 *
 * The 'op' in 'rasterop' is represented by an integer
 * composed with Boolean functions using the set of five integers
 * given below.  The integers, and the op codes resulting from
 * boolean expressions on them, need only be in the range from 0 to 15.
 * The function is applied on a per-pixel basis.
 *
 * Examples: the op code representing ORing the src and dest
 * is computed using the bit OR, as PIX_SRC | PIX_DST;  the op
 * code representing XORing src and dest is found from
 * PIX_SRC ^ PIX_DST;  the op code representing ANDing src and dest
 * is found from PIX_SRC & PIX_DST.  Note that
 * PIX_NOT(PIX_CLR) = PIX_SET, and v.v., as they must be.
 *
 * We would like to use the following set of definitions:
 *
 *      #define   PIX_SRC      0xc
 *      #define   PIX_DST      0xa
 *      #define   PIX_NOT(op)  ((op) ^ 0xf)
 *      #define   PIX_CLR      0x0
 *      #define   PIX_SET      0xf
 *
 * Now, these definitions differ from Sun's, in that Sun
 * left-shifted each value by 1 pixel, and used the least
 * significant bit as a flag for the "pseudo-operation" of
 * clipping.  We don't need this bit, because it is both
 * efficient and safe ALWAYS to clip the rectangles to the src
 * and dest images, which is what we do.  See the notes in rop.h
 * on the general choice of these bit flags.
 *
 * However, if you include Sun's xview package, you will get their
 * definitions, and because I like using these flags, we will
 * adopt the original Sun definitions to avoid redefinition conflicts.
 *
 * Then we have, for reference, the following 16 unique op flags:
 *
 *      PIX_CLR                           00000             0x0
 *      PIX_SET                           11110             0x1e
 *      PIX_SRC                           11000             0x18
 *      PIX_DST                           10100             0x14
 *      PIX_NOT(PIX_SRC)                  00110             0x06
 *      PIX_NOT(PIX_DST)                  01010             0x0a
 *      PIX_SRC | PIX_DST                 11100             0x1c
 *      PIX_SRC & PIX_DST                 10000             0x10
 *      PIX_SRC ^ PIX_DST                 01100             0x0c
 *      PIX_NOT(PIX_SRC) | PIX_DST        10110             0x16
 *      PIX_NOT(PIX_SRC) & PIX_DST        00100             0x04
 *      PIX_SRC | PIX_NOT(PIX_DST)        11010             0x1a
 *      PIX_SRC & PIX_NOT(PIX_DST)        01000             0x08
 *      PIX_NOT(PIX_SRC | PIX_DST)        00010             0x02
 *      PIX_NOT(PIX_SRC & PIX_DST)        01110             0x0e
 *      PIX_NOT(PIX_SRC ^ PIX_DST)        10010             0x12
 *
 *-------------------------------------------------------------------------*/
#define   PIX_SRC      (0xc << 1)
#define   PIX_DST      (0xa << 1)
#define   PIX_NOT(op)  ((op) ^ 0x1e)
#define   PIX_CLR      (0x0 << 1)
#define   PIX_SET      (0xf << 1)

#define   PIX_PAINT    (PIX_SRC | PIX_DST)
#define   PIX_MASK     (PIX_SRC & PIX_DST)
#define   PIX_XOR      (PIX_SRC ^ PIX_DST)



/*-------------------------------------------------------------------------*
 *
 *   Important Notes:
 *
 *       (1) The image data is stored in a single contiguous
 *           array of l_uint32, into which the pixels are packed.
 *           By "packed" we mean that there are no unused bits
 *           between pixels, except for end-of-line padding to
 *           satisfy item (2) below.
 *
 *       (2) Every image raster line begins on a 32-bit word
 *           boundary within this array.
 *
 *       (3) Pix image data is stored in 32-bit units, with the
 *           pixels ordered from left to right in the image being
 *           stored in order from the MSB to LSB within the word,
 *           for both big-endian and little-endian machines.
 *           This is the natural ordering for big-endian machines,
 *           as successive bytes are stored and fetched progressively
 *           to the right.  However, for little-endians, when storing
 *           we re-order the bytes from this byte stream order, and
 *           reshuffle again for byte access on 32-bit entities.
 *           So if the bytes come in sequence from left to right, we
 *           store them on little-endians in byte order: 
 *                3 2 1 0 7 6 5 4 ...
 *           This MSB to LSB ordering allows left and right shift
 *           operations on 32 bit words to move the pixels properly.
 *
 *       (4) For 24-bit color images, use 32 bpp data, leaving
 *           the fourth byte unused.  Within each 4 byte pixel, the
 *           colors are ordered from MSB to LSB, as follows:
 *
 *                |  MSB  |  2nd MSB  |  3rd MSB  |  LSB  |
 *                   red      green       blue      unused
 *                    0         1           2         3   (big-endian)
 *                    3         2           1         0   (little-endian)
 *
 *           Because we use MSB to LSB ordering within the 32-bit word,
 *           the individual 8-bit samples can be accessed with 
 *           GET_DATA_BYTE and SET_DATA_BYTE macros, using the
 *           (implicitly big-ending) ordering
 *                 red:    byte 0  (MSB)
 *                 green:  byte 1  (2nd MSB)
 *                 blue:   byte 2  (3rd MSB)
 *
 *           For simplicity and consistency, we have defined (COLOR_RED,
 *           COLOR_GREEN, COLOR_BLUE) so that we can retrieve red, green
 *           and blue sample values using
 *              redval = GET_DATA_BYTE(&pixel, COLOR_RED);
 *              greenval = GET_DATA_BYTE(&pixel, COLOR_GREEN);
 *              blueval = GET_DATA_BYTE(&pixel, COLOR_BLUE);
 *           and we can set them in the pixel using
 *              SET_DATA_BYTE(&pixel, COLOR_RED, redval);
 *              SET_DATA_BYTE(&pixel, COLOR_GREEN, greenval);
 *              SET_DATA_BYTE(&pixel, COLOR_BLUE, blueval);
 *
 *           In some functions, for extra speed we extract the R, G and B
 *           colors directly by shifting and masking, implicitly using the
 *           specific values in COLOR_RED, COLOR_GREEN and COLOR_BLUE.
 *           For example:
 *                   pixel32 >> 24;              (red)
 *                   (pixel32 >> 16) & 0xff;     (green)
 *                   (pixel32 >> 8) & 0xff;      (blue)
 *           These operations work properly on both big- and little-endians.
 *           Functions where this is done are marked with ***.
 *
 *       (5) A reference count is held within each pix, giving the
 *           number of ptrs to the pix.  When a pixClone() call
 *           is made, the ref count is increased by 1, and
 *           when a pixDestroy() call is made, the reference count
 *           of the pix is decremented.  The pix is only destroyed
 *           when the reference count goes to zero.
 *
 *-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------*
 *                              Array of pix                               *
 *-------------------------------------------------------------------------*/
struct Pixa
{
    l_int32             n;            /* number of Pix in ptr array        */
    l_int32             nalloc;       /* number of Pix ptrs allocated      */
    l_uint32            refcount;     /* reference count (1 if no clones)  */
    struct Pix        **pix;          /* the array of ptrs to pix          */
    struct Boxa        *boxa;         /* array of boxes                    */
};
typedef struct Pixa PIXA;


struct Pixaa
{
    l_int32             n;            /* number of Pixa in ptr array       */
    l_int32             nalloc;       /* number of Pixa ptrs allocated     */
    struct Pixa       **pixa;         /* array of ptrs to pixa             */
};
typedef struct Pixaa PIXAA;



/*-------------------------------------------------------------------------*
 *                    Basic rectangle and rectangle arrays                 *
 *-------------------------------------------------------------------------*/
struct Box
{
    l_int32            x;
    l_int32            y;
    l_int32            w;
    l_int32            h;
    l_uint32           refcount;      /* reference count (1 if no clones)  */

};
typedef struct Box    BOX;

struct Boxa
{
    l_int32            n;             /* number of box in ptr array        */
    l_int32            nalloc;        /* number of box ptrs allocated      */
    l_uint32           refcount;      /* reference count (1 if no clones)  */
    struct Box       **box;           /* box ptr array                     */
};
typedef struct Boxa  BOXA;

struct Boxaa
{
    l_int32            n;             /* number of boxa in ptr array       */
    l_int32            nalloc;        /* number of boxa ptrs allocated     */
    struct Boxa      **boxa;          /* boxa ptr array                    */
};
typedef struct Boxaa  BOXAA;


/*-------------------------------------------------------------------------*
 *                               Array of points                           *
 *-------------------------------------------------------------------------*/
struct Pta
{
    l_int32            n;             /* actual number of pts              */
    l_int32            nalloc;        /* size of allocated arrays          */
    l_int32            refcount;      /* reference count (1 if no clones)  */
    l_float32         *x, *y;         /* arrays of floats                  */
};
typedef struct Pta PTA;


/*-------------------------------------------------------------------------*
 *                              Array of Pta                               *
 *-------------------------------------------------------------------------*/
struct Ptaa
{
    l_int32              n;           /* number of pta in ptr array        */
    l_int32              nalloc;      /* number of pta ptrs allocated      */
    struct Pta         **pta;         /* pta ptr array                     */
};
typedef struct Ptaa PTAA;


/*-------------------------------------------------------------------------*
 *                       Pix accumulator container                         *
 *-------------------------------------------------------------------------*/
struct Pixacc
{
    l_int32             w;            /* array width                       */
    l_int32             h;            /* array height                      */
    l_int32             offset;       /* used to allow negative            */
                                      /* intermediate results              */
    struct Pix         *pix;          /* the 32 bit accumulator pix        */
};
typedef struct Pixacc PIXACC;


/*-------------------------------------------------------------------------*
 *                         Access and storage flags                        *
 *-------------------------------------------------------------------------*/
/*
 *  For Pix, Box, Pta and Numa, there are 3 standard methods for handling
 *  the retrieval or insertion of a struct:
 *     (1) direct insertion (Don't do this if there is another handle
 *                           somewhere to this same struct!)
 *     (2) copy (Always safe, sets up a refcount of 1 on the new object.
 *               Can be undesirable if very large, such as an image or
 *               an array of images.)
 *     (3) clone (Makes another handle to the same struct, and bumps the
 *                refcount up by 1.  Safe to do unless you're changing
 *                data through one of the handles but don't want those
 *                changes to be seen by the other handle.)
 *
 *  For Pixa and Boxa, which are structs that hold an array of clonable
 *  structs, there is an additional method:
 *     (4) copy-clone (Makes a new higher-level struct with a refcount
 *                     of 1, but clones all the structs in the array.)
 */
enum {
    L_INSERT = 0,     /* stuff it in; no copy, clone or copy-clone    */
    L_COPY = 1,       /* make/use a copy of the object                */
    L_CLONE = 2,      /* make/use clone (ref count) of the object     */
    L_COPY_CLONE = 3  /* make a new object and fill with with clones  */
	              /* of each object in the array(s)               */
};


/*-------------------------------------------------------------------------*
 *                              Sort flags                                 *
 *-------------------------------------------------------------------------*/
enum {
    L_SORT_INCREASING = 1,        /* sort in increasing order              */
    L_SORT_DECREASING = 2         /* sort in decreasing order              */
};

enum {
    L_SORT_BY_X = 3,              /* sort box or c.c. by horiz location    */
    L_SORT_BY_Y = 4,              /* sort box or c.c. by vert location     */
    L_SORT_BY_WIDTH = 5,          /* sort box or c.c. by width             */
    L_SORT_BY_HEIGHT = 6,         /* sort box or c.c. by height            */
    L_SORT_BY_MIN_DIMENSION = 7,  /* sort box or c.c. by min dimension     */
    L_SORT_BY_MAX_DIMENSION = 8,  /* sort box or c.c. by max dimension     */
    L_SORT_BY_PERIMETER = 9,      /* sort box or c.c. by perimeter         */
    L_SORT_BY_AREA = 10           /* sort box or c.c. by area              */
};


/*-------------------------------------------------------------------------*
 *                             Blend flags                                 *
 *-------------------------------------------------------------------------*/
enum {
    L_BLEND_WITH_INVERSE = 1,     /* add some of src inverse to itself     */
    L_BLEND_TO_WHITE = 2,         /* shift src colors towards white        */
    L_BLEND_TO_BLACK = 3,         /* shift src colors towards black        */
    L_BLEND_GRAY = 4,             /* blend src directly with blender       */
    L_BLEND_GRAY_WITH_INVERSE = 5 /* add amount of src inverse to itself,  */
                                  /* based on blender pix value            */
};

enum {
    L_PAINT_LIGHT = 1,            /* colorize non-black pixels             */
    L_PAINT_DARK = 2              /* colorize non-white pixels             */
};


/*-------------------------------------------------------------------------*
 *                        Graphics pixel setting                           *
 *-------------------------------------------------------------------------*/
enum {
    L_SET_PIXELS = 1,             /* set all bits in each pixel to 1       */
    L_CLEAR_PIXELS = 2,           /* set all bits in each pixel to 0       */
    L_FLIP_PIXELS = 3             /* flip all bits in each pixel           */
};


/*-------------------------------------------------------------------------*
 *                           Size filter flags                             *
 *-------------------------------------------------------------------------*/
enum {
    L_REMOVE_IF_EITHER = 1,       /* remove component if either constraint */
                                  /* is not met                            */
    L_REMOVE_IF_BOTH = 2          /* remove component only if both         */
                                  /* constraints are not met               */
};


/*-------------------------------------------------------------------------*
 *                        Rotate and shear flags                           *
 *-------------------------------------------------------------------------*/
enum {
    L_ROTATE_AREA_MAP = 1,       /* use area map rotation, if possible     */
    L_ROTATE_SHEAR = 2           /* use shear rotation                     */
};

enum {
    L_BRING_IN_WHITE = 1,        /* bring in white pixels from the outside */
    L_BRING_IN_BLACK = 2         /* bring in black pixels from the outside */
};


/*-------------------------------------------------------------------------*
 *                           Dither parameters                             *
 *         If within this grayscale distance from black or white,          *
 *         do not propagate excess or deficit to neighboring pixels.       *
 *-------------------------------------------------------------------------*/
enum {
    DEFAULT_CLIP_LOWER_1 = 10,   /* dist to black with no prop; 1 bpp      */
    DEFAULT_CLIP_UPPER_1 = 10,   /* dist to black with no prop; 1 bpp      */
    DEFAULT_CLIP_LOWER_2 = 5,    /* dist to black with no prop; 2 bpp      */
    DEFAULT_CLIP_UPPER_2 = 5     /* dist to black with no prop; 2 bpp      */
};


/*-------------------------------------------------------------------------*
 *                             Distance flags                              *
 *-------------------------------------------------------------------------*/
enum {
    L_MANHATTAN_DISTANCE = 1,    /* L1 distance (e.g., in color space)     */
    L_EUCLIDEAN_DISTANCE = 2     /* L2 distance                            */
};


#endif  /* PIX_H_INCLUDED */

