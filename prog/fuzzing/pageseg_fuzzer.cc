#include "leptfuzz.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    leptSetStdNullHandler();

    l_int32      score;
    PIX          *pixs;
    PIX          *pixhm, *pixtm, *pixtb, *ppixdebug;
    PIXA         *pixadb, *ppixa;
    PIXAC        *pixac;
    BOX          *box;
    BOXA         *boxa;

    pixs = pixReadMemSpix(data, size);
    if(pixs==NULL) return 0;

    pixadb = pixaCreate(0);
    pixDecideIfTable(pixs, NULL, L_PORTRAIT_MODE, &score, pixadb);
    pixaDestroy(&pixadb);

    pixadb = pixaCreate(0);
    pixGetRegionsBinary(pixs, &pixhm, &pixtm, &pixtb, pixadb);
    pixDestroy(&pixhm);
    pixDestroy(&pixtm);
    pixDestroy(&pixtb);
    pixaDestroy(&pixadb);

    pixac = pixacompReadMem(data, size);
    box = pixFindPageForeground(pixs, 170, 70, 30, 0, pixac);
    boxDestroy(&box);
    pixacompDestroy(&pixac);

    pixSplitIntoCharacters(pixs, 4, 4, &boxa, &ppixa, &ppixdebug);
    boxaDestroy(&boxa);
    pixaDestroy(&ppixa);
    pixDestroy(&ppixdebug);


    pixDestroy(&pixs);
    return 0;
}
