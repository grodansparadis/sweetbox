/***************************************************************************************************
    Project:
    Project desc.:   Software MP3 decoder for dsPIC33
    Author:

 ***************************************************************************************************
    Distribution:

    libmad - MPEG audio decoder library
    -----------------------------------

    Copyright (C) 2000-2004 Underbit Technologies, Inc.

    This program is free software; you can redistribute it and/or modify it under the terms of the
    GNU General Public License as published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with this program; if
    not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
    MA 02111-1307 USA

 ***************************************************************************************************
    Compiled Using:  Microchip C30 V.3.1
    Processor:       dsPIC33 Family

 **************************************************************************************************
    File:            mad_port_im36.S
    Description:

 ***************************************************************************************************
    History:         2007/05/31 [GS]  File created


 **************************************************************************************************/

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

#include "mad_pt_defs.h"

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

        #define X_PT            W10

/***************************************************************************************************
    Private Types
 **************************************************************************************************/

/***************************************************************************************************
    Private Data
 **************************************************************************************************/

/***************************************************************************************************
    Public Data
 **************************************************************************************************/

        .extern mad_const_psv

        .extern imdct_l_coeff
        .extern window_l_w_b0
        .extern window_l_w_b1
        .extern window_l_w_b3

/***************************************************************************************************
    Private Function Prototypes
 **************************************************************************************************/

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

/***************************************************************************************************
    Public Functions
 **************************************************************************************************/

        .section .text, code
        .global  _mad_port_III_imdct_36_w


/* -------------------------------------------------------------------------------------------------
    function:    mad_port_III_imdct_36_w

    prototype:   void mad_port_III_imdct_36_w (MAD_FIXED *X, MAD_FIXED *x, UWORD blockType);

    description: X[18] -> x[36] IMDCT
                 x[36] = x[36]*w[36]

                 x[i]=[SUM_by_k(X[k]*imdct_l_coeff[i][k])]*w[i]


    parameters:  W0 - pointer to X[18]
                 W1 - pointer to x[36]
                 W2 - blockType (0, 1, 3)

    return:

   ---------------------------------------------------------------------------------------------- */

_mad_port_III_imdct_36_w:

        push    PSVPAG
        push    CORCON
        push.d  W8
        push.d  W10

        /* prepare dsp-core and PSV */

        mov     #(DSP_MODE_FRACTIONAL   | \
                  DSP_MODE_RND_DIS      | \
                  DSP_MODE_PSV_EN       | \
                  DSP_MODE_SAT_SUPER    | \
                  DSP_MODE_DATA_SAT_EN  | \
                  DSP_MODE_ACCA_SAT_DIS | \
                  DSP_MODE_ACCB_SAT_DIS | \
                  DSP_MODE_SIGNED),     W8
        mov     W8,                     CORCON          /* set required dsp-core mode */

        mov     #psvpage(mad_const_psv),W8
        mov     W8,                     PSVPAG          /* set required PSV page */


        /* === prepare registers for calculation === */

        mov     #psvoffset(window_l_w_b0), W7           /* assume blockType = 0 */

        cp0     W2
        bra     Z,                      1f              /* go to start if blockType = 0 */

        cp      W2,                     #1
        bra     NZ,                     2f

        mov     #psvoffset(window_l_w_b1), W7           /* assume blockType = 1 and goto start */
        bra     1f

2:      mov     #psvoffset(window_l_w_b3), W7           /* blockType = 3 */


1:      mov     #psvoffset(imdct_l_coeff), W2           /* W2 = &coeff[0][0] */

        mov     W2,                     W3
        add     #(18*5*2),              W3              /* W3 = &coeff[1][0] */

        mov     W0,                     X_PT            /* save &X[0] */

        mov     W1,                     W0
        add     #(17*2),                W1


        /* === perform calculation === */

        /*
           W2 = &coeff[0][i]
           W5 =  coeff[0][i]

           W3 = &coeff[1][i]
           W6 =  coeff[1][i]

           W8 = &X[i]
           W4 =  X[i]

           W7 = &window

           W0 = &x[i]
           W1 = &x[i+17]

        */

        /*  i = [0:7]  */

        do      #(4-1),                 1f

        mov     X_PT,                   W8
        mov     [W8],                   W4              /* W4 = X[0] */

        clr     A
        clr     B

        do      #(18-1),                2f

        mov     [W2++],                 W5              /* W5 = coeff[0][] */
        mov     [W3++],                 W6              /* W5 = coeff[1][] */

        mac     W4*W5, A, [W8]+=2, W4
2:      mac     W4*W6, B, [W8],    W4

        DSP_A_B_SAT_EN

        mov     W7,                     W8
        mov     [W8++],                 W4              /* W4 = window[] */

        sac.r   A,      #0,             W5
        sac.r   B,      #0,             W6

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        mpy     W4*W6, B, [W8]+=2, W4                   /* windowing */

        sac.r   A,      #0,             [W0++]
        sac.r   B,      #0,             [W0++]

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        mpy     W4*W6, B, [W8]+=2, W4                   /* windowing */

        sac.r   A,      #0,             [W1--]
        sac.r   B,      #0,             [W1--]

        add     #(4*2),                 W7              /* save window[] */

        DSP_A_B_SAT_DIS
1:      nop


        /*   i = [8]   */

        mov     X_PT,                   W8
        mov     [W8++],                 W4              /* W4 = X[0] */

        clr     A

        do      #(18-1),                1f
        mov     [W2++],                 W5              /* W5 = coeff[8][] */
1:      mac     W4*W5, A, [W8]+=2, W4

        mov     W7,                     W8
        mov     [W8++],                 W4              /* W4 = window[] */
        sac.r   A,      #0,             W5

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        sac.r   A,      #0,             [W0]

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        sac.r   A,      #0,             [W1]

        add     #(2*2),                 W7              /* save window[] */


        /* i = [18:25] */

        add     #(18*4*2),              W2              /* W2 = &coeff[18][0] */
        add     #(18*5*2),              W3              /* W2 = &coeff[19][0] */
        add     #(10*2),                W0              /* W0 = &X[18]        */
        add     #(26*2),                W1              /* W0 = &X[35]        */


        do      #(4-1),                 1f

        mov     X_PT,                   W8
        mov     [W8],                   W4              /* W4 = X[0] */

        clr     A
        clr     B

        do      #(18-1),                2f

        mov     [W2++],                 W5              ; coeff[18][]
        mov     [W3++],                 W6              ; coeff[19][]

        mac     W4*W5, A, [W8]+=2, W4
2:      mac     W4*W6, B, [W8],    W4

        DSP_A_B_SAT_EN

        mov     W7,                     W8
        mov     [W8++],                 W4              ; now W4 = window[i]

        sac.r   A,      #0,             W5
        sac.r   B,      #0,             W6

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        mpy     W4*W6, B, [W8]+=2, W4                   /* windowing */

        sac.r   A,      #0,             [W0++]
        sac.r   B,      #0,             [W0++]

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        mpy     W4*W6, B, [W8]+=2, W4                   /* windowing */

        sac.r   A,      #0,             [W1--]
        sac.r   B,      #0,             [W1--]

        add     #(4*2),                 W7              /* save window[] */

        DSP_A_B_SAT_DIS
1:      nop


        /*   i = [26]  */

        mov     X_PT,                   W8
        mov     [W8++],                 W4              /* W4 = X[0] */

        clr     A

        do      #(18-1),                1f
        mov     [W2++],                 W5              /* W5 = coeff[26][] */
1:      mac     W4*W5, A, [W8]+=2, W4

        mov     W7,                     W8
        mov     [W8++],                 W4              /* W4 = window[] */

        sac.r   A,      #0,             W5

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        sac.r   A,      #0,             [W0]

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        sac.r   A,      #0,             [W1]




        pop.d   W10
        pop.d   W8
        pop     CORCON                                  /* restore CORCON */
        pop     PSVPAG                                  /* restore PSVPAG */
        return

        .end

/***************************************************************************************************
    end of file: mad_port_im36.S
 **************************************************************************************************/
