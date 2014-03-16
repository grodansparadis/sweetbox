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
        #define WIND_PT         W7

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

        .extern imdct_s_coeff
        .extern window_s

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
        .global  _mad_port_III_imdct_12_w


/* -------------------------------------------------------------------------------------------------
    function:    mad_port_III_imdct_12_w

    prototype:   void mad_port_III_imdct_12_w (MAD_FIXED *X, MAD_FIXED *x);

    description: x[i] = SUM_by_k(X[k]*imdct_l_coeff[i][k])

                X[18] -> x[36] IMDCT
                y[36] =  x[36]*w[36]
                z[36] = overlap y[36]

    parameters:  W0 - pointer to X[18]
                 W1 - pointer to x[36]

        used:

    return:

   ---------------------------------------------------------------------------------------------- */

_mad_port_III_imdct_12_w:

        push    PSVPAG                                  /* save PSVPAG */
        push    CORCON                                  /* save CORCON */
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

        mov     #psvoffset(window_s),   WIND_PT
        mov     #psvoffset(imdct_s_coeff), W2           /* W2 = &coeff[0][0]; */

        mov     W2,                     W3
        add     #(1*6*2),               W3              /* W3 = &coeff[1][0]; */

        mov     W0,                     X_PT            /* save &X[0] */

        mov     W1,                     W0              /* W0 = &x[0] */
        add     #(5*2),                 W1              /* W1 = &x[5] */


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
           W1 = &x[i+5]

        */

        do      #(3-1),                 1f              /* 3 times */


        /* === i = [0:1] === */

        mov     X_PT,                   W8
        mov     [W8],                   W4              /* W4 = X[0] */

        clr     A
        clr     B

        do      #(6-1),                 2f

        mov     [W2++],                 W5              /* W5 = coeff[0][] */
        mov     [W3++],                 W6              /* W6 = coeff[1][] */

        mac     W4*W5, A, [W8]+=2, W4
2:      mac     W4*W6, B, [W8],    W4

        DSP_A_B_SAT_EN

        mov     WIND_PT,                W8              /* W8 = &window  */
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

        add     #(4*2),                 WIND_PT         /* save window[] */

        DSP_A_B_SAT_DIS


        /* === i = [2] === */

        mov     X_PT,                   W8
        mov     [W8++],                 W4              /* W4 = X[0] */

        clr     A

        do      #(6-1),                 2f
        mov     [W3++],                 W5              /* W6 = coeff[2][] */
2:      mac     W4*W5, A, [W8]+=2, W4

        mov     WIND_PT,                W8
        mov     [W8++],                 W4              /* W4 = window[] */

        sac.r   A,      #0,             W5

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        sac.r   A,      #0,             [W0]

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        sac.r   A,      #0,             [W1]

        add     #(2*2),                 WIND_PT         /* save window[] */


        /* === i = [6:7] === */

        add     #(6*3*2),               W2              /* W2 = &coeff[7][0] */
        add     #(4*2),                 W0              /* W0 = &x[6]        */
        add     #(8*2),                 W1              /* W0 = &x[11]       */

        mov     X_PT,                   W8
        mov     [W8],                   W4              /* W4 = X[0] */

        clr     A
        clr     B

        do      #(6-1),                 2f

        mov     [W2++],                 W5              /* W5 = coeff[7][] */
        mov     [W3++],                 W6              /* W6 = coeff[6][] */

        mac     W4*W5, A, [W8]+=2, W4
2:      mac     W4*W6, B, [W8],    W4

        DSP_A_B_SAT_EN

        mov     WIND_PT,                W8              /* W8 = &window  */
        mov     [W8++],                 W4              /* W4 = window[] */

        sac.r   A,      #0,             W5
        sac.r   B,      #0,             W6

        mpy     W4*W6, B, [W8]+=2, W4                   /* windowing */
        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */

        sac.r   B,      #0,             [W0++]
        sac.r   A,      #0,             [W0++]

        mpy     W4*W6, B, [W8]+=2, W4                   /* windowing */
        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */

        sac.r   B,      #0,             [W1--]
        sac.r   A,      #0,             [W1--]

        add     #(4*2),                 WIND_PT         /* save window[] */

        DSP_A_B_SAT_DIS


        /* === i = [8] === */

        mov     X_PT,                   W8
        mov     [W8++],                 W4              /* W4 = X[0] */

        clr     A

        do      #(6-1),                 2f
        mov     [W2++],                 W5              /* W6 = coeff[8][] */
2:      mac     W4*W5, A, [W8]+=2, W4

        mov     WIND_PT,                W8
        mov     [W8++],                 W4              /* W4 = window[] */

        sac.r   A,      #0,             W5

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        sac.r   A,      #0,             [W0]

        mpy     W4*W5, A, [W8]+=2, W4                   /* windowing */
        sac.r   A,      #0,             [W1]

        add     #(2*2),                 WIND_PT         /* save window[] */


        /* prepare pointers for next loop */

        add     #(6*2),                 X_PT            ; W10 = &X[i + 6]

        sub     #(6*6*2),               W2
        sub     #(3*6*2),               W3
        sub     #(12*2),                WIND_PT

        add     #(4*2),                 W0
1:      add     #((3+5)*2),             W1


        /* === perform overlaping === */

        DSP_A_B_SAT_EN

        mov     W1,                     W2
        sub     #((36+5-6)*2),          W1              /* W1 = &x[6]  = &y1[6] */
        sub     #((36-12)*2),           W0              /* W0 = &x[12] = &y2[0] */
        sub     #((36+5-24)*2),         W2              /* W2 = &x[24] = &y3[0] */

        /* z[12:17] */

        do      #(6-1),                 1f
        lac     [W0],   #0,             A
        add     [W1++], #0,             A
1:      sac.r   A,      #0,             [W0++]

        /* z[18:23] */

        do      #(6-1),                 1f
        lac     [W0],   #0,             A
        add     [W2++], #0,             A
1:      sac.r   A,      #0,             [W0++]

        /* z[24:29] */

        repeat  #(6-1)
        mov     [W2++],                 [W0++]

        /* z[30:35] */

        repeat  #(6-1)
        clr                             [W0++]

        /* z[6:11] */

        sub     #(12*2),                W1
        sub     #(30*2),                W0
        repeat  #(6-1)
        mov     [W1++],                 [W0++]

        /* z[0:5] */

        sub     #(12*2),                W0
        repeat  #(6-1)
        clr                             [W0++]


        pop.d   W10
        pop.d   W8
        pop     CORCON                                  /* restore CORCON */
        pop     PSVPAG                                  /* restore PSVPAG */
        return

        .end

/***************************************************************************************************
    end of file: mad_port_im36.S
 **************************************************************************************************/

