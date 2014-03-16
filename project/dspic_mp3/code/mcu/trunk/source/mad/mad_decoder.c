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
    File:            mad_decoder.c
    Description:

 ***************************************************************************************************
    History:         2007/12/11 [GS]  File created


 **************************************************************************************************/

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

#include <string.h>

#include "mad_types.h"
#include "mad_bits.h"
#include "mad_stream.h"
#include "mad_frame.h"
#include "mad_synth.h"
#include "mad_huffman.h"
#include "mad_layer3.h"
#include "mad_decoder.h"

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

void *mad_decoder_alloc(UWORD size);
void  mad_decoder_free(void *ptr);

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

/**
 *
 * @param data
 * @param stream
 * @param frame
 *
 * @return MAD_FLOW
 */
MAD_FLOW mad_error_default (void *data, MAD_STREAM *stream, MAD_FRAME *frame)
{
    SWORD *bad_last_frame = data;

    switch (stream->error)
    {
        case MAD_ERROR_BADCRC:
            if (*bad_last_frame)
                mad_frame_mute(frame);
            else
                *bad_last_frame = 1;
            return (MAD_FLOW_IGNORE);

        default:
            return (MAD_FLOW_CONTINUE);
    }
}


MAD_RETVAL mad_run_sync (MAD_DECODER *decoder)
{
    MAD_FLOW (*error_func)(void *, MAD_STREAM *, MAD_FRAME *);

    void       *error_data;
    SWORD      bad_last_frame = 0;
    MAD_STREAM *stream;
    MAD_FRAME  *frame;
    MAD_SYNTH  *synth;
    MAD_RETVAL result = RET_OK;

    /* test input_func - mp3 data "source" */

    if (decoder->input_func == NULL)
        return (RET_OK);

    /* test for error handle function */

    if (decoder->error_func)
    {
        error_func = decoder->error_func;
        error_data = decoder->cb_data;
    }
    else
    {
        /* use default */
        error_func = mad_error_default;
        error_data = &bad_last_frame;
    }

    stream = &decoder->sync->stream;
    frame  = &decoder->sync->frame;
    synth  = &decoder->sync->synth;

    mad_stream_init(stream);
    mad_frame_init(frame);
    mad_synth_init(synth);

    mad_stream_options(stream, decoder->options);

    /* here loop up to end of data or some unrecoverable error */

    do
    {
        switch (decoder->input_func(decoder->cb_data, stream))
        {
            case MAD_FLOW_STOP:     goto done;
            case MAD_FLOW_BREAK:    goto fail;
            case MAD_FLOW_IGNORE:   continue;
            case MAD_FLOW_CONTINUE: break;
        }

        for (;;)
        {
            if (decoder->header_func)
            {
                if (mad_header_decode(&frame->header, stream) == RET_ERR)
                {
                    if (!MAD_RECOVERABLE(stream->error))
                        break;

                    switch (error_func(error_data, stream, frame))
                    {
                        case MAD_FLOW_STOP:      goto done;
                        case MAD_FLOW_BREAK:     goto fail;
                        case MAD_FLOW_IGNORE:
                        case MAD_FLOW_CONTINUE:
                        default:                 continue;
                    }
                }
                switch (decoder->header_func(decoder->cb_data, &frame->header))
                {
                    case MAD_FLOW_STOP:     goto done;
                    case MAD_FLOW_BREAK:    goto fail;
                    case MAD_FLOW_IGNORE:   continue;
                    case MAD_FLOW_CONTINUE: break;
                }
            } /* if(decoder->header_func) */

            /* here - decoding process */

            if (mad_frame_decode(frame, stream) == RET_ERR)
            {
                if (!MAD_RECOVERABLE(stream->error))
                    break;

                switch (error_func(error_data, stream, frame))
                {
                    case MAD_FLOW_STOP:     goto done;
                    case MAD_FLOW_BREAK:    goto fail;
                    case MAD_FLOW_IGNORE:   break;
                    case MAD_FLOW_CONTINUE:
                    default:                continue;
                }
            }
            else
                bad_last_frame = 0;

            if (decoder->filter_func)
            {
                switch (decoder->filter_func(decoder->cb_data, stream, frame))
                {
                    case MAD_FLOW_STOP:     goto done;
                    case MAD_FLOW_BREAK:    goto fail;
                    case MAD_FLOW_IGNORE:   continue;
                    case MAD_FLOW_CONTINUE: break;
                }
            } /* (decoder->filter_func) - фильтер какой то? :) */

            /* here synthesis of pcm data */

            mad_synth_frame(synth, frame);

            /* output function */

            if (decoder->output_func)
            {
                switch (decoder->output_func(decoder->cb_data,&frame->header, &synth->pcm))
                {
                    case MAD_FLOW_STOP:     goto done;
                    case MAD_FLOW_BREAK:    goto fail;
                    case MAD_FLOW_IGNORE:
                    case MAD_FLOW_CONTINUE: break;
                }
            } /* if(decoder->output_func) */
        } /* for(;;)*/
    }
    while (stream->error == MAD_ERROR_BUFLEN);

  fail:
    result = (RET_ERR);

  done:
    mad_synth_finish(synth);
    mad_frame_finish(frame);
    mad_stream_finish(stream);

    return (result);
}

/***************************************************************************************************
    Public Functions
 **************************************************************************************************/

/**
 * Initialize a decoder object with callback routines
 *
 * @param decoder
 * @param data
 * @param input_func
 * @param header_func
 * @param filter_func
 * @param output_func
 * @param error_func
 * @param message_func
 */
void mad_decoder_init (MAD_DECODER *decoder,
                       void        *data,
                       MAD_FLOW (*input_func)  (void *, MAD_STREAM *),
                       MAD_FLOW (*header_func) (void *, MAD_HEADER *),
                       MAD_FLOW (*filter_func) (void *, MAD_STREAM *, MAD_FRAME *),
                       MAD_FLOW (*output_func) (void *, MAD_HEADER *, MAD_PCM *),
                       MAD_FLOW (*error_func)  (void *, MAD_STREAM *, MAD_FRAME *),
                       MAD_FLOW (*message_func)(void *, void *, UWORD *)
                      )
{
    decoder->options      = MAD_OPTION_NONE;
    decoder->sync         = NULL;
    decoder->cb_data      = data;
    decoder->input_func   = input_func;
    decoder->header_func  = header_func;
    decoder->filter_func  = filter_func;
    decoder->output_func  = output_func;
    decoder->error_func   = error_func;
    decoder->message_func = message_func;
}

/**
 * Run the decoder thread either synchronously or asynchronously
 *
 * @param decoder
 *
 * @return MAD_RETVAL
 */
MAD_RETVAL mad_decoder_run (MAD_DECODER *decoder)
{
    MAD_RETVAL result;

    decoder->sync = (MAD_SYNC*)mad_decoder_alloc(sizeof(MAD_SYNC)); /* here allocating many memory :) */

    if (decoder->sync == 0)
        return (RET_ERR);

    result = mad_run_sync(decoder);

    mad_decoder_free(decoder->sync);

    decoder->sync = 0;
    return (result);
}

/**
 *
 * @param decoder
 *
 * @return MAD_RETVAL
 */
MAD_RETVAL mad_decoder_finish (MAD_DECODER *decoder)
{
    return (RET_OK);
}


/***************************************************************************************************
    end of file: mad_decoder.c
 **************************************************************************************************/
