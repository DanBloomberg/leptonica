/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

/*
 *  hash_reg.c
 *
 *  Tests hashing functions for strings and points, and the use of them with:
 *    *  sets (underlying rbtree implementation for sorting)
 *    *  hash maps (underlying dnaHash implementation for accessing)
 *
 *  For sets, it's important to use good 64-bit hashes to ensure that
 *  collisions are very rare.  With solid randomization, you expect
 *  that a collision is likely with 2^32 or more hashed entities.
 *  The probability of a collision goes as n^2, so with 10M entities,
 *  the collision probabililty is about 10^-5.
 *
 *  For the dna hashing, a faster but weaker hash function is used.
 *  The hash should do a reasonable job of randomizing the lower order
 *  bits corresponding to the prime number used with the mod function
 *  for assigning to buckets. (To the extent that those bits are not
 *  randomized, the calculation will run slower because bucket
 *  occupancy will not be random, but the result will still be exact.)
 *  Hash collisions in the key are allowed because the dna in
 *  the selected bucket stores integers into arrays (of pts or strings,
 *  for example), and not keys.  The input point or string is hashed to
 *  a bucket (a dna), which is then traversed, and each stored value
 *  (an index) is used check if the point or string is in the associated
 *  array at that location.
 *
 *  Also tests similar functions directly (without hashing the number) for dna.
 *  This will allow handling of both float64 and large integers that are
 *  accurately represented by float64.
 */

#ifdef HAVE_CONFIG_H
#include <config_auto.h>
#endif  /* HAVE_CONFIG_H */

#include "allheaders.h"

static SARRAY *BuildShortStrings(l_int32 nchars, l_int32 add_dups);
static PTA *BuildPointSet(l_int32 w, l_int32 h, l_int32 add_dups);


l_int32 main(int    argc,
             char **argv)
{
l_int32       ncolors, c1, c2, c3, s1;
L_ASET       *set;
L_DNA        *da1, *da2, *da3, *da4, *da5, *da6, *da7, *da8, *dav, *dac;
L_DNAHASH    *dahash;
GPLOT        *gplot;
NUMA         *nav, *nac;
PTA          *pta1, *pta2, *pta3;
SARRAY       *sa1, *sa2, *sa3, *sa4;
PIX          *pix1;
L_REGPARAMS  *rp;

    if (regTestSetup(argc, argv, &rp))
        return 1;

    lept_mkdir("lept/hash");

        /* Test string hashing with aset */
    sa1 = BuildShortStrings(3, 0);
    sa2 = BuildShortStrings(3, 1);
    c1 = sarrayGetCount(sa1);
    c2 = sarrayGetCount(sa2);
    regTestCompareValues(rp, 18278, c1, 0);  /* 0 */
    regTestCompareValues(rp, 20982, c2, 0);  /* 1 */
    if (rp->display) {
        lept_stderr("Set results with string hashing:\n");
        lept_stderr("  size with unique strings: %d\n", c1);
        lept_stderr("  size with dups: %d\n", c2);
    }
    startTimer();
    set = l_asetCreateFromSarray(sa2);
    s1 = l_asetSize(set);
    regTestCompareValues(rp, 18278, s1, 0);  /* 2 */
    if (rp->display) {
        lept_stderr("  time to make set: %5.3f sec\n", stopTimer());
        lept_stderr("  size of set without dups: %d\n", s1);
    }
    l_asetDestroy(&set);
    startTimer();
    sa3 = sarrayRemoveDupsByAset(sa2);
    c1 = sarrayGetCount(sa3);
    regTestCompareValues(rp, 18278, c1, 0);  /* 3 */
    if (rp->display) {
        lept_stderr("  time to remove dups: %5.3f sec\n", stopTimer());
        lept_stderr("  size without dups = %d\n", c1);
    }
    startTimer();
    sa4 = sarrayIntersectionByAset(sa1, sa2);
    c1 = sarrayGetCount(sa4);
    regTestCompareValues(rp, 18278, c1, 0);  /* 4 */
    if (rp->display) {
        lept_stderr("  time to intersect: %5.3f sec\n", stopTimer());
        lept_stderr("  intersection size = %d\n", c1);
    }
    sarrayDestroy(&sa3);
    sarrayDestroy(&sa4);

        /* Test sarray set operations with dna hash.
         * We use the same hash function as is used with aset. */
    if (rp->display) {
        lept_stderr("\nDna hash results for sarray:\n");
        lept_stderr("  size with unique strings: %d\n", sarrayGetCount(sa1));
        lept_stderr("  size with dups: %d\n", sarrayGetCount(sa2));
    }
    startTimer();
    dahash = l_dnaHashCreateFromSarray(sa2);
    s1 = l_dnaHashGetTotalCount(dahash);
    regTestCompareValues(rp, 20982, s1, 0);  /* 5 */
    if (rp->display) {
        lept_stderr("  time to make hashmap: %5.3f sec\n", stopTimer());
        lept_stderr("  entries in hashmap with dups: %d\n", s1);
    }
    l_dnaHashDestroy(&dahash);
    startTimer();
    sarrayRemoveDupsByHash(sa2, &sa3, NULL);
    c1 = sarrayGetCount(sa3);
    regTestCompareValues(rp, 18278, c1, 0);  /* 6 */
    if (rp->display) {
        lept_stderr("  time to remove dups: %5.3f sec\n", stopTimer());
        lept_stderr("  size without dups = %d\n", c1);
    }
    startTimer();
    sa4 = sarrayIntersectionByHash(sa1, sa2);
    c1 = sarrayGetCount(sa4);
    regTestCompareValues(rp, 18278, c1, 0);  /* 7 */
    if (rp->display) {
        lept_stderr("  time to intersect: %5.3f sec\n", stopTimer());
        lept_stderr("  intersection size = %d\n", c1);
    }
    sarrayDestroy(&sa3);
    sarrayDestroy(&sa4);
    sarrayDestroy(&sa1);
    sarrayDestroy(&sa2);

        /* Test point hashing with aset.
         * Enter all points within a 300 x 300 image in pta1, and include
         * 18,000 duplicates in pta2.  With this pt hashing function,
         * there are no hash collisions among any of the 400 million pixel
         * locations in a 20000 x 20000 image. */
    pta1 = BuildPointSet(300, 300, 0);
    pta2 = BuildPointSet(300, 300, 1);
    c1 = ptaGetCount(pta1);
    c2 = ptaGetCount(pta2);
    regTestCompareValues(rp, 90000, c1, 0);  /* 8 */
    regTestCompareValues(rp, 108000, c2, 0);  /* 9 */
    if (rp->display) {
        lept_stderr("\nSet results for pta:\n");
        lept_stderr("  pta1 size with unique points: %d\n", c1);
        lept_stderr("  pta2 size with dups: %d\n", c2);
    }
    startTimer();
    pta3 = ptaRemoveDupsByAset(pta2);
    c1 = ptaGetCount(pta3);
    regTestCompareValues(rp, 90000, c1, 0);  /* 10 */
    if (rp->display) {
        lept_stderr("  Time to remove dups: %5.3f sec\n", stopTimer());
        lept_stderr("  size without dups = %d\n", ptaGetCount(pta3));
    }
    ptaDestroy(&pta3);

    startTimer();
    pta3 = ptaIntersectionByAset(pta1, pta2);
    c1 = ptaGetCount(pta3);
    regTestCompareValues(rp, 90000, c1, 0);  /* 11 */
    if (rp->display) {
        lept_stderr("  Time to intersect: %5.3f sec\n", stopTimer());
        lept_stderr("  intersection size = %d\n", c1);
    }
    ptaDestroy(&pta1);
    ptaDestroy(&pta2);
    ptaDestroy(&pta3);

        /* Test pta set operations with dna hash, using the same pt hashing
         * function.  Although there are no collisions in 20K x 20K images,
         * the dna hash implementation works properly even if there are some. */
    pta1 = BuildPointSet(400, 400, 0);
    pta2 = BuildPointSet(400, 400, 1);
    c1 = ptaGetCount(pta1);
    c2 = ptaGetCount(pta2);
    regTestCompareValues(rp, 160000, c1, 0);  /* 12 */
    regTestCompareValues(rp, 192000, c2, 0);  /* 13 */
    if (rp->display) {
        lept_stderr("\nDna hash results for pta:\n");
        lept_stderr("  pta1 size with unique points: %d\n", c1);
        lept_stderr("  pta2 size with dups: %d\n", c2);
    }
    startTimer();
    ptaRemoveDupsByHash(pta2, &pta3, NULL);
    c1 = ptaGetCount(pta3);
    regTestCompareValues(rp, 160000, c1, 0);  /* 14 */
    if (rp->display) {
        lept_stderr("  Time to remove dups: %5.3f sec\n", stopTimer());
        lept_stderr("  size without dups = %d\n", c1);
    }
    ptaDestroy(&pta3);

    startTimer();
    pta3 = ptaIntersectionByHash(pta1, pta2);
    c1 = ptaGetCount(pta3);
    regTestCompareValues(rp, 160000, c1, 0);  /* 15 */
    if (rp->display) {
        lept_stderr("  Time to intersect: %5.3f sec\n", stopTimer());
        lept_stderr("  intersection size = %d\n", c1);
    }
    ptaDestroy(&pta1);
    ptaDestroy(&pta2);
    ptaDestroy(&pta3);

        /* Test dna set and histo operations using dna hash */
    da1 = l_dnaMakeSequence(0.0, 0.125, 8000);
    da2 = l_dnaMakeSequence(300.0, 0.125, 8000);
    da3 = l_dnaMakeSequence(600.0, 0.125, 8000);
    da4 = l_dnaMakeSequence(900.0, 0.125, 8000);
    da5 = l_dnaMakeSequence(1200.0, 0.125, 8000);
    l_dnaJoin(da1, da2, 0, -1);
    l_dnaJoin(da1, da3, 0, -1);
    l_dnaJoin(da1, da4, 0, -1);
    l_dnaJoin(da1, da5, 0, -1);
    l_dnaRemoveDupsByHash(da1, &da6, &dahash);
    l_dnaHashDestroy(&dahash);
    c1 = l_dnaGetCount(da1);
    c2 = l_dnaGetCount(da6);
    regTestCompareValues(rp, 40000, c1, 0);  /* 16 */
    regTestCompareValues(rp, 17600, c2, 0);  /* 17 */
    if (rp->display) {
        lept_stderr("\nDna hash results for dna:\n");
        lept_stderr("  dna size with dups = %d\n", c1);
        lept_stderr("  dna size of unique numbers = %d\n", c2);
    }
    l_dnaMakeHistoByHash(da1, &dahash, &dav, &dac);
    nav = l_dnaConvertToNuma(dav);
    nac = l_dnaConvertToNuma(dac);
    c1 = l_dnaGetCount(dac);
    regTestCompareValues(rp, 17600, c1, 0);  /* 18 */
    if (rp->display)
        lept_stderr("  dna number of histo points = %d\n", c1);

        /* Plot it twice */
    pix1 = gplotGeneralPix2(nav, nac, GPLOT_IMPULSES, "/tmp/lept/hash/histo",
                            "Histo", NULL, NULL);
    gplot = gplotSimpleXY1(nav, nac, GPLOT_IMPULSES, GPLOT_PNG,
                           "/tmp/lept/hash/histo", "Histo");
    if (rp->display) {
        pixDisplay(pix1, 700, 100);
        l_fileDisplay("/tmp/lept/hash/histo.png", 700, 600, 1.0);
    }
    pixDestroy(&pix1);
    gplotDestroy(&gplot);

    da7 = l_dnaIntersectionByHash(da2, da3);
    c1 = l_dnaGetCount(da2);
    c2 = l_dnaGetCount(da3);
    c3 = l_dnaGetCount(da7);
    regTestCompareValues(rp, 8000, c1, 0);  /* 19 */
    regTestCompareValues(rp, 8000, c2, 0);  /* 20 */
    regTestCompareValues(rp, 5600, c3, 0);  /* 21 */
    if (rp->display) {
        lept_stderr("  dna num points: da2 = %d, da3 = %d\n", c1, c2);
        lept_stderr("  dna num da2/da3 intersection points = %d\n", c3);
    }
    l_dnaDestroy(&da1);
    l_dnaDestroy(&da2);
    l_dnaDestroy(&da3);
    l_dnaDestroy(&da4);
    l_dnaDestroy(&da5);
    l_dnaDestroy(&da6);
    l_dnaDestroy(&da7);
    l_dnaDestroy(&dac);
    l_dnaDestroy(&dav);
    l_dnaHashDestroy(&dahash);
    numaDestroy(&nav);
    numaDestroy(&nac);

        /* Test pixel counting operations with hashmap and ordered map */
    pix1 = pixRead("wet-day.jpg");
    pixCountRGBColorsByHash(pix1, &c1);
    pixCountRGBColors(pix1, 1, &c2);
    regTestCompareValues(rp, 42427, c1, 0);  /* 22 */
    regTestCompareValues(rp, 42427, c2, 0);  /* 23 */
    if (rp->display) {
        lept_stderr("\nColor count using dna hash: %d\n", c1);
        lept_stderr("Color count using amap: %d\n", c2);
    }
    pixDestroy(&pix1);

        /* Test aset operations on dna. */
    da1 = l_dnaMakeSequence(0, 3, 10000);
    da2 = l_dnaMakeSequence(0, 5, 10000);
    da3 = l_dnaMakeSequence(0, 7, 10000);
    l_dnaJoin(da1, da2, 0, -1);
    l_dnaJoin(da1, da3, 0, -1);
    set = l_asetCreateFromDna(da1);
    c1 = l_dnaGetCount(da1);
    c2 = l_asetSize(set);
    regTestCompareValues(rp, 30000, c1, 0);  /* 24 */
    regTestCompareValues(rp, 25428, c2, 0);  /* 25 */
    if (rp->display) {
        lept_stderr("\nDna results using set:\n");
        lept_stderr("  da1 count: %d\n", c1);
        lept_stderr("  da1 set size: %d\n\n", c2);
    }
    l_asetDestroy(&set);

    da4 = l_dnaUnionByAset(da2, da3);
    set = l_asetCreateFromDna(da4);
    c1 = l_dnaGetCount(da4);
    c2 = l_asetSize(set);
    regTestCompareValues(rp, 18571, c1, 0);  /* 26 */
    regTestCompareValues(rp, 18571, c2, 0);  /* 27 */
    if (rp->display) {
        lept_stderr("  da4 count: %d\n", c1);
        lept_stderr("  da4 set size: %d\n\n", c2);
    }
    l_asetDestroy(&set);

    da5 = l_dnaIntersectionByAset(da1, da2);
    set = l_asetCreateFromDna(da5);
    c1 = l_dnaGetCount(da5);
    c2 = l_asetSize(set);
    regTestCompareValues(rp, 10000, c1, 0);  /* 28 */
    regTestCompareValues(rp, 10000, c2, 0);  /* 29 */
    if (rp->display) {
        lept_stderr("  da5 count: %d\n", c1);
        lept_stderr("  da5 set size: %d\n\n", c2);
    }
    l_asetDestroy(&set);

    da6 = l_dnaMakeSequence(100000, 11, 5000);
    l_dnaJoin(da6, da1, 0, -1);
    set = l_asetCreateFromDna(da6);
    c1 = l_dnaGetCount(da6);
    c2 = l_asetSize(set);
    regTestCompareValues(rp, 35000, c1, 0);  /* 30 */
    regTestCompareValues(rp, 30428, c2, 0);  /* 31 */
    if (rp->display) {
        lept_stderr("  da6 count: %d\n", c1);
        lept_stderr("  da6 set size: %d\n\n", c2);
    }
    l_asetDestroy(&set);

    da7 = l_dnaIntersectionByAset(da6, da3);
    set = l_asetCreateFromDna(da7);
    c1 = l_dnaGetCount(da7);
    c2 = l_asetSize(set);
    regTestCompareValues(rp, 10000, c1, 0);  /* 32 */
    regTestCompareValues(rp, 10000, c2, 0);  /* 33 */
    if (rp->display) {
        lept_stderr("  da7 count: %d\n", c1);
        lept_stderr("  da7 set size: %d\n\n", c2);
    }
    l_asetDestroy(&set);

    da8 = l_dnaRemoveDupsByAset(da1);
    c1 = l_dnaGetCount(da8);
    regTestCompareValues(rp, 25428, c1, 0);  /* 34 */
    if (rp->display)
        lept_stderr("  da8 count: %d\n\n", c1);
    l_dnaDestroy(&da1);
    l_dnaDestroy(&da2);
    l_dnaDestroy(&da3);
    l_dnaDestroy(&da4);
    l_dnaDestroy(&da5);
    l_dnaDestroy(&da6);
    l_dnaDestroy(&da7);
    l_dnaDestroy(&da8);

    return regTestCleanup(rp);
}


    /* Build all possible strings, up to a max of 5 roman alphabet characters */
static SARRAY *
BuildShortStrings(l_int32  nchars,  /* 3, 4 or 5 */
                  l_int32  add_dups)
{
char      buf[64];
l_int32   i, j, k, l, m;
l_uint64  hash;
SARRAY   *sa;

    sa = sarrayCreate(1000);
    for (i = 0; i < 26; i++) {
        snprintf(buf, sizeof(buf), "%c", i + 0x61);
        sarrayAddString(sa, buf, L_COPY);
        for (j = 0; j < 26; j++) {
            snprintf(buf, sizeof(buf), "%c%c", i + 0x61, j + 0x61);
            sarrayAddString(sa, buf, L_COPY);
            for (k = 0; k < 26; k++) {
                snprintf(buf, sizeof(buf), "%c%c%c", i + 0x61, j + 0x61,
                         k + 0x61);
                sarrayAddString(sa, buf, L_COPY);
                if (add_dups && k < 4)  /* add redundant strings */
                    sarrayAddString(sa, buf, L_COPY);
                if (nchars > 3) {
                    for (l = 0; l < 26; l++) {
                        snprintf(buf, sizeof(buf), "%c%c%c%c", i + 0x61,
                                 j + 0x61, k + 0x61, l + 0x61);
                        sarrayAddString(sa, buf, L_COPY);
                        if (add_dups && l < 4)  /* add redundant strings */
                            sarrayAddString(sa, buf, L_COPY);
                        if (nchars > 4) {
                            for (m = 0; m < 26; m++) {
                                snprintf(buf, sizeof(buf), "%c%c%c%c%c",
                                         i + 0x61, j + 0x61, k + 0x61,
                                         l + 0x61, m + 0x61);
                                sarrayAddString(sa, buf, L_COPY);
                                if (!add_dups && i == 17 && j == 12 &&
                                    k == 4 && l == 21) {
                                    l_hashStringToUint64(buf, &hash);
                                    lept_stderr("  %llx\n", hash);
                                }
                                if (add_dups && m < 4)  /* add redundant */
                                    sarrayAddString(sa, buf, L_COPY);
                            }
                        }
                    }
                }
            }
        }
    }

    return sa;
}

static PTA *
BuildPointSet(l_int32  w, l_int32 h, l_int32 add_dups)
{
l_int32  i, j;
PTA     *pta;

    pta = ptaCreate(w * h);
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++)
            ptaAddPt(pta, j, i);
        if (add_dups) { /* extra (0.2 * w * h) points */
            for (j = 0.4 * w; j < 0.6 * w; j++)
                ptaAddPt(pta, j, i);
        }
    }

    return pta;
}
