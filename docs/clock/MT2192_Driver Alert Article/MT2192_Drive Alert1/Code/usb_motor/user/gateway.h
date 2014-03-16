/*********************************************************************
 *
 *                Microchip USB C18 Firmware Version 1.0
 *
 *********************************************************************
 * FileName:        gateway.h
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC18
 * Compiler:        C18 2.30.01+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PICmicro® Microcontroller is intended and
 * supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PICmicro Microcontroller products. The
 * software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Rawin Rojvanit       11/19/04    Original.
 ********************************************************************/

#ifndef USER_H
#define USER_H
//#include "user\UARTIntC.h"

#define USART_BUFFER_SIZE  64

// Declare USART Master Buffer variables here

extern volatile byte USART_Rx_Buffer[];                  	// USART Receive Buffer
extern volatile byte USART_Tx_Buffer[];                  	// USART Transmit Buffer
extern volatile byte USART_Start_Buffer;                         		// Start pointer for ring buffer
extern volatile byte USART_End_Buffer;								// USART End Buffer Index

extern volatile byte USART_Rx_Flag;                        		// Indicates that data was received into the ring buffer
extern volatile int USART_Timeout;                            		// USART I/O Timeout counter
extern volatile byte USART_Timeout_Flag;                           		// USART Timeout flag
extern volatile int USART_Number_Characters;						 	// USART Number of characters in buffer
extern volatile int USART_Communications_Error;                    		// USART I/O error flag
                                                 		// -1 = USART Input Bufer overrun error
                                                 		// -2 = USART Communications timeout error
                                                 		// -3 = USART buffer empty error

extern volatile int USART_Interrupt_Count;                   	// Number of MSSP interrupts
extern volatile int USART_Data_In;								// Data byte read from the USART Master
extern volatile int USART_Data_Out;							// Data byte sent to the USART Master
extern volatile int USART_Status;									// USART Status returned from routines
extern volatile int MessageIndex;							// Message ID number

/** P U B L I C  P R O T O T Y P E S *****************************************/
void main(void);
void UserInit(void);
void ProcessIO(void);
void InitializeUSART(void);
void Transfer_USB_To_USART(void);
void Transfer_USART_To_USB(void);
void Transfer_USB_To_SPI(void);
void Transfer_SPI_To_USB(void);
void Transfer_USB_To_I2C(void);
void Transfer_I2C_To_USB(void);

void Printf(char Buffer[USART_BUFFER_SIZE]);
void Printf_USB(char Buffer[USART_BUFFER_SIZE]);

#endif //USER_H
