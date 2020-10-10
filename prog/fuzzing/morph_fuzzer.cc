#include "leptfuzz.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) { 
    if(size<3) return 0;
 
    leptSetStdNullHandler();


    PIX *pixs_payload = pixReadMemSpix(data, size);
    if(pixs_payload == NULL) return 0;


    PIX *pix;
    PIX *pix1;

    pix = pixRead("../test8.jpg");
    pix1 = pixCreate(size, size, 1);
    SEL *sel;
    sel = selCreateFromPix(pix1, 6, 6, "plus_sign");
    PIX *pix_copy1 = pixCopy(NULL, pixs_payload);
    pixCloseGeneralized(pix_copy1, pix, sel);
    pixDestroy(&pix_copy1);
    pixDestroy(&pix);
    pixDestroy(&pix1);
    selDestroy(&sel);

    pix1 = pixCreate(size, size, 1);
    sel = selCreateFromPix(pix1, 6, 6, "plus_sign");
    PIX *pix_copy2 = pixCopy(NULL, pixs_payload);
    pixCloseSafe(pix_copy2, pix1, sel);
    pixDestroy(&pix_copy2);
    pixDestroy(&pix1);
    selDestroy(&sel);

    pix = pixRead("../test8.jpg");
    sel = selCreateFromPix(pix, 6, 6, "plus_sign");
    PIX *pix_copy3 = pixCopy(NULL, pixs_payload);
    pixOpenGeneralized(pix_copy3, pix, sel);
    pixDestroy(&pix_copy3);
    pixDestroy(&pix);
    selDestroy(&sel);

    pixDestroy(&pixs_payload);

    return 0;
}