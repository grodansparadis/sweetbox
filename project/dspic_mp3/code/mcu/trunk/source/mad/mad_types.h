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
    File:            mad_types.h
    Description:

 ***************************************************************************************************
    History:         2007/12/10 [GS]  File created


 **************************************************************************************************/

#ifndef __MAD_TYPES_H
#define __MAD_TYPES_H

/***************************************************************************************************
    Included Files
 **************************************************************************************************/

#include <limits.h>     /* for CHAR_BITS definition */

/***************************************************************************************************
    Definitions
 **************************************************************************************************/

#ifndef NULL
#define NULL    ((void*)0)
#endif

/***************************************************************************************************
    Public Types
 **************************************************************************************************/

typedef unsigned char           U08;
typedef unsigned int            U16;
typedef unsigned long           U32;
typedef unsigned long long      U64;

typedef signed   char           S08;
typedef signed   int            S16;
typedef signed   long           S32;

typedef          float          FLOAT;
typedef          long double    DOUBLE;

typedef          U16            UWORD;          /* machine word unsigned type */
typedef          S16            SWORD;          /* machine word signed type   */

typedef enum __BOOL
{
    FALSE,
    TRUE
}BOOL;

typedef          SWORD          MAD_FIXED;      /* fixed pointer type, Q1.15 */

typedef enum __MAD_RETVAL
{
    RET_ERR = (-1),
    RET_OK  =  (0)
} MAD_RETVAL;

/***************************************************************************************************
    Global Variables
 **************************************************************************************************/

/***************************************************************************************************
    Public Function Prototypes
 **************************************************************************************************/


#endif /* __MAD_TYPES_H */
/***************************************************************************************************
    end of file: mad_types.h
 **************************************************************************************************/
