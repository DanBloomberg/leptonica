// formats_fuzzer.cc — image file-format decoder/encoder harness for Leptonica.
//
// Threat model (rationale):
//   All pre-existing fuzzers decode input only via pixReadMemSpix(), so the
//   format-detecting pixReadMem() and every per-format decoder/encoder is
//   unreachable by them (2026-11-05 own-code coverage: pnmio.c 0%,
//   psio2.c 0%, jpegio.c 1.2%, readfile.c 19.4%). Real applications call
//   pixReadMem()/pixRead() on files of arbitrary type, so the attacker
//   controls the whole buffer. A real bug here is any sanitizer finding
//   while parsing crafted bytes, or while encoding attacker-derived pixel
//   data and re-parsing the encoded bytes.
//
// Input layout (fixed order, stable so saved crashes reproduce):
//   data[0]  : codec selector (which targeted decoder/encoder direction runs)
//   data[1:] : payload — an image file in any format leptonica reads
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

        /* 1. Format auto-detection (findFileFormatBuffer + dispatch). */
    if ((pix = pixReadMem(payload, psize)) != NULL)
        pixDestroy(&pix);

        /* 2. Targeted per-format decoders, selected by the codec byte.
         *    Deterministic reach of each parser independent of magic bytes. */
    switch (sel % 6) {
    case 0:
        pix = pixReadMemBmp(payload, psize);
        break;
    case 1: {
        l_int32 warn;
        pix = pixReadMemJpeg(payload, psize, 0, 1, &warn, 0);
        break;
    }
    case 2:
        pix = pixReadMemPng(payload, psize);
        break;
    case 3:
        pix = pixReadMemPnm(payload, psize);
        break;
    case 4:
        pix = pixReadMemWebP(payload, psize);
        break;
    default:
        pix = pixReadMemSpix(payload, psize);
        break;
    }
    if (pix != NULL)
        pixDestroy(&pix);

        /* 3. Multi-page TIFF readers. */
    {
        size_t offset = 0;
        for (l_int32 page = 0; page < 4; page++) {
            pix = pixReadMemFromMultipageTiff(payload, psize, &offset);
            if (pix == NULL)
                break;
            pixDestroy(&pix);
        }
        PIXA *pixa = pixaReadMemMultipageTiff(payload, psize);
        pixaDestroy(&pixa);
    }

        /* 4. Encode/re-parse round trip. The payload must decode as a valid
         *    SPIX image first; encoder configuration is fixed (the attacker
         *    controls the image, not the codec settings). Write-only formats
         *    (PS, PDF) exercise their encoder without a re-parse. */
    PIX *pixs = pixReadMemSpix(payload, psize);
    if (pixs == NULL)
        return 0;

    static const l_int32 TiffComptype[5] = {IFF_TIFF, IFF_TIFF_G4, IFF_TIFF_LZW,
                                            IFF_TIFF_ZIP, IFF_TIFF_PACKBITS};
    l_uint8 *enc = NULL;
    size_t   encsize = 0;
    PIX     *pixd = NULL;
    l_ok     ok = 1;

    switch (sel % 9) {
    case 0:
        ok = pixWriteMemBmp(&enc, &encsize, pixs);
        if (ok == 0 && enc != NULL) pixd = pixReadMemBmp(enc, encsize);
        break;
    case 1:
        ok = pixWriteMemJpeg(&enc, &encsize, pixs, 75, 0);
        if (ok == 0 && enc != NULL) {
            l_int32 warn;
            pixd = pixReadMemJpeg(enc, encsize, 0, 1, &warn, 0);
        }
        break;
    case 2:
        ok = pixWriteMemPng(&enc, &encsize, pixs, 2.2f);
        if (ok == 0 && enc != NULL) pixd = pixReadMemPng(enc, encsize);
        break;
    case 3:
        if ((sel / 9) % 2 == 0) {
            ok = pixWriteMemPnm(&enc, &encsize, pixs);
            if (ok == 0 && enc != NULL) pixd = pixReadMemPnm(enc, encsize);
        } else {
            ok = pixWriteMemPam(&enc, &encsize, pixs);
            if (ok == 0 && enc != NULL) pixd = pixReadMemPnm(enc, encsize);
        }
        break;
    case 4:
        ok = pixWriteMemTiff(&enc, &encsize, pixs, TiffComptype[(sel / 9) % 5]);
        if (ok == 0 && enc != NULL) pixd = pixReadMemTiff(enc, encsize, 0);
        break;
    case 5:
        ok = pixWriteMemWebP(&enc, &encsize, pixs, 75.0f, 0);
        if (ok == 0 && enc != NULL) pixd = pixReadMemWebP(enc, encsize);
        break;
    case 6:
        ok = pixWriteMemSpix(&enc, &encsize, pixs);
        if (ok == 0 && enc != NULL) pixd = pixReadMemSpix(enc, encsize);
        break;
    case 7:
        ok = pixWriteMemPS(&enc, &encsize, pixs, NULL, 0, 1.0f);
        break;
    default:
        ok = pixWriteMemPdf(&enc, &encsize, pixs, 0, NULL);
        break;
    }

    if (pixd != NULL)
        pixDestroy(&pixd);
    if (enc != NULL)
        lept_free(enc);
    pixDestroy(&pixs);
    return 0;
}
