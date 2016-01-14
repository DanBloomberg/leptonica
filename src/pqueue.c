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
 *   pqueue.c
 *
 *      Create/Destroy PQueue
 *          PQUEUE    *pqueueCreate()
 *          void      *pqueueDestroy()
 *
 *      Operations to add/remove to/from a PBuffer
 *          l_int32    pqueueAdd()
 *          l_int32    pqueueExtendArray()
 *          void      *pqueueRemove()
 *
 *      Accessors
 *          l_int32    pqueueGetCount()
 *
 *      Debug output
 *          l_int32    pqueuePrint()
 *
 *    The pqueue is a fifo that implements a queue of void* pointers.
 *    It can be used to hold a queue of any type of struct.
 *    Internally, it maintains two counters: 
 *        nhead:  location of head (in ptrs) from the beginning
 *                of the buffer
 *        nelem:  number of ptr elements stored in the queue
 *    As items are added to the queue, nelem increases.
 *    As items are removed, nhead increases and nelem decreases.
 *    Any time the tail reaches the end of the allocated buffer,
 *      all the pointers are shifted to the left, so that the head
 *      is at the beginning of the array.
 *    If the buffer becomes more than 3/4 full, it doubles in size.
 *
 *    [A circular queue would allow us to skip the shifting and
 *    to resize only when the buffer is full.  For most applications,
 *    the extra work we do for a linear queue is not significant.]
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32  MIN_BUFFER_SIZE = 20;             /* n'importe quoi */
static const l_int32  INITIAL_BUFFER_ARRAYSIZE = 1024;  /* n'importe quoi */


/*--------------------------------------------------------------------------*
 *                         PQueue create/destroy                           *
 *--------------------------------------------------------------------------*/
/*!
 *  pqueueCreate()
 *
 *      Input:  size of ptr array to be alloc'd (0 for default)
 *      Return: pqueue, or null on error
 *
 *  Action: allocates a ptr array of given size, and initializes counters.
 */
PQUEUE *
pqueueCreate(l_int32  nalloc)
{
PQUEUE  *pq;

    PROCNAME("pqueueCreate");

    if (nalloc < MIN_BUFFER_SIZE)
        nalloc = INITIAL_BUFFER_ARRAYSIZE;

    if ((pq = (PQUEUE *)CALLOC(1, sizeof(PQUEUE))) == NULL)
        return (PQUEUE *)ERROR_PTR("pq not made", procName, NULL);
    if ((pq->array = (void **)CALLOC(nalloc, sizeof(l_intptr_t))) == NULL)
        return (PQUEUE *)ERROR_PTR("ptr array not made", procName, NULL);
    pq->nalloc = nalloc;
    pq->nhead = pq->nelem = 0;
    return pq;
}


/*!
 *  pqueueDestroy()
 *
 *      Input:  &pqueue  (<to be nulled>)
 *              freeflag (TRUE to free each remaining struct in the array)
 *      Return: void
 *
 *  Action: If freeflag is TRUE, frees each struct in the array.
 *          If freeflag is FALSE but there are elements on the array,
 *            gives a warning and destroys the array.  This will
 *            cause a memory leak of all the items that were on the queue.
 *            So if the items require their own destroy function, they
 *            must be destroyed before the queue.
 *          To destroy the PQueue, we destroy the ptr array, then
 *            the pqueue, and then null the contents of the input ptr.
 */
void
pqueueDestroy(PQUEUE  **ppq,
              l_int32   freeflag)
{
void    *item;
PQUEUE  *pq;

    PROCNAME("pqueueDestroy");

    if (ppq == NULL) {
        L_WARNING("ptr address is NULL", procName);
        return;
    }
    if ((pq = *ppq) == NULL)
        return;

    if (freeflag) {
        while(pq->nelem > 0) {
            item = pqueueRemove(pq);
            FREE(item);
        }
    }
    else if (pq->nelem > 0)
        L_WARNING_INT("memory leak of %d items in pqueue!",
                      procName, pq->nelem);

    if (pq->array)
        FREE(pq->array);
    FREE((void *)pq);
    *ppq = NULL;

    return;
}


/*--------------------------------------------------------------------------*
 *                                  Accessors                               *
 *--------------------------------------------------------------------------*/
/*!
 *  pqueueAdd()
 *
 *      Input:  pqueue
 *              item to be added to the tail of the queue
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) The algorithm is as follows.  If the queue is populated
 *          to the end of the allocated array, shift all ptrs toward
 *          the beginning of the array, so that the head of the queue
 *          is at the beginning of the array.  Then, if the array is
 *          more than 0.75 full, realloc with double the array size.
 *          Finally, add the item to the tail of the queue.
 */
l_int32
pqueueAdd(PQUEUE  *pq,
          void    *item)
{
    PROCNAME("pqueueAdd");

    if (!pq)
        return ERROR_INT("pq not defined", procName, 1);
    if (!item)
        return ERROR_INT("item not defined", procName, 1);
    
        /* If filled to the end and the ptrs can be shifted to the left,
         * shift them. */
    if ((pq->nhead + pq->nelem >= pq->nalloc) && (pq->nhead != 0)) {
        memmove((void *)(pq->array), 
                (void *)(pq->array + pq->nhead),
                sizeof(l_intptr_t) * pq->nelem);
        pq->nhead = 0;
    }

        /* If necessary, expand the allocated array by a factor of 2 */
    if (pq->nelem > 0.75 * pq->nalloc)
        pqueueExtendArray(pq);

        /* Now add the item */
    pq->array[pq->nhead + pq->nelem] = (void *)item;
    pq->nelem++;

    return 0;
}


/*!
 *  pqueueExtendArray()
 *
 *      Input:  pqueue
 *      Return: 0 if OK, 1 on error
 */
l_int32
pqueueExtendArray(PQUEUE  *pq)
{
    PROCNAME("pqueueExtendArray");

    if (!pq)
        return ERROR_INT("pq not defined", procName, 1);

    if ((pq->array = (void **)reallocNew((void **)&pq->array,
                                sizeof(l_intptr_t) * pq->nalloc,
                                2 * sizeof(l_intptr_t) * pq->nalloc)) == NULL)
        return ERROR_INT("new ptr array not returned", procName, 1);

    pq->nalloc = 2 * pq->nalloc;
    return 0;
}


/*!
 *  pqueueRemove()
 *
 *      Input:  pqueue
 *      Return: ptr to item popped from the head of the queue,
 *              or null if the queue is empty or on error
 *
 *  Notes:
 *      (1) If this is the last item on the queue, so that the queue
 *          becomes empty, nhead is reset to the beginning of the array.
 */
void *
pqueueRemove(PQUEUE  *pq)
{
void  *item;

    PROCNAME("pqueueRemove");

    if (!pq)
        return (void *)ERROR_PTR("pq not defined", procName, NULL);

    if (pq->nelem == 0)
        return NULL;
    item = pq->array[pq->nhead];
    pq->array[pq->nhead] = NULL;
    if (pq->nelem == 1) 
        pq->nhead = 0;  /* reset head ptr */
    else
        (pq->nhead)++;  /* can't go off end of array because nelem > 1 */
    pq->nelem--;
    return item;
}
       

/*!
 *  pqueueGetCount()
 *
 *      Input:  pqueue
 *      Return: count, or 0 on error
 */
l_int32
pqueueGetCount(PQUEUE  *pq)
{
    PROCNAME("pqueueGetCount");

    if (!pq)
        return ERROR_INT("pq not defined", procName, 0);

    return pq->nelem;
}
        

/*---------------------------------------------------------------------*
 *                            Debug output                             *
 *---------------------------------------------------------------------*/
/*!
 *  pqueuePrint()
 *  
 *      Input:  stream
 *              pqueue 
 *      Return: 0 if OK; 1 on error
 */
l_int32
pqueuePrint(FILE    *fp,
            PQUEUE  *pq)
{
l_int32  i;

    PROCNAME("pqueuePrint");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!pq)
        return ERROR_INT("pq not defined", procName, 1);

    fprintf(fp, "\n PQueue: nalloc = %d, nhead = %d, nelem = %d, array = %p\n",
            pq->nalloc, pq->nhead, pq->nelem, pq->array);
    for (i = pq->nhead; i < pq->nhead + pq->nelem; i++)
    fprintf(fp,   "array[%d] = %p\n", i, pq->array[i]);

    return 0;
}

