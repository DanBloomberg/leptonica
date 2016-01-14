/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -  This software is distributed in the hope that it will be
 -  useful, but with NO WARRANTY OF ANY KIND.
 -  No author or distributor accepts responsibility to anyone for the
 -  consequences of using this software, or for whether it serves any
 -  particular purpose or works at all, unless he or she says so in
 -  writing.  Everyone is granted permission to copy, modify and
 -  redistribute this source code, for commercial or non-commercial
 -  purposes, with the following restrictions: (1) the origin of this
 -  source code must not be misrepresented; (2) modified versions must
 -  be plainly marked as such; and (3) this notice may not be removed
 -  or altered from any source or modified source distribution.
 *====================================================================*/


/*
 *  stack.c
 *
 *      Generic stack
 *
 *      The pstack is an array of void * ptrs, onto which
 *      objects can be stored.  At any time, the number of
 *      stored objects is pstack->n.  The object at the bottom
 *      of the pstack is at array[0]; the object at the top of
 *      the pstack is at array[n-1].  New objects are added
 *      to the top of the pstack; i.e., the first available 
 *      location, which is at array[n].  The pstack is expanded
 *      by doubling, when needed.  Objects are removed
 *      from the top of the pstack.  When an attempt is made
 *      to remove an object from an empty pstack, the result is null.
 *
 *      Create/Destroy
 *           PSTACK    *pstackCreate()
 *           void       pstackDestroy()
 *
 *      Accessors
 *           l_int32    pstackAdd()
 *           void      *pstackRemove()
 *           l_int32    pstackExtendArray()
 *           l_int32    pstackGetCount()
 *
 *      Text description
 *           l_int32    pstackPrint()
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32  INITIAL_PTR_ARRAYSIZE = 20;


/*---------------------------------------------------------------------*
 *                          Create/Destroy                             *
 *---------------------------------------------------------------------*/
/*!
 *  pstackCreate()
 *
 *      Input:  nalloc (initial ptr array size; use 0 for default)
 *      Return: pstack, or null on error
 */
PSTACK *
pstackCreate(l_int32  nalloc)
{
PSTACK  *pstack;

    PROCNAME("pstackCreate");

    if (nalloc <= 0)
        nalloc = INITIAL_PTR_ARRAYSIZE;

    if ((pstack = (PSTACK *)CALLOC(1, sizeof(PSTACK))) == NULL)
        return (PSTACK *)ERROR_PTR("pstack not made", procName, NULL);

    if ((pstack->array = (void **)CALLOC(nalloc, sizeof(void *))) == NULL)
        return (PSTACK *)ERROR_PTR("pstack ptr array not made", procName, NULL);

    pstack->nalloc = nalloc;
    pstack->n = 0;
    
    return pstack;
}


/*!
 *  pstackDestroy()
 *
 *      Input:  &pstack (<to be nulled>)
 *              freeflag (TRUE to free each remaining struct in the array)
 *      Return: void
 *
 *  Notes:
 *      (1) If freeflag is TRUE, frees each struct in the array.
 *      (2) If freeflag is FALSE but there are elements on the array,
 *          gives a warning and destroys the array.  This will
 *          cause a memory leak of all the items that were on the pstack.
 *          So if the items require their own destroy function, they
 *          must be destroyed before the pstack.
 *      (3) To destroy the pstack, we destroy the ptr array, then
 *          the pstack, and then null the contents of the input ptr.
 */
void
pstackDestroy(PSTACK  **ppstack,
              l_int32   freeflag)
{
void    *item;
PSTACK  *pstack;

    PROCNAME("pstackDestroy");

    if (ppstack == NULL) {
        L_WARNING("ptr address is NULL", procName);
        return;
    }
    if ((pstack = *ppstack) == NULL)
        return;

    if (freeflag) {
        while(pstack->n > 0) {
            item = pstackRemove(pstack);
            FREE(item);
        }
    }
    else if (pstack->n > 0)
        L_WARNING_INT("memory leak of %d items in pstack", procName, pstack->n);

    if (pstack->auxstack)
        pstackDestroy(&pstack->auxstack, freeflag);

    if (pstack->array)
        FREE(pstack->array);
    FREE(pstack);
    *ppstack = NULL;
}



/*---------------------------------------------------------------------*
 *                               Accessors                             *
 *---------------------------------------------------------------------*/
/*!
 *  pstackAdd()
 *
 *      Input:  pstack
 *              item to be added to the pstack
 *      Return: 0 if OK; 1 on error.
 */
l_int32
pstackAdd(PSTACK  *pstack,
          void    *item)
{
    PROCNAME("pstackAdd");

    if (!pstack)
        return ERROR_INT("pstack not defined", procName, 1);
    if (!item)
        return ERROR_INT("item not defined", procName, 1);

        /* Do we need to extend the array? */
    if (pstack->n >= pstack->nalloc)
        pstackExtendArray(pstack);

        /* Store the new pointer */
    pstack->array[pstack->n] = (void *)item;
    pstack->n++;

    return 0;
}


/*!
 *  pstackRemove()
 *
 *      Input:  pstack 
 *      Return: ptr to item popped from the top of the pstack,
 *              or null if the pstack is empty or on error
 */
void *
pstackRemove(PSTACK  *pstack)
{
void  *item;

    PROCNAME("pstackRemove");

    if (!pstack)
        return ERROR_PTR("pstack not defined", procName, NULL);

    if (pstack->n == 0)
        return NULL;

    pstack->n--;
    item = pstack->array[pstack->n];
        
    return item;
}


/*!
 *  pstackExtendArray()
 *
 *      Input:  pstack
 *      Return: 0 if OK; 1 on error
 */
l_int32
pstackExtendArray(PSTACK  *pstack)
{
    PROCNAME("pstackExtendArray");

    if (!pstack)
        return ERROR_INT("pstack not defined", procName, 1);

    if ((pstack->array = (void **)reallocNew((void **)&pstack->array,
                              sizeof(l_intptr_t) * pstack->nalloc,
                              2 * sizeof(l_intptr_t) * pstack->nalloc)) == NULL)
        return ERROR_INT("new pstack array not defined", procName, 1);

    pstack->nalloc = 2 * pstack->nalloc;
    return 0;
}


/*!
 *  pstackGetCount()
 *
 *      Input:  pstack
 *      Return: count, or 0 on error
 */
l_int32
pstackGetCount(PSTACK  *pstack)
{
    PROCNAME("pstackGetCount");

    if (!pstack)
        return ERROR_INT("pstack not defined", procName, 1);

    return pstack->n;
}



/*---------------------------------------------------------------------*
 *                            Debug output                             *
 *---------------------------------------------------------------------*/
/*!
 *  pstackPrint()
 *
 *      Input:  stream
 *              pstack
 *      Return: 0 if OK; 1 on error
 */
l_int32
pstackPrint(FILE    *fp,
            PSTACK  *pstack)
{
l_int32  i;

    PROCNAME("pstackPrint");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!pstack)
        return ERROR_INT("pstack not defined", procName, 1);

    fprintf(fp, "\n Stack: nalloc = %d, n = %d, array = %p\n", 
            pstack->nalloc, pstack->n, pstack->array);
    for (i = 0; i < pstack->n; i++)
        fprintf(fp,   "array[%d] = %p\n", i, pstack->array[i]);
    
    return 0;
}

