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
    File:            mad_bits.h
    Description:

 ***************************************************************************************************
    History:         2007/12/10 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_BITS_H
#define __MAD_BITS_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

#define MAD_CRC_POLY    0x8005

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

typedef struct __MAD_BITPTR
{
    U08   *byte;
    UWORD  cache;
    UWORD  left;
} MAD_BITPTR;

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/

void    mad_bit_init(MAD_BITPTR *bitptr, U08 *byte);
UWORD   mad_bit_length(MAD_BITPTR *begin, MAD_BITPTR *end);
U08    *mad_bit_nextbyte(MAD_BITPTR *bitptr);
void    mad_bit_skip(MAD_BITPTR *bitptr, UWORD len);
U32     mad_bit_read(MAD_BITPTR *bitptr, UWORD len);
/* void  mad_bit_write(MAD_BITPTR *bitptr, UWORD len, U32 value); */
UWORD   mad_bit_crc(MAD_BITPTR bitptr, UWORD len, UWORD init);

#define mad_bit_finish(bitptr)      /* nothing */
#define mad_bit_bitsleft(bitptr)    ((bitptr)->left)


#endif /* __MAD_BITS_H */
/***************************************************************************************************
    end of file: mad_bits.h
 **************************************************************************************************/
