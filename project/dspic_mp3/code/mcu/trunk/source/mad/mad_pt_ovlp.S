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
    File:            mad_port_ovlp.S
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
        .global  _mad_port_III_overlap

/* -------------------------------------------------------------------------------------------------
       function: mad_port_III_overlap

      prototype: void mad_port_III_overlap (MAD_FIXED *output,
                                            MAD_FIXED *overlap,
                                            MAD_FIXED *sample,
                                            UWORD sb
                                           );

    description: perform IMDCT z[i] overlapping

     parameters: W0 = &output[0]    -> pointer to array output[36]
                 W1 = &overlap[0]   -> pointer to array overlap[18]
                 W2 = &sample[a][0] -> pointer to array sample[18][32]
                 W3 = sb

         return: no return

   ---------------------------------------------------------------------------------------------- */

_mad_port_III_overlap:

        push    CORCON                                  /* store CORCON */

        mov     #(DSP_MODE_FRACTIONAL   | \
                  DSP_MODE_RND_EN       | \
                  DSP_MODE_PSV_EN       | \
                  DSP_MODE_SAT_SUPER    | \
                  DSP_MODE_DATA_SAT_EN  | \
                  DSP_MODE_ACCA_SAT_EN  | \
                  DSP_MODE_ACCB_SAT_EN  | \
                  DSP_MODE_SIGNED),     W4
        mov     W4,                     CORCON          /* set required dsp-core mode */

        /* === prepare registers for calculation === */


        sl      W3,     #1,             W3              /* for alingment */
        add     W2,     W3,             W2              /* W2 = &sample[a][sb] */

        mov     W0,                     W3
        add     #(18*2),                W3              /* W3 = &output[18]  */

        /* === perform calculation === */

        do      #(18-1),                1f

        lac     [W1],   #0,             A
        add     [W0++], #0,             A
        mov     [W3++],                 [W1++]
        sac.r   A,      #0,             [W2]
1:      add     #(32*2),                W2


        pop     CORCON                                  /* restore CORCON */
        return

        .end

/***************************************************************************************************
    end of file: mad_port_ovlp.S
 **************************************************************************************************/
