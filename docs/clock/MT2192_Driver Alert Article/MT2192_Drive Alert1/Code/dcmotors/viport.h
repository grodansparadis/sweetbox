//*********************************************************************************
//*                                                                               *
//*           Name:  viport.h                                                     *
//* Project Number:  MT2192                                                       *                                                   
//*           Date:  10/15/2007                                                   *                                                  
//*                                                                               *
//*   Description:                                                                * 
//*   	This is a DSPIC utility  which will implement the following               * 
//* "C" versions of the BS2 Stamp functions: high, low, toggle, input, output,    *
//* pulsin, pulsout, shiftin, shiftout.  For test purposes, switches are          *
//* connected to Port D in order select each pin's state.  It also provides       *
//* two 8-Bit virtual ports for the DSPIC30F2010, Virtual PORT B and PORT E.      *              
//*                                                                               * 
//*     Future upgrades to this application include: Implementing the Stamp       *
//* BS2 functions: pwm, dtmfout and freqout and diagnostics functions.            *
//*                                                                               *
//* Note:                                                                         *
//* 	1) The names of all BS2 equivalent functions use lower case.              *
//*                                                                               *
//*     2) The following BS2 to eZ80F91 I/O pin map is used by most of the        *
//*        Port Utility functions:                                                *                                                                                                             *
//*                                                                               *
//*       BS2        PIC        PIC           PIC                                 *
//*       PIN        PIN        PORT          BIT                                 *
//*     ---------------------------------------------                             *
//*         0         2           0             0                                 *
//*         1         3           0             1                                 *
//*         2         4           0             2                                 *
//*         3         5        A  0             3                                 *
//*         4         6           0             4                                 *
//*         5         7           0             5                                 *
//*         6        14           0             6                                 *
//*     ---------------------------------------------                             *
//*         7        33           1             0                                 *
//*         8        34           1             1                                 *
//*         9        35           1             2                                 *
//*        10        36           1             3                                 *
//*        11        37        B  1             4                                 *
//*        12        38           1             5                                 *
//*        13        39           1             6                                 *
//*        14        40           1             7                                 *
//*     ---------------------------------------------                             *
//*        15        15           2             0                                 *
//*        16        16           2             1                                 *
//*        17        17           2             2                                 *
//*        18        18           2             3                                 *
//*        19        23        C  2             4                                 *
//*        20        24           2             5                                 *
//*        21        25           2             6                                 *
//*        22        26           2             7                                 *
//*     ---------------------------------------------                             *
//*        23        19           3             0                                 *
//*        24        20           3             1                                 *
//*        25        21           3             2                                 *
//*        26        22           3             3                                 *
//*        27        27        D  3             4                                 *
//*        28        28           3             5                                 *
//*        29        29           3             6                                 *
//*        30        30           3             7                                 *
//*     ---------------------------------------------                             *
//*        31         8           4             0                                 *
//*        32         9        E  4             1                                 *
//*        33        10           4             2                                 *
//*     ---------------------------------------------                             *
//*                                                                               *
//*                                                                               *
//*********************************************************************************
       
  
// Include file headers here
#include "flags.h"

#ifdef DSPIC30F2010     
#include <p30F2010.h>					// Register definitions for DSPIC30F2010
#endif

#ifdef DSPIC30F6014     
#include <p30F6014.h>					// Register definitions for DSPIC30F6014
#endif
 
  
//#include <stdarg.h>
//#include <ctype.h>
#include <string.h>
#include <uart.h>
#include <ports.h>
//#include <stdio.h>
#include "my_types.h"
// #include "portutil.h"                                   // Timer utility functions
#include "timeutil.h"                                   // Timer utility functions
#include "stdio.h"                                      // I/O support functions
 
#ifndef VIPORTUTIL_LOADED
#define VIPORTUTIL_LOADED

 
// Declare global types here
 

enum Port_T {PORT_A, PORT_B, PORT_C, PORT_D, PORT_E, PORT_F, PORT_G, PORT_H, PORT_J}; 
 
// Define data structure for virtual 8-Bit Port.  This can be 
// extended to 16-Bit or 32-Bit virtual ports similar to the BS2/BSX Stamp

typedef volatile union  {
    byte Port_Value;		// The port value as a byte
    struct {
        byte VR_BIT_0:1;
        byte VR_BIT_1:1;
        byte VR_BIT_2:1;
        byte VR_BIT_3:1;
        byte VR_BIT_4:1;
        byte VR_BIT_5:1;
        byte VR_BIT_6:1;
        byte VR_BIT_7:1;
    } ;
 
} VR_8_BIT_PORT_T;   

 
// Declare function headers here

 
  
//#ifdef STDIO_INTERRUPTS

// Include stdio interrupt handlers for USART

//#include "stdio.inc"
//#endif



// Functions for handling virtual ports
extern VR_8_BIT_PORT_T Read_Virtual_Port(byte Port_ID);
extern void Write_Virtual_Port(byte Port_ID, VR_8_BIT_PORT_T The_Data);
 

#endif
