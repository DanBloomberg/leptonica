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
 *   pheap.c
 *
 *      Create/Destroy PHeap
 *          PHEAP     *pheapCreate()
 *          void      *pheapDestroy()
 *
 *      Operations to add/remove to/from the heap
 *          l_int32    pheapAdd()
 *          l_int32    pheapExtendArray()
 *          void      *pheapRemove()
 *
 *      Heap operations
 *          l_int32    pheapSwapUp()
 *          l_int32    pheapSwapDown()
 *          l_int32    pheapSort()
 *          l_int32    pheapSortStrictOrder()
 *
 *      Accessors
 *          l_int32    pheapGetCount()
 *
 *      Debug output
 *          l_int32    pheapPrint()
 *
 *    The pheap is useful to implement a priority queue, that is sorted
 *    on a key in each element of the heap.  The heap is an array
 *    of nearly arbitrary structs, with a l_float32 the first field.
 *    This field is the key on which the heap is sorted.
 *
 *    Internally, we keep track of the heap size, n.  The item at the
 *    root of the heap is at the head of the array.  Items are removed
 *    from the head of the array and added to the end of the array.
 *    When an item is removed from the head, the item at the end
 *    of the array is moved to the head.  When items are either
 *    added or removed, it is usually necesary to swap array items
 *    to restore the heap order.  It is guaranteed that the number
 *    of swaps does not exceed log(n).
 *
 *    --------------------------  N.B.  ------------------------------
 *    The items on the heap (or, equivalently, in the array) are cast
 *    to void*.  Their key is a l_float32, and it is REQUIRED that the
 *    key be the first field in the struct.  That allows us to get the
 *    key by simply dereferencing the struct.  Alternatively, we could
 *    choose (but don't) to pass an application-specific comparison
 *    function into the heap operation functions.
 *    --------------------------  N.B.  ------------------------------
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32  MIN_BUFFER_SIZE = 20;             /* n'importe quoi */
static const l_int32  INITIAL_BUFFER_ARRAYSIZE = 128;   /* n'importe quoi */

#define SWAP_ITEMS(i, j)       { void *tempitem = ph->array[(i)]; \
                                 ph->array[(i)] = ph->array[(j)]; \
                                 ph->array[(j)] = tempitem; }


/*--------------------------------------------------------------------------*
 *                          PHeap create/destroy                           *
 *--------------------------------------------------------------------------*/
/*!
 *  pheapCreate()
 *
 *      Input:  size of ptr array to be alloc'd (0 for default)
 *              direction (L_SORT_INCREASING, L_SORT_DECREASING)
 *      Return: pheap, or null on error
 */
PHEAP *
pheapCreate(l_int32  nalloc,
            l_int32  direction)
{
PHEAP  *ph;

    PROCNAME("pheapCreate");

    if (nalloc < MIN_BUFFER_SIZE)
        nalloc = MIN_BUFFER_SIZE;

        /* Allocate ptr array and initialize counters. */
    if ((ph = (PHEAP *)CALLOC(1, sizeof(PHEAP))) == NULL)
        return (PHEAP *)ERROR_PTR("ph not made", procName, NULL);
    if ((ph->array = (void **)CALLOC(nalloc, sizeof(l_intptr_t))) == NULL)
        return (PHEAP *)ERROR_PTR("ptr array not made", procName, NULL);
    ph->nalloc = nalloc;
    ph->n = 0;
    ph->direction = direction;
    return ph;
}


/*!
 *  pheapDestroy()
 *
 *      Input:  &pheap  (<to be nulled>)
 *              freeflag (TRUE to free each remaining struct in the array)
 *      Return: void
 *
 *  Notes:
 *      (1) Use freeflag == TRUE when the items in the array can be
 *          simply destroyed using free.  If those items require their
 *          own destroy function, they must be destroyed before
 *          calling this function, and then this function is called
 *          with freeflag == FALSE.
 *      (2) To destroy the pheap, we destroy the ptr array, then
 *          the pheap, and then null the contents of the input ptr.
 */
void
pheapDestroy(PHEAP   **pph,
             l_int32   freeflag)
{
l_int32  i;
PHEAP   *ph;

    PROCNAME("pheapDestroy");

    if (pph == NULL) {
        L_WARNING("ptr address is NULL", procName);
        return;
    }
    if ((ph = *pph) == NULL)
        return;

    if (freeflag) {  /* free each struct in the array */
        for (i = 0; i < ph->n; i++)
            FREE(ph->array[i]);
    }
    else if (ph->n > 0)  /* freeflag == FALSE but elements exist on array */
        L_WARNING_INT("memory leak of %d items in pheap!", procName, ph->n);

    if (ph->array)
        FREE(ph->array);
    FREE(ph);
    *pph = NULL;

    return;
}

/*--------------------------------------------------------------------------*
 *                                  Accessors                               *
 *--------------------------------------------------------------------------*/
/*!
 *  pheapAdd()
 *
 *      Input:  pheap
 *              item to be added to the tail of the heap
 *      Return: 0 if OK, 1 on error
 */
l_int32
pheapAdd(PHEAP  *ph,
         void   *item)
{
    PROCNAME("pheapAdd");

    if (!ph)
        return ERROR_INT("ph not defined", procName, 1);
    if (!item)
        return ERROR_INT("item not defined", procName, 1);
    
        /* If necessary, expand the allocated array by a factor of 2 */
    if (ph->n >= ph->nalloc)
        pheapExtendArray(ph);

        /* Add the item */
    ph->array[ph->n] = item;
    ph->n++;

        /* Restore the heap */
    pheapSwapUp(ph, ph->n - 1);
    return 0;
}


/*!
 *  pheapExtendArray()
 *
 *      Input:  pheap
 *      Return: 0 if OK, 1 on error
 */
l_int32
pheapExtendArray(PHEAP  *ph)
{
    PROCNAME("pheapExtendArray");

    if (!ph)
        return ERROR_INT("ph not defined", procName, 1);

    if ((ph->array = (void **)reallocNew((void **)&ph->array,
                                sizeof(l_intptr_t) * ph->nalloc,
                                2 * sizeof(l_intptr_t) * ph->nalloc)) == NULL)
        return ERROR_INT("new ptr array not returned", procName, 1);

    ph->nalloc = 2 * ph->nalloc;
    return 0;
}


/*!
 *  pheapRemove()
 *
 *      Input:  pheap
 *      Return: ptr to item popped from the root of the heap,
 *              or null if the heap is empty or on error
 */
void *
pheapRemove(PHEAP  *ph)
{
void   *item;

    PROCNAME("pheapRemove");

    if (!ph)
        return (void *)ERROR_PTR("ph not defined", procName, NULL);

    if (ph->n == 0)
        return NULL;

    item = ph->array[0];
    ph->array[0] = ph->array[ph->n - 1];  /* move last to the head */
    ph->array[ph->n - 1] = NULL;  /* set ptr to null */
    ph->n--;

    pheapSwapDown(ph);  /* restore the heap */
    return item;
}
       

/*!
 *  pheapGetCount()
 *
 *      Input:  pheap
 *      Return: count, or 0 on error
 */
l_int32
pheapGetCount(PHEAP  *ph)
{
    PROCNAME("pheapGetCount");

    if (!ph)
        return ERROR_INT("ph not defined", procName, 0);

    return ph->n;
}
        


/*--------------------------------------------------------------------------*
 *                               Heap operations                            *
 *--------------------------------------------------------------------------*/
/*!
 *  pheapSwapUp()
 *
 *      Input:  ph (heap)
 *              index (of array corresponding to node to be swapped up)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is called after a new item is put on the heap, at the
 *          bottom of a complete tree.
 *      (2) To regain the heap order, we let it bubble up,
 *          iteratively swapping with its parent, until it either
 *          reaches the root of the heap or it finds a parent that
 *          is in the correct position already vis-a-vis the child.
 */
l_int32
pheapSwapUp(PHEAP   *ph,
            l_int32  index)
{
l_int32    ip;  /* index to heap for parent; 1 larger than array index */
l_int32    ic;  /* index into heap for child */
l_float32  valp, valc;

  PROCNAME("pheapSwapUp");

  if (!ph)
      return ERROR_INT("ph not defined", procName, 1);
  if (index < 0 || index >= ph->n)
      return ERROR_INT("invalid index", procName, 1);

  ic = index + 1;  /* index into heap: add 1 to array index */
  if (ph->direction == L_SORT_INCREASING) {
      while (1) {
          if (ic == 1)  /* root of heap */
              break;
          ip = ic / 2;
          valc = *(l_float32 *)(ph->array[ic - 1]);
          valp = *(l_float32 *)(ph->array[ip - 1]);
          if (valp <= valc)
             break;
          SWAP_ITEMS(ip - 1, ic - 1);
          ic = ip;
      }
  }
  else {  /* ph->direction == L_SORT_DECREASING */
      while (1) {
          if (ic == 1)  /* root of heap */
              break;
          ip = ic / 2;
          valc = *(l_float32 *)(ph->array[ic - 1]);
          valp = *(l_float32 *)(ph->array[ip - 1]);
          if (valp >= valc)
             break;
          SWAP_ITEMS(ip - 1, ic - 1);
          ic = ip;
      }
  }
  return 0;
}


/*!
 *  pheapSwapDown()
 *
 *      Input:  ph (heap)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This is called after an item has been popped off the
 *          root of the heap, and the last item in the heap has
 *          been placed at the root.
 *      (2) To regain the heap order, we let it bubble down,
 *          iteratively swapping with one of its children.  For a
 *          decreasing sort, it swaps with the largest child; for
 *          an increasing sort, the smallest.  This continues until
 *          it either reaches the lowest level in the heap, or the
 *          parent finds that neither child should swap with it
 *          (e.g., for a decreasing heap, the parent is larger
 *          than or equal to both children).
 */
l_int32
pheapSwapDown(PHEAP  *ph)
{
l_int32    ip;  /* index to heap for parent; 1 larger than array index */
l_int32    icr, icl;  /* index into heap for left/right children */
l_float32  valp, valcl, valcr;

  PROCNAME("pheapSwapDown");

  if (!ph)
      return ERROR_INT("ph not defined", procName, 1);
  if (pheapGetCount(ph) < 1)
      return 0;

  ip = 1;  /* index into top of heap: corresponds to array[0] */
  if (ph->direction == L_SORT_INCREASING) {
      while (1) {
          icl = 2 * ip;
          if (icl > ph->n)
             break;
          valp = *(l_float32 *)(ph->array[ip - 1]);
          valcl = *(l_float32 *)(ph->array[icl - 1]);
          icr = icl + 1;
          if (icr > ph->n) {  /* only a left child; no iters below */
              if (valp > valcl)
                  SWAP_ITEMS(ip - 1, icl - 1);
              break;
          }
          else {  /* both children present; swap with the smallest if bigger */
              valcr = *(l_float32 *)(ph->array[icr - 1]);
              if (valp <= valcl && valp <= valcr)  /* smaller than both */
                  break;
              if (valcl <= valcr) {  /* left smaller; swap */
                  SWAP_ITEMS(ip - 1, icl - 1);
                  ip = icl;
              }
              else { /* right smaller; swap */
                  SWAP_ITEMS(ip - 1, icr - 1);
                  ip = icr;
              }
          }
      }
  }
  else {  /* ph->direction == L_SORT_DECREASING */
      while (1) {
          icl = 2 * ip;
          if (icl > ph->n)
             break;
          valp = *(l_float32 *)(ph->array[ip - 1]);
          valcl = *(l_float32 *)(ph->array[icl - 1]);
          icr = icl + 1;
          if (icr > ph->n) {  /* only a left child; no iters below */
              if (valp < valcl)
                  SWAP_ITEMS(ip - 1, icl - 1);
              break;
          }
          else {  /* both children present; swap with the biggest if smaller */
              valcr = *(l_float32 *)(ph->array[icr - 1]);
              if (valp >= valcl && valp >= valcr)  /* bigger than both */
                  break;
              if (valcl >= valcr) {  /* left bigger; swap */
                  SWAP_ITEMS(ip - 1, icl - 1);
                  ip = icl;
              }
              else { /* right bigger; swap */
                  SWAP_ITEMS(ip - 1, icr - 1);
                  ip = icr;
              }
          }
      }
  }

  return 0;
}


/*!
 *  pheapSort()
 *
 *      Input:  ph (heap, with internal array)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This sorts an array into heap order.  If the heap is already
 *          in heap order for the direction given, this has no effect.
 */
l_int32
pheapSort(PHEAP  *ph)
{
l_int32  i;

  PROCNAME("pheapSort");

  if (!ph)
      return ERROR_INT("ph not defined", procName, 1);

  for (i = 0; i < ph->n; i++)
      pheapSwapUp(ph, i);

  return 0;
}


/*!
 *  pheapSortStrictOrder()
 *
 *      Input:  ph (heap, with internal array)
 *      Return: 0 if OK, 1 on error
 *
 *  Notes:
 *      (1) This sorts a heap into strict order.
 *      (2) For each element, starting at the end of the array and
 *          working forward, the element is swapped with the head
 *          element and then allowed to swap down onto a heap of
 *          size reduced by one.  The result is that the heap is
 *          reversed but in strict order.  The array elements are
 *          then reversed to put it in the original order.
 */
l_int32
pheapSortStrictOrder(PHEAP  *ph)
{
l_int32  i, index, size;

  PROCNAME("pheapSortStrictOrder");

  if (!ph)
      return ERROR_INT("ph not defined", procName, 1);

  size = ph->n;  /* save the actual size */
  for (i = 0; i < size; i++) {
      index = size - i;
      SWAP_ITEMS(0, index - 1);
      ph->n--;  /* reduce the apparent heap size by 1 */
      pheapSwapDown(ph);
  }
  ph->n = size;  /* restore the size */

  for (i = 0; i < size / 2; i++)  /* reverse */
      SWAP_ITEMS(i, size - i - 1);

  return 0;
}



/*---------------------------------------------------------------------*
 *                            Debug output                             *
 *---------------------------------------------------------------------*/
/*!
 *  pheapPrint()
 *  
 *      Input:  stream
 *              pheap
 *      Return: 0 if OK; 1 on error
 */
l_int32
pheapPrint(FILE   *fp,
           PHEAP  *ph)
{
l_int32  i;

    PROCNAME("pheapPrint");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!ph)
        return ERROR_INT("ph not defined", procName, 1);

    fprintf(fp, "\n PHeap: nalloc = %d, n = %d, array = %p\n",
            ph->nalloc, ph->n, ph->array);
    for (i = 0; i < ph->n; i++)
        fprintf(fp,   "keyval[%d] = %f\n", i, *(l_float32 *)ph->array[i]);

    return 0;
}

