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
 *      The stack is an array of void * ptrs, onto which
 *      objects can be stored.  At any time, the number of
 *      stored objects is stack->n.  The object at the bottom
 *      of the stack is at array[0]; the object at the top of
 *      the stack is at array[n-1].  New objects are added
 *      to the top of the stack; i.e., the first available 
 *      location, which is at array[n].  The stack is expanded
 *      by doubling, when needed.  Objects are removed
 *      from the top of the stack.  When an attempt is made
 *      to remove an object from an empty stack, the result is null.
 *
 *      Create/Destroy
 *           STACK     *stackCreate()
 *           void       stackDestroy()
 *
 *      Accessors
 *           l_int32    stackAdd()
 *           void      *stackRemove()
 *           l_int32    stackExtendArray()
 *           l_int32    stackGetCount()
 *
 *      Text description
 *           l_int32    stackPrint()
 */

#include <stdio.h>
#include <stdlib.h>
#include "allheaders.h"

static const l_int32  INITIAL_PTR_ARRAYSIZE = 20;


/*---------------------------------------------------------------------*
 *                          Create/Destroy                             *
 *---------------------------------------------------------------------*/
/*!
 *  stackCreate()
 *
 *      Input:  nalloc (initial ptr array size; use 0 for default)
 *      Return: stack, or null on error
 */
STACK *
stackCreate(l_int32  nalloc)
{
STACK  *stack;

    PROCNAME("stackCreate");

    if (nalloc <= 0)
        nalloc = INITIAL_PTR_ARRAYSIZE;

    if ((stack = (STACK *)CALLOC(1, sizeof(STACK))) == NULL)
        return (STACK *)ERROR_PTR("stack not made", procName, NULL);

    if ((stack->array = (void **)CALLOC(nalloc, sizeof(void *))) == NULL)
        return (STACK *)ERROR_PTR("stack ptr array not made", procName, NULL);

    stack->nalloc = nalloc;
    stack->n = 0;
    
    return stack;
}


/*!
 *  stackDestroy()
 *
 *      Input:  &stack (<to be nulled>)
 *              freeflag (TRUE to free each remaining struct in the array)
 *      Return: void
 *
 *  Action: If freeflag is TRUE, frees each struct in the array.
 *          If freeflag is FALSE but there are elements on the array,
 *            gives a warning and destroys the array.  This will
 *            cause a memory leak of all the items that were on the stack.
 *            So if the items require their own destroy function, they
 *            must be destroyed before the stack.
 *          To destroy the Stack, we destroy the ptr array, then
 *            the stack, and then null the contents of the input ptr.
 */
void
stackDestroy(STACK  **pstack,
             l_int32  freeflag)
{
void   *item;
STACK  *stack;

    PROCNAME("stackDestroy");

    if (pstack == NULL) {
        L_WARNING("ptr address is NULL", procName);
        return;
    }
    if ((stack = *pstack) == NULL)
        return;

    if (freeflag) {
        while(stack->n > 0) {
            item = stackRemove(stack);
            FREE(item);
        }
    }
    else if (stack->n > 0)
        L_WARNING_INT("memory leak of %d items in stack!", procName, stack->n);

    if (stack->auxstack)
        stackDestroy(&stack->auxstack, freeflag);

    if (stack->array)
        FREE((void *)stack->array);
    FREE((void *)stack);
    *pstack = NULL;
}



/*---------------------------------------------------------------------*
 *                               Accessors                             *
 *---------------------------------------------------------------------*/
/*!
 *  stackAdd()
 *
 *      Input:  stack
 *              item to be added to the stack
 *      Return: 0 if OK; 1 on error.
 */
l_int32
stackAdd(STACK  *stack,
         void   *item)
{
    PROCNAME("stackAdd");

    if (!stack)
        return ERROR_INT("stack not defined", procName, 1);
    if (!item)
        return ERROR_INT("item not defined", procName, 1);

        /* do we need to extend the array? */
    if (stack->n >= stack->nalloc)
        stackExtendArray(stack);

        /* store the new pointer */
    stack->array[stack->n] = (void *)item;
    stack->n++;

    return 0;
}


/*!
 *  stackRemove()
 *
 *      Input:  stack 
 *      Return: ptr to item popped from the top of the stack,
 *              or null if the stack is empty or on error
 */
void *
stackRemove(STACK  *stack)
{
void  *item;

    PROCNAME("stackRemove");

    if (!stack)
        return ERROR_PTR("stack not defined", procName, NULL);

    if (stack->n == 0)
        return NULL;

    stack->n--;
    item = stack->array[stack->n];
        
    return item;
}


/*!
 *  stackExtendArray()
 *
 *      Input:  stack
 *      Return: 0 if OK; 1 on error
 */
l_int32
stackExtendArray(STACK  *stack)
{
    PROCNAME("stackExtendArray");

    if (!stack)
        return ERROR_INT("stack not defined", procName, 1);

    if ((stack->array = (void **)reallocNew((void **)&stack->array,
                              sizeof(l_intptr_t) * stack->nalloc,
                              2 * sizeof(l_intptr_t) * stack->nalloc)) == NULL)
        return ERROR_INT("new stack array not defined", procName, 1);

    stack->nalloc = 2 * stack->nalloc;
    return 0;
}


/*!
 *  stackGetCount()
 *
 *      Input:  stack
 *      Return: count, or 0 on error
 */
l_int32
stackGetCount(STACK  *stack)
{
    PROCNAME("stackGetCount");

    if (!stack)
        return ERROR_INT("stack not defined", procName, 1);

    return stack->n;
}



/*---------------------------------------------------------------------*
 *                            Debug output                             *
 *---------------------------------------------------------------------*/
/*!
 *  stackPrint()
 *
 *      Input:  stream
 *              stack
 *      Return: 0 if OK; 1 on error
 */
l_int32
stackPrint(FILE   *fp,
           STACK  *stack)
{
l_int32  i;

    PROCNAME("stackPrint");

    if (!fp)
        return ERROR_INT("stream not defined", procName, 1);
    if (!stack)
        return ERROR_INT("stack not defined", procName, 1);

    fprintf(fp, "\n Stack: nalloc = %d, n = %d, array = %p\n", 
            stack->nalloc, stack->n, stack->array);
    for (i = 0; i < stack->n; i++)
        fprintf(fp,   "array[%d] = %p\n", i, stack->array[i]);
    
    return 0;
}

