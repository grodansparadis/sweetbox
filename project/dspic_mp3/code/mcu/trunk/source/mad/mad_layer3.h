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
    File:            mad_layer3.h
    Description:

 ***************************************************************************************************
    History:         2007/12/11 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_LAYER3_H
#define __MAD_LAYER3_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

enum
{
    count1table_select  = 0x01,
    scalefac_scale      = 0x02,
    preflag             = 0x04,
    mixed_block_flag    = 0x08
};

enum
{
    I_STEREO  = 0x1,
    MS_STEREO = 0x2
};

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

typedef struct __CHANNEL
{
    /* from side info */

    UWORD part2_3_length;
    UWORD big_values;
    UWORD global_gain;
    UWORD scalefac_compress;

    UWORD flags;
    UWORD block_type;
    UWORD table_select[3];
    UWORD subblock_gain[3];
    UWORD region0_count;
    UWORD region1_count;

    /* from main_data */

    UWORD scalefac[39];     /* scalefac_l and/or scalefac_s */
} CHANNEL;

//---------------------------------------
typedef struct _GRANULE
{
    CHANNEL ch[2];
}GRANULE;

//---------------------------------------
typedef struct _SIDEINFO
{
    UWORD   main_data_begin;
    UWORD   private_bits;
    UWORD   scfsi[2];
    GRANULE gr[2];
} SIDEINFO;

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/

MAD_RETVAL mad_layer_III(MAD_STREAM *stream, MAD_FRAME *frame);

#endif /* __MAD_LAYER3_H */
/***************************************************************************************************
    end of file: mad_layer3.h
 **************************************************************************************************/
