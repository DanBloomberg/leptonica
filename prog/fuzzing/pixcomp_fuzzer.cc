// pixcomp_fuzzer.cc — compressed-image container (PIXC/PIXAC) harness.
//
// Threat model (rationale):
//   Untrusted input: the whole payload. Real applications read compressed
//   pix containers (PIXAC, the "Pixacomp" serialisation used by
//   pixacompReadMem() and segmented-image archives) and raw compressed
//   strings (pixcompCreateFromString()) produced by other tools, so the
//   attacker controls the whole buffer. 2026-11-05 own-code coverage:
//   pixcomp.c 2.7% (1,012 dark lines) — no existing harness touches it.
//   Trust boundary: pixacompReadMem() (header parse + embedded compressed
//   streams dispatched to the png/jpeg/spix/tiff-g4 decoders),
//   pixcompCreateFromString() (compressed-format identification) and
//   pixCreateFromPixcomp() (decode of contained data).  A real bug is any
//   sanitizer finding while parsing container headers or decoding the
//   embedded attacker bytes.
//
// Input layout (fixed order, stable so saved crashes reproduce):
//   data[0]  : compression-type selector for the round trip
//   data[1:] : payload — container bytes or compressed image bytes
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
    PIX           *pix;
    l_int32        n, i;

        /* 1. Parse the PIXAC container and decode every contained pixcomp. */
    PIXAC *pixac = pixacompReadMem(payload, psize);
    if (pixac != NULL) {
        n = pixacompGetCount(pixac);
        for (i = 0; i < n && i < 8; i++) {
            pix = pixacompGetPix(pixac, i);
            pixDestroy(&pix);
        }
        pixacompDestroy(&pixac);
    }

        /* 2. Compressed-format identification on arbitrary bytes; wrap the
         *    resulting PIXC in a container so pixCreateFromPixcomp() decodes
         *    it. L_COPY: the harness never owns the payload. */
    PIXC *pixc = pixcompCreateFromString((l_uint8 *)payload, psize, L_COPY);
    if (pixc != NULL) {
        PIXAC *pac1 = pixacompCreate(1);
        pixacompAddPixcomp(pac1, pixc, L_INSERT);
        pix = pixacompGetPix(pac1, 0);
        pixDestroy(&pix);
        pixacompDestroy(&pac1);   /* owns pixc (L_INSERT) */
    }

        /* 3. Round trip: SPIX payload image -> PIXAC in one of four valid
         *    compression types -> serialise -> re-parse -> decode. */
    PIX *pixs = pixReadMemSpix(payload, psize);
    if (pixs == NULL)
        return 0;

    static const l_int32 Comptype[4] = {IFF_DEFAULT, IFF_TIFF_G4, IFF_PNG,
                                        IFF_JFIF_JPEG};
    PIXA *pixa = pixaCreate(1);
    pixaAddPix(pixa, pixs, L_CLONE);
    PIXAC *pac = pixacompCreateFromPixa(pixa, Comptype[sel % 4], L_COPY);
    pixaDestroy(&pixa);
    if (pac != NULL) {
        l_uint8 *enc = NULL;
        size_t   encsize = 0;
        if (pixacompWriteMem(&enc, &encsize, pac) == 0 && enc != NULL) {
            PIXAC *pac2 = pixacompReadMem(enc, encsize);
            if (pac2 != NULL) {
                n = pixacompGetCount(pac2);
                for (i = 0; i < n && i < 4; i++) {
                    pix = pixacompGetPix(pac2, i);
                    pixDestroy(&pix);
                }
                pixacompDestroy(&pac2);
            }
            lept_free(enc);
        }
        pixacompDestroy(&pac);
    }

    pixDestroy(&pixs);
    return 0;
}
