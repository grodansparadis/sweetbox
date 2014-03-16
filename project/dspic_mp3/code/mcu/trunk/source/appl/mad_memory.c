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
    File:            mad_memory.c
    Description:

 ***************************************************************************************************
    History:         2007/12/10 [GS]  File created


 **************************************************************************************************/

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

#include <string.h>
#include "..\\mad\\mad.h"

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

MAD_SYNC                                  decoder_data;
U08       __attribute__((space(ymemory))) stream_data[MAD_BUFFER_MDLEN];
MAD_FIXED __attribute__((space(ymemory))) frame_data[2][32][18];

/***************************************************************************************************
    Private Function Prototypes
 **************************************************************************************************/

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

/***************************************************************************************************
    Public Functions
 **************************************************************************************************/



/**
 *
 * @param size
 *
 * @return U08*
 */
void *mad_stream_alloc(UWORD size)
{
    return (void*)stream_data;
}

/**
 *
 * @param ptr
 */
void mad_stream_free(void *ptr)
{
    ptr = NULL;
}

/**
 *
 * @param size
 *
 * @return U08*
 */
void *mad_frame_alloc(UWORD size, UWORD el_size)
{
    /* init all memory with 0 - calloc */

    memset(frame_data, 0, sizeof(frame_data));
    return (void*)frame_data;
}

/**
 *
 * @param ptr
 */
void mad_frame_free(void *ptr)
{
    ptr = NULL;
}

/**
 *
 * @param size
 *
 * @return void*
 */
void *mad_decoder_alloc(UWORD size)
{
    return (void*)&decoder_data;
}

void mad_decoder_free(void *ptr)
{
    ptr = NULL;
}


/***************************************************************************************************
    end of file: mad_memory.c
 **************************************************************************************************/
