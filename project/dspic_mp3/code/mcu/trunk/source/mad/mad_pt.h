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
    File:            mad_pt.h
    Description:

 ***************************************************************************************************
    History:         2007/10/05 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_PT_H
#define __MAD_PT_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/

SWORD mad_port_III_requantize(UWORD value, SWORD exp);
void  mad_port_III_alias_reduce(MAD_FIXED *xr, UWORD steps);
void  mad_port_III_imdct_36_w(MAD_FIXED *X, MAD_FIXED *x, UWORD blockType);
void  mad_port_III_imdct_12_w(MAD_FIXED *X, MAD_FIXED *x);
void  mad_port_III_overlap(MAD_FIXED *output, MAD_FIXED *overlap, MAD_FIXED *sample, UWORD sb);
void  mad_port_III_overlap_z(MAD_FIXED *overlap, MAD_FIXED *sample, UWORD sb);

void  mad_port_synth_frame(MAD_FIXED *pcm, MAD_FIXED *subbandSamples, MAD_FIXED *V, UWORD nsamples);

void  mad_port_stereo_ms(MAD_FIXED *xr0, MAD_FIXED *xr1, U08 const *sfbwidth, SWORD *modes);
void  mad_port_stereo_intensity(MAD_FIXED *xr0, MAD_FIXED *xr1, U08 const *sfbwidth, SWORD *modes, UWORD *scalefac);
void  mad_port_stereo_intensity_lsf(MAD_FIXED *xr0, MAD_FIXED *xr1, U08 const *sfbwidth, SWORD *modes, UWORD *scalefac, MAD_FIXED const *lsf_scale, UWORD *illegal_pos);


#endif /* __MAD_PT_H */
/***************************************************************************************************
    end of file: mad_pt.h
 **************************************************************************************************/

