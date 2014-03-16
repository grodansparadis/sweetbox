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
    File:            mad_pt_defs.h
    Description:

 ***************************************************************************************************
    History:         2007/05/31 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_PT_DEFS_H
#define __MAD_PT_DEFS_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

    /* For setup CORCON */

#define DSP_MODE_INTEGER        (1 << 0)
#define DSP_MODE_FRACTIONAL     (0 << 0)

#define DSP_MODE_RND_EN         (1 << 1)
#define DSP_MODE_RND_DIS        (0 << 1)

#define DSP_MODE_PSV_EN         (1 << 2)
#define DSP_MODE_PSV_DIS        (0 << 2)

#define DSP_MODE_SAT_SUPER      (1 << 4)
#define DSP_MODE_SAT_NORMAL     (0 << 4)

#define DSP_MODE_DATA_SAT_EN    (1 << 5)
#define DSP_MODE_DATA_SAT_DIS   (0 << 5)

#define DSP_MODE_ACCB_SAT_EN    (1 << 6)
#define DSP_MODE_ACCB_SAT_DIS   (0 << 6)

#define DSP_MODE_ACCA_SAT_EN    (1 << 7)
#define DSP_MODE_ACCA_SAT_DIS   (0 << 7)

#define DSP_MODE_UNSIGNED       (1 << 12)
#define DSP_MODE_SIGNED         (0 << 12)

    /* macro */

.macro  DSP_A_B_SAT_EN
    bset    CORCON, #7
    bset    CORCON, #6
.endm

.macro DSP_A_B_SAT_DIS
    bclr    CORCON, #7
    bclr    CORCON, #6
.endm

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/


#endif /* __MAD_PT_DEFS_H */
/***************************************************************************************************
    end of file: mad_pt_defs.h
 **************************************************************************************************/
