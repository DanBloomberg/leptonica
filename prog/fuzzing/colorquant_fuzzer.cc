#include "allheaders.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    PIX *pixs, *pix1;

    pixs = pixReadMemSpix(data, size);
    if(pixs==NULL) return 0;
    
    pix1 = pixFewColorsMedianCutQuantMixed(pixs, 30, 30, 100, 0, 0, 0);

    pixDestroy(&pixs);
    pixDestroy(&pix1);
    return 0;
}
