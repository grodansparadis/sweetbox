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
    File:            mad_port_ovlz.S
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
        .global  _mad_port_III_overlap_z

/* -------------------------------------------------------------------------------------------------
       function: mad_port_III_overlap_z

      prototype: void mad_port_III_overlap_Z (MAD_FIXED *overlap,
                                              MAD_FIXED *sample,
                                              UWORD sb
                                             );

    description: perform IMDCT z[i] overlapping

     parameters: W0 = &overlap[0]   -> pointer to array overlap[18]
                 W1 = &sample[a][0] -> pointer to array sample[18][32]
                 W2 = sb

         return: no return

   ---------------------------------------------------------------------------------------------- */

_mad_port_III_overlap_z:

        push    CORCON                                  /* store CORCON */

        mov     #(DSP_MODE_FRACTIONAL   | \
                  DSP_MODE_RND_EN       | \
                  DSP_MODE_PSV_EN       | \
                  DSP_MODE_SAT_SUPER    | \
                  DSP_MODE_DATA_SAT_EN  | \
                  DSP_MODE_ACCA_SAT_EN  | \
                  DSP_MODE_ACCB_SAT_EN  | \
                  DSP_MODE_SIGNED),     W3
        mov     W3,                     CORCON          /* set required dsp-core mode */

        /* === prepare registers for calculation === */

        sl      W2,     #1,             W2
        add     W1,     W2,             W1              /* W1 = &sample[a][sb] */

        /* === perform calculation === */

        do      #(18-1),                1f
        mov     [W0++],                 [W1]
1:      add     #(32*2),                W1

        mov     #(18*2),                W2
        sub     W0,     W2,             W0

        repeat  #(18-1)
        clr     [W0++]



        pop     CORCON                                  /* restore CORCON */
        return

        .end

/***************************************************************************************************
    end of file: mad_port_ovlz.S
 **************************************************************************************************/
