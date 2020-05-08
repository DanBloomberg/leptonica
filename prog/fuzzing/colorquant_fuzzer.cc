#include "leptfuzz.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    BOX *box;
    PIX *pixs, *pixd;
    PIX *pix1, *pix2, *pix3, *pix4, *pix5, *pix6, *pix7, *pix8;
    PIXCMAP *cmap;

    leptSetStdNullHandler();
    pixs = pixReadMemSpix(data, size);
    if(pixs==NULL) return 0;

    pix1 = pixThresholdTo4bpp(pixs, 6, 1);
    box = boxCreate(120, 30, 200, 200);
    pixColorGray(pix1, box, L_PAINT_DARK, 220, 0, 0, 255);
    boxDestroy(&box);

    pix2 = pixScale(pix1, 1.5, 1.5);

    cmap = pixGetColormap(pix1);
    pix3 = pixOctcubeQuantFromCmap(pix2, cmap, 4,
                                   3, L_EUCLIDEAN_DISTANCE);

    pix4 = pixConvertTo32(pix3);
    pix5 = pixMedianCutQuant(pix4, 0);
    pix6 = pixFewColorsMedianCutQuantMixed(pix4, 30, 30, 100, 0, 0, 0);

    pixd = pixDeskew(pixs, 0);
    pixWriteImpliedFormat("/tmp/lept/deskew/result1", pixd, 0, 0);

    pixOctreeQuantByPopulation(pixs, 0, 0);
    pix7 = pixFewColorsOctcubeQuantMixed(pix4, 3, 20, 244, 20, 0.05, 15);
    pix8 = pixColorSegment(pixs, 50, 6, 6, 6, 0);
    
    pixDestroy(&pix1);
    pixDestroy(&pix2);
    pixDestroy(&pix3);
    pixDestroy(&pix4);
    pixDestroy(&pix5);
    pixDestroy(&pix6);
    pixDestroy(&pix7);
    pixDestroy(&pix8);

    pixDestroy(&pixd);
    pixDestroy(&pixs);
    
    return 0;
}
