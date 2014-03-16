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
    File:            mad_stream.c
    Description:

 ***************************************************************************************************
    History:         2007/12/10 [GS]  File created


 **************************************************************************************************/

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

#include "mad_types.h"
#include "mad_bits.h"
#include "mad_stream.h"

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
    External Function Prototypes
 **************************************************************************************************/

void mad_stream_free(void *ptr);

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

/***************************************************************************************************
    Public Functions
 **************************************************************************************************/
/**
 * Initialize stream struct
 *
 * @param stream
 */
void mad_stream_init (MAD_STREAM *stream)
{
    stream->buffer     = 0;
    stream->bufend     = 0;
    stream->skiplen    = 0;
    stream->sync       = FALSE;
    stream->freerate   = 0;
    stream->this_frame = 0;
    stream->next_frame = 0;

    mad_bit_init(&stream->ptr,     0);
    mad_bit_init(&stream->anc_ptr, 0);

    stream->anc_bitlen = 0;
    stream->main_data  = 0;
    stream->md_len     = 0;
    stream->options    = MAD_OPTION_NONE;
    stream->error      = MAD_ERROR_NONE;
}

/**
 * Deallocate any memory associated with stream
 *
 * @param stream
 */
void mad_stream_finish (MAD_STREAM *stream)
{
    if (stream->main_data)
    {
        mad_stream_free(stream->main_data);
        stream->main_data = 0;
    }

    mad_bit_finish(&stream->anc_ptr);
    mad_bit_finish(&stream->ptr);
}

/**
 * Set stream buffer pointers
 *
 * @param stream
 * @param buffer
 * @param length
 */
void mad_stream_buffer (MAD_STREAM *stream, U08 *buffer, U32 length)
{
    stream->buffer     = buffer;
    stream->bufend     = buffer + length;
    stream->this_frame = buffer;
    stream->next_frame = buffer;
    stream->sync       = TRUE;

    mad_bit_init(&stream->ptr, buffer);
}

/**
 * Arrange to skip bytes before the next frame
 *
 * @param stream
 * @param length
 */
void mad_stream_skip (MAD_STREAM *stream, U32 length)
{
    stream->skiplen += length;
}

/**
 * Locate the next stream sync word
 *
 * @param stream
 *
 * @return SWORD
 */
MAD_RETVAL mad_stream_sync (MAD_STREAM *stream)
{
    U08 *ptr, *end;

    ptr = mad_bit_nextbyte(&stream->ptr);
    end = stream->bufend;

    while ((ptr < end - 1) &&
           !(ptr[0] == 0xFF && (ptr[1] & 0xE0) == 0xE0)
          )
        ++ptr;

    if (end - ptr < MAD_BUFFER_GUARD)
        return (RET_ERR);

    mad_bit_init(&stream->ptr, ptr);

    return (RET_OK);
}

#if __DEBUG__

/**
 * Return a string description of the current error condition
 *
 * @param stream
 *
 * @return S08*
 */
S08 *mad_stream_errorstr (MAD_STREAM *stream)
{
    switch (stream->error)
    {
        case MAD_ERROR_NONE:            return ("no error");
        case MAD_ERROR_BUFLEN:          return ("input buffer too small (or EOF)");
        case MAD_ERROR_BUFPTR:          return ("invalid (null) buffer pointer");
        case MAD_ERROR_NOMEM:           return ("not enough memory");
        case MAD_ERROR_LOSTSYNC:        return ("lost synchronization");
        case MAD_ERROR_BADLAYER:        return ("reserved header layer value");
        case MAD_ERROR_BADBITRATE:      return ("forbidden bitrate value");
        case MAD_ERROR_BADSAMPLERATE:   return ("reserved sample frequency value");
        case MAD_ERROR_BADEMPHASIS:     return ("reserved emphasis value");
        case MAD_ERROR_BADCRC:          return ("CRC check failed");
        case MAD_ERROR_BADBITALLOC:     return ("forbidden bit allocation value");
        case MAD_ERROR_BADSCALEFACTOR:  return ("bad scalefactor index");
        case MAD_ERROR_BADFRAMELEN:     return ("bad frame length");
        case MAD_ERROR_BADBIGVALUES:    return ("bad big_values count");
        case MAD_ERROR_BADBLOCKTYPE:    return ("reserved block_type");
        case MAD_ERROR_BADSCFSI:        return ("bad scalefactor selection info");
        case MAD_ERROR_BADDATAPTR:      return ("bad main_data_begin pointer");
        case MAD_ERROR_BADPART3LEN:     return ("bad audio data length");
        case MAD_ERROR_BADHUFFTABLE:    return ("bad Huffman table select");
        case MAD_ERROR_BADHUFFDATA:     return ("Huffman data overrun");
        case MAD_ERROR_BADSTEREO:       return ("incompatible block_type for JS");
    }

    return (NULL);
}

#endif

/***************************************************************************************************
    end of file: mad_stream.c
 **************************************************************************************************/
