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
    File:            mad_port_inst.S
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

        .extern is_table

/***************************************************************************************************
    Private Function Prototypes
 **************************************************************************************************/

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

/***************************************************************************************************
    Public Functions
 **************************************************************************************************/

        .global  _mad_port_stereo_intensity
        .section .text, code

/* -------------------------------------------------------------------------------------------------
       function: mad_port_stereo_intensity

      prototype: void mad_port_stereo_intensity (MAD_FIXED *xr0,
                                                 MAD_FIXED *xr1,
                                                 U08 const *sfbwidth,
                                                 SWORD     *modes,
                                                 UWORD     *scalefac
                                                );

    description: Intensity stereo processing - no LSF EXT

     parameters: W0 = &xr[0][0]
                 W1 = &xr[1][0]
                 W2 = &sfbwidth[0]
                 W3 = &modes[0]
                 W4 = &scalefac[0]

           used:

         return: no return

   ---------------------------------------------------------------------------------------------- */

_mad_port_stereo_intensity:

        push    PSVPAG                                  /* save PSVPAG */
        push    CORCON                                  /* save CORCON */
        push.d  W8
        push.d  W10
        push.d  W12

        /* prepare dsp-core and PSV */

        mov     #(DSP_MODE_FRACTIONAL   | \
                  DSP_MODE_RND_EN       | \
                  DSP_MODE_PSV_EN       | \
                  DSP_MODE_SAT_SUPER    | \
                  DSP_MODE_DATA_SAT_EN  | \
                  DSP_MODE_ACCA_SAT_EN  | \
                  DSP_MODE_ACCB_SAT_EN  | \
                  DSP_MODE_SIGNED),     W8
        mov     W8,                     CORCON          /* set required dsp-core mode */

        mov     #psvpage(mad_const_psv),W8
        mov     W8,                     PSVPAG          /* set required PSV page */

        /* === prepare registers for calculation === */


        mov     #576,                   W7              /* W7 = loop_counter */

        mov     #psvoffset(is_table),   W8
        mov     W8,                     W9              /* W9 = &is_table */


        /* === perform calculation === */

intensity_stereo_loop:

        mov     [W3++],                 W8              /* W8  = mode[] */
        mov.b   [W2++],                 W10             /* W10 = n      */
        ze      W10,                    W10

        mov     [W4++],                 W11
        sl      W11,    #1,             W11             /* W11 = is_pos */


        dec     W8,                     W8
        bra     NZ,                     no_intensity_stereo     /* skip processing if (mode != 1) */

        subr    W11,    #(6*2),         W12
        bra     N,                      no_intensity_stereo     /* skip processing if (is_pos < 6) */

        push    W4                                      /* save &scalefac[0] */

        mov     [W9+W11],               W5              /* W5 = is_table[is_pos]     */
        mov     [W9+W12],               W6              /* W6 = is_table[6 - is_pos] */


        dec     W10,                    W13
        do      W13,                    1f

        /* W4  = left_chan
           W5  = is_table[is_pos]
           W6  = is_table[6 - is_pos]

           W9  = &is_table

           W11 = is_pos
           W12 = 6 - is_pos
        */

        mov     [W0],                   W4              /* W4 = left_chan */

        mpy     W4*W5,  A                               /* A = left_ch * is_table[is_pos]     */
        mpy     W4*W6,  B                               /* B = left_ch * is_table[6 - is_pos] */

        sac.r   A,      #0,             [W0++]
1:      sac.r   B,      #0,             [W1++]

        pop     W4                                      /* restore &scalefac[0] */

no_intensity_stereo:

        cp0     W8
        bra     Z,                      1f

        sl      W10,    #1,             W10
        add     W0,     W10,            W0              /* modify pointers */
        add     W1,     W10,            W1
        asr     W10,    #1,             W10

1:      sub     W7,     W10,            W7
        bra     NZ,                     intensity_stereo_loop   /* and again, if (loop_counter != 0) */



        pop.d   W12
        pop.d   W10
        pop.d   W8
        pop     CORCON                                  /* restore CORCON */
        pop     PSVPAG                                  /* restore PSVPAG */
        return

        .end

/***************************************************************************************************
    end of file: mad_port_inst.S
 **************************************************************************************************/
