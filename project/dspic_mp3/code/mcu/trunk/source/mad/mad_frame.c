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
    File:            mad_frame.c
    Description:

 ***************************************************************************************************
    History:         2007/12/11 [GS]  File created


 **************************************************************************************************/

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

#include "mad_types.h"
#include "mad_bits.h"
#include "mad_stream.h"
#include "mad_frame.h"
#include "mad_synth.h"
#include "mad_huffman.h"
#include "mad_layer3.h"
#include "mad_pt.h"

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

/***************************************************************************************************
    Private Types
 **************************************************************************************************/

/***************************************************************************************************
    Private Data
 **************************************************************************************************/

static U32 const bitrate_table[5][15] =
{
    /* MPEG-1 */

    { 0, 32000, 64000, 96000, 128000, 160000, 192000, 224000, 256000, 288000, 320000, 352000, 384000, 416000, 448000},  /* Layer I   */
    { 0, 32000, 48000, 56000,  64000,  80000,  96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000, 384000},  /* Layer II  */
    { 0, 32000, 40000, 48000,  56000,  64000,  80000,  96000, 112000, 128000, 160000, 192000, 224000, 256000, 320000},  /* Layer III */

    /* MPEG-2 LSF */

    { 0, 32000, 48000, 56000,  64000,  80000,  96000, 112000, 128000, 144000, 160000, 176000, 192000, 224000, 256000},  /* Layer I           */
    { 0,  8000, 16000, 24000,  32000,  40000,  48000,  56000,  64000,  80000,  96000, 112000, 128000, 144000, 160000}   /* Layers II and III */
};

static UWORD const samplerate_table[3] =
{
    44100,
    48000,
    32000
};

static MAD_RETVAL (*const decoder_table[3])(MAD_STREAM *, MAD_FRAME *) =
{
    NULL,           /* no function for  mad_layer_I   */
    NULL,           /* no function for  mad_layer_II  */
    mad_layer_III
};


/***************************************************************************************************
    Public Data
 **************************************************************************************************/

/***************************************************************************************************
    Private Function Prototypes
 **************************************************************************************************/

/***************************************************************************************************
    Private Function Prototypes
 **************************************************************************************************/

/***************************************************************************************************
    External Function Prototypes
 **************************************************************************************************/

void  mad_frame_free(void *ptr);

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

/**
 * Initialize header struct
 *
 * @param header
 */
static void mad_header_init (MAD_HEADER *header)
{
    header->layer          = MAD_LAYER_NONE;
    header->mode           = MAD_MODE_SINGLE_CHANNEL;
    header->mode_extension = 0;
    header->emphasis       = MAD_EMPHASIS_NONE;
    header->bitrate        = 0;
    header->samplerate     = 0;
    header->crc_check      = 0;
    header->crc_target     = 0;
    header->flags          = MAD_FLAG_NONE;
    header->private_bits   = MAD_PRIVATE_NONE;
}

/**
 * Read header data and following CRC word
 *
 * @param header
 * @param stream
 *
 * @return int
 */
static MAD_RETVAL mad_header_load (MAD_HEADER *header, MAD_STREAM *stream)
{
    UWORD index;

    header->flags        = MAD_FLAG_NONE;
    header->private_bits = MAD_PRIVATE_NONE;

    mad_bit_skip(&stream->ptr, 11);                     /* Get syncword */

    if (mad_bit_read(&stream->ptr, 1) == 0)             /* MPEG 2.5 indicator */
        header->flags |= MAD_FLAG_MPEG_2_5_EXT;

    if (mad_bit_read(&stream->ptr, 1) == 0)             /* ID */
        header->flags |= MAD_FLAG_LSF_EXT;
    else if (header->flags & MAD_FLAG_MPEG_2_5_EXT)
    {
        stream->error = MAD_ERROR_LOSTSYNC;
        return (RET_ERR);
    }

    header->layer = 4 - mad_bit_read(&stream->ptr, 2);  /* Layer */

    if (header->layer == 4)
    {
        stream->error = MAD_ERROR_BADLAYER;
        return (RET_ERR);
    }

    if (mad_bit_read(&stream->ptr, 1) == 0)             /* protection_bit */
    {
        header->flags    |= MAD_FLAG_PROTECTION;
        header->crc_check = mad_bit_crc(stream->ptr, 16, 0xffff);
    }

    index = mad_bit_read(&stream->ptr, 4);              /* bitrate_index */

    if (index == 15)
    {
        stream->error = MAD_ERROR_BADBITRATE;
        return (RET_ERR);
    }

    if (header->flags & MAD_FLAG_LSF_EXT)
        header->bitrate = bitrate_table[3 + (header->layer >> 1)][index];
    else
        header->bitrate = bitrate_table[header->layer - 1][index];


    index = mad_bit_read(&stream->ptr, 2);              /* sampling_frequency */

    if (index == 3)
    {
        stream->error = MAD_ERROR_BADSAMPLERATE;
        return (RET_ERR);
    }

    header->samplerate = samplerate_table[index];

    if (header->flags & MAD_FLAG_LSF_EXT)
    {
        header->samplerate /= 2;
        if (header->flags & MAD_FLAG_MPEG_2_5_EXT)
            header->samplerate /= 2;
    }

    if (mad_bit_read(&stream->ptr, 1))                  /* padding_bit */
        header->flags |= MAD_FLAG_PADDING;

    if (mad_bit_read(&stream->ptr, 1))                  /* private_bit */
        header->private_bits |= MAD_PRIVATE_HEADER;

    header->mode = 3 - mad_bit_read(&stream->ptr, 2);   /* mode */

    header->mode_extension = mad_bit_read(&stream->ptr, 2); /* mode_extension */

    if (mad_bit_read(&stream->ptr, 1))                  /* copyright */
        header->flags |= MAD_FLAG_COPYRIGHT;

    if (mad_bit_read(&stream->ptr, 1))                  /* original/copy */
        header->flags |= MAD_FLAG_ORIGINAL;

    header->emphasis = mad_bit_read(&stream->ptr, 2);   /* emphasis */

#if defined(OPT_STRICT)

    /* ISO/IEC 11172-3 says this is a reserved emphasis value, but streams exist which use
       it anyway. Since the value is not important to the decoder proper, we allow it unless
       OPT_STRICT is defined.
     */

    if (header->emphasis == MAD_EMPHASIS_RESERVED)
    {
        stream->error = MAD_ERROR_BADEMPHASIS;
        return (RET_ERR);
    }

#endif

    /* CRC check */

    if (header->flags & MAD_FLAG_PROTECTION)
        header->crc_target = mad_bit_read(&stream->ptr, 16);

    return (RET_OK);
}

/**
 * Attempt to discover the bitstream's free bitrate
 *
 * @param stream
 * @param header
 *
 * @return int
 */
static MAD_RETVAL mad_free_bitrate (MAD_STREAM *stream, MAD_HEADER *header)
{
    MAD_BITPTR keep_ptr;
    U32        rate = 0;
    UWORD      pad_slot, slots_per_frame;
    U08        *ptr = 0;

    keep_ptr = stream->ptr;
    pad_slot = (header->flags & MAD_FLAG_PADDING) ? 1 : 0;

    slots_per_frame = (header->layer == MAD_LAYER_III && (header->flags & MAD_FLAG_LSF_EXT)) ? 72 : 144;

    while (mad_stream_sync(stream) == RET_OK)
    {
        MAD_STREAM peek_stream;
        MAD_HEADER peek_header;

        peek_stream = *stream;
        peek_header = *header;

        if (mad_header_load(&peek_header, &peek_stream) == RET_OK &&
            peek_header.layer      == header->layer               &&
            peek_header.samplerate == header->samplerate
           )
        {
            UWORD N;

            ptr = mad_bit_nextbyte(&stream->ptr);

            N = ptr - stream->this_frame;

            if (header->layer == MAD_LAYER_I)
            {
                rate = (U32) header->samplerate *  (N - 4 * pad_slot + 4) / 48 / 1000;
            }
            else
            {
                rate = (U32) header->samplerate *  (N - pad_slot + 1) / slots_per_frame / 1000;
            }

            if (rate >= 8)
                break;
        }

        mad_bit_skip(&stream->ptr, 8);
    }

    stream->ptr = keep_ptr;

    if (rate < 8 || (header->layer == MAD_LAYER_III && rate > 640))
    {
        stream->error = MAD_ERROR_LOSTSYNC;
        return (RET_ERR);
    }

    stream->freerate = rate * 1000;
    return (RET_OK);
}


/***************************************************************************************************
    Public Functions
 **************************************************************************************************/

/**
 * Initialize frame struct
 *
 * @param frame
 */
void mad_frame_init (MAD_FRAME *frame)
{
    mad_header_init(&frame->header);
    frame->options = 0;
    frame->overlap = NULL;
    mad_frame_mute(frame);
}


/**
 * Zero all subband values so the frame becomes silent
 * One of example - this function is used very infrequet - so for speed no problem but for size...
 *  - may be rewritten on asm
 *
 * @param frame
 */
void mad_frame_mute (MAD_FRAME *frame)
{
    UWORD s, sb;

    for (s = 0; s < 36; ++s)
    {
        for (sb = 0; sb < 32; ++sb)
        {
            frame->sbsample[0][s][sb] =
            frame->sbsample[1][s][sb] = 0;
        }
    }

    if (frame->overlap)
    {
        for (s = 0; s < 18; ++s)
        {
            for (sb = 0; sb < 32; ++sb)
            {
                (*frame->overlap)[0][sb][s] =
                (*frame->overlap)[1][sb][s] = 0;
            }
        }
    }
}

/**
 * Deallocate any dynamic memory associated with frame
 *
 * @param frame
 */
void mad_frame_finish (MAD_FRAME *frame)
{
    mad_header_finish(&frame->header);

    if (frame->overlap)
    {
        mad_frame_free(frame->overlap);
        frame->overlap = 0;
    }
}

/**
 * Read the next frame header from the stream
 *
 * @param header
 * @param stream
 *
 * @return MAD_RETVAL
 */
MAD_RETVAL mad_header_decode (MAD_HEADER *header, MAD_STREAM *stream)
{
    U08   *ptr, *end;
    UWORD pad_slot, N;

    ptr = stream->next_frame;
    end = stream->bufend;

    if (ptr == 0)
    {
        stream->error = MAD_ERROR_BUFPTR;
        goto fail;
    }

    /* Stream skip */

    if (stream->skiplen)
    {
        if (stream->sync == FALSE)
            ptr = stream->this_frame;

        if (end - ptr < stream->skiplen)
        {
            stream->skiplen   -= end - ptr;
            stream->next_frame = end;
            stream->error      = MAD_ERROR_BUFLEN;
            goto fail;
        }

        ptr            += stream->skiplen;
        stream->skiplen = 0;
        stream->sync    = TRUE;
    }

  sync:

    /* Syncronize */

    if (stream->sync != FALSE)
    {
        if ((U32)(end - ptr) < MAD_BUFFER_GUARD)
        {
            stream->next_frame = ptr;
            stream->error      = MAD_ERROR_BUFLEN;
            goto fail;
        }
        else if (!(ptr[0] == 0xff && (ptr[1] & 0xe0) == 0xe0))
        {
            /* mark point where frame sync word was expected */

            stream->this_frame = ptr;
            stream->next_frame = ptr + 1;
            stream->error      = MAD_ERROR_LOSTSYNC;
            goto fail;
        }
    }
    else
    {
        mad_bit_init(&stream->ptr, ptr);

        if (mad_stream_sync(stream) == RET_ERR)
        {
            if (end - stream->next_frame >= MAD_BUFFER_GUARD)
                stream->next_frame = end - MAD_BUFFER_GUARD;

            stream->error = MAD_ERROR_BUFLEN;
            goto fail;
        }

        ptr = mad_bit_nextbyte(&stream->ptr);
    }

    /* Begin processing */

    stream->this_frame = ptr;
    stream->next_frame = ptr + 1;                   /* possibly bogus sync word */

    mad_bit_init(&stream->ptr, stream->this_frame);

    if (mad_header_load(header, stream) == RET_ERR)
        goto fail;

    /* Calculate frame duration */

    /* I remove timer - so remove this too */
    /* mad_timer_set(&header->duration, 0,32 * MAD_NSBSAMPLES(header), header->samplerate); */

    /* calculate free bit rate */

    if (header->bitrate == 0)
    {
        if ((stream->freerate == 0 || !stream->sync) &&
            mad_free_bitrate(stream, header) == RET_ERR
           )
            goto fail;

        header->bitrate = stream->freerate;
        header->flags  |= MAD_FLAG_FREEFORMAT;
    }

    /* Calculate beginning of next frame */

    pad_slot = (header->flags & MAD_FLAG_PADDING) ? 1 : 0;

    if (header->layer == MAD_LAYER_I)
        N = ((12 * header->bitrate / header->samplerate) + pad_slot) * 4;
    else
    {
        UWORD slots_per_frame;

        slots_per_frame = (header->layer == MAD_LAYER_III && (header->flags & MAD_FLAG_LSF_EXT)) ? 72 : 144;
        N               = (slots_per_frame * header->bitrate / header->samplerate) + pad_slot;
    }

    // Verify there is enough data left in buffer to decode this frame */

    if (N + MAD_BUFFER_GUARD > (U32)(end - stream->this_frame))
    {
        stream->next_frame = stream->this_frame;
        stream->error      = MAD_ERROR_BUFLEN;
        goto fail;
    }

    stream->next_frame = stream->this_frame + N;

    if (stream->sync == FALSE)
    {
        /* check that a valid frame header follows this frame */

        ptr = stream->next_frame;

        if (!(ptr[0] == 0xff && (ptr[1] & 0xe0) == 0xe0))
        {
            ptr = stream->next_frame = stream->this_frame + 1;
            goto sync;
        }
        stream->sync = TRUE;
    }

    header->flags |= MAD_FLAG_INCOMPLETE;
    return (RET_OK);

  fail:
    stream->sync = FALSE;
    return (RET_ERR);
}

/**
 * Decode a single frame from a bitstream
 *
 * @param frame
 * @param stream
 *
 * @return MAD_RETVAL
 */
MAD_RETVAL mad_frame_decode (MAD_FRAME *frame, MAD_STREAM *stream)
{
    frame->options = stream->options;

    /* header() */
    /* error_check() */

    if (!(frame->header.flags & MAD_FLAG_INCOMPLETE) &&
        mad_header_decode(&frame->header, stream) == RET_ERR)
        goto fail;

    /* audio_data() */

    frame->header.flags &= ~MAD_FLAG_INCOMPLETE;

    if (decoder_table[frame->header.layer - 1](stream, frame) == RET_ERR)    /* call decoder */
    {
        if (!MAD_RECOVERABLE(stream->error))
            stream->next_frame = stream->this_frame;
        goto fail;
    }

    /* ancillary_data() */

    if (frame->header.layer != MAD_LAYER_III)
    {
        MAD_BITPTR next_frame;

        mad_bit_init(&next_frame, stream->next_frame);
        stream->anc_ptr    = stream->ptr;
        stream->anc_bitlen = mad_bit_length(&stream->ptr, &next_frame);
        mad_bit_finish(&next_frame);
    }

    return (RET_OK);

  fail:
    stream->anc_bitlen = 0;
    return (RET_ERR);
}


/***************************************************************************************************
    end of file: mad_frame.c
 **************************************************************************************************/
