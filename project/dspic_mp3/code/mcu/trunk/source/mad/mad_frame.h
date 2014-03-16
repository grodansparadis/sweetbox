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
    File:            mad_frame.h
    Description:

 ***************************************************************************************************
    History:         2007/12/11 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_FRAME_H
#define __MAD_FRAME_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

#define MAD_NCHANNELS(header)   ((header)->mode ? 2 : 1)
#define MAD_NSBSAMPLES(header)  ((header)->layer == MAD_LAYER_I ? 12 : (((header)->layer == MAD_LAYER_III && ((header)->flags & MAD_FLAG_LSF_EXT)) ? 18 : 36))

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

typedef enum __MAD_LAYER
{
    MAD_LAYER_NONE = 0,
    MAD_LAYER_I    = 1,             /* Layer I                      */
    MAD_LAYER_II   = 2,             /* Layer II                     */
    MAD_LAYER_III  = 3              /* Layer III                    */
} MAD_LAYER;

typedef enum __MAD_MODE
{
    MAD_MODE_SINGLE_CHANNEL = 0,    /* single channel               */
    MAD_MODE_DUAL_CHANNEL   = 1,    /* dual channel                 */
    MAD_MODE_JOINT_STEREO   = 2,    /* joint (MS/intensity) stereo  */
    MAD_MODE_STEREO         = 3     /* normal LR stereo             */
} MAD_MODE;

typedef enum __MAD_EMPHASIS
{
    MAD_EMPHASIS_NONE       = 0,    /* no emphasis                  */
    MAD_EMPHASIS_50_15_US   = 1,    /* 50/15 microseconds emphasis  */
    MAD_EMPHASIS_CCITT_J_17 = 3,    /* CCITT J.17 emphasis          */
    MAD_EMPHASIS_RESERVED   = 2     /* unknown emphasis             */
} MAD_EMPHASIS;


typedef enum __MAD_FLAG
{
    MAD_FLAG_NONE         = 0x0000,
    MAD_FLAG_NPRIVATE_III = 0x0007, /* number of Layer III private bits */
    MAD_FLAG_INCOMPLETE   = 0x0008, /* header but not data is decoded   */
    MAD_FLAG_PROTECTION   = 0x0010, /* frame has CRC protection         */
    MAD_FLAG_COPYRIGHT    = 0x0020, /* frame is copyright               */
    MAD_FLAG_ORIGINAL     = 0x0040, /* frame is original (else copy)    */
    MAD_FLAG_PADDING      = 0x0080, /* frame has additional slot        */
    MAD_FLAG_I_STEREO     = 0x0100, /* uses intensity joint stereo      */
    MAD_FLAG_MS_STEREO    = 0x0200, /* uses middle/side joint stereo    */
    MAD_FLAG_FREEFORMAT   = 0x0400, /* uses free format bitrate         */
    MAD_FLAG_LSF_EXT      = 0x1000, /* lower sampling freq. extension   */
    MAD_FLAG_MC_EXT       = 0x2000, /* multichannel audio extension     */
    MAD_FLAG_MPEG_2_5_EXT = 0x4000  /* MPEG 2.5 (unofficial) extension  */
} MAD_FLAG;

typedef enum __MAD_PRIVATE
{
    MAD_PRIVATE_NONE   = 0x0000,
    MAD_PRIVATE_HEADER = 0x0100,    /* header private bit               */
    MAD_PRIVATE_III    = 0x001f     /* Layer III private bits (up to 5) */
} MAD_PRIVATE;


typedef struct _MAD_HEADER
{
    MAD_LAYER    layer;             /* audio layer (1, 2, or 3)         */
    MAD_MODE     mode;              /* channel mode (see above)         */
    SWORD        mode_extension;    /* additional mode info             */
    MAD_EMPHASIS emphasis;          /* de-emphasis to use (see above)   */
    U32          bitrate;           /* stream bitrate (bps)             */
    UWORD        samplerate;        /* sampling frequency (Hz)          */
    UWORD        crc_check;         /* frame CRC accumulator            */
    UWORD        crc_target;        /* final target CRC checksum        */
    MAD_FLAG     flags;             /* flags                            */
    MAD_PRIVATE  private_bits;      /* private bits                     */
} MAD_HEADER;

typedef struct __MAD_FRAME
{
    MAD_HEADER  header;                 /* MPEG audio header                */
    SWORD       options;                /* decoding options (from stream)   */
    MAD_FIXED   sbsample[2][36][32];    /* synthesis subband filter samples */
    MAD_FIXED   (*overlap)[2][32][18];  /* Layer III block overlap data     */
} MAD_FRAME;

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/

void       mad_frame_init(MAD_FRAME *frame);
void       mad_frame_mute(MAD_FRAME *frame);
void       mad_frame_finish(MAD_FRAME *frame);
MAD_RETVAL mad_frame_decode(MAD_FRAME *frame, MAD_STREAM *stream);

MAD_RETVAL mad_header_decode(MAD_HEADER *header, MAD_STREAM *stream);
#define    mad_header_finish(header)        /* nothing */

#endif /* __MAD_FRAME_H */
/***************************************************************************************************
    end of file: mad_frame.h
 **************************************************************************************************/
