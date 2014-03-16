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
    File:            mad_port_requ.S
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

        .extern a_i
        .extern C_n
        .extern exp_fract

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
        .global  _mad_port_III_requantize


/* -------------------------------------------------------------------------------------------------
    function:    mad_port_III_requantize

    prototype:   SWORD mad_port_III_requantize (UWORD value, SWORD exp)

    description: result=(VAL)^(4/3)*2^(EXP), where 0 < VAL < 8207 and -88.5 < EXP < 11.5 (EXP coming
                 in *4 representation, i.e. EXP step is 0.25).

                 Represent this like:
                 (2^n * (1 - x))^(4/3)*2^(EXP)

                 1) found n and x

                 2) calculate (1 - x)^(4/3) by Taylor calculation - 8 step is enough

                 3) take m & C_mantissa by n

                 4) multiply by 2^EXP


    parameters:  VAL - W0
                 EXP - W1

    return:      W0 - result in Q1.15: It is assumed that result < 1 – but in generally it isn’t
                      true! But in average case it is. This is one of source of this mp3 decoder
                      uncertainty
   ---------------------------------------------------------------------------------------------- */

_mad_port_III_requantize:

        push    PSVPAG                                  /* store PSVPAG */
        push    CORCON                                  /* store CORCON */
        push    W8

        mov     #(DSP_MODE_FRACTIONAL   | \
                  DSP_MODE_RND_EN       | \
                  DSP_MODE_PSV_EN       | \
                  DSP_MODE_SAT_SUPER    | \
                  DSP_MODE_DATA_SAT_EN  | \
                  DSP_MODE_ACCA_SAT_DIS | \
                  DSP_MODE_ACCB_SAT_DIS | \
                  DSP_MODE_SIGNED),     W8
        mov     W8,                     CORCON          /* set required dsp-core mode */

        mov     #psvpage(mad_const_psv),W8
        mov     W8,                     PSVPAG          /* set required PSV page */

        /* === perform calculation === */

        /* 1) calculate n and x */

        fbcl    W0,                     W2
        neg     W2,                     W2              /* W2 =  n */

        sl      W0,     W2,             W0
        neg     W0,                     W0              /* W0 = -(1 - x) (Q1.15) */

        mov     #0x7FFF,                W3              /* W3 = 1 (Q1.15) */
        add     W0,     W3,             W0              /* W0 = x (Q1.15) */
        inc     W0,                     W0              /* because 0x7FFF is not exactly 1 (Q1.15) */


        /* 2) calculate (1 - x)^(4/3) by Taylor calculation */

        mov     #psvoffset(a_i)+8*2,    W8              /* W8 = &a[8] */

        mov     [W8--],                 W4              /* W4 = a[8]  */
        mov     W0,                     W5              /* W5 = x     */

        do      #(8-1-1),               1f              /* Taylor cycle */

        mpy     W4*W5,  A, [W8]-=2,     W4              /* A  = a[i]*x,  W8 = &a[i-1]  */
        add     W4,     A                               /* A  = a[i]*x + a[i-1]        */
1:      sac.r   A,      #0,             W4              /* W4 = round(a[i]*x + a[i-1]) */

        inc2    W8,                     W8              /* W8  = a[1] */
        mpy     W4*W5,  A, [W8]-=2,     W4              /* A  *= x    */
        mac     W4*W5,  A, [W8],        W4              /* A  *= x - twice, because of |a[1]| > 1 */
        add     W4,     A                               /* A += a[0] */

        sac.r   A,      #0,             W3
        inc     W3,                     W3              /* A += 1, because a[0] = 0x7fff - it is not exactly 1 (Q1.15) */


        /* 3) take m & C_mantissa by n */

        mov     #psvoffset(C_n),        W4              /* W4  = &C[0][0]          */

        subr    W2,     #(16-1),        W6              /* W6  = (16 - n)          */
        sl      W6,     #2,             W6              /* W6 *= 4 for alingment   */
        add     W4,     W6,             W4              /* W4  = &C[16-n]          */
        mov     [W4++],                 W7              /* W7  = C[W4]   - it's m! */
        and     W7,     #3,             W5              /* W5  &= 3;               */

        bra     Z,                      1f              /* if !(m & 3), skip ....  */

        mov     W3,                     W5              /* W5  = (1-x)^(4/3)       */
        mov     [W4],                   W6
        mpy     W5*W6,  A                               /* A  = C_mantissa*(1-x)^4/3 */
        sac.r   A,      #0,             W3              /* W3 = round(C_mantissa*(1-x)^4/3) */


1:      /* 4) multiply by 2^EXP */

        mov     #psvoffset(exp_fract),  W4              /* W4 = &exp_fract[0]      */

        and     W1,     #0x0003,        W5              /* W5  = EXP & 3           */
        sl      W5,     #1,             W5              /* W5 *= 2 for alingment   */
        add     W4,     W5,             W4              /* W4  = &exp_fract[EXP&3] */
        asr     W5,     #1,             W5              /* W5  = EXP & 3           */

        cp0     W1
        bra     GE,                     1f              /* if (EXP < 0)            */
        neg     W1,                     W6              /* W6 = -EXP  - (assumed, that EXP < 0 always!! may be reduce code size?) */
1:
        asr     W6,     #2,             W6              /* EXP /= 4, because of input is (* 4) */
        sub     W7,     W6,             W6              /* W6  = (m - EXP) */


        cp0     W5                                      /* if ((EXP & 3) == 0), skip ... */
        bra     Z,                      1f

        cp0     W1                                      /* if (EXP > 0), skip ... */
        bra     LE,                     2f
        inc     W6,                     W6              /* W6 = 1 + (m - EXP) */

2:      mov     [W4],                   W7
        mov     W3,                     W4
        mpy     W4*W7,  A                               /* A = 2^exp_fract*C_mantissa*(1-x)^(4/3) */

1:      neg     W6,                     W6              /* W6 = (EXP - 1 - m) */
        cp      W6,                     #16
        bra     N,                      3f
        mov     #16,                    W6

3:      sftac   A,                      W6              /* W6 = ACCA >> W6    */
        sac.r   A,                      W0              /* return result */


        pop     W8
        pop     CORCON                                  /* restore CORCON */
        pop     PSVPAG                                  /* restore PSVPAG */
        return


        .end

/***************************************************************************************************
    end of file: mad_port_requ.S
 **************************************************************************************************/
