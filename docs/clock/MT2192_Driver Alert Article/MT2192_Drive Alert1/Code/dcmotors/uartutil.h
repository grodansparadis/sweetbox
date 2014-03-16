//********************************************************************************
//*        Name:  uartutil.h                                                     *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This file contains all I/O functions used by the PIC "C" compiler.       * 
//* Included are low level functions to initialize the USART, read the USART,    * 
//* and write to the USART.  Also included are the Standard "C" I/O functions    *
//* such as printf, scanf, getc, putc.  A ring buffer will be used to buffer the *
//* serial input from the USART, so that data between the host and the PIC       *
//* micro-controller are not lost.                                               *
//*                                                                              *
//*      Future upgrades to this application include: File I/O, I2C, and         *
//* Internet networking.                                                         *
//*                                                                              *
//*                                                                              *
//********************************************************************************

#ifndef UARTUTIL_LOADED
#define UARTUTIL_LOADED

// Include file headers here
#include "flags.h"

#ifdef DSPIC30F2010     
#include <p30F2010.h>					// Register definitions for DSPIC30F2010
#endif

#ifdef DSPIC30F4011     
#include <p30F4011.h>					// Register definitions for DSPIC30F4011
#endif

#ifdef DSPIC30F6014     
#include <p30F6014.h>					// Register definitions for DSPIC30F6014
#endif
 
  
//#include <stdarg.h>
//#include <ctype.h>
#include <string.h>
//#include <stdio.h>
#include "my_types.h"

// My C definitions:

#define dec Dec
#define hex Hex
#define hex2 Hex2
#define printf Printf
#define getc GetByte
#define getch GetByte
#define getchar GetByte
#define putchar SendByte
#define putc SendByte
#define putch SendByte


// UART Utility Functions
 
void InitializeUart1(unsigned long The_Baud_Rate);					// Initialize UART on the DSPIC30F4011
void InitializeUart2(unsigned long The_Baud_Rate);					// Initialize UART on the DSPIC30F4011
void InitPorts(void);						// Initialize PORTS on the DSPIC30F2010
void SetupUART(void);						// Initialize UART on the DSPIC30F2010
byte GetByte(void);                         // Read a byte from the serial port UART 1 on the DSPIC30F4011
byte GetByte2(void);                         // Read a byte from the serial port UART 2 on the DSPIC30F4011
void SendByte(byte c);                      // Write a byte to the serial port on the DSPIC30F4011
void SendByte2(byte c);                      // Write a byte to the serial port on the DSPIC30F4011
void Printf(char *s);						// DSPIC30FXXXX version of stdio printf
void Printf_2(char *s);						// DSPIC30FXXXX version of stdio printf
void Dec(int Value); 						// DSPIC30FXXXX version of Parallax Stamp dec 
void Dec_2(int Value); 						// DSPIC30FXXXX version of Parallax Stamp dec 
void Hex2(byte Value);						// DSPIC30FXXXX version of Parallax Stamp hex2
void Hex2_2(byte Value);						// DSPIC30FXXXX version of Parallax Stamp hex2
void Hex(word Value);						// DSPIC30FXXXX version of Parallax Stamp hex
void Hex_2(word Value);						// DSPIC30FXXXX version of Parallax Stamp hex
#ifdef NEW_CODE
int Printf(const char *format, ...); 		// DSPIC30FXXXX version of stdio printf
void Puts(char *s);							// DSPIC30FXXXX version of stdio puts 

#endif				 
 

#endif
