#include "leptfuzz.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if(size<3) return 0;

    leptSetStdNullHandler();

    PIX *pixs1, *pixs2, *pix1;

    pixs1 = pixReadMemSpix(data, size);
    if(pixs1==NULL) return 0;
    pixs2 = pixReadMemSpix(&data[1], size);
    if(pixs2==NULL){
        pixDestroy(&pixs1);
        return 0;
    }
    
    pix1 = pixBlend(pixs1, pixs2, 140, 40, 2);

    pixDestroy(&pix1);
    pixDestroy(&pixs1);
    pixDestroy(&pixs2);

    return 0;
}
