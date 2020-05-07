#include "leptfuzz.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if(size<3) return 0;

    leptSetStdNullHandler();

    l_float32 conf;
    PIX *pixs;

    pixs = pixReadMemSpix(data, size);
    if(pixs==NULL) return 0;

    pixMirrorDetectDwa(pixs, &conf, 0, 0);

    pixDestroy(&pixs);
    return 0;
}
