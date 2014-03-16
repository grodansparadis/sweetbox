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
    File:            mad_layer3.c
    Description:

 ***************************************************************************************************
    History:         2007/12/10 [GS]  File created


 **************************************************************************************************/

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

#include <string.h>
#include <stdio.h>
#include <assert.h>

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

/* Scalefactor bit lengths -
   derived from section 2.4.2.7 of ISO/IEC 11172-3
 */

struct
{
    U08 slen1;
    U08 slen2;
} const sflen_table[16] =
{
    { 0, 0}, { 0, 1}, { 0, 2}, { 0, 3},
    { 3, 0}, { 1, 1}, { 1, 2}, { 1, 3},
    { 2, 1}, { 2, 2}, { 2, 3}, { 3, 1},
    { 3, 2}, { 3, 3}, { 4, 2}, { 4, 3}
};


/* Number of LSF scalefactor band values -
   derived from section 2.4.3.2 of ISO/IEC 13818-3
 */

U08 const nsfb_table[6][3][4] =
{
    {
        {  6,  5,  5, 5},
        {  9,  9,  9, 9},
        {  6,  9,  9, 9}
    },

    {
        {  6,  5,  7, 3},
        {  9,  9, 12, 6},
        {  6,  9, 12, 6}
    },

    {
        { 11, 10,  0, 0},
        { 18, 18,  0, 0},
        { 15, 18,  0, 0}
    },

    {
        {  7,  7,  7, 0},
        { 12, 12, 12, 0},
        {  6, 15, 12, 0}
    },

    {
        {  6,  6,  6, 3},
        { 12,  9,  9, 6},
        {  6, 12,  9, 6}
    },

    {
        {  8,  8,  5, 0},
        { 15, 12,  9, 0},
        {  6, 18,  9, 0}
    }
};


/* MPEG-1 scalefactor band widths -
   derived from Table B.8 of ISO/IEC 11172-3
 */

U08 const sfb_48000_long[] =
{
     4,  4,  4,  4,  4,  4,  6,  6,  6,   8,  10,
    12, 16, 18, 22, 28, 34, 40, 46, 54,  54, 192
};


U08 const sfb_44100_long[] =
{
     4,  4,  4,  4,  4,  4,  6,  6,  8,   8,  10,
    12, 16, 20, 24, 28, 34, 42, 50, 54,  76, 158
};


U08 const sfb_32000_long[] =
{
     4,  4,  4,  4,  4,  4,  6,  6,  8,  10,  12,
    16, 20, 24, 30, 38, 46, 56, 68, 84, 102,  26
};


U08 const sfb_48000_short[] =
{
     4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  6,
     6,  6,  6,  6,  6, 10, 10, 10, 12, 12, 12, 14, 14,
    14, 16, 16, 16, 20, 20, 20, 26, 26, 26, 66, 66, 66
};


U08 const sfb_44100_short[] =
{
     4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  6,
     6,  6,  8,  8,  8, 10, 10, 10, 12, 12, 12, 14, 14,
    14, 18, 18, 18, 22, 22, 22, 30, 30, 30, 56, 56, 56
};


U08 const sfb_32000_short[] =
{
     4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  6,
     6,  6,  8,  8,  8, 12, 12, 12, 16, 16, 16, 20, 20,
    20, 26, 26, 26, 34, 34, 34, 42, 42, 42, 12, 12, 12
};


U08 const sfb_48000_mixed[] =
{
    /* long */   4,  4,  4,  4,  4,  4,  6,  6,
    /* short */  4,  4,  4,  6,  6,  6,  6,  6,  6, 10,
    10, 10, 12, 12, 12, 14, 14, 14, 16, 16,
    16, 20, 20, 20, 26, 26, 26, 66, 66, 66
};


U08 const sfb_44100_mixed[] =
{
    /* long */   4,  4,  4,  4,  4,  4,  6,  6,
    /* short */  4,  4,  4,  6,  6,  6,  8,  8,  8, 10,
    10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
    18, 22, 22, 22, 30, 30, 30, 56, 56, 56
};


U08 const sfb_32000_mixed[] =
{
    /* long */   4,  4,  4,  4,  4,  4,  6,  6,
    /* short */  4,  4,  4,  6,  6,  6,  8,  8,  8, 12,
    12, 12, 16, 16, 16, 20, 20, 20, 26, 26,
    26, 34, 34, 34, 42, 42, 42, 12, 12, 12
};

/* MPEG-2 scalefactor band widths -
   derived from Table B.2 of ISO/IEC 13818-3
 */

U08 const sfb_24000_long[] =
{
     6,  6,  6,  6,  6,  6,  8, 10, 12,  14,  16,
    18, 22, 26, 32, 38, 46, 54, 62, 70,  76,  36
};


U08 const sfb_22050_long[] =
{
     6,  6,  6,  6,  6,  6,  8, 10, 12,  14,  16,
    20, 24, 28, 32, 38, 46, 52, 60, 68,  58,  54
};

#define sfb_16000_long  sfb_22050_long

U08 const sfb_24000_short[] =
{
     4,  4,  4,  4,  4,  4,  4,  4,  4,  6,  6,  6,  8,
     8,  8, 10, 10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
    18, 24, 24, 24, 32, 32, 32, 44, 44, 44, 12, 12, 12
};


U08 const sfb_22050_short[] =
{
     4,  4,  4,  4,  4,  4,  4,  4,  4,  6,  6,  6,  6,
     6,  6,  8,  8,  8, 10, 10, 10, 14, 14, 14, 18, 18,
    18, 26, 26, 26, 32, 32, 32, 42, 42, 42, 18, 18, 18
};


U08 const sfb_16000_short[] =
{
     4,  4,  4,  4,  4,  4,  4,  4,  4,  6,  6,  6,  8,
     8,  8, 10, 10, 10, 12, 12, 12, 14, 14, 14, 18, 18,
    18, 24, 24, 24, 30, 30, 30, 40, 40, 40, 18, 18, 18
};


U08 const sfb_24000_mixed[] =
{
    /* long */   6,  6,  6,  6,  6,  6,
    /* short */  6,  6,  6,  8,  8,  8, 10, 10, 10, 12,
    12, 12, 14, 14, 14, 18, 18, 18, 24, 24,
    24, 32, 32, 32, 44, 44, 44, 12, 12, 12
};


U08 const sfb_22050_mixed[] =
{
    /* long */   6,  6,  6,  6,  6,  6,
    /* short */  6,  6,  6,  6,  6,  6,  8,  8,  8, 10,
    10, 10, 14, 14, 14, 18, 18, 18, 26, 26,
    26, 32, 32, 32, 42, 42, 42, 18, 18, 18
};


U08 const sfb_16000_mixed[] =
{
    /* long */   6,  6,  6,  6,  6,  6,
    /* short */  6,  6,  6,  8,  8,  8, 10, 10, 10, 12,
    12, 12, 14, 14, 14, 18, 18, 18, 24, 24,
    24, 30, 30, 30, 40, 40, 40, 18, 18, 18
};


/* MPEG 2.5 scalefactor band widths -
   derived from public sources
 */
#define sfb_12000_long  sfb_16000_long
#define sfb_11025_long  sfb_12000_long

U08 const sfb_8000_long[] =
{
    12, 12, 12, 12, 12, 12, 16, 20, 24,  28,  32,
    40, 48, 56, 64, 76, 90,  2,  2,  2,   2,   2
};

#define sfb_12000_short  sfb_16000_short
#define sfb_11025_short  sfb_12000_short

U08 const sfb_8000_short[] =
{
     8,  8,  8,  8,  8,  8,  8,  8,  8, 12, 12, 12, 16,
    16, 16, 20, 20, 20, 24, 24, 24, 28, 28, 28, 36, 36,
    36,  2,  2,  2,  2,  2,  2,  2,  2,  2, 26, 26, 26
};

#define sfb_12000_mixed  sfb_16000_mixed
#define sfb_11025_mixed  sfb_12000_mixed

/* the 8000 Hz short block scalefactor bands do not break after
   the first 36 frequency lines, so this is probably wrong */

U08 const sfb_8000_mixed[] =
{
    /* long */  12, 12, 12,
    /* short */  4,  4,  4,  8,  8,  8, 12, 12, 12, 16, 16, 16,
    20, 20, 20, 24, 24, 24, 28, 28, 28, 36, 36, 36,
     2,  2,  2,  2,  2,  2,  2,  2,  2, 26, 26, 26
};

struct
{
    U08 const *l;
    U08 const *s;
    U08 const *m;
} const sfbwidth_table[9] =
{
    {sfb_48000_long, sfb_48000_short, sfb_48000_mixed},
    {sfb_44100_long, sfb_44100_short, sfb_44100_mixed},
    {sfb_32000_long, sfb_32000_short, sfb_32000_mixed},
    {sfb_24000_long, sfb_24000_short, sfb_24000_mixed},
    {sfb_22050_long, sfb_22050_short, sfb_22050_mixed},
    {sfb_16000_long, sfb_16000_short, sfb_16000_mixed},
    {sfb_12000_long, sfb_12000_short, sfb_12000_mixed},
    {sfb_11025_long, sfb_11025_short, sfb_11025_mixed},
    { sfb_8000_long,  sfb_8000_short,  sfb_8000_mixed}
};


/* Scalefactor band preemphasis (used only when preflag is set) -
   derived from Table B.6 of ISO/IEC 11172-3
 */

U08 const pretab[22] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 3, 3, 2, 0
};


/* Coefficients for LSF intensity stereo processing -
   derived from section 2.4.3.2 of ISO/IEC 13818-3

   is_lsf_table[0][i] = (1 / sqrt(sqrt(2)))^(i + 1)
   is_lsf_table[1][i] = (1 /      sqrt(2)) ^(i + 1)
 */

MAD_FIXED const is_lsf_table[2][15] =
{
    {
        27554 /* 0.840896415 */,
        23170 /* 0.707106781 */,
        19484 /* 0.594603558 */,
        16384 /* 0.500000000 */,
        13777 /* 0.420448208 */,
        11585 /* 0.353553391 */,
         9742 /* 0.297301779 */,
         8192 /* 0.250000000 */,
         6889 /* 0.210224104 */,
         5793 /* 0.176776695 */,
         4871 /* 0.148650889 */,
         4096 /* 0.125000000 */,
         3444 /* 0.105112052 */,
         2896 /* 0.088388348 */,
         2435 /* 0.074325445 */
    }, {
        23170 /* 0.707106781 */,
        16384 /* 0.500000000 */,
        11585 /* 0.353553391 */,
         8192 /* 0.250000000 */,
         5793 /* 0.176776695 */,
         4096 /* 0.125000000 */,
         2896 /* 0.088388348 */,
         2048 /* 0.062500000 */,
         1448 /* 0.044194174 */,
         1024 /* 0.031250000 */,
          724 /* 0.022097087 */,
          512 /* 0.015625000 */,
          362 /* 0.011048543 */,
          256 /* 0.007812500 */,
          181 /* 0.005524272 */
    }
};

/***************************************************************************************************
    Public Data
 **************************************************************************************************/

/***************************************************************************************************
    Private Function Prototypes
 **************************************************************************************************/

/***************************************************************************************************
    External Function Prototypes
 **************************************************************************************************/

void *mad_stream_alloc(UWORD size);
void *mad_frame_alloc(UWORD size, UWORD el_size);

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

// --------------------------------------
// NAME:    III_sideinfo()
// DESCRIPTION: decode frame side information from a bitstream


static MAD_ERROR mad_III_sideinfo (MAD_BITPTR  *ptr,
                            UWORD       nch,
                            SWORD       lsf,
                            SIDEINFO    *si,
                            UWORD       *data_bitlen,
                            UWORD       *priv_bitlen
                           )
{
    UWORD      ngr, gr, ch, i;
    MAD_ERROR  result = MAD_ERROR_NONE;

    *data_bitlen = 0;
    *priv_bitlen = lsf ? ((nch == 1) ? 1 : 2) : ((nch == 1) ? 5 : 3);

    si->main_data_begin = mad_bit_read(ptr, lsf ? 8 : 9);
    si->private_bits    = mad_bit_read(ptr, *priv_bitlen);

    ngr = 1;

    if (!lsf)
    {
        ngr = 2;
        for (ch = 0; ch < nch; ++ch)
            si->scfsi[ch] = mad_bit_read(ptr, 4);
    }

    for (gr = 0; gr < ngr; ++gr)
    {
        GRANULE *granule = &si->gr[gr];

        for (ch = 0; ch < nch; ++ch)
        {
            CHANNEL *channel              = &granule->ch[ch];
            channel->part2_3_length       = mad_bit_read(ptr, 12);
            channel->big_values           = mad_bit_read(ptr, 9);
            channel->global_gain          = mad_bit_read(ptr, 8);
            channel->scalefac_compress    = mad_bit_read(ptr, lsf ? 9 : 4);

            *data_bitlen += channel->part2_3_length;

            if (channel->big_values > 288 &&
                result == 0
               )
                result = MAD_ERROR_BADBIGVALUES;

            channel->flags = 0;

            /* window_switching_flag */

            if (mad_bit_read(ptr, 1))
            {
                channel->block_type = mad_bit_read(ptr, 2);

                if (channel->block_type == 0 &&
                    result == 0
                   )
                    result = MAD_ERROR_BADBLOCKTYPE;

                if (!lsf && channel->block_type == 2 &&
                     si->scfsi[ch]                   &&
                     result == 0
                   )
                    result = MAD_ERROR_BADSCFSI;

                channel->region0_count = 7;
                channel->region1_count = 36;

                if (mad_bit_read(ptr, 1))
                    channel->flags |= mixed_block_flag;

                else if (channel->block_type == 2)
                    channel->region0_count = 8;

                for (i = 0; i < 2; ++i)
                    channel->table_select[i] = mad_bit_read(ptr, 5);

#if defined(__DEBUG__)
                channel->table_select[2] = 4;  /* not used */
#endif

                for (i = 0; i < 3; ++i)
                    channel->subblock_gain[i] = mad_bit_read(ptr, 3);
            }
            else
            {
                channel->block_type = 0;

                for (i = 0; i < 3; ++i)
                    channel->table_select[i] = mad_bit_read(ptr, 5);

                channel->region0_count = mad_bit_read(ptr, 4);
                channel->region1_count = mad_bit_read(ptr, 3);
            }

            /* [preflag,] scalefac_scale, count1table_select */

            channel->flags |= mad_bit_read(ptr, lsf ? 2 : 3);
        } /* by chanels */
    } /* by granules */

    return (result);
}

/**
 * Decode channel scalefactors for LSF from a bitstream
 *
 * @param ptr
 * @param channel
 * @param gr1ch
 * @param mode_extension
 *
 * @return unsigned int
 */
static UWORD mad_III_scalefactors_lsf (MAD_BITPTR *ptr,
                                CHANNEL    *channel,
                                CHANNEL    *gr1ch,
                                SWORD      mode_extension
                               )
{
    MAD_BITPTR start;
    UWORD      scalefac_compress;
    UWORD      index;
    UWORD      slen[4];
    UWORD      part;
    UWORD      n;
    UWORD      i;
    U08 const  *nsfb;

    start = *ptr;

    scalefac_compress = channel->scalefac_compress;

    index = (channel->block_type == 2) ? ((channel->flags & mixed_block_flag) ? 2 : 1) : 0;

    if (!((mode_extension & I_STEREO) && gr1ch))
    {
        if (scalefac_compress < 400)
        {
            slen[0] = (scalefac_compress >> 4) / 5;
            slen[1] = (scalefac_compress >> 4) % 5;
            slen[2] = (scalefac_compress % 16) >> 2;
            slen[3] =  scalefac_compress %  4;

            nsfb    = nsfb_table[0][index];
        }
        else if (scalefac_compress < 500)
        {
            scalefac_compress -= 400;

            slen[0] = (scalefac_compress >> 2) / 5;
            slen[1] = (scalefac_compress >> 2) % 5;
            slen[2] =  scalefac_compress %  4;
            slen[3] = 0;
            nsfb    = nsfb_table[1][index];
        }
        else
        {
            scalefac_compress -= 500;

            slen[0] = scalefac_compress / 3;
            slen[1] = scalefac_compress % 3;
            slen[2] = 0;
            slen[3] = 0;
            nsfb    = nsfb_table[2][index];
            channel->flags |= preflag;
        }

        n = 0;

        for (part = 0; part < 4; ++part)
        {
            for (i = 0; i < nsfb[part]; ++i)
                channel->scalefac[n++] = mad_bit_read(ptr, slen[part]);
        }

        while (n < 39)
            channel->scalefac[n++] = 0;
    }
    else
    {
        /* (mode_extension & I_STEREO) && gr1ch (i.e. ch == 1) */

        scalefac_compress >>= 1;

        if (scalefac_compress < 180)
        {
            slen[0] =  scalefac_compress / 36;
            slen[1] = (scalefac_compress % 36) / 6;
            slen[2] = (scalefac_compress % 36) % 6;
            slen[3] = 0;
            nsfb    = nsfb_table[3][index];
        }
        else if (scalefac_compress < 244)
        {
            scalefac_compress -= 180;

            slen[0] = (scalefac_compress % 64) >> 4;
            slen[1] = (scalefac_compress % 16) >> 2;
            slen[2] =  scalefac_compress %  4;
            slen[3] = 0;
            nsfb    = nsfb_table[4][index];
        }
        else
        {
            scalefac_compress -= 244;

            slen[0] = scalefac_compress / 3;
            slen[1] = scalefac_compress % 3;
            slen[2] = 0;
            slen[3] = 0;
            nsfb    = nsfb_table[5][index];
        }
        n = 0;

        for (part = 0; part < 4; ++part)
        {
            UWORD max, is_pos;

            max = (1 << slen[part]) - 1;

            for (i = 0; i < nsfb[part]; ++i)
            {
                is_pos               = mad_bit_read(ptr, slen[part]);
                channel->scalefac[n] = is_pos;
                gr1ch->scalefac[n++] = (is_pos == max);
            }
        }

        while (n < 39)
        {
            channel->scalefac[n] = 0;
            gr1ch->scalefac[n++] = 0;  /* apparently not illegal */
        }
    }

    return (mad_bit_length(&start, ptr));
}

/**
 * Decode channel scalefactors of one granule from a bitstream
 *
 * @param ptr
 * @param channel
 * @param gr0ch
 * @param scfsi
 *
 * @return UWORD
 */
static UWORD mad_III_scalefactors (MAD_BITPTR *ptr,
                            CHANNEL    *channel,
                            CHANNEL    *gr0ch,
                            UWORD      scfsi
                           )
{
    MAD_BITPTR start;
    UWORD slen1;
    UWORD slen2;
    UWORD sfbi;

    start = *ptr;

    slen1 = sflen_table[channel->scalefac_compress].slen1;
    slen2 = sflen_table[channel->scalefac_compress].slen2;

    if (channel->block_type == 2)
    {
        UWORD nsfb;

        sfbi = 0;
        nsfb = (channel->flags & mixed_block_flag) ? 8 + 3 * 3 : 6 * 3;

        while (nsfb--)
            channel->scalefac[sfbi++] = mad_bit_read(ptr, slen1);

        nsfb = 6 * 3;

        while (nsfb--)
            channel->scalefac[sfbi++] = mad_bit_read(ptr, slen2);

        nsfb = 1 * 3;

        while (nsfb--)
            channel->scalefac[sfbi++] = 0;
    }
    else
    {
        /* channel->block_type != 2 */

        if (scfsi & 0x8)
        {
            for (sfbi = 0; sfbi < 6; ++sfbi)
                channel->scalefac[sfbi] = gr0ch->scalefac[sfbi];
        }
        else
        {
            for (sfbi = 0; sfbi < 6; ++sfbi)
                channel->scalefac[sfbi] = mad_bit_read(ptr, slen1);
        }

        if (scfsi & 0x4)
        {
            for (sfbi = 6; sfbi < 11; ++sfbi)
                channel->scalefac[sfbi] = gr0ch->scalefac[sfbi];
        }
        else
        {
            for (sfbi = 6; sfbi < 11; ++sfbi)
                channel->scalefac[sfbi] = mad_bit_read(ptr, slen1);
        }

        if (scfsi & 0x2)
        {
            for (sfbi = 11; sfbi < 16; ++sfbi)
                channel->scalefac[sfbi] = gr0ch->scalefac[sfbi];
        }
        else
        {
            for (sfbi = 11; sfbi < 16; ++sfbi)
                channel->scalefac[sfbi] = mad_bit_read(ptr, slen2);
        }

        if (scfsi & 0x1)
        {
            for (sfbi = 16; sfbi < 21; ++sfbi)
                channel->scalefac[sfbi] = gr0ch->scalefac[sfbi];
        }
        else
        {
            for (sfbi = 16; sfbi < 21; ++sfbi)
                channel->scalefac[sfbi] = mad_bit_read(ptr, slen2);
        }

        channel->scalefac[21] = 0;
    }

    return (mad_bit_length(&start, ptr));
}

/**
 * Calculate scalefactor exponents
 *
 * @param channel
 * @param sfbwidth
 * @param exponents
 */
static void mad_III_exponents (CHANNEL   *channel,
                        U08 const *sfbwidth,
                        SWORD     exponents[39]
                       )
{
    SWORD gain;
    UWORD scalefac_multiplier;
    UWORD sfbi;

    gain = (SWORD) channel->global_gain - 210;

    scalefac_multiplier = (channel->flags & scalefac_scale) ? 2 : 1;

    if (channel->block_type == 2)
    {
        UWORD l;
        SWORD gain0;
        SWORD gain1;
        SWORD gain2;

        sfbi = l = 0;

        if (channel->flags & mixed_block_flag)
        {
            UWORD premask;

            premask = (channel->flags & preflag) ? ~0 : 0;

            /* long block subbands 0-1 */

            while (l < 36)
            {
                exponents[sfbi] = gain - (SWORD)((channel->scalefac[sfbi] + (pretab[sfbi] & premask)) << scalefac_multiplier);
                l += sfbwidth[sfbi++];
            }
        }

        /*  this is probably wrong for 8000 Hz short/mixed blocks */

        gain0 = gain - 8 * (SWORD)channel->subblock_gain[0];
        gain1 = gain - 8 * (SWORD)channel->subblock_gain[1];
        gain2 = gain - 8 * (SWORD)channel->subblock_gain[2];

        while (l < 576)
        {
            exponents[sfbi + 0] = gain0 - (SWORD)(channel->scalefac[sfbi + 0] << scalefac_multiplier);
            exponents[sfbi + 1] = gain1 - (SWORD)(channel->scalefac[sfbi + 1] << scalefac_multiplier);
            exponents[sfbi + 2] = gain2 - (SWORD)(channel->scalefac[sfbi + 2] << scalefac_multiplier);

            l    += 3 * sfbwidth[sfbi];
            sfbi += 3;
        }
    }
    else
    {
        /* channel->block_type != 2 */

        if (channel->flags & preflag)
        {
            for (sfbi = 0; sfbi < 22; ++sfbi)
                exponents[sfbi] = gain - (SWORD)((channel->scalefac[sfbi] + pretab[sfbi]) << scalefac_multiplier);
        }
        else
        {
            for (sfbi = 0; sfbi < 22; ++sfbi)
                exponents[sfbi] = gain - (SWORD)(channel->scalefac[sfbi] << scalefac_multiplier);
        }
    }
}


/* we must take care that sz >= bits and sz < sizeof(long) lest bits == 0 */

#define MASK(cache, sz, bits)   (((cache) >> ((sz) - (bits))) & ((1UL << (bits)) - 1))
#define MASK1BIT(cache, sz)     (U32)((cache) & (1UL << ((sz) - 1)))

/**
 * Decode Huffman code words of one channel of one granule
 *
 * @param ptr
 * @param xr
 * @param channel
 * @param sfbwidth
 * @param part2_length
 *
 * @return MAD_ERROR
 */
static MAD_ERROR mad_III_huffdecode(MAD_BITPTR *ptr,
                             MAD_FIXED  xr[576],
                             CHANNEL    *channel,
                             U08 const  *sfbwidth,
                             UWORD      part2_length
                            )
{
    SWORD      exponents[39], exp;
    SWORD      *expptr;
    MAD_BITPTR peek;
    SWORD      bits_left, cachesz;

    MAD_FIXED  *xrptr;
    MAD_FIXED  *sfbound;
    U32        bitcache;

    bits_left = (signed) channel->part2_3_length - (signed) part2_length;

    if (bits_left < 0)
        return (MAD_ERROR_BADPART3LEN);

    mad_III_exponents(channel, sfbwidth, exponents);

    peek = *ptr;

    mad_bit_skip(ptr, bits_left);

    /* align bit reads to byte boundaries ----- takes up to 32 bit ?? */

    cachesz    = mad_bit_bitsleft(&peek);
    cachesz   += ((32 - 1 - 24) + (24 - cachesz)) & ~7;

    bitcache   = mad_bit_read(&peek, cachesz);
    bits_left -= cachesz;

    xrptr = &xr[0];

    /* big_values */

    {
        UWORD                  region;
        UWORD                  rcount;
        struct hufftable const *entry;
        HUFF_PAIR const        *table;
        UWORD                  linbits, startbits, big_values, reqhits;
        MAD_FIXED              reqcache[16];

        sfbound = xrptr + *sfbwidth++;
        rcount  = channel->region0_count + 1;

        entry     = &mad_huff_pair_table[channel->table_select[region = 0]];
        table     = entry->table;
        linbits   = entry->linbits;
        startbits = entry->startbits;

        if (table == 0)
            return (MAD_ERROR_BADHUFFTABLE);

        expptr  = &exponents[0];
        exp     = *expptr++;
        reqhits = 0;

        big_values = channel->big_values;

        while (big_values-- && cachesz + bits_left > 0)
        {
            HUFF_PAIR const *pair;
            UWORD     clumpsz, value;
            MAD_FIXED requantized;

            if (xrptr == sfbound)
            {
                sfbound += *sfbwidth++;

                /* change table if region boundary */

                if (--rcount == 0)
                {
                    if (region == 0)
                        rcount = channel->region1_count + 1;
                    else
                        rcount = 0;  /* all remaining */

                    entry     = &mad_huff_pair_table[channel->table_select[++region]];
                    table     = entry->table;
                    linbits   = entry->linbits;
                    startbits = entry->startbits;

                    if (table == 0)
                        return (MAD_ERROR_BADHUFFTABLE);
                }

                if (exp != *expptr)
                {
                    exp     = *expptr;
                    reqhits = 0;
                }

                ++expptr;
            }

            if (cachesz < 21)
            {
                UWORD bits;

                bits       = ((32 - 1 - 21) + (21 - cachesz)) & ~7;
                bitcache   = (bitcache << bits) | mad_bit_read(&peek, bits);
                cachesz   += bits;
                bits_left -= bits;
            }

            /* hcod (0..19) */

            clumpsz = startbits;
            pair    = &table[MASK(bitcache, cachesz, clumpsz)];

            while (!(pair->final & 0x1))
            {
                cachesz -= clumpsz;
                clumpsz = pair->ptr.bits;
                pair    = &table[pair->ptr.offset + MASK(bitcache, cachesz, clumpsz)];
            }

            cachesz -= pair->value.hlen;

            if (linbits)
            {
                /* x (0..14) */

                value = pair->value.x;

                switch (value)
                {
                    case 0:
                        xrptr[0] = 0;
                        break;

                    case 15:
                        if (cachesz < linbits + 2)
                        {
                            bitcache   = (bitcache << 16) | mad_bit_read(&peek, 16);
                            cachesz   += 16;
                            bits_left -= 16;
                        }
                        value      += MASK(bitcache, cachesz, linbits);
                        cachesz    -= linbits;
                        requantized = mad_port_III_requantize(value, exp);
                        goto x_final;

                    default:
                        if (reqhits & (1 << value))
                            requantized = reqcache[value];
                        else
                        {
                            reqhits |= (1 << value);
                            requantized = reqcache[value] = mad_port_III_requantize(value, exp);
                        }

                      x_final:
                        xrptr[0] = MASK1BIT(bitcache, cachesz--) ?  -requantized : requantized;
                }

                /* y (0..14) */

                value = pair->value.y;

                switch (value)
                {
                    case 0:
                        xrptr[1] = 0;
                        break;

                    case 15:
                        if (cachesz < linbits + 1)
                        {
                            bitcache   = (bitcache << 16) | mad_bit_read(&peek, 16);
                            cachesz   += 16;
                            bits_left -= 16;
                        }
                        value      += MASK(bitcache, cachesz, linbits);
                        cachesz    -= linbits;
                        requantized = mad_port_III_requantize(value, exp);

                        goto y_final;

                    default:
                        if (reqhits & (1 << value))
                            requantized = reqcache[value];
                        else
                        {
                            reqhits |= (1 << value);
                            requantized = reqcache[value] = mad_port_III_requantize(value, exp);
                        }

                      y_final:
                        xrptr[1] = MASK1BIT(bitcache, cachesz--) ? -requantized : requantized;
                }
            }
            else
            {
                /* x (0..1) */

                value = pair->value.x;

                if (value == 0)
                    xrptr[0] = 0;
                else
                {
                    if (reqhits & (1 << value))
                        requantized = reqcache[value];
                    else
                    {
                        reqhits |= (1 << value);
                        requantized = reqcache[value] = mad_port_III_requantize(value, exp);
                    }
                    xrptr[0] = MASK1BIT(bitcache, cachesz--) ? -requantized : requantized;
                }

                /* y (0..1) */

                value = pair->value.y;

                if (value == 0)
                    xrptr[1] = 0;
                else
                {
                    if (reqhits & (1 << value))
                        requantized = reqcache[value];
                    else
                    {
                        reqhits |= (1 << value);
                        requantized = reqcache[value] = mad_port_III_requantize(value, exp);
                    }

                    xrptr[1] = MASK1BIT(bitcache, cachesz--) ? -requantized : requantized;
                }
            }

            xrptr += 2;
        }
    }

    if (cachesz + bits_left < 0)
        return (MAD_ERROR_BADHUFFDATA);  /* big_values overrun */

    /* count1 */

    {
        HUFF_QUAD const *table;
        MAD_FIXED       requantized;

        table       = mad_huff_quad_table[channel->flags & count1table_select];
        requantized = mad_port_III_requantize(1,exp);

        while (cachesz + bits_left > 0 && xrptr <= &xr[572])
        {
            HUFF_QUAD const *quad;

            /* hcod (1..6) */

            if (cachesz < 10)
            {
                bitcache   = (bitcache << 16) | mad_bit_read(&peek, 16);
                cachesz   += 16;
                bits_left -= 16;
            }

            quad = &table[MASK(bitcache, cachesz, 4)];

            /* quad tables guaranteed to have at most one extra lookup */

            if (!(quad->final & 0x1))
            {
                cachesz -= 4;
                quad     = &table[quad->ptr.offset + MASK(bitcache, cachesz, quad->ptr.bits)];
            }

            cachesz -= quad->value.hlen;

            if (xrptr == sfbound)
            {
                sfbound += *sfbwidth++;

                if (exp != *expptr)
                {
                    exp         = *expptr;
                    requantized = mad_port_III_requantize(1,exp);
                }
                ++expptr;
            }

            /* v (0..1) */

            xrptr[0] = quad->value.v ? (MASK1BIT(bitcache, cachesz--) ? -requantized : requantized) : 0;

            /* w (0..1) */

            xrptr[1] = quad->value.w ? (MASK1BIT(bitcache, cachesz--) ? -requantized : requantized) : 0;
            xrptr += 2;

            if (xrptr == sfbound)
            {
                sfbound += *sfbwidth++;

                if (exp != *expptr)
                {
                    exp = *expptr;
                    requantized = mad_port_III_requantize(1, exp);
                }
                ++expptr;
            }

            /* x (0..1) */

            xrptr[0] = quad->value.x ? (MASK1BIT(bitcache, cachesz--) ? -requantized : requantized) : 0;

            /* y (0..1) */

            xrptr[1] = quad->value.y ? (MASK1BIT(bitcache, cachesz--) ? -requantized : requantized) : 0;
            xrptr   += 2;
        }

        if (cachesz + bits_left < 0)
        {

#if defined(__DEBUG__)
            printf("huffman count1 overrun (%d bits)\n",-(cachesz + bits_left));
#endif
            /* technically the bitstream is misformatted, but apparently
               some encoders are just a bit sloppy with stuffing bits
            */
            xrptr -= 4;
        }
    }

    assert(-bits_left <= MAD_BUFFER_GUARD * CHAR_BIT);

#if 0 && defined(__DEBUG__)
    if (bits_left < 0)
        fprintf(stderr, "read %d bits too many\n", -bits_left);
    else if (cachesz + bits_left > 0)
        fprintf(stderr, "%d stuffing bits\n", cachesz + bits_left);
#endif

    /* rzero */

    while (xrptr < &xr[576])
    {
        xrptr[0] = 0;
        xrptr[1] = 0;
        xrptr += 2;
    }

    return (MAD_ERROR_NONE);
}

#undef MASK
#undef MASK1BIT

/**
 * Reorder frequency lines of a short block into subband order
 *
 * @param xr
 * @param channel
 * @param sfbwidth
 */
static void mad_III_reorder (MAD_FIXED xr[576],
                      CHANNEL   *channel,
                      U08 const sfbwidth[39]
                     )
{
    MAD_FIXED tmp[32][3][6];
    UWORD sb, l, f, w, sbw[3], sw[3];

    /* this is probably wrong for 8000 Hz mixed blocks */

    sb = 0;

    if (channel->flags & mixed_block_flag)
    {
        sb = 2;
        l  = 0;

        while (l < 36)
            l += *sfbwidth++;
    }

    for (w = 0; w < 3; ++w)
    {
        sbw[w] = sb;
        sw[w]  = 0;
    }

    f = *sfbwidth++;
    w = 0;

    for (l = 18 * sb; l < 576; ++l)
    {
        if (f-- == 0)
        {
            f = *sfbwidth++ - 1;
            w = (w + 1) % 3;
        }

        tmp[sbw[w]][w][sw[w]++] = xr[l];

        if (sw[w] == 6)
        {
            sw[w] = 0;
            ++sbw[w];
        }
    }

    memcpy(&xr[18 * sb], &tmp[sb], (576 - 18 * sb) * sizeof(MAD_FIXED));
}

/**
 * Perform joint stereo processing on a granule
 *
 * @param xr
 * @param granule
 * @param header
 * @param sfbwidth
 *
 * @return MAD_ERROR
 */
static MAD_ERROR mad_III_stereo(MAD_FIXED  xr[2][576],
                                GRANULE    *granule,
                                MAD_HEADER *header,
                                U08 const  *sfbwidth
                               )
{
    SWORD modes[39];
    UWORD sfbi, l, n, i;

    if (granule->ch[0].block_type != granule->ch[1].block_type ||
        (granule->ch[0].flags & mixed_block_flag) != (granule->ch[1].flags & mixed_block_flag))
        return (MAD_ERROR_BADSTEREO);

    for (i = 0; i < 39; ++i)
        modes[i] = header->mode_extension;

    /* intensity stereo */

    if (header->mode_extension & I_STEREO)
    {
        CHANNEL   *right_ch = &granule->ch[1];
        MAD_FIXED *right_xr = xr[1];

        header->flags |= MAD_FLAG_I_STEREO;

        /* first determine which scalefactor bands are to be processed */

        if (right_ch->block_type == 2)
        {
            UWORD lower, start, max, bound[3], w;

            lower = start = max = bound[0] = bound[1] = bound[2] = 0;
            sfbi  = l = 0;

            if (right_ch->flags & mixed_block_flag)
            {
                while (l < 36)
                {
                    n = sfbwidth[sfbi++];

                    for (i = 0; i < n; ++i)
                    {
                        if (right_xr[i])
                        {
                            lower = sfbi;
                            break;
                        }
                    }
                    right_xr += n;
                    l += n;
                }
                start = sfbi;
            }

            w = 0;

            while (l < 576)
            {
                n = sfbwidth[sfbi++];

                for (i = 0; i < n; ++i)
                {
                    if (right_xr[i])
                    {
                        max = bound[w] = sfbi;
                        break;
                    }
                }

                right_xr += n;
                l        += n;
                w = (w + 1) % 3;
            }

            if (max)
                lower = start;

            /* long blocks */

            for (i = 0; i < lower; ++i)
                modes[i] = header->mode_extension & ~I_STEREO;

            /* short blocks */

            w = 0;

            for (i = start; i < max; ++i)
            {
                if (i < bound[w])
                    modes[i] = header->mode_extension & ~I_STEREO;

                w = (w + 1) % 3;
            }
        }
        else
        {
            /* right_ch->block_type != 2 */

            UWORD bound;

            bound = 0;

            for (sfbi = l = 0; l < 576; l += n)
            {
                n = sfbwidth[sfbi++];

                for (i = 0; i < n; ++i)
                {
                    if (right_xr[i])
                    {
                        bound = sfbi;
                        break;
                    }
                }
                right_xr += n;
            }
            for (i = 0; i < bound; ++i)
                modes[i] = header->mode_extension & ~I_STEREO;
        }

        /* now do the actual processing */

        if (header->flags & MAD_FLAG_LSF_EXT)
        {
            MAD_FIXED const  *lsf_scale;
            UWORD            *illegal_pos;

            illegal_pos = granule[1].ch[1].scalefac;
            lsf_scale   = is_lsf_table[right_ch->scalefac_compress & 0x1];

            /* intensity_scale */

            mad_port_stereo_intensity_lsf(&xr[0][0],
                                          &xr[1][0],
                                          sfbwidth,
                                          &modes[0],
                                          &right_ch->scalefac[0],
                                          lsf_scale,
                                          illegal_pos
                                         );
        }
        else
        {
            /* !(header->flags & MAD_FLAG_LSF_EXT) */

            mad_port_stereo_intensity(&xr[0][0],
                                      &xr[1][0],
                                      sfbwidth,
                                      &modes[0],
                                      &right_ch->scalefac[0]
                                     );
        }
    }

    /* middle/side stereo */

    if (header->mode_extension & MS_STEREO)
    {
        header->flags |= MAD_FLAG_MS_STEREO;
        mad_port_stereo_ms(&xr[0][0],&xr[1][0],sfbwidth, &modes[0]);
    }

    return (MAD_ERROR_NONE);
}

/**
 * Perform subband frequency inversion for odd sample lines
 *
 * @param sample
 * @param sb
 */
static void mad_III_freqinver(MAD_FIXED sample[18][32], UWORD sb)
{
    UWORD     i;
    MAD_FIXED tmp1, tmp2;

    tmp1 = sample[1][sb];
    tmp2 = sample[3][sb];

    for (i = 1; i < 13; i += 4)
    {
        sample[i + 0][sb] = -tmp1;
        tmp1 = sample[i + 4][sb];

        sample[i + 2][sb] = -tmp2;
        tmp2 = sample[i + 6][sb];
    }

    sample[13][sb] = -tmp1;
    tmp1 = sample[17][sb];

    sample[15][sb] = -tmp2;
    sample[17][sb] = -tmp1;

}

MAD_FIXED  __attribute__((space(xmemory))) xr[2][576];

/*-----------------------------------------------------------------
 * NAME:    III_decode()
 * DESCRIPTION: decode frame main_data
 */
static MAD_ERROR mad_III_decode (MAD_BITPTR   *ptr,
                                 MAD_FRAME    *frame,
                                 SIDEINFO     *si,
                                 unsigned int nch
                                )
{
    MAD_HEADER *header = &frame->header;
    UWORD      sfreqi, ngr, gr;
    GRANULE    *granule;
    CHANNEL    *channel;
    U08 const  *sfbwidth[2];

    UWORD      ch;
    MAD_ERROR  error;
    UWORD      sb, l, i, sblimit;
    MAD_FIXED  output[36];

    {
        UWORD sfreq;

        sfreq = header->samplerate;

        if (header->flags & MAD_FLAG_MPEG_2_5_EXT)
            sfreq *= 2;

        /* 48000 => 0, 44100 => 1, 32000 => 2,
           24000 => 3, 22050 => 4, 16000 => 5 */

        sfreqi = ((sfreq >>  7) & 0x000f) + ((sfreq >> 15) & 0x0001) - 8;

        if (header->flags & MAD_FLAG_MPEG_2_5_EXT)
            sfreqi += 3;
    }

    /* scalefactors, Huffman decoding, requantization */

    ngr = (header->flags & MAD_FLAG_LSF_EXT) ? 1 : 2;

    for (gr = 0; gr < ngr; ++gr)
    {
        granule = &si->gr[gr];

        for (ch = 0; ch < nch; ++ch)
        {
            UWORD part2_length;

            channel      = &granule->ch[ch];
            sfbwidth[ch] = sfbwidth_table[sfreqi].l;

            if (channel->block_type == 2)
            {
                sfbwidth[ch] = (channel->flags & mixed_block_flag) ? sfbwidth_table[sfreqi].m : sfbwidth_table[sfreqi].s;
            }
            if (header->flags & MAD_FLAG_LSF_EXT)
            {
                part2_length = mad_III_scalefactors_lsf(ptr,
                                                        channel,
                                                        ch == 0 ? 0 : &si->gr[1].ch[1],
                                                        header->mode_extension
                                                       );
            }
            else
            {
                part2_length = mad_III_scalefactors(ptr,
                                                    channel,
                                                    &si->gr[0].ch[ch],gr == 0 ? 0 : si->scfsi[ch]
                                                   );
            }

            error = mad_III_huffdecode(ptr, xr[ch], channel, sfbwidth[ch], part2_length);

            if (error)
                return (error);
        }

        /* joint stereo processing */

        if (header->mode == MAD_MODE_JOINT_STEREO && header->mode_extension)
        {
            error = mad_III_stereo(xr, granule, header, sfbwidth[0]);

            if (error)
                return (error);
        }


        /* reordering, alias reduction, IMDCT, overlap-add, frequency inversion */

        for (ch = 0; ch < nch; ++ch)
        {
            MAD_FIXED *sample = &frame->sbsample[ch][18 * gr][0];

            channel = &granule->ch[ch];

            if (channel->block_type == 2)
            {
                mad_III_reorder(xr[ch], channel, sfbwidth[ch]);

#if !defined(OPT_STRICT)
                /*
                * According to ISO/IEC 11172-3, "Alias reduction is not applied for
                * granules with block_type == 2 (short block)." However, other
                * sources suggest alias reduction should indeed be performed on the
                * lower two subbands of mixed blocks. Most other implementations do
                * this, so by default we will too.
                */
                if (channel->flags & mixed_block_flag)
                    mad_port_III_alias_reduce(xr[ch], 1-1); /* "-1" to respect to internel rptb */
#endif
            }
            else
                mad_port_III_alias_reduce(xr[ch], 31-1); /* "-1" to respect to internel rptb */

            l = 0;

            /* subbands 0-1 */

            if ((channel->block_type != 2) ||
                (channel->flags & mixed_block_flag)
               )
            {
                UWORD block_type;

                block_type = channel->block_type;

                if (channel->flags & mixed_block_flag)
                    block_type = 0;

                /* long blocks */

                for (sb = 0; sb < 2; ++sb, l += 18)
                {
                    mad_port_III_imdct_36_w(&xr[ch][l], output, block_type);
                    mad_port_III_overlap(output, (*frame->overlap)[ch][sb], sample, sb);
                }
            }
            else
            {
                /* short blocks */
                for (sb = 0; sb < 2; ++sb, l += 18)
                {
                    mad_port_III_imdct_12_w(&xr[ch][l], output);
                    mad_port_III_overlap(output, (*frame->overlap)[ch][sb], sample, sb);
                }
            }
            mad_III_freqinver((MAD_FIXED (*)[32])sample, 1);


            /* (nonzero) subbands 2-31 */

            i = 576;

            while (i > 36 && xr[ch][i - 1] == 0)
                --i;

            sblimit = 32 - (576 - i) / 18;

            if (channel->block_type != 2)
            {
                /* long blocks */

                for (sb = 2; sb < sblimit; ++sb, l += 18)
                {
                    mad_port_III_imdct_36_w(&xr[ch][l], output, channel->block_type);
                    mad_port_III_overlap(output, (*frame->overlap)[ch][sb], sample, sb);
                    if (sb & 1)
                        mad_III_freqinver((MAD_FIXED (*)[32])sample, sb);
                }
            }
            else
            {
                /* short blocks */

                for (sb = 2; sb < sblimit; ++sb, l += 18)
                {
                    mad_port_III_imdct_12_w(&xr[ch][l], output);
                    mad_port_III_overlap(output, (*frame->overlap)[ch][sb], sample, sb);

                    if (sb & 1)
                        mad_III_freqinver((MAD_FIXED (*)[32])sample, sb);
                }
            }

            /* remaining (zero) subbands */

            for (sb = sblimit; sb < 32; ++sb)
            {
                mad_port_III_overlap_z((*frame->overlap)[ch][sb],sample, sb);
                if (sb & 1)
                    mad_III_freqinver((MAD_FIXED (*)[32])sample, sb);
            }
        }
    }

    return (MAD_ERROR_NONE);
}

/***************************************************************************************************
    Public Functions
 **************************************************************************************************/

/**
 * Decode a single Layer III frame
 *
 * @param stream
 * @param frame
 *
 * @return MAD_RETVAL
 */
MAD_RETVAL mad_layer_III (MAD_STREAM *stream, MAD_FRAME *frame)
{
    MAD_HEADER *header = &frame->header;

    UWORD      nch,         priv_bitlen, next_md_begin = 0;
    UWORD      si_len,      data_bitlen, md_len;
    UWORD      frame_space, frame_used,  frame_free;

    MAD_BITPTR ptr;
    SIDEINFO   si;

    MAD_ERROR  error;
    MAD_RETVAL result = RET_OK;

    /* allocate Layer III dynamic structures */

    if (stream->main_data == 0)
    {
        stream->main_data = mad_stream_alloc(MAD_BUFFER_MDLEN);

        if (stream->main_data == 0)
        {
            stream->error = MAD_ERROR_NOMEM;
            return (RET_ERR);
        }
    }

    if (frame->overlap == 0)
    {
        frame->overlap = mad_frame_alloc(2 * 32 * 18, sizeof(MAD_FIXED));

        if (frame->overlap == 0)
        {
            stream->error = MAD_ERROR_NOMEM;
            return (RET_ERR);
        }
    }

    nch    = MAD_NCHANNELS(header);
    si_len = (header->flags & MAD_FLAG_LSF_EXT) ? (nch == 1 ? 9 : 17) : (nch == 1 ? 17 : 32);

    /* check frame sanity */

    if (stream->next_frame - mad_bit_nextbyte(&stream->ptr) < (SWORD)si_len)
    {
        stream->error  = MAD_ERROR_BADFRAMELEN;
        stream->md_len = 0;
        return (RET_ERR);
    }

    /* check CRC word */

    if (header->flags & MAD_FLAG_PROTECTION)
    {
        header->crc_check = mad_bit_crc(stream->ptr, si_len * CHAR_BIT, header->crc_check);

        if (header->crc_check != header->crc_target && !(frame->options & MAD_OPTION_IGNORECRC))
        {
            stream->error = MAD_ERROR_BADCRC;
            result = RET_ERR;
        }
    }

    /* decode frame side information */

    error = mad_III_sideinfo(&stream->ptr, nch, header->flags & MAD_FLAG_LSF_EXT, &si, &data_bitlen, &priv_bitlen);

    if (error &&
        result == RET_OK
       )
    {
        stream->error = error;
        result = RET_ERR;
    }

    header->flags        |= priv_bitlen;
    header->private_bits |= si.private_bits;

    /* find main_data of next frame */

    {
        MAD_BITPTR peek;
        U32        header;

        mad_bit_init(&peek, stream->next_frame);

        header = mad_bit_read(&peek, 32);

        if ((header & 0xffe60000L) /* syncword | layer */ == 0xffe20000L)
        {
            if (!(header & 0x00010000L))  /* protection_bit */
                mad_bit_skip(&peek, 16);  /* crc_check */

            next_md_begin = mad_bit_read(&peek, (header & 0x00080000L) /* ID */ ? 9 : 8);
        }

        mad_bit_finish(&peek);
    }

    /* find main_data of this frame */

    frame_space = stream->next_frame - mad_bit_nextbyte(&stream->ptr);

    if (next_md_begin > si.main_data_begin + frame_space)
        next_md_begin = 0;

    md_len     = si.main_data_begin + frame_space - next_md_begin;
    frame_used = 0;

    if (si.main_data_begin == 0)
    {
        ptr = stream->ptr;
        stream->md_len = 0;
        frame_used = md_len;
    }
    else
    {
        if (si.main_data_begin > stream->md_len)
        {
            if (result == RET_OK)
            {
                stream->error = MAD_ERROR_BADDATAPTR;
                result        = RET_ERR;
            }
        }
        else
        {
            mad_bit_init(&ptr, (U08*)(stream->main_data + stream->md_len - si.main_data_begin));

            if (md_len > si.main_data_begin)
            {
                assert(stream->md_len + md_len - si.main_data_begin <= MAD_BUFFER_MDLEN);
                memcpy((void*)(stream->main_data + stream->md_len), mad_bit_nextbyte(&stream->ptr), frame_used = md_len - si.main_data_begin);
                stream->md_len += frame_used;
            }
        }
    }

    frame_free = frame_space - frame_used;

    /* decode main_data */

    if (result == RET_OK)
    {
        error = mad_III_decode(&ptr, frame, &si, nch);

        if (error)
        {
            stream->error = error;
            result = RET_ERR;
        }
        /* designate ancillary bits */

        stream->anc_ptr    = ptr;
        stream->anc_bitlen = md_len * CHAR_BIT - data_bitlen;
    }


#if 0 && defined(__DEBUG__)
    printf("main_data_begin:%u, md_len:%u, frame_free:%u, "" data_bitlen:%u, anc_bitlen: %u\n",
            si.main_data_begin,    md_len,    frame_free,       data_bitlen, stream->anc_bitlen);
#endif

    /* preload main_data buffer with up to 511 bytes for next frame(s) */

    if (frame_free >= next_md_begin)
    {
        memcpy(stream->main_data, stream->next_frame - next_md_begin, next_md_begin);

        stream->md_len = next_md_begin;
    }
    else
    {
        if (md_len < si.main_data_begin)
        {
            UWORD extra;

            extra = si.main_data_begin - md_len;

            if (extra + frame_free > next_md_begin)
                extra = next_md_begin - frame_free;

            if (extra < stream->md_len)
            {
                memmove(stream->main_data, stream->main_data + stream->md_len - extra, extra);
                stream->md_len = extra;
            }
        }
        else
            stream->md_len = 0;

        memcpy(stream->main_data + stream->md_len, stream->next_frame - frame_free, frame_free);

        stream->md_len += frame_free;
    }

    return (result);
}

/***************************************************************************************************
    end of file: mad_layer3.c
 **************************************************************************************************/

