#include "string.h"
#include "leptfuzz.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
        if(size<3) return 0;
        PIX *pix, *pix0, *pix1, *pix2, *pix3, *pix4;

        leptSetStdNullHandler();

        pix = pixReadMemSpix(data, size);
        if (pix == NULL) return 0;
        
        pix0 = pixModifyHue(NULL, pix, 0.01 + 0.05 * 1);
        pix1 = pixModifySaturation(NULL, pix, -0.9 + 0.1 * 1);
        pix2 = pixMosaicColorShiftRGB(pix, -0.1, 0.0, 0.0, 0.0999, 1);
        pix3 = pixMultConstantColor(pix, 0.7, 0.4, 1.3);
        pix4 = pixUnsharpMasking(pix, 3, 0.01 + 0.15 * 1);

        pixDestroy(&pix);
        pixDestroy(&pix0);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
        pixDestroy(&pix3);
        pixDestroy(&pix4);
        return 0;	
}
