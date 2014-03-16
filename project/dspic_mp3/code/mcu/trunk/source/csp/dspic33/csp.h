/* *************************************************************************************************
     Project:         dsPIC33 Family CPU Support Library
     Author:          [GS] Gamma SPb (Ñ) 2007, http://www.gamma.spb.ru

   *************************************************************************************************
     Distribution:

     dsPIC33 Family Peripheral Support Library
     ----------------------------------------
     Copyright © 2007 Gamma SPb

   *************************************************************************************************
     MCU Family:      dsPIC33
     Compiler:        Microchip C30 3.01

   *************************************************************************************************
     File:            csp.h
     Description:

   ********************************************************************************************** */

#ifndef __CSP_H
#define __CSP_H

#include <limits.h>

/* Global types definition */
/* ----------------------- */

typedef unsigned char        U08;
typedef unsigned int         U16;
typedef unsigned long        U32;
typedef unsigned long long   U64;

typedef signed   char        S08;
typedef signed   int         S16;
typedef signed   long        S32;

typedef          float       FLOAT;
typedef          long double DOUBLE;

typedef          U16         UWORD;
typedef          S16         SWORD;

typedef          UWORD       BOOL;

#ifndef     TRUE
#define     TRUE        1
#endif

#ifndef     FALSE
#define     FALSE       0
#endif


#endif  /* __CSP_H */
/* ********************************************************************************************** */
/* end of file: csp.h*/

