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
    File:            appl.c
    Description:

 ***************************************************************************************************
    History:         2007/05/31 [GS]  File created


 **************************************************************************************************/

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <libpic30.h>
#include <p33Fxxxx.h>

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

BUFFER      buffer_desc;
MAD_DECODER decoder_desc;
FILE        *fmp3, *fpcm;
U32         pcmLength;

UWORD       mp3_buf[2048];

/***************************************************************************************************
    Public Data
 **************************************************************************************************/

/***************************************************************************************************
    Private Function Prototypes
 **************************************************************************************************/

/***************************************************************************************************
    External Function Prototypes
 **************************************************************************************************/

#if defined(__DEBUG__)
    void mad_timer_conf(void);
    void mad_timer_start(void);
    void mad_timer_stop(void);
    U32  mad_timer_get(void);
    void mad_timer_reset(void);
#endif

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

/**
 *
 * @param data
 * @param stream
 *
 * @return MAD_FLOW
 */
MAD_FLOW mp3_get (void *data, MAD_STREAM *stream)
{
    BUFFER *buffer = data;
    U08    *destination;
    UWORD   num_get;
    UWORD   num;
    U32     rest;

    mad_timer_stop();

    destination = buffer->start;                            /* set the destination */

    rest = stream->bufend - stream->next_frame;

    /* copy rest of unconsumed data to begin of buffer */

    if (stream->buffer)
    {
        /* test for present of buffer - test for first call */

        memmove(stream->buffer, stream->next_frame, rest);  /* it is possible to use memcpy */
        destination = destination + rest;
    }

    /* try to read portion of data */

    num_get = 2048*2 - rest;                                /* try to read full page */

    num = fread(destination, 1, num_get, fmp3);

    if (!num)
        return (MAD_FLOW_STOP);                             /*looks like end of file */

    mad_stream_buffer(stream, buffer->start, (num + rest));

    mad_timer_start();

    return (MAD_FLOW_CONTINUE);
}

/**
 *
 * @param data
 * @param header
 * @param pcm
 *
 * @return MAD_FLOW
 */
MAD_FLOW pcm_put (void *data, MAD_HEADER *header, MAD_PCM *pcm)
{
    MAD_FIXED *left_ch;
    MAD_FIXED *right_ch;
    U32       i;

    mad_timer_stop();

    /* pcm->samplerate contains the sampling frequency */

    left_ch   = &pcm->samples[0][0];
    right_ch  = &pcm->samples[1][0];

    for (i = 0; i < pcm->length; i++)
    {
        fwrite(left_ch++, 1, 2, fpcm);

        if (pcm->channels > 1)
            fwrite(right_ch++, 1, 2, fpcm);
    }

    pcmLength += pcm->length;

    mad_timer_start();

    return (MAD_FLOW_CONTINUE);
}

/**
 *
 * @param data
 * @param stream
 * @param frame
 *
 * @return MAD_FLOW
 */
MAD_FLOW user_error (void *data, MAD_STREAM *stream, MAD_FRAME *frame)
{
    BUFFER *buffer = data;

    printf("decoding error 0x%04x at byte offset %u\n",
           stream->error, stream->this_frame - buffer->start);

    return (MAD_FLOW_STOP);
}


/**
 *
 * @param start
 * @param length
 */
void mp3_decode (U08 *start, U32 length)
{
    buffer_desc.start  = start;
    buffer_desc.length = length;

    /* configure input, output, and error functions */

    mad_decoder_init(&decoder_desc,
                     &buffer_desc,
                     mp3_get,           /* input mp3 data */
                     NULL,              /* header */
                     NULL,              /* filter */
                     pcm_put,           /* output pcm data */
                     user_error,        /* error manage function */
                     NULL               /* message - <<not used ???>> */
                    );

    /* start the decoder */

    mad_decoder_run(&decoder_desc);
    mad_decoder_finish(&decoder_desc);
}


/***************************************************************************************************
    Public Functions
 **************************************************************************************************/

char const *files_in[] =
{
    "test_tones\\m0_44_64.mp3",
    "test_tones\\m0_44_96.mp3",
    "test_tones\\m0_44_128.mp3",
    "test_tones\\m0_44_256.mp3",
    "test_tones\\m0_44_320.mp3",

    "test_tones\\s0_8_20.mp3",

    "test_tones\\s0_11_20.mp3",
    "test_tones\\s0_11_24.mp3",
    "test_tones\\s0_11_32.mp3",

    "test_tones\\s0_16_32.mp3",
    "test_tones\\s0_16_40.mp3",
    "test_tones\\s0_16_48.mp3",
    "test_tones\\s0_16_56.mp3",

    "test_tones\\s0_22_40.mp3",
    "test_tones\\s0_22_48.mp3",
    "test_tones\\s0_22_56.mp3",
    "test_tones\\s0_22_64.mp3",
    "test_tones\\s0_22_80.mp3",
    "test_tones\\s0_22_96.mp3",

    "test_tones\\s0_32_80.mp3",
    "test_tones\\s0_32_96.mp3",
    "test_tones\\s0_32_112.mp3",
    "test_tones\\s0_32_128.mp3",
    "test_tones\\s0_32_160.mp3",
    "test_tones\\s0_32_192.mp3",
    "test_tones\\s0_32_224.mp3",
    "test_tones\\s0_32_256.mp3",
    "test_tones\\s0_32_320.mp3",

    "test_tones\\s0_44_96.mp3",
    "test_tones\\s0_44_112.mp3",
    "test_tones\\s0_44_128.mp3",
    "test_tones\\s0_44_160.mp3",
    "test_tones\\s0_44_192.mp3",
    "test_tones\\s0_44_224.mp3",
    "test_tones\\s0_44_256.mp3",
    "test_tones\\s0_44_320.mp3"
};

char const *files_out[] =
{
    "test_tones\\m0_44_64.pcm",
    "test_tones\\m0_44_96.pcm",
    "test_tones\\m0_44_128.pcm",
    "test_tones\\m0_44_256.pcm",
    "test_tones\\m0_44_320.pcm",

    "test_tones\\s0_8_20.pcm",

    "test_tones\\s0_11_20.pcm",
    "test_tones\\s0_11_24.pcm",
    "test_tones\\s0_11_32.pcm",

    "test_tones\\s0_16_32.pcm",
    "test_tones\\s0_16_40.pcm",
    "test_tones\\s0_16_48.pcm",
    "test_tones\\s0_16_56.pcm",

    "test_tones\\s0_22_40.pcm",
    "test_tones\\s0_22_48.pcm",
    "test_tones\\s0_22_56.pcm",
    "test_tones\\s0_22_64.pcm",
    "test_tones\\s0_22_80.pcm",
    "test_tones\\s0_22_96.pcm",

    "test_tones\\s0_32_80.pcm",
    "test_tones\\s0_32_96.pcm",
    "test_tones\\s0_32_112.pcm",
    "test_tones\\s0_32_128.pcm",
    "test_tones\\s0_32_160.pcm",
    "test_tones\\s0_32_192.pcm",
    "test_tones\\s0_32_224.pcm",
    "test_tones\\s0_32_256.pcm",
    "test_tones\\s0_32_320.pcm",

    "test_tones\\s0_44_96.pcm",
    "test_tones\\s0_44_112.pcm",
    "test_tones\\s0_44_128.pcm",
    "test_tones\\s0_44_160.pcm",
    "test_tones\\s0_44_192.pcm",
    "test_tones\\s0_44_224.pcm",
    "test_tones\\s0_44_256.pcm",
    "test_tones\\s0_44_320.pcm"
};

char const *files_type[] =
{
    "mono,     44100 Hz,     64 kbps",
    "mono,     44100 Hz,     96 kbps",
    "mono,     44100 Hz,    128 kbps",
    "mono,     44100 Hz,    256 kbps",
    "mono,     44100 Hz,    320 kbps",

    "stereo,    8000 Hz,     20 kbps",

    "stereo,   11025 Hz,     20 kbps",
    "stereo,   11025 Hz,     24 kbps",
    "stereo,   11025 Hz,     32 kbps",

    "stereo,   16000 Hz,     32 kbps",
    "stereo,   16000 Hz,     40 kbps",
    "stereo,   16000 Hz,     48 kbps",
    "stereo,   16000 Hz,     56 kbps",

    "stereo,   22050 Hz,     40 kbps",
    "stereo,   22050 Hz,     48 kbps",
    "stereo,   22050 Hz,     56 kbps",
    "stereo,   22050 Hz,     64 kbps",
    "stereo,   22050 Hz,     80 kbps",
    "stereo,   22050 Hz,     96 kbps",

    "stereo,   32000 Hz,     80 kbps",
    "stereo,   32000 Hz,     96 kbps",
    "stereo,   32000 Hz,    112 kbps",
    "stereo,   32000 Hz,    128 kbps",
    "stereo,   32000 Hz,    160 kbps",
    "stereo,   32000 Hz,    192 kbps",
    "stereo,   32000 Hz,    224 kbps",
    "stereo,   32000 Hz,    256 kbps",
    "stereo,   32000 Hz,    320 kbps",

    "stereo,   44100 Hz,     96 kbps",
    "stereo,   44100 Hz,    112 kbps",
    "stereo,   44100 Hz,    128 kbps",
    "stereo,   44100 Hz,    160 kbps",
    "stereo,   44100 Hz,    192 kbps",
    "stereo,   44100 Hz,    224 kbps",
    "stereo,   44100 Hz,    256 kbps",
    "stereo,   44100 Hz,    320 kbps"
};

#define FOSC        40000000.0
#define TMR_PS      8

extern MAD_SYNC decoder_data;

int main (void)
{
    UWORD i;

    mad_timer_conf();                       /* config counting 32-bit timer */

    printf("channels: sample_rate: bitrate:   num_of_samples:   play_time:   decode_time:\n");
    printf("-----------------------------------------------------------------------------\n");

    for (i = 0; i < sizeof(files_in)/sizeof(files_in[0]); i++)
    {
        fmp3 = fopen(files_in[i],  "rb");

        if (fmp3 == NULL)
            continue;

        fpcm = fopen(files_out[i], "wb");

        if (fpcm == NULL)
        {
            fclose(fmp3);
            continue;
        }

        pcmLength = 0;
        mad_timer_reset();                  /* reset timer */    
        mad_timer_start();
    
        mp3_decode((U08*)&mp3_buf[0], 0);   /* all data from input_mp3 function */
    
        mad_timer_stop();
    
        fclose(fmp3);
        fclose(fpcm);

        printf("%s   %5lu             %.3f        %.3f\n",

               files_type[i],
               pcmLength,
               1.0 / (decoder_data.frame.header.samplerate) * pcmLength,
               mad_timer_get() / FOSC * TMR_PS
              );
    }


    for (;;);

    return (0);
}

/***************************************************************************************************
    end of file: appl.c
 **************************************************************************************************/

