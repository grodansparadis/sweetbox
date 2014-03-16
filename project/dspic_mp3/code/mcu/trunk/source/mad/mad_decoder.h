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
    File:            mad_decoder.h
    Description:

 ***************************************************************************************************
    History:         2007/12/11 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_DECODER_H
#define __MAD_DECODER_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

typedef enum __MAD_DECODER_MODE
{
    MAD_DECODER_MODE_SYNC  = 0,
    MAD_DECODER_MODE_ASYNC = 1
} MAD_DECODER_MODE;

typedef enum __MAD_FLOW
{
    MAD_FLOW_CONTINUE = 0x0000,   /* continue normally                  */
    MAD_FLOW_STOP     = 0x0010,   /* stop decoding normally             */
    MAD_FLOW_BREAK    = 0x0011,   /* stop decoding and signal an error  */
    MAD_FLOW_IGNORE   = 0x0020    /* ignore the current frame           */
} MAD_FLOW;

typedef struct __MAD_ASYNC
{
    S32    pid;
    SWORD  in;
    SWORD  out;
} MAD_ASYNC;

typedef struct __MAD_SYNC
{
    MAD_STREAM stream;
    MAD_FRAME  frame;
    MAD_SYNTH  synth;
} MAD_SYNC;

typedef struct __MAD_DECODER
{
    MAD_OPTION options;
    MAD_SYNC   *sync;
    void       *cb_data;

    MAD_FLOW (*input_func)  (void *, MAD_STREAM  *);
    MAD_FLOW (*header_func) (void *, MAD_HEADER  *);
    MAD_FLOW (*filter_func) (void *, MAD_STREAM  *, MAD_FRAME *);
    MAD_FLOW (*output_func) (void *, MAD_HEADER  *, MAD_PCM *);
    MAD_FLOW (*error_func)  (void *, MAD_STREAM  *, MAD_FRAME *);
    MAD_FLOW (*message_func)(void *, void *, UWORD *);
} MAD_DECODER;

/* private message buffer */

typedef struct __BUFFER
{
    U08 *start;
    U32 length;
} BUFFER;

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/

void mad_decoder_init(MAD_DECODER *decoder,
                      void        *data,
                      MAD_FLOW (*input_func)  (void *, MAD_STREAM *),
                      MAD_FLOW (*header_func) (void *, MAD_HEADER *),
                      MAD_FLOW (*filter_func) (void *, MAD_STREAM *, MAD_FRAME *),
                      MAD_FLOW (*output_func) (void *, MAD_HEADER *, MAD_PCM *),
                      MAD_FLOW (*error_func)  (void *, MAD_STREAM *, MAD_FRAME *),
                      MAD_FLOW (*message_func)(void *, void *, UWORD *)
                     );

MAD_RETVAL mad_decoder_run(MAD_DECODER *decoder);
MAD_RETVAL mad_decoder_finish(MAD_DECODER *decoder);

#endif /* __MAD_DECODER_H */
/***************************************************************************************************
    end of file: mad_decoder.h
 **************************************************************************************************/
