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
    File:            mad_synth.c
    Description:

 ***************************************************************************************************
    History:         2007/12/10 [GS]  File created


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



/***************************************************************************************************
    Public Data
 **************************************************************************************************/

/***************************************************************************************************
    Private Function Prototypes
 **************************************************************************************************/

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

/**
 * Zero all polyphase filterbank values, resetting synthesis
 *
 * @param synth
 */
static void mad_synth_mute (MAD_SYNTH *synth)
{
    memset(synth->V, 0, 2*512*sizeof(MAD_FIXED));
}

/***************************************************************************************************
    Public Functions
 **************************************************************************************************/

/**
 * Initialize synth struct
 *
 * @param synth
 */
void mad_synth_init (MAD_SYNTH *synth)
{
    mad_synth_mute(synth);
    synth->pcm.samplerate = 0;
    synth->pcm.channels   = 0;
    synth->pcm.length     = 0;
}

/**
 * Perform PCM synthesis of frame subband samples
 *
 * @param synth
 * @param frame
 */
void mad_synth_frame (MAD_SYNTH *synth, MAD_FRAME *frame)
{
    UWORD nch, ns;

    nch = MAD_NCHANNELS(&frame->header);
    ns  = MAD_NSBSAMPLES(&frame->header);

    synth->pcm.samplerate = frame->header.samplerate;
    synth->pcm.channels   = nch;
    synth->pcm.length     = 32 * ns;

    /* Here - half sample rate - not implemented */

    /*

    if(frame->options & MAD_OPTION_HALFSAMPLERATE)
    {
        synth->pcm.samplerate /= 2;
        synth->pcm.length     /= 2;
    }
    */

    /* 0 -> left */

    mad_port_synth_frame(&(synth->pcm.samples[0][0]),
                         &(frame->sbsample[0][0][0]),
                         &(synth->V[0][0]),
                         ns
                        );


    /* 1 -> right? any way next :) if it is present */

    if (nch == 2)
    {
        mad_port_synth_frame(&(synth->pcm.samples[1][0]),
                             &(frame->sbsample[1][0][0]),
                             &(synth->V[1][0]),
                             ns
                            );
    }
}

/***************************************************************************************************
    end of file: mad_synth.c
 **************************************************************************************************/

