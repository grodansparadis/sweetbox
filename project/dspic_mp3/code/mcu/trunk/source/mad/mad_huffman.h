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
    File:            mad_huffman.h
    Description:

 ***************************************************************************************************
    History:         2007/12/11 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_HUFFMAN_H
#define __MAD_HUFFMAN_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

typedef union __HUFF_QUAD //huffquad
{
    struct
    {
        unsigned final   :  1;
        unsigned hlen    :  3;
        unsigned v       :  1;
        unsigned w       :  1;
        unsigned x       :  1;
        unsigned y       :  1;
        unsigned         :  8;
    } value;

    struct
    {
        unsigned final   :  1;
        unsigned bits    :  3;
        unsigned offset  : 12;
    } ptr;

    struct
    {
        unsigned final   :  1;
        unsigned         : 15;
    };
} HUFF_QUAD;

typedef union __HUFF_PAIR //huffpair
{
    struct
    {
        unsigned final  :  1;
        unsigned hlen   :  3;
        unsigned x      :  4;
        unsigned y      :  4;
        unsigned        :  4;
    } value;

    struct
    {
        unsigned final  :  1;
        unsigned bits   :  3;
        unsigned offset : 12;
    } ptr;

    struct
    {
        unsigned final  :  1;
        unsigned        : 15;
    };
} HUFF_PAIR;

struct hufftable
{
    HUFF_PAIR const  *table;
    unsigned short    linbits;
    unsigned short    startbits;
};

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

extern HUFF_QUAD const *const mad_huff_quad_table[2];
extern struct hufftable const mad_huff_pair_table[32];

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/

#endif /* __MAD_HUFFMAN_H */
/***************************************************************************************************
    end of file: mad_huffman.h
 **************************************************************************************************/
