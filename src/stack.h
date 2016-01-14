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
 *  stack.h
 *
 *       Expandable pointer stack for arbitrary void* data.
 *
 *       The stack is an array of void * ptrs, onto which arbitrary
 *       objects can be stored.  At any time, the number of
 *       stored objects is stack->n.  The object at the bottom
 *       of the stack is at array[0]; the object at the top of
 *       the stack is at array[n-1].  New objects are added
 *       to the top of the stack, at the first available location,
 *       which is array[n].  Objects are removed from the top of the
 *       stack.  When an attempt is made to remove an object from an
 *       empty stack, the result is null.   When the stack becomes
 *       filled, so that n = nalloc, the size is doubled.
 *
 *       The auxiliary stack can be used to store and remove
 *       objects for re-use.  It must be created by a separate
 *       call to stackCreate().  [Just imagine the chaos if
 *       stackCreate() created the auxiliary stack!]   
 *       stackDestroy() checks for the auxiliary stack and removes it.
 */

#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED


    /* note that array[n] is the first null ptr in the array */
struct Stack
{
    l_int32        nalloc;       /* size of ptr array              */
    l_int32        n;            /* number of stored elements      */
    void         **array;        /* ptr array                      */
    struct Stack  *auxstack;     /* auxiliary stack                */
};
typedef struct Stack  STACK;



#endif /* STACK_H_INCLUDED */
