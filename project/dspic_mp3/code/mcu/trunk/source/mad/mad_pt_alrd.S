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
    File:            mad_port_alrd.S
    Description:

 ***************************************************************************************************
    History:         2007/10/05 [GS]  File created


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

        .extern cs_ca

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
        .global  _mad_port_III_alias_reduce


/* -------------------------------------------------------------------------------------------------
    function:    mad_port_III_alias_reduce

    prototype:   void mad_port_III_alias_reduce (MAD_FIXED *xr, UWORD steps);

    description:


    parameters:  W0 - pointer to xr[576]
                 W1 - 1 for short block or 31 for long block

    return:      no return

   ---------------------------------------------------------------------------------------------- */

_mad_port_III_alias_reduce:

        push    PSVPAG                                  /* save PSVPAG */
        push    CORCON                                  /* save CORCON */
        push    W8

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


        /* prepare for alias resuce */

        add     #(18*2),                W0              /* W0 = &xr_up[i] */
        mov     [W0],                   W5              /* W5 = xr_up[i]  */

        dec2    W0,                     W2              /* W2 = &xr_dw[i] */
        mov     [W2],                   W6              /* W6 = xr_dw[i]  */

        mov     #psvoffset(cs_ca),      W8
        mov     W8,                     W3              /* save &cs[0] */

        mov     [W8],                   W4              /* W4 = cs[0]  */


        dec     W1,                     W1              /* for correct loop counting */

        do      W1,                     1f              /* external loop */

        do      #(8-1),                 2f              /* internal loop */

        mpy     W4*W5, A, [W8]+=2, W4                   /* ACCA = xr_up[i] * cs */
        mpy     W4*W6, B, [W8],    W4                   /* ACCB = xr_dw[i] * cs, prepare ca[i] */

        msc     W4*W6, A, [W8]+=2, W4                   /* ACCA =    (xr_up[i]*cs) - ca*(xr_down[i]*cs)                  */
        mac     W4*W5, B, [W8],    W4                   /* ACCB = ca*(xr_up[i]*cs) +    (xr_down[i]*cs), prepare cs[i+1] */

        sac.r   A,      #0,             [W0++]          /* xr_up[i++] =    (xr_up[i]*cs) - ca*(xr_down[i]*cs) */
        sac.r   B,      #0,             [W2--]          /* xr_dw[i--] = ca*(xr_up[i]*cs) +    (xr_down[i]*cs) */

        mov     [W0],                   W5              /* update xr_up[] */
2:      mov     [W2],                   W6              /* update xr_dw[] */


        mov     W3,                     W8              /* restore &cs[0] */
        mov     [W8],                   W4              /* W4 = cs[0]     */

        add     #(18*2 - 8*2),          W0
        mov     [W0],                   W5              /* update xr_up[] */

        add     #(18*2 + 8*2),          W2
1:      mov     [W2],                   W6              /* update xr_dw[i] */



        /* === end of function, restore required regs and return === */

        pop     W8
        pop     CORCON                                  /* restore CORCON */
        pop     PSVPAG                                  /* restore PAVPAG */
        return

        .end

/***************************************************************************************************
    end of file: mad_port_alrd.S
 **************************************************************************************************/
