#include "string.h"
#include "allheaders.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
        if(size<10) return 0;

        char filename[256];
        sprintf(filename, "/tmp/libfuzzer.pa");
        FILE *fp = fopen(filename, "wb");
        if (!fp) return 0;
        fwrite(data, size, 1, fp);
        fclose(fp);

        PIXA      *pixa1, *pixa2, *pixa3, *pixa4;
        L_RECOG   *recog1, *recog2;
        PIX       *pix1, *pix2;

        pixa1 = pixaRead(filename);
        pixa2 = pixaCreate(0);

        recog1 = recogCreateFromPixa(pixa1, 0, 40, 1, 128, 1);
        
        pixa2 = recogTrainFromBoot(recog1, pixa1, 0.75, 128, 1);

        pixa3 = pixaRemoveOutliers1(pixa1, 0.8, 4, 3, &pix1, &pix2);
        recog2 = recogCreateFromPixa(pixa1, 4, 40, 1, 128, 1);

        recogIdentifyMultiple(recog2, pix2, 0, 0, NULL, &pixa4, NULL, 1);

        recogDestroy(&recog1);
        recogDestroy(&recog2);
        pixaDestroy(&pixa1);
        pixaDestroy(&pixa2);
        pixaDestroy(&pixa3);
        pixaDestroy(&pixa4);
        pixDestroy(&pix1);
        pixDestroy(&pix2);
        unlink(filename);

        return 0;
}
