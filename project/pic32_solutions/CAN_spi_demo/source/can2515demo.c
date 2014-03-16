/*********************************************************************
 *
 *                  CAN Lib MCP2515 Example file
 *
 *********************************************************************
 * FileName:        CAN2515demo.c
 * Dependencies:	Spi.h
 *
 * Processor:       PIC32
 *
 * Complier:        MPLAB C32 1.0+
 *                  MPLAB IDE 8.0+
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the “Company”) for its PIC32/PIC24 Microcontroller is intended
 * and supplied to you, the Company’s customer, for use solely and
 * exclusively on Microchip PIC32/PIC24 Microcontroller products.
 * The software is owned by the Company and/or its supplier, and is
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
 *
 * $Id: $
 *
 ********************************************************************/
 
#include <plib.h>
#include "mstimer.h"
#include "GenericTypedefs.h"
#include <CAN\canlib2515.h>

// configuration settings
#pragma config FNOSC = PRIPLL, POSCMOD = XT, FPLLMUL = MUL_18, FPLLIDIV = DIV_2, FPBDIV = DIV_2, FPLLODIV = DIV_1
#pragma config FWDTEN = OFF

UINT32 pbClk;

/*********************************************************************
 * Function:        void InitializeCAN( int Channel )
 *
 * PreCondition:    None
 *
 * Input:			Channel: SPI channel, 1 based
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:		Call library functions to initialize CAN controller
 *
 * Note:            None.
 ********************************************************************/
void InitializeCAN( int Channel )
{
	CAN2515InitSPI( Channel, pbClk, 9000000 );  //set spi speed at 9Mhz
	
	CANReset(Channel);
	
	//setup the filter to recieve all messages
	UINT16 Flt[] = {0,0,0,0,0,0}; 
	CAN2515SetRXB0Filters(Channel, 0, Flt);
	CAN2515SetRXB1Filters(Channel, 0, &Flt[2]);
	
	CANEnable(Channel, CAN_125kbps);
}

/*********************************************************************
 * Function:        int main(void)
 *
 * PreCondition:    None
 *
 * Input:			None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:		Examples for the usage of the CAN library
 *
 * Note:            None.
 ********************************************************************/
int	main(void)
{
	int Channel=1;	// the SPI channel to use

	// Turn on the interrupts
	INTEnableSystemMultiVectoredInt();

	// Configure the device for maximum performance.
	// This macro sets flash wait states, PBCLK divider and DRM wait states based on the specified
	// clock frequency. It also turns on the cache mode if avaialble.
	// Based on the current frequency, the PBCLK divider will be set at 1:2. This knoweldge
	// is required to correctly set UART baud rate, timer reload value and other time sensitive
	// setting.

	pbClk = SYSTEMConfigPerformance(72000000L);
	
	MSTimerInit( pbClk );

	//Set digital output dir for RB5 = CS for CAN
	mCONFIG_CAN_CS();

	//Initialize CAN Controller
	InitializeCAN( Channel );

	//Setup LEDs
	mPORTAClearBits( BIT_6|BIT_7 );
	mPORTASetPinsDigitalOut( BIT_6|BIT_7 );

	UINT16 xmit = 0, Ident;
	int StartRxLEDCntr = 0, StartTxLEDCntr = 0;
	UINT8 msg[8], mSize;
	
	while(1)
	{		
		//Clear memory contents
		msg[0] = 0;
		msg[1] = 1;
		msg[2] = 2;
		msg[3] = 3;
		
		//Read any message available
		if( CANGetMsg( Channel, &Ident, msg, &mSize ) )
		{
			StartRxLEDCntr = ReadCoreTimer()+0x80000;
		}

		//Send a message
		if( ReadCoreTimer() & 0x200000 )
		{
			if( !xmit )
			{	
				msg[0] = 12;
				msg[1] = 1;
				msg[2] = 2;
				if( CANSendMsg( Channel, 0x780|1, msg, 3 ) )
				{
					xmit = 1;
					StartTxLEDCntr = ReadCoreTimer()+0x80000;
				}
			}
		}
		else
			xmit = 0;
			
		if( StartRxLEDCntr )
		{
	 		mPORTASetBits( BIT_7 );
	 		if( (StartRxLEDCntr-ReadCoreTimer()) < 10000 )
	 		{
		 		StartRxLEDCntr = 0;
	 			mPORTAClearBits( BIT_7 );
	 		}
	 	}
		 		
		if( StartTxLEDCntr )
		{
	 		mPORTASetBits( BIT_6 );
	 		if( (StartTxLEDCntr-ReadCoreTimer()) < 10000 )
	 		{
		 		StartTxLEDCntr = 0;
	 			mPORTAClearBits( BIT_6 );
	 		}
	 	}
	}

	return 1;
}

