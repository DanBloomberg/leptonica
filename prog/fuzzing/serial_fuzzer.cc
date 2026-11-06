// serial_fuzzer.cc — structured-object serialiser harness for Leptonica.
//
// Threat model (rationale):
//   Untrusted input: the whole payload, parsed by the object serialisers
//   (BOXA, BOXAA, NUMA, NUMAA, PTA, PIXA, PIXAA, PIXCMAP). Real applications
//   call boxaRead()/numaRead()/ptaRead()/pixaRead() on side-car files written
//   next to images by other tools, so the attacker controls the whole buffer.
//   Trust boundary: the *ReadMem family. 2026-11-05 own-code coverage shows
//   the parse and write halves of boxfunc1/boxfunc5 (boxfunc5.c 0%),
//   ptafunc1, numafunc1/2 and pixabasic essentially dark for these parsers.
//   A real bug here is any sanitizer finding while parsing crafted bytes; the
//   write/read round trips additionally re-parse self-produced bytes.
//
// Input layout (fixed order, stable so saved crashes reproduce):
//   data[0]  : parser selector
//   data[1:] : payload — serialised object bytes in any of the formats above
#include "leptfuzz.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 2)
        return 0;
    leptSetStdNullHandler();

    const l_uint8 *payload = data + 1;
    size_t         psize   = size - 1;
    l_int32        sel     = data[0];

        /* 1. Targeted serialiser parse of arbitrary bytes. */
    switch (sel % 8) {
    case 0: {
        BOXA *boxa = boxaReadMem(payload, psize);
        boxaDestroy(&boxa);
        break;
    }
    case 1: {
        BOXAA *boxaa = boxaaReadMem(payload, psize);
        boxaaDestroy(&boxaa);
        break;
    }
    case 2: {
        NUMA *na = numaReadMem(payload, psize);
        numaDestroy(&na);
        break;
    }
    case 3: {
        NUMAA *naa = numaaReadMem(payload, psize);
        numaaDestroy(&naa);
        break;
    }
    case 4: {
        PTA *pta = ptaReadMem(payload, psize);
        ptaDestroy(&pta);
        break;
    }
    case 5: {
        PIXA *pixa = pixaReadMem(payload, psize);
        pixaDestroy(&pixa);
        break;
    }
    case 6: {
        PIXAA *paa = pixaaReadMem(payload, psize);
        pixaaDestroy(&paa);
        break;
    }
    default: {
        PIXCMAP *cmap = pixcmapReadMem(payload, psize);
        pixcmapDestroy(&cmap);
        break;
    }
    }

        /* 2. Write/read round trips over the serialisers. Objects are built
         *    with contract-valid, fixed parameters derived from the decoded
         *    image, so the written bytes still depend on the fuzz input. */
    PIX *pixs = pixReadMemSpix(payload, psize);
    if (pixs == NULL)
        return 0;

    l_int32  w = pixGetWidth(pixs);
    l_int32  h = pixGetHeight(pixs);
    l_uint8 *enc = NULL;
    size_t   encsize = 0;
    l_ok     ok;
    BOXA    *boxa;
    NUMA    *na;
    NUMAA   *naa;
    PTA     *pta;
    PIXA    *pixa;
    PIXAA   *paa;
    PIXCMAP *cmap, *cmap2;

        /* BOXA round trip. */
    boxa = boxaCreate(2);
    boxaAddBox(boxa, boxCreate(0, 0, w, h), L_INSERT);
    boxaAddBox(boxa, boxCreate(w / 2, h / 2, w / 4, h / 4), L_INSERT);
    ok = boxaWriteMem(&enc, &encsize, boxa);
    boxaDestroy(&boxa);
    if (ok == 0 && enc != NULL) {
        BOXA *boxa2 = boxaReadMem(enc, encsize);
        boxaDestroy(&boxa2);
        lept_free(enc);
        enc = NULL;
    }

        /* NUMA round trip. */
    na = numaCreate(2);
    numaAddNumber(na, 0.5);
    numaAddNumber(na, (l_float32)w + 0.25);
    ok = numaWriteMem(&enc, &encsize, na);
    numaDestroy(&na);
    if (ok == 0 && enc != NULL) {
        NUMA *na2 = numaReadMem(enc, encsize);
        numaDestroy(&na2);
        lept_free(enc);
        enc = NULL;
    }

        /* NUMAA round trip. */
    naa = numaaCreate(1);
    na = numaCreate(1);
    numaAddNumber(na, (l_float32)h);
    numaaAddNuma(naa, na, L_INSERT);
    ok = numaaWriteMem(&enc, &encsize, naa);
    numaaDestroy(&naa);
    if (ok == 0 && enc != NULL) {
        NUMAA *naa2 = numaaReadMem(enc, encsize);
        numaaDestroy(&naa2);
        lept_free(enc);
        enc = NULL;
    }

        /* PTA round trip (float storage written as float). */
    pta = ptaCreate(2);
    ptaAddPt(pta, 0.0, 0.0);
    ptaAddPt(pta, (l_float32)w, (l_float32)h);
    ok = ptaWriteMem(&enc, &encsize, pta, 0);
    ptaDestroy(&pta);
    if (ok == 0 && enc != NULL) {
        PTA *pta2 = ptaReadMem(enc, encsize);
        ptaDestroy(&pta2);
        lept_free(enc);
        enc = NULL;
    }

        /* PIXA round trip (embeds the payload image as PNG). */
    pixa = pixaCreate(1);
    pixaAddPix(pixa, pixs, L_CLONE);
    ok = pixaWriteMem(&enc, &encsize, pixa);
    pixaDestroy(&pixa);
    if (ok == 0 && enc != NULL) {
        PIXA *pixa2 = pixaReadMem(enc, encsize);
        pixaDestroy(&pixa2);
        lept_free(enc);
        enc = NULL;
    }

        /* PIXAA round trip. */
    pixa = pixaCreate(1);
    pixaAddPix(pixa, pixs, L_CLONE);
    paa = pixaaCreate(1);
    pixaaAddPixa(paa, pixa, L_INSERT);
    ok = pixaaWriteMem(&enc, &encsize, paa);
    pixaaDestroy(&paa);
    if (ok == 0 && enc != NULL) {
        PIXAA *paa2 = pixaaReadMem(enc, encsize);
        pixaaDestroy(&paa2);
        lept_free(enc);
        enc = NULL;
    }

        /* PIXCMAP round trip (fixed 8 bpp, 4 colours). */
    cmap = pixcmapCreate(8);
    pixcmapAddColor(cmap, 10, 20, 30);
    pixcmapAddColor(cmap, 40, 50, 60);
    pixcmapAddColor(cmap, 70, 80, 90);
    pixcmapAddRGBA(cmap, 100, 110, 120, 200);
    ok = pixcmapWriteMem(&enc, &encsize, cmap);
    pixcmapDestroy(&cmap);
    if (ok == 0 && enc != NULL) {
        cmap2 = pixcmapReadMem(enc, encsize);
        pixcmapDestroy(&cmap2);
        lept_free(enc);
        enc = NULL;
    }

    pixDestroy(&pixs);
    return 0;
}
