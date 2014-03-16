/*********************************************************************
 *
 *     Microchip MCP2515 PICtail Eval Board Firmware Version 1.0
 *
 *********************************************************************
 * FileName:        00023R1.C
 * Dependencies:    See INCLUDES section below
 * Processor:       PIC16
 * Compiler:        Hi-Tech C V8.05PL2
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
 * Pat Richards       06/28/2005    Original.
 ********************************************************************/

#include  <pic.h>
#include  <SPI.H>
#include  <REGS2515.h>


//-----------------------------------------------------------
//	Function prototypes
//-----------------------------------------------------------
void InitPICmicro(void);
void Init2515(void);
void SPIReset(void);
void Delay_ms(unsigned char num_ms);
void SPIByteWrite(unsigned char addr,unsigned char data);
unsigned char SPIByteRead(unsigned char addr);
void RTS(unsigned char buffer);
void LoadTXBuffers(unsigned char buffers, unsigned char *ID);

//-----------------------------------------------------------
//#defines
//-----------------------------------------------------------
//#define I2CMODE 0
#define SPIMODE 1

#define Mode00  0
#define Mode11  1

#define CAN_500kbps 1       //BRP setting in CNF1
#define CAN_250kbps 3       //Based on 16 MHz Fosc
#define CAN_125kbps 7       //


//#define CSL RA5 = 0
//#define CSH RA5 = 1
#define CS        RA5
#define PBSwitch  RC1
#define	LED			  RC2

#define	ON	1
#define	OFF	0

#define HIGH  1
#define LOW   0

//-------------------------------------------------------------------
//  Globals
//-------------------------------------------------------------------
 
unsigned char dummy;
unsigned char gAddrPins = 0;
unsigned char gSampleFlag = 0;
unsigned char gRXFlag = 0;
unsigned char verify = 1;
unsigned char TimerCounter = 0xEF;

//-----------------------------------------------------------
//	ISR
//-----------------------------------------------------------
//#pragma interrupt_level 0
void interrupt ISR(void)
{
  if(INTF)
  {
    gRXFlag = 1;
    INTF = 0;
  }

  //Timer interrupts every 32 ms and set a flag every 224 ms (32 ms x 07h = 224 ms)
  //Initial value = FFh - 07h = F8h
  //Will use to sample button and to TX message
  if(T0IF)	
  {
  	TimerCounter++;
  	if (!TimerCounter) //if rolled over, set flag. Main will handle the rest.
  	{
  	  TimerCounter = 0xF8;
  	  gSampleFlag = 1;
  	}
  	T0IF = 0;
  }
}

//-----------------------------------------------------------
//	main()
//-----------------------------------------------------------
void main(void)
{
  unsigned int n, CANData;
  InitPICmicro();
  SPIReset();
  Delay_ms(1);  //Wait for MCP2515 to come out of reset
  Init2515();
  
  GIE = 1;

  //Service I/O
  do
  {
    if(gSampleFlag)   //Check gSampleFlag bit
    {
      //Set/Clear TX0D2 bit4 as needed
      if(PBSwitch)        //If input is high
        CANData = 0x10; //Will load into TXB0D2 to send a "Write Register" command to MCP25020
      else              //If input is low
        CANData = 0x00; //Will load into TXB0D2 to send a "Write Register" command to MCP25020
      
      SPIByteWrite(TXB0D2,CANData);  //Write the value to data byte #3
    
     //SendMSG()
        RTS(CAN_RTS_TXB0);  //RTS msg - Input msg to 25020
        RTS(CAN_RTS_TXB1);  //RTS msg - IRM message

      gSampleFlag = 0;                  //Clear gSampleFlag bit
    }
    if(gRXFlag)     //Check gRXFlag bit
    {
      LED = (SPIByteRead(RXB0D1));  //Set/Clear LED as needed (RXB0D1 bit 0 value)
      SPIByteWrite(CANINTF, 0x00);  //Clear RX0IF; Actually clear all of CANINTF
      gRXFlag = 0;  //Clear gRXFlag bit
    }
  }while(1);
}

//-----------------------------------------------------------
//	InitPIC()
//-----------------------------------------------------------
void InitPICmicro()
{
  //Configure I/O
    //TRISA = b'00 1111' = 0x0F
    //RA5 = CS = OUT
    //RA4 = mSO = OUT
    //RA3 = default
    //RA2 = INT = IN
    //RA1 = default
    //RA0 = default
  TRISA = 0x0F;
    
    //TRISC = b'10 1011' = 0x2B
    //RC5 = mSI = IN
    //RC4 = mSCK = OUT
    //RC3 = default
    //RC2 = LED1-1 = OUT
    //RC1 = SW1-1 = IN
    //RC0 = default
  TRISC = 0x2B;

  ANSEL = 0x00; //Pins are digital
	CS = HIGH;;
	
	//Timer 0
	PSA = 0;	//Assign prescaler to Timer 0
	PS2 = 1;	//Setup prescaler
	PS1 = 1;	//Will time out every 32 ms based on
	PS0 = 0;	//8 MHz Fosc
	T0CS = 0;	//Increment on instuction cycle

  //Timer 1 used for general delays {Delay_ms() function}
  T1CON = 0x00;		//16-bit R/W, 1:1 Prescaler, 1:1 Postscaler, Stop timer

  //Interrupts
	INTEDG = 0;		// INT on falling edge.
	INTE = 1;     // Enable INT pin
	T0IE = 1;     // Timer 0
	PEIE	=	1;    // Enable unmasked peripheral INTs
}


//-----------------------------------------------------------
//	Init2515()
//-----------------------------------------------------------
void Init2515(void)
{
  SPIReset();
  Delay_ms(1);
  
  //Clear masks to RX all messages
  SPIByteWrite(RXM0SIDH,0x00);
  SPIByteWrite(RXM0SIDL,0x00);

  //Clear filter... really only concerned to clear EXIDE bit
  SPIByteWrite(RXF0SIDL,0x00);

  //Set CNF1
  SPIByteWrite(CNF1,CAN_125kbps);

  //Set CNF2
  SPIByteWrite(CNF2,0x80 | PHSEG1_3TQ | PRSEG_1TQ);

  //Set CNF3
  SPIByteWrite(CNF3, PHSEG2_3TQ);

  //Set TXB0 DLC and Data for a "Write Register" Input Message to the MCP25020
  SPIByteWrite(TXB0SIDH,0xA0);    //Set TXB0 SIDH
  SPIByteWrite(TXB0SIDL,0x00);    //Set TXB0 SIDL
  SPIByteWrite(TXB0DLC,DLC_3);    //Set DLC = 3 bytes
  SPIByteWrite(TXB0D0,0x1E);      //D0 = Addr = 0x1E
  SPIByteWrite(TXB0D1,0x10);      //D1 = Mask = 0x10

  //Set TXB1 DLC and Data for a "READ I/O IRM"
  SPIByteWrite(TXB1SIDH,0x50);    //Set TXB0 SIDH
  SPIByteWrite(TXB1SIDL,0x00);    //Set TXB0 SIDL
  SPIByteWrite(TXB1DLC,0x40 | DLC_8);    //Set DLC = 3 bytes and RTR bit
  
  //Interrupt on RXB0 - CANINTE
  SPIByteWrite(CANINTE,0x01);    //

  //Set NORMAL mode
  SPIByteWrite(CANCTRL,REQOP_NORMAL | CLKOUT_ENABLED);
  
  //Verify device entered Normal mode
  dummy = SPIByteRead(CANSTAT);
  if (OPMODE_NORMAL != (dummy && 0xE0))
	  SPIByteWrite(CANCTRL,REQOP_NORMAL | CLKOUT_ENABLED);
}

//-----------------------------------------------------------------------------
//  SPIReset()
//-----------------------------------------------------------------------------
void SPIReset(void)
{
	CS = LOW;;  //
	WriteSPI(CAN_RESET);
  CS = HIGH;;
}

//-----------------------------------------------------------------------------
//  Delay_ms()
//  Tcy = 1 us
//  Tcy X 1000 = 1 ms
//  FFFFh - 3E8h (1000d) = FC17h
//  For(){} loop is in 1 ms increments
//-----------------------------------------------------------------------------
void Delay_ms(unsigned char num_ms)
{
  unsigned char n;
  
  TMR1H = 0xFC;
  TMR1L = 0x17;
  
  TMR1ON = 1; //Start timer
  
  for(n = 0; n < num_ms; n++)
  {
    while(!TMR1IF);  //Wait for timer flag
    TMR1IF = 0;      //Clear flag
    TMR1H = 0xFC;
    TMR1L = 0x17;
  }

  TMR1ON = 0; //Stop timer
}

//-------------------------------------------------------------------------
//  SPIByteWrite()
//-------------------------------------------------------------------------
void SPIByteWrite(unsigned char addr, unsigned char value )
{
  CS = LOW;;  //
  WriteSPI(CAN_WRITE);
  WriteSPI(addr);
  WriteSPI(value);
  CS = HIGH;;
}


//-----------------------------------------------------------------------------
//  SPIByteRead()
//-----------------------------------------------------------------------------
unsigned char SPIByteRead(unsigned char addr)
{
  unsigned char tempdata;
	CS = LOW;;  //
  WriteSPI(CAN_READ);
  WriteSPI(addr);
  tempdata = ReadSPI();
  CS = HIGH;;
  return tempdata;
}

//-----------------------------------------------------------------------------
//  RTS(buffer)
//	buffer = CAN_RTS_TXBn; where 'n' = 0, 1, 2
//	buffer = CAN_RTS; followed by | 0 - 7 (e.g., CAN_RTS | 7)
//-----------------------------------------------------------------------------
void RTS(unsigned char buffer)
{
  unsigned char tempdata;
	tempdata = SPIByteRead(CAN_RD_STATUS);
	
	if(buffer & 0x01)	//Buffer 0	
		if(tempdata & 0x04)	//Check TXREQ first
		{
			Delay_ms(1);
			SPIByteWrite(TXB0CTRL, 0);			//Clear TXREQ (and everything else... not clean)
			while(SPIByteRead(CAN_RD_STATUS) & 0x04); //Wait for TXREQ to clear
	  }
	if(buffer & 0x02)	//Buffer 1
		if(tempdata & 0x10)	//Check TXREQ first
		{
  		Delay_ms(1);
  		SPIByteWrite(TXB1CTRL, 0);			//Clear TXREQ (and everything else... not clean)
			while(SPIByteRead(CAN_RD_STATUS) & 0x10); //Wait for TXREQ to clear
		}
	if(buffer & 0x04)	//Buffer 2
		if(tempdata & 0x40)	//Check TXREQ first
		{
  		Delay_ms(1);
  		SPIByteWrite(TXB2CTRL, 0);			//Clear TXREQ (and everything else... not clean)
			while(SPIByteRead(CAN_RD_STATUS) & 0x40); //Wait for TXREQ to clear
		}
	
	CS = LOW;;
	WriteSPI(buffer);
	CS = HIGH;;
}

