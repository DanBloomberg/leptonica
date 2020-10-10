#include "leptfuzz.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) { 
    if(size<3) return 0;

    leptSetStdNullHandler();


    PIX *pixs_payload = pixReadMemSpix(data, size);
    if(pixs_payload == NULL) return 0;

    PIX *pix1;
    PIX *pix2;

    PIX *pix1 = pixRead("../test8.jpg");
    PIX *pix2 = pixRead("../test8.jpg");
    PIX *pix_return1 = pixDisplayMatchedPattern(pixs_payload, pix1, pix2,  1,  2, 3, 0.5, 1);
    pixDestroy(&pix1);
    pixDestroy(&pix_return1);
    pixDestroy(&pix2);


    PIX *pix_return2 = pixFastTophat(pixs_payload, 2, 2, L_TOPHAT_WHITE);
    pixDestroy(&pix_return2);

    PIX *pix_return3 = pixHDome(pixs_payload, 1, 4);
    pixDestroy(&pix_return3);

    SELA *sela1 = selaCreate(0);
    PIX *pix_return4 = pixIntersectionOfMorphOps(pixs_payload, sela1, L_MORPH_DILATE);
    selaDestroy(&sela1);
    pixDestroy(&pix_return4);

    PIX *pix_return5 = pixMorphGradient(pixs_payload, 5, 5, 1);
    pixDestroy(&pix_return5);


    pix1 = pixRead("../test8.jpg");
    BOXA *boxa1;
    const char *sequence = "sequence";
    PIX *pix_return6 = pixMorphSequenceByRegion(pixs_payload, pix1, sequence, 4, 1, 1, &boxa1);
    boxaDestroy(&boxa1);
    pixDestroy(&pix1);
    pixDestroy(&pix_return6);

    pix1 = pixRead("../test8.jpg");
    PIX *pix_return7 = pixMorphSequenceMasked(pixs_payload, pix1, sequence, 0);
    pixDestroy(&pix1);
    pixDestroy(&pix_return7);


    pix1 = pixCreate(300, 300, 32);
    pix2 = pixCreate(300, 300, 32);
    pixRemoveMatchedPattern(pixs_payload, pix1, pix2, 2, 2, 2);
    pixDestroy(&pix1);
    pixDestroy(&pix2);


    pix = pixCreate(300, 300, 32);
    PIX *pix_return8 = pixSeedfillMorph(pixs_payload, pix, 0, 4);
    pixDestroy(&pix_return8);
    pixDestroy(&pix);


    PIX *pix_return9 = pixSelectiveConnCompFill(pixs_payload, 4, 1, 1);
    pixDestroy(&pix_return9);

    pixDestroy(&pixs_payload);
    return 0;
}