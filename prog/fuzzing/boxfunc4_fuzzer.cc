#include "leptfuzz.h"

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) { 
	if(size<3) return 0;
 
	leptSetStdNullHandler();

	BOXA *boxa_payload, *boxa1;
	boxa_payload = boxaReadMem(data, size);
	if(boxa_payload == NULL) return 0;

	PIX          *pix1;
	l_float32    pfract;
	l_int32      pminx, pminy, pmaxx, pmaxy, pchanged;
	l_int32      pminw, pminh, pmaxw, pmaxh;
	
	pix1 = boxaDisplayTiled(boxa_payload, NULL, 0, -1, 1500, 2, 1.0, 0, 3, 2);
	pixDestroy(&pix1);


	boxaGetCoverage(boxa_payload, 0, 0, 0, &pfract);


	boxaLocationRange(boxa_payload, &pminx, &pminy, &pmaxx, &pmaxy);


	boxa1 = boxaPermutePseudorandom(boxa_payload);
	boxaDestroy(&boxa1);


	boxaPermuteRandom(boxa_payload, boxa_payload);


	boxa1 = boxaSelectByWHRatio(boxa_payload, 1, 
				    L_SELECT_IF_LT, &pchanged)
	boxaDestroy(&boxa1);


	boxa1 = boxaSelectRange(boxa_payload, 0, -1, L_COPY)
	boxaDestroy(&boxa1);

	boxaDestroy(&boxa_payload);

	return 0;
}
