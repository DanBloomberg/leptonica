/*====================================================================*
 -  Copyright (C) 2001 Leptonica.  All rights reserved.
 -
 -  Redistribution and use in source and binary forms, with or without
 -  modification, are permitted provided that the following conditions
 -  are met:
 -  1. Redistributions of source code must retain the above copyright
 -     notice, this list of conditions and the following disclaimer.
 -  2. Redistributions in binary form must reproduce the above
 -     copyright notice, this list of conditions and the following
 -     disclaimer in the documentation and/or other materials
 -     provided with the distribution.
 -
 -  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 -  ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 -  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 -  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL ANY
 -  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 -  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 -  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 -  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 -  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 -  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 -  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *====================================================================*/

#ifndef  LEPTONICA_CCBORD_H
#define  LEPTONICA_CCBORD_H

/*!
 * \file ccbord.h
 *
 * <pre>
 *           CCBord:   represents a single connected component
 *           CCBorda:  an array of CCBord
 * </pre>
 */

    /*! Use in ccbaStepChainsToPixCoords() */
/*! CCB Coords */
enum {
      CCB_LOCAL_COORDS = 1,
      CCB_GLOBAL_COORDS = 2
};

    /*! Use in ccbaGenerateSPGlobalLocs() */
/*! CCB Points */
enum {
      CCB_SAVE_ALL_PTS = 1,
      CCB_SAVE_TURNING_PTS = 2
};


typedef struct CCBord CCBORD;

/*! Array of CCBord */
struct CCBorda
{
    struct Pix          *pix;          /*!< input pix (may be null)          */
    l_int32              w;            /*!< width of pix                     */
    l_int32              h;            /*!< height of pix                    */
    l_int32              n;            /*!< number of ccbord in ptr array    */
    l_int32              nalloc;       /*!< number of ccbord ptrs allocated  */
    struct CCBord      **ccb;          /*!< ccb ptr array                    */
};
typedef struct CCBorda CCBORDA;


#endif  /* LEPTONICA_CCBORD_H */

