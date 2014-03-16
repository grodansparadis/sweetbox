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
    File:            mad_port_msst.S
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
        .global  _mad_port_stereo_ms

/* -------------------------------------------------------------------------------------------------
       function: mad_port_stereo_ms

      prototype: void mad_port_stereo_ms (MAD_FIXED *xr0,
                                          MAD_FIXED *xr1,
                                          U08  const *sfbwidth,
                                          SWORD *modes
                                         );

    description: Middle/Side stereo processing

     parameters: W0 = &xr[0][0]
                 W1 = &xr[1][0]
                 W2 = &sfbwidth[0] - MUST BE IN .mad_const PAGE!!!
                 W3 = &modes[0]

         return: no return

   ---------------------------------------------------------------------------------------------- */

_mad_port_stereo_ms:

        push    PSVPAG                                  /* save PSVPAG */
        push    CORCON                                  /* save CORCON */
        push.d  W8


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

        mov     #23170,                 W4              /* W4 = 2^(-0.5) */
        mov     #576,                   W7              /* W7 = loop counter */


        /* === perform calculation === */

ms_stereo_loop:

        mov     [W3++],                 W8              /* W8 = mode     */
        mov.b   [W2++],                 W9              /* W9 = sfbwidth */
        ze      W9,                     W9

        cp      W8,                     #2
        bra     NZ,                     no_ms_stereo    /* skip processing if (mode != 2) */


        dec     W9,                     W8
        do      W8,                     1f

        /*      W0 = &xr[0][]
                W1 = &xr[1][]

                w2 = &sfbwidth[]
                W3 = &modes

                W4 = 2^(-1/2) == 0.70710678118655
        */

        lac     [W0],   #0,             A               /* A = m */
        lac     [W1],   #0,             B               /* B = s */

        add     A                                       /* A  = (m+s) */
        sac.r   A,      #0,             W5              /* save (m+s) */

        lac     [W0],   #0,             A               /* A = m */
        lac     [W1],   #0,             B               /* B = s */

        sub     A                                       /* A  = (m-s) */
        sac.r   A,      #0,             W6              /* save (m-s) */

        mpy     W4*W5,  A                               /* A = (m+s)*2^(-0.5) */
        mpy     W4*W6,  B                               /* B = (m+s)*2^(-0.5) */

        sac.r   A,      #0,             [W0++]
1:      sac.r   B,      #0,             [W1++]

no_ms_stereo:

        sub     W7,     W9,             W7
        bra     NZ,                     ms_stereo_loop  /* and again if loop_counter != 0 */


        pop.d   W8
        pop     CORCON                                  /* restore CORCON */
        pop     PSVPAG                                  /* restore PSVPAG */
        return

        .end

/***************************************************************************************************
    end of file: mad_port_msst.S
 **************************************************************************************************/
