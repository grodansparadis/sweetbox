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
    File:            mad_port_inls.S
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
        .global  _mad_port_stereo_intensity_lsf

/* -------------------------------------------------------------------------------------------------
       function: mad_port_stereo_intensity_lsf - not work correctly now

      prototype: void mad_port_stereo_intensity_lsf (MAD_FIXED       *xr0,
                                                     MAD_FIXED       *xr1,
                                                     U08 const       *sfbwidth,
                                                     SWORD           *modes,
                                                     UWORD           *scalefac,
                                                     MAD_FIXED const *lsf_scale,
                                                     UWORD           *illegal_pos
                                                    );

    description: Intensity stereo processing - LSF EXT


                 for (sfbi = l = 0; l < 576; ++sfbi, l += n)
                 {
                     n = sfbwidth[sfbi];
                
                     if (!(modes[sfbi] & i_stereo))
                         continue;
                
                     if (illegal_pos[sfbi])
                     {
                         modes[sfbi] &= ~i_stereo;
                         continue;
                     }
                
                     is_pos = right_ch->scalefac[sfbi];
                
                     for (i = 0; i < n; ++i)
                     {
                         register mad_fixed_t left;
                
                         left = xr[0][l + i];
                
                         if (is_pos == 0)
                             xr[1][l + i] = left;
                         else
                         {
                             register mad_fixed_t opposite;
                
                             opposite = mad_f_mul(left, lsf_scale[(is_pos - 1) / 2]);
                
                             if (is_pos & 1)
                             {
                                 xr[0][l + i] = opposite;
                                 xr[1][l + i] = left;
                             }
                             else
                                 xr[1][l + i] = opposite;
                         }
                     }
                 }


     parameters: W0 = &xr[0][0]
                 W1 = &xr[1][0]
                 W2 = &sfbwidth[0]
                 W3 = &modes[0]
                 W4 = &scalefac[0]
                 W5 = &lsf_scale[0]
                 W6 = &illegal_pos[0]

           used:

         return: no return

   ---------------------------------------------------------------------------------------------- */

_mad_port_stereo_intensity_lsf:

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

        mov     #576,                   W7              /* W7 = loop counter */

        /* === perform calculation === */

        /*      W0 = &xr[0][]
                W1 = &xr[1][]
                W2 = &sfbwidth[]
                W3 = &modes[]
                W4 = &scalefac[]
                W5 = &lsf_scale[]
                W6 = &illegal_pos[]

                W7 = l-counter

                w8 = mode[]

                W9 = (is_pos-1)/2

                W10 = n
                W11 = is_pos

                W13 - temp variable
        */

intensity_stereo_lsf_loop:

        mov     [W3++],                 W8      /* W8  = mode[] */
        mov.b   [W2++],                 W10     /* W10 = n      */
        ze      W10,                    W10
        mov     [W4++],                 W11     /* W11 = is_pos */

        dec     W11,                    W9
        asr     W9,     #1,             W9      /* !!!! may be not need, but alingment =( */
        sl      W9,     #1,             W9      /* W9 = (is_pos - 1)/2 */

        dec     W8,                     W8
        mov     [W6++],                 W13
        cp0     W13
        bra     NZ,                     no_intensity_lsf_stereo         /* skip processing if (illegal_pos[] != 0) */

        cp0     W8
        bra     NZ,                     no_intensity_lsf_stereo         /* skip processing if (modes[] != 1) */

        dec     W10,                    W13
        do      W13,                    1f

        lac     [W0],   #0,             A
        lac     [W0],   #0,             B

        cp0     W11
        bra     Z,                      2f      /* skip if (is_pos == 0) */

        push.d  W4                              /* save W4:W5 */

        mov     [W0],                   W4
        mov     [W5+W9],                W5

        mpy     W4*W5,  B

        pop.d   W4                              /* restore W4:W5 */

        btss    W11,                    #0      /* if (is_pos & 1)... */
        bra                             2f

        sac.r   B,      #0,             [W0++]
        sac.r   A,      #0,             [W1++]
        bra                             1f

2:      sac.r   A,      #0,             [W0++]
        sac.r   B,      #0,             [W1++]

1:      nop

        bra                             1f

no_intensity_lsf_stereo:

        sl      W10,    #1,             W10
        add     W0,     W10,            W0      /* modify pointers */
        add     W1,     W10,            W1
        asr     W10,    #1,             W10

1:      sub     W7,     W10,            W7
        bra     NZ,                     intensity_stereo_lsf_loop       /* and again, if (loop_counter != 0) */


        pop.d   W12
        pop.d   W10
        pop.d   W8
        pop     CORCON                          /* restore CORCON */
        pop     PSVPAG                          /* restore PSVPAG */
        return

        .end

/***************************************************************************************************
    end of file: mad_port_inls.S
 **************************************************************************************************/
