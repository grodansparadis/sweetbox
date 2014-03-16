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
    File:            mad_port_synt.S
    Description:

 ***************************************************************************************************
    History:         2007/05/31 [GS]  File created


 **************************************************************************************************/

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

#include "mad_pt_defs.h"

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

        #define SAMPLES_NUM     W13

        #define SYNT_NIK_STORE  0
        #define SUBBAND_STORE   2
        #define V_STORE         4
        #define DI_STORE        6

/***************************************************************************************************
    Private Types
 **************************************************************************************************/

/***************************************************************************************************
    Private Data
 **************************************************************************************************/

/***************************************************************************************************
    Public Data
 **************************************************************************************************/

        .extern mad_const_psv

        .extern synthesisNik
        .extern Di

/***************************************************************************************************
    Private Function Prototypes
 **************************************************************************************************/

/***************************************************************************************************
    Private Functions
 **************************************************************************************************/

/***************************************************************************************************
    Public Functions
 **************************************************************************************************/

        .section .text, code
        .global  _mad_port_synth_frame

/* -------------------------------------------------------------------------------------------------
       function: mad_port_synth_frame

      prototype: void mad_port_synth_frame (MAD_FIXED *pcm,
                                            MAD_FIXED *subbandSamples,
                                            MAD_FIXED *V,
                                            UWORD nsamples
                                           );

    description: synthesis pcm for one chanel

     parameters: W0 = &pcm[0]               -> pointer to pcm[1152]
                 W1 = &subbandSamples[0][0] -> pointer to subbandSamples[36][32]
                 W2 = &V[0]                 -> pointer to V[512]
                 W3 = nsamples

         return: no return

   ---------------------------------------------------------------------------------------------- */

_mad_port_synth_frame:

        push    PSVPAG                                  /* store PSVPAG */
        push    CORCON                                  /* store CORCON */
        push.d  W8
        push.d  W10
        push.d  W12
        lnk     #8                                      /* link 8 bytes for local storage */

        mov     #(DSP_MODE_FRACTIONAL   | \
                  DSP_MODE_RND_EN       | \
                  DSP_MODE_PSV_EN       | \
                  DSP_MODE_SAT_SUPER    | \
                  DSP_MODE_DATA_SAT_EN  | \
                  DSP_MODE_ACCA_SAT_DIS | \
                  DSP_MODE_ACCB_SAT_DIS | \
                  DSP_MODE_SIGNED),     W8
        mov     W8,                     CORCON          /* set required dsp-core mode */

        mov     #psvpage(mad_const_psv),W8
        mov     W8,                     PSVPAG          /* set required PSV page */


        /* === prepare registers for calculation === */

        mov     #psvoffset(synthesisNik), W8
        mov     W8,                     [W14+SYNT_NIK_STORE]    /* save &synthesisNik[] */

        mov     W1,                     [W14+SUBBAND_STORE]     /* save &subbandSamples[0][0] */
        mov     W2,                     [W14+V_STORE]           /* save &V[0] */

        mov     #psvoffset(Di),         W8
        mov     W8,                     [W14+DI_STORE]          /* save &Di[] */

        mov     W3,                     SAMPLES_NUM             /* loops counter */


        /* === perform calculation === */

synth_step:

        /* W1 = &subbandSamples[i][0]
           W2 = &V[0]

           W7 = &synthesisNik[0]
        */

        /* Shift V - preparing space for new V[0..63] */

        mov     [W14+V_STORE],          W2              /* restore &V[]     */
        mov     #(1023*2),              W8
        add     W2,     W8,             W2              /* W2 = &V[1023]    */

        mov     W2,                     W8              /* W8 = &V[1023]    */
        sub     #(64*2),                W8              /* W8 = &V[1023-64] */

        repeat  #(1024-64-1)
        mov     [W8--],                 [W2--]

        /* calculate new V[0..63] */


        /*  i = [0:2:14]  */

        mov     [W14+V_STORE],          W9
        mov     [W14+V_STORE],          W10
        add     #(32*2),                W10

        mov     [W14+SYNT_NIK_STORE],   W2
        mov     [W14+SYNT_NIK_STORE],   W3
        add     #(32*8*2),              W3

        do      #(8-1),                 1f

        mov     [W14+SUBBAND_STORE],    W8              /* W8 = &subbandSamples[0][0] */
        mov     [W8],                   W4

        clr     A
        clr     B

        do      #(32-1),                2f

        /* W8  = &subbandSamples[]    W4 = subbandSamples[]
           W2  = &synthesisNik[0][]   W5 = synthesisNik[0][]
           W3  = &synthesisNik[1][]   W6 = synthesisNik[1][]

           W9  = V[i]
           W10 = vsymm[i]
        */

        mov     [W2++],                 W5
        mov     [W3++],                 W6

        mac     W4*W5, A, [W8]+=2, W4
2:      mac     W4*W6, B, [W8],    W4

        sac.r   A,      #0,             [W9++]
        sac.r   B,      #0,             [W9++]

        neg     A
        neg     B

        sac.r   A,      #0,             [W10--]
1:      sac.r   B,      #0,             [W10--]


        /*  i = [16]  */

        clr     [W9]


        /*  i = [33]  */

        mov     [W14+SUBBAND_STORE],    W8              /* W8 = subbandSamples[0][0] */
        mov     [W8++],                 W4

        add     #((33-16)*2),           W9              /* W9  = &V[33] */
        add     #((63-16)*2),           W10             /* W10 = &V[63] */

        clr     A

        do      #(32-1),                1f
        mov     [W3++],                 W5
1:      mac     W4*W5, A, [W8]+=2, W4

        sac.r   A,      #0,             [W9++]
        sac.r   A,      #0,             [W10--]


        /* i = [34:2:46] */

        mov     W3,                     W2              /* W2 = &Nik[34][0] */
        add     #(32*7*2),              W3              /* W3 = &Nik[35][0] */

        do      #(7-1),                 1f

        mov     [W14+SUBBAND_STORE],    W8              /* W8 = subbandSamples[0][0] */
        mov     [W8],                   W4

        clr     A
        clr     B

        do      #(32-1),                2f

        mov     [W2++],                 W5
        mov     [W3++],                 W6

        mac     W4*W5, A, [W8]+=2, W4
2:      mac     W4*W6, B, [W8],    W4

        sac.r   A,      #0,             [W9++]
        sac.r   B,      #0,             [W9++]

        sac.r   A,      #0,             [W10--]
1:      sac.r   B,      #0,             [W10--]


        /*  i = [48]  */

        mov     [W14+SUBBAND_STORE],    W8              /* W8 = subbandSamples[0][0] */

        lac     [W8++], #0,             A
        repeat  #(31-1)
        add     [W8++], #0,             A
        neg     A
        sac.r   A,      #0,             [W9]


        /* Calculate PCM  */

        /*
                W0 = &pcm[]

                W1 = &V[64*8]
                W2 = &V[]
                W3 = &Di[]

                W4 = V[]
                W5 = Di[]

                W9  = &Di[0]
                W12 = 0..512

                W10 = storage for W2
                W11 = storage for W3

                W12 = 32*2
        */

        mov     [W14+V_STORE],          W2              /* W2 = &V[0]   */
        mov     [W14+V_STORE],          W1
        add     #((64*8-1)*2),          W1
        inc2    W1,                     W1              /* W1 = &V[512] */

        mov     [W14+DI_STORE],         W9              /* W9 = &Di[0]  */
        mov     #(32*2),                W12             /*  offset      */

        mov     W9,                     W10
        mov     W2,                     W11

        do      #(32-1),                1f              /* for (j = 0; j < 32; j++) */

        mov     [W2],                   W4              /* W4 = V[j]  */
        mov     [W9],                   W5              /* W5 = Di[j] */

        clr     A

        do      #(8-1),                 2f

        mac     W4*W5, A, [W9+W12], W5                  /* even i V[64*i+j]*D[32*i+j], W5 = Di[j+32] */

        add     #(32*2),                W12             /* next offset */
        add     #((32+64)*2),           W2
        mov     [W2],                   W4

        mac     W4*W5, A, [W9+W12], W5                  /* odd i V[64*i+j+32]*D[32*i+j] */

        add     #(32*2),                W12             /* next offset */
        add     #(32*2),                W2
2:      mov     [W2],                   W4

        mov     [W1],                   W4

        mac     W4*W5, A, [W9+W12], W5

        add     #(32*2),                W12             /* next offset */
        sub     #(32*2),                W1
        mov     [W1],                   W4

        mac     W4*W5, A

        add     #(32*2),                W1

        sac.r   A,      #0,             [W0++]          /* store result sample */


        inc2    W10,                    W10
        inc2    W11,                    W11
        inc2    W1,                     W1

        mov     W10,                    W9
        mov     W11,                    W2
1:      mov     #(32*2),                W12



        /* prepare for next samples loop */

        mov     [W14+SUBBAND_STORE],    W1
        add     #(32*2),                W1
        mov     W1,                     [W14+SUBBAND_STORE]     /* next 32 samples */

        dec     SAMPLES_NUM,            SAMPLES_NUM
        bra     NZ,                     synth_step

        /* ============================ */

        ulnk                                            /* unlink stack */
        pop.d   W12
        pop.d   W10
        pop.d   W8
        pop     CORCON                                  /* restore CORCON */
        pop     PSVPAG                                  /* restore PSVPAG */
        return

        .end

/***************************************************************************************************
    end of file: mad_port_synt.S
 **************************************************************************************************/
