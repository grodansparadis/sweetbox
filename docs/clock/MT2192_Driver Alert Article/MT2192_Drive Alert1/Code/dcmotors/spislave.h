//********************************************************************************
//*                                                                              *
//* Description:                                                                 *
//*                                                                              *
//*        Name:  spislave.h                                                     *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This application implements an 8-Bit or 16-Bit SPI Slave Co-Processor controller that    *
//* emulates Microchip's 24LCXX SPI Memory chip interface and acts as a smart    *
//* peripheral, by using the SPI hardware module contained in the PIC18C452      *  
//* micro-controller.  In addition it can act as an SPI network server to an     *
//* SPI Client or function as an SPI based RPN Floating Point co-processor.  In  *
//* the simplest mode of operation, it simulates writing to the EEPROM and       *
//* reading it back under control from the Master SPI controller.  Included is a *
//* diagnostic function that will write a ramp to the EEPROM using SPI and read  *
//* it back to the Host PC via the USART when used with the spimast.c            *
//* application. A more sophisticated SPI Networking mode is also available to   *
//* applications that require message passing with CRC or checksum.  The common  *
//* message format between the SPI Master and SPI Slave is defined in the        *
//* file spimsg.h.  SPI transmit and receive buffers are available to process    *
//* messages to/from the Master controller.                                      *                         
//*                                                                              *
//*      A communication interface is provided that allows the host or SPI or    *
//* I2C Master to request individual sensor readings, average sensor readings,   *
//* calibrating a sensor or perform a sensor processing algorithm using command  *
//* message packets.  The commands available are described below.  This header   *
//* file provides the common Master and Slave message formats and descriptors.   *                                                                
//*                                                                              *
//*     This SPI slave controller can be used with a PC host or any SPI master,  *
//* in order to  give any PC the capability to serve as an SPI controller.       * 
//* It can also be used for robotic applications where multiple controllers and  *
//* SPI devices are used, with a minimum of data and control lines needed to     *
//* connect them.  Each SPI device has its own unique slave select enable line   *
//* (SS_BAR) and only listens to the SPI Master commands when SS_BAR is cleared. *  
//*                                                                              *
//*    This application emulates a Microchip 24LCXX EEPROM that has 8 registers  *
//* accessed as sub-addresses 1:8 modulo 7.  Sub-address 0 is used as a data     *
//* channel.  All the read and write data modes available for this device        *
//* including: Byte Write, Page Write, Current Address Read, Random Read and     *
//* Sequential Read are emulated by this application.  As far as the Master SPI  * 
//* controller knows, it is talking to a real EEPROM instead of another PIC.     *
//*                                                                              *
//*                                                                              * 
//*     The SPI message format used by this application is for the master to     *
//* send commands to the Digital Pot SPI slave device.  This translates to the   *
//* following sequence of operations for the Master:                             *                    
//*                                                                              *
//*  1) The SPI Co-Processors Command Bytes: X|X|C1|C0|X|X|P1|P0                 *
//*                                                                              *
//*                                                                              *
//*  2) Data is latched in on the rising edge of the SCK.                        *
//*                                                                              *
//*  3) Data is clocked out the SO pin after the falling edge of SCK.            *
//*                                                                              *
//*  4) The Clock Iddle is high.                                                 *
//*                                                                              *
//*  5) The SPI Clock speed is FOSC/16                                           *
//*                                                                              *
//*  6) The Reset Pin, RS_BAR will automatically set all potentiometer data      *
//*     latches to mid scale (Code 80h).                                         *
//*                                                                              *
//*  7) The Shutdown Pin, SHDN_BAR when held low will (provided that the pin is  *
//*     held low for at least 150 ns and CS_BAR is high, will cause the          *
//*     application circuit to into a power saver mode.                          *
//*                                                                              *
//*  8) The device when powered up for the first time, will automatically set    *
//*     reset all potentiometer data latches to mid scale (Code 80h).  A power   *
//*     on will insure that the device powers up in this known state             *
//*                                                                              *
//*  9) This device can operate in SPI Mode 0,0, in which the clock idles in the *
//*     low state.                                                               *
//*                                                                              *
//*  10) The following SPI data transfers are available:                         *
//*                                                                              *
//*    		Master sends data byte  -  Slave sends dummy data byte               *                                                                        
//*    		Master sends data byte  -  Slave sends data byte                     *                                                                  
//*    		Master sends dummy data byte  -  Slave sends data byte               *                                                                         
//*                                                                              *
//*                                                                              *
//* Note:                                                                        *
//*		The SPI networking is handled using a simple data packet messaging       *
//* protocol.  Each message includes an SPI source address field (Master), an SPI*
//* destination address field, the actual data and a CRC or checksum field.  Data*
//* packets that are determined to be corrupted will be retransmitted up to a    *
//* specified number of times.                                                   *
//*                                                                              * 
//*                                                                              *
//*      Future upgrades to this application include: the ability to communicate * 
//* with other SPI slave devices such as Microchip MPC3002 A/D converter, the    *
//* Maxim MAX7219 LED Driver, the Microchip SPI Serial EEPROMS such as the       *
//* 93XXXX Series and the ISD 33270 Voice Recorder IC.  It may also be used to   *
//* communicate as a master or a slave for applications requiring that dual      *
//* capability.  Other upgrades include using SPI to implement the C printf      *
//* function to be used for a "cheap" embedded debugger similar to JTAG used by  *
//* high end micro-processors. Explore using communication protocols and error   *
//* detection/correction algorithms such as parity, CRC and various other codes  *
//* for robust operation and fault tolerance. TMR and N-Module redundancy could  *
//* also be investigated for fault tolerant systems that work in hazardous       *
//* environments. The same SPI networking functions may also be used with the    *
//* asynchronous serial RS-232 interface and the I2C interface.                  *                         
//*                                                                              *
//*                                                                              *
//********************************************************************************

#ifndef SPISLAVE_LOADED  /* prevent multiple loading */
#define SPISLAVE_LOADED
  

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
 
 
#include <ctype.h>

//#include <stdlib.h>
//#include <conio.h>
#include <math.h>
#include <string.h>
#include "my_types.h"
#include "dcmotor.h"

 
// Port B definitions
//#define BUSY_FLAG PORTBbits.RB3					 			//  (0=Not Busy, 1=Busy)
//#define BUSY_FLAG_DIR TRISBbits.TRISB3					 	//  Direction (0=output, 1=input)
//#define BUSY_FLAG PORTDbits.RD1					 			//  (0=Not Busy, 1=Busy)
//#define BUSY_FLAG_DIR TRISDbits.TRISD1					 	//  Direction (0=output, 1=input)
// Port B definitions
#define BUSY_FLAG PORTBbits.RB8					 			//  (0=Not Busy, 1=Busy)
#define BUSY_FLAG_DIR TRISBbits.TRISB8					 	//  Direction (0=output, 1=input)

 
// Declare SPI Master Buffer external variables here
#ifdef SPI_8
extern volatile byte SPI_Rx_Buffer[SPI_BUFFER_SIZE];                // SPI Receive Buffer (bytes)
extern volatile byte SPI_Tx_Buffer[SPI_BUFFER_SIZE];                // SPI Transmit Buffer (bytes)
#endif

#ifdef SPI_16
extern volatile word SPI_Rx_Buffer[SPI_BUFFER_SIZE];                // SPI Receive Buffer (words)
extern volatile word SPI_Tx_Buffer[SPI_BUFFER_SIZE];                // SPI Transmit Buffer (words)
#endif

extern volatile int  SPI_Start_Buffer;                         	// Start pointer for ring buffer
extern volatile int SPI_End_Buffer;								// SPI End Buffer Index

extern volatile byte SPI_Rx_Flag;                        	// Indicates that data was received into the ring buffer
extern volatile word SPI_Timeout;                            	// SPI I/O Timeout counter
extern volatile byte SPI_Timeout_Flag;                           	// SPI Timeout flag
extern volatile int SPI_Number_Characters;						 // SPI Number of characters in buffer
extern volatile int SPI_Communications_Error;                    	// SPI I/O error flag
                                                 	// -1 = SPI Input Bufer overrun error
                                                 	// -2 = SPI Communications timeout error
                                                 	// -3 = SPI buffer empty error

extern volatile int SPI_Interrupt_Count;                   		// Number of MSSP interrupts
extern volatile int SPI_Data_In;								// Data byte read from the SPI Master
extern volatile int SPI_Data_Out;								// Data byte sent to the SPI Master
extern volatile byte SPI_Status;										// SPI Status returned from routines
 
// SPI Services

 
//******************************************************************************
//* InitializeSPISlave - Initialize variables and hardware used by the SPI 
//* slave device.
//******************************************************************************

void InitializeSPISlave(void);
 

// SPI input with interrupts and SPI buffer

//******************************************************************************
//* SPI_Get_Byte - Get next character from the SPI Rx Buffer.  The function
//* returns a -1 if the routine times out. 
//******************************************************************************
 
byte	SPI_Get_Byte(void);

 
#endif
