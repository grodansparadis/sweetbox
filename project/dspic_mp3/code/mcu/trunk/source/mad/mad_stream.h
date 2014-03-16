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
    File:            mad_stream.h
    Description:

 ***************************************************************************************************
    History:         2007/12/10 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_STREAM_H
#define __MAD_STREAM_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

#define MAD_RECOVERABLE(error)  ((error) & 0xff00)

#define MAD_BUFFER_GUARD        8
#define MAD_BUFFER_MDLEN        (511 + 2048 + MAD_BUFFER_GUARD)

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

typedef enum __MAD_ERROR
{
    MAD_ERROR_NONE            = 0x0000, /* no error                         */

    MAD_ERROR_BUFLEN          = 0x0001, /* input buffer too small (or EOF)  */
    MAD_ERROR_BUFPTR          = 0x0002, /* invalid (null) buffer pointer    */

    MAD_ERROR_NOMEM           = 0x0031, /* not enough memory                */

    MAD_ERROR_LOSTSYNC        = 0x0101, /* lost synchronization             */
    MAD_ERROR_BADLAYER        = 0x0102, /* reserved header layer value      */
    MAD_ERROR_BADBITRATE      = 0x0103, /* forbidden bitrate value          */
    MAD_ERROR_BADSAMPLERATE   = 0x0104, /* reserved sample frequency value  */
    MAD_ERROR_BADEMPHASIS     = 0x0105, /* reserved emphasis value          */

    MAD_ERROR_BADCRC          = 0x0201, /* CRC check failed                 */
    MAD_ERROR_BADBITALLOC     = 0x0211, /* forbidden bit allocation value   */
    MAD_ERROR_BADSCALEFACTOR  = 0x0221, /* bad scalefactor index            */
    MAD_ERROR_BADFRAMELEN     = 0x0231, /* bad frame length                 */
    MAD_ERROR_BADBIGVALUES    = 0x0232, /* bad big_values count             */
    MAD_ERROR_BADBLOCKTYPE    = 0x0233, /* reserved block_type              */
    MAD_ERROR_BADSCFSI        = 0x0234, /* bad scalefactor selection info   */
    MAD_ERROR_BADDATAPTR      = 0x0235, /* bad main_data_begin pointer      */
    MAD_ERROR_BADPART3LEN     = 0x0236, /* bad audio data length            */
    MAD_ERROR_BADHUFFTABLE    = 0x0237, /* bad Huffman table select         */
    MAD_ERROR_BADHUFFDATA     = 0x0238, /* Huffman data overrun             */
    MAD_ERROR_BADSTEREO       = 0x0239  /* incompatible block_type for JS   */
} MAD_ERROR;

typedef enum __MAD_OPTION
{
    MAD_OPTION_NONE           = 0x0000, /* no decoding options              */
    MAD_OPTION_IGNORECRC      = 0x0001, /* ignore CRC errors                */
    MAD_OPTION_HALFSAMPLERATE = 0x0002  /* generate PCM at 1/2 sample rate  */

#if 0  /* not yet implemented */
    MAD_OPTION_LEFTCHANNEL    = 0x0010, /* decode left channel only         */
    MAD_OPTION_RIGHTCHANNEL   = 0x0020, /* decode right channel only        */
    MAD_OPTION_SINGLECHANNEL  = 0x0030  /* combine channels                 */
#endif
} MAD_OPTION;

typedef struct __MAD_STREAM
{
    U08        *buffer;                         /* input bitstream buffer           */
    U08        *bufend;                         /* end of buffer                    */
    U32        skiplen;                         /* bytes to skip before next frame  */
    BOOL       sync;                            /* stream sync found                */
    U32        freerate;                        /* free bitrate (fixed)             */
    U08        *this_frame;                     /* start of current frame           */
    U08        *next_frame;                     /* start of next frame              */
    MAD_BITPTR ptr;                             /* current processing bit pointer   */
    MAD_BITPTR anc_ptr;                         /* ancillary bits pointer           */
    UWORD      anc_bitlen;                      /* number of ancillary bits         */
    U08        *main_data;



    UWORD      md_len;                          /* bytes in main_data               */
    MAD_OPTION options;                         /* decoding options                 */
    MAD_ERROR  error;                           /* error code                       */
} MAD_STREAM;

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/

void       mad_stream_init(MAD_STREAM *stream);
void       mad_stream_finish(MAD_STREAM *stream);
void       mad_stream_buffer(MAD_STREAM *stream, U08 *buffer, U32 length);
void       mad_stream_skip(MAD_STREAM *stream, U32 length);
MAD_RETVAL mad_stream_sync(MAD_STREAM *stream);
#define    mad_stream_options(stream, opts)     ((void)((stream)->options = (opts)))
#if __DEBUG__
S08       *mad_stream_errorstr(MAD_STREAM *stream);
#endif

#endif /* __MAD_STREAM_H */
/***************************************************************************************************
    end of file: mad_stream.h
 **************************************************************************************************/
