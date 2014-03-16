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
//*      Future upgrades to this application include: File I/O, I2c, and         *
//* Internet networking.                                                         *
//*                                                                              *
//*                                                                              *
//********************************************************************************
  
#ifndef UARTUTIL_LOADED  /* prevent multiple loading */
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
 
#include <stdio.h>
#include <dsp.h>
#include <uart.h> 					// USRT library functions
#include <spi.h>
#include <i2c.h>
#include <math.h>
#include <adc12.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <timer.h>                                     // Timer library functions                                                                 
//#include <delays.h>                                     // High resolution delay functions
#include "timeutil.h"                                   // Timer library functions                               
//#include "portutil.h"                                   // My own I/O Port library functions
#include "my_types.h"					// General C/C++ types
//#include "lcd.h"						// LCD Support using SPI2 


  
//********************************************************************************
// Constant Definitions
//********************************************************************************

#define TRUE  1                                         // Boolean TRUE value
#define FALSE 0                                         // Boolean FALSE value
  

//#define ATT           0xA5                            // 5 - Attention, ENQ (for serial handshaking)
//ACK                   0xA6                            // 6 - Acknowledgement (for serial handshaking)
#define ATT             "~"                             // "~"- Attention, ENQ (for serial handshaking)
#define ACK             "#"                             // "#" - Acknowledgement (for serial handshaking)
//#define START           "@"                             // Indicates start of message string
//#define READY           ">"                             // Indicates Stamp is ready to receive next byte

  

// My C definitions:

#define dec Dec
#define hex Hex
#define hex2 Hex2

// UART1 specific functions...

//#define printf Printf
#define getc GetByte
//#define getch GetByte
#define getchar GetByte
#define putchar SendByte
#define putc SendByte
#define putch SendByte

#define getc1 GetByte
#define getch1 getch
#define getchar1 GetByte
#define putchar1 SendByte
#define putc1 SendByte
#define putch1 SendByte

// UART2 specific functions...
#define getc2 GetByte2
//#define getch2 GetByte2
#define getchar2 GetByte2
#define putchar2 SendByte2
#define putc2 SendByte2
#define putch2 SendByte2

//*******************************************************************************
// Define typedef structures
//*******************************************************************************

 
 /* Declare general file function headers */           
//*******************************************************************************
// Define port bit definitions here
//*******************************************************************************

 
#define CR 13														// Carriage Return
#define LF 10														// Line Feed

#define BUFFER_SIZE 256                                  // Size of USART ring buffer, range (0..255). Must be a power of 2
//#define STRING_SIZE 255     /* MAX length of one converted field    */
#define STRING_SIZE 80     /* MAX length of one converted field    */
 
#define BufferSize		20								// Floating point buffer size

 
//********************************************************************************
// Variable declarations
//********************************************************************************

extern char            inpbuf[];                                 // Input command buffer
                                                                 // (USART)
extern char            str[];                                    // Temporary string
extern char            blanks[];                                 // Blank delimiter field
extern char TheOperator;        // Current operator function key: "+","-","*","/"
extern char TheEqualSign;       // Equal sign, "="

// Debug variables used by MPSIM

extern  char InputBuffer[];             			    // Input buffer
extern  char OutputBuffer[];                          	// Output  buffer
extern  byte InputBufferIndex;                        	// Input buffer index
extern  byte OutputBufferIndex; 
 

extern int             result;
 
// Floating Point support variables

extern char Buffer[];
 
// Declare Serial Ring Buffer variables here

extern  volatile int NumberCharacters;                 // Total number of characters in ring buffer.
extern volatile int CharactersToRead;					// Number of characters to be read

extern volatile byte RingBuffer[];                      // Ring Buffer for incomming data from serial port (UART 1)

extern volatile int StartBuffer;                        // Start pointer for ring buffer       
extern volatile int EndBuffer;                          // End pointer for ring buffer
extern volatile byte ReceivedDataFlag;                  // Indicates that data was received into the ring buffer
extern volatile word Timeout;                           // Serial I/O Timeout counter
extern volatile byte TimeoutFlag;                       // Timeout flag

extern  volatile int NumberCharacters2;                 // Total number of characters in ring buffer.
extern volatile int CharactersToRead2;					// Number of characters to be read

extern volatile byte RingBuffer2[];                      // Ring Buffer for incomming data from serial port (UART 2)

extern volatile int StartBuffer2;                        // Start pointer for ring buffer       
extern volatile int EndBuffer2;                          // End pointer for ring buffer
extern volatile byte ReceivedDataFlag2;                  // Indicates that data was received into the ring buffer

extern volatile byte RX_Data_Received; 					// Indicates that data was received into the ring buffer 1
extern volatile byte RX_Data_Received2; 				// Indicates that data was received into the ring buffer 2

extern volatile int SerialCommunicationsError;                   // Serial I/O error flag
                                                        // -1 = Serial Input Bufer overrun error
                                                        // -2 = Communications timeout error
                                                        // -3 = Ring buffer empty error
                  
extern volatile word  SerialInterruptCount;             // Interrupt 0 count
extern volatile word  RxSerialInterruptCount1;          // Interrupt UART 1 RX  count
extern volatile word  RxSerialInterruptCount2;          // Interrupt UART 2 RX  count
extern volatile word  TxSerialInterruptCount1;          // Interrupt UART 1 TX  count
extern volatile word  TxSerialInterruptCount2;          // Interrupt UART 2 TX  count

  

 
//********************************************************************************
// Function Prototypes
//********************************************************************************
 
// General  functions 

void 		InitializeUart1(unsigned long The_Baud_Rate);                     // Initializes the UART 1
void 		InitializeUart2(unsigned long The_Baud_Rate);                     // Initializes the UART 2
void 		RxProcessUart1(void);                        // Services the UART 1 RX interrupt in order to buffer incomming characters
void 		RxProcessUart2(void);                        // Services the UART 2 RX interrupt in order to buffer incomming characters
void 		TxProcessUart1(void);                        // Services the UART 1 TX interrupt in order to buffer incomming characters
void 		TxProcessUart2(void);                        // Services the UART 2 TX interrupt in order to buffer incomming characters

byte     RxDataWaiting(void);                       // Checks to see if the Serial Ring Buffer has data to be read.  Returns TRUE if 
                                                    // there is data to be read and returns FALSE otherwise.  
                          
byte	ReadNoWait(void);                             // Read data from ring buffer without waiting for valid data 

void     send_byte(byte c);                         // Write a character to the USART
byte     get_byte(void);                            // Read a character from the USART
byte 	GetByte(void);                         // Read a byte from the serial port on the DSPIC30F2010
void 	SendByte(byte c);                      // Write a byte to the serial port on the DSPIC30F2010

void     send_byte2(byte c);                         // Write a character to the USART
byte     get_byte2(void);                            // Read a character from the USART
byte 	GetByte2(void);                         // Read a byte from the serial port on the DSPIC30F2010
void 	SendByte2(byte c);                      // Write a byte to the serial port on the DSPIC30F2010

void 		SplitWord(word WordValue,     /* Word value  */
                  	byte *DataByte1,  /* Current data byte (MSB) */
                  	byte *DataByte2);  /* Current data byte (LSB) */

 
void 		SplitInteger(int IntValue,     /* Integer value  */
                  	byte *DataByte1,  /* Current data byte (MSB) */
                  	byte *DataByte2);  /* Current data byte (LSB) */
 
void 		SplitLongInteger(long int LongIntValue,     /* Long integer value   */
                  	byte *DataByte1,  /* Current data byte (MSB) */
                  	byte *DataByte2,  /* Current data byte  */
                  	byte *DataByte3,  /* Current data byte  */
                  	byte *DataByte4);  /* Current data byte (LSB) */

void 		SplitUnsignedLongInteger(unsigned long int UnsignedLongIntValue,     /* Unsigned long integer value */
                  	byte *DataByte1,  /* Current data byte (MSB) */
                  	byte *DataByte2,  /* Current data byte  */
                  	byte *DataByte3,  /* Current data byte  */
                  	byte *DataByte4);  /* Current data byte (LSB) */ 

byte  		ReadHexByte(void);		  	   // Read 2 hex bytes from the USART and convert them to a 8-Bit unsigned byte
word 		ReadHexWord(void);		   	   // Read 4 hex bytes from the USART and convert them to a 16-Bit unsigned word
int 		ReadHexInteger(void);                      // Read 4 hex bytes from the USART and convert them to a 16-Bit signed integer
long int 	ReadHexLongInteger(void);              	   // Read 8 hex bytes from the USART and convert them to a 32-Bit long signed integer
long unsigned int ReadHexUnsignedLongInteger(void);        // Read 8 hex bytes from the USART and convert them to a 32-Bit long unsigned integer
float 		ReadHexFloat(void);			   // Read 8 hex bytes from the USART and convert them to a 32-Bit float

void  string(char *s);				// Write a string terminated with a null from RAM to the serial port
void  string2(char *s);				// Write a string terminated with a null from RAM to the serial port
void    FlushBuffer(void);          // Flush the ring buffer
void    FlushBuffer2(void);          // Flush the ring buffer

 
#endif 

// Stamp BS2 compatability functions


int 		hex2(byte Value);			   // Converts a byte to an ascii string of 2 hex characters and writes                                                          
                                                           // them to the USART.

int 		hex4(word Value);			   // Converts an integer to an ascii string of 4 hex characters and writes                                                          
                                                           // them to the USART.

int 		hex8(unsigned long int  Value);		   // Converts a 32-Bit unsigned integer to an ascii string of 8 hex characters and writes                                                          
                                                           // them to the USART.

int             dec(int Value);    			   // Converts an integer to an ascii string of numeric characters and writes                                                          
                                                           // them to the USART.

int             tofloat(float Value);     		   // Converts a float to an ascii string of numeric characters and writes                                                          
                                                           // them to the USART.               
// Type conversion functions

void 		btoh(byte ByteValue, HexByte_T HexValue);  // Convert from a byte to a hex byte range (00..FF) 
void 		wtoh(word WordValue, HexWord_T HexValue);  // Convert from 16-Bit unsigned word to a hex word range (0000..FFFF)
void            itoh(int IntValue, HexWord_T HexValue);    // Convert from 16-Bit integer to a hex word range (0000..FFFF) 
void 		ltoh(long int LongIntValue, 
                     	HexLongWord_T HexValue);           // Convert from 32-Bit long integer to a hex word range (00000000..FFFFFFFF)
void 		ultoh(unsigned long int UnsignedLongIntValue, 
			HexLongWord_T HexValue);	   // Convert from 32-Bit unsigned long integer to a hex word range (00000000..FFFFFFFF)
byte 		htob(byte *s);                             // Converts from 2 character hex string to an unsigned byte
word 		htow(byte *s);                             // Convert from 4 character hex string to a 16-Bit unsigned word
int 		htoi(byte *s);                             // Convert from 4 character hex string to a 16-Bit signed integer

long int 	htol(byte *s );      			   // Convert from 8 character hex string to a 32-Bit signed long  integer            
unsigned long int htoul(byte *s );                         // Convert from 8 character hex string to a 32-Bit unsigned long integer
float 		htof(byte *s );			           // Convert from 8 character hex string to a 32-Bit float                              

 
 
int Member(char Ch, char * Set);

float GetFloat(void);
double GetDouble(void);

 
void ConfigureInterrupts(void);
char GetKey(void);

#ifdef NETWORK_SUPPORT

// Network functions

void ComputeChecksum(Packet_U *Message, word *Checksum);
void ComputeShortChecksum(ShortPacketData_T *ShortMessage, word *Checksum);
void SendMessage(void);
void SendShortMessage(void);
void ReceiveMessage(void);
void ReceiveMessage(void);
void ProcessReadMemory(void);
void ProcessWriteMemory(void);
void UnitTest(void);
#endif

 

