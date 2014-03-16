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
    File:            mad_synth.h
    Description:

 ***************************************************************************************************
    History:         2007/12/11 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_SYNTH_H
#define __MAD_SYNTH_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

typedef struct __MAD_PCM
{
    UWORD     samplerate;               /* Sampling frequency (Hz)          */
    UWORD     channels;                 /* Number of channels               */
    UWORD     length;                   /* Number of samples per channel    */
    MAD_FIXED samples[2][1152];         /* PCM output samples [ch][sample]  */
} MAD_PCM;

typedef struct __MAD_SYNTH
{
    MAD_FIXED V[2][1024];               /* [ch][sbsamples] polyphase filterbank outputs */
    MAD_PCM   pcm;                      /* PCM output */
} MAD_SYNTH;

/* Single channel PCM selector */

enum
{
    MAD_PCM_CHANNEL_SINGLE = 0
};

/* Dual channel PCM selector */

enum
{
    MAD_PCM_CHANNEL_DUAL_1 = 0,
    MAD_PCM_CHANNEL_DUAL_2 = 1
};

/* Stereo PCM selector */

enum
{
    MAD_PCM_CHANNEL_STEREO_LEFT  = 0,
    MAD_PCM_CHANNEL_STEREO_RIGHT = 1
};

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/

void    mad_synth_init(MAD_SYNTH *synth);
void    mad_synth_frame(MAD_SYNTH *synth, MAD_FRAME *frame);
#define mad_synth_finish(synth)     /* nothing */

#endif /* __MAD_SYNTH_H */
/***************************************************************************************************
    end of file: mad_synth.h
 **************************************************************************************************/
