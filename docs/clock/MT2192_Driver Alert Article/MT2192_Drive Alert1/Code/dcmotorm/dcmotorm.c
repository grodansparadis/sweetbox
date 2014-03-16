//********************************************************************************
//*                                                                              *
//*           Name:  dcmotorm.c                                                  *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This application implements all the DC Motor Controller Master functions *
//* necessary send messages to the DC Motor Controller slave application or to   *
//* act as a Client to any 8-Bit or 16-Bit SPI Slave devices.  These include devices include     *
//* my SPI Math Co-processor, the Microchip's 24XCXX SPI Memory chips, SPI based * 
//* digital potentiometers and A/D devices, and SPI Client/Server networking,    *
//* using the SPI hardware module  contained in the DSPIC30F6014 micro-controller.  *
//*                                                                              *
//* 	The application writes data to the Math Co-Processor or EEPROM and reads *
//* back the data.  Also included is a diagnostic function that will write a     *
//* ramp (0..n) to the Math Co-Processor or EEPROM using SPI and read it back to *
//* the Host PC via the USART.                                                   *
//*                                                                              *
//*		The SPI networking is handled using a simple data packet messaging       *
//* protocol.  Each message includes an SPI source address field (Master), an SPI*
//* destination address field, the actual data and a CRC or checksum field.  Data*
//* packets that are determined to be corrupted will be retransmitted up to a    *
//* specified number of times.  The SPI Slave responds by reading these messages *
//* into local buffers and proceeds to process them.  Results are sent back to   *
//* the Master SPI processor via the SPI Bus.    These SPI networking functions  *
//* are also available to other applications that require the SPI interface.     *                                
//*                                                                              *
//* 	Message definitions are defined in the file spimsg.h which are common to * 
//* the spimast.c and spislave.c applications.  SPI transmit and receive buffers *
//* are available to process messages to/from the Master controller.             *                             
//*                                                                              *
//*                                                                              *
//*		The following are simple code examples illustrating the SSP module       *
//* communicating with a Microchip 24C080 SPI EE Memory Device. In all the       *
//* examples provided no error checking utilizing the value returned from a      *
//* function is implemented.                                                     *
//*                                                                              *
//*     This controller can be used with a PC host in order to  give any PC the  *
//* capability to serve as an SPI Master.  It can also be used for robotic       *
//* applications where multiple controllers and SPI devices are used, with a     *
//* minimum of data and control lines needed to connect them.  Each SPI device   *
//* has its own unique address and only listens to commands associated with it.  *
//*                                                                              *
//*     The SPI message format used by this application is for the master to     *
//* send commands to the Digital Pot SPI slave device.  This translates to the   *
//* following sequence of operations for the Master:                             *                    
//*                                                                              *
//*  2) Data is latched in on the rising edge of the SCK.                        *
//*                                                                              *
//*  3) Data is clocked out the SO pin after the falling edge of SCK.            *
//*                                                                              *
//*  4) The Clock Iddle is low, Mode 0,0.                                        *
//*                                                                              *
//*  5) The SPI Clock speed is FOSC/16                                           *
//*                                                                              *
//*  6) The Reset Pin, RS_BAR needs to be brough low to select the MCP3002 A/D.  *
//*                                                                              *
//*                                                                              *
//*  7) This device can also operate in SPI Mode 1,1, in which the clock idles   *
//*  in the high state.                                                          *
//*                                                                              *
//*  8) The following SPI data transfers are available:                          *
//*                                                                              *
//*    		Master sends data byte  -  Slave sends dummy data byte               *                                                                        
//*    		Master sends data byte  -  Slave sends data byte                     *                                                                  
//*    		Master sends dummy data byte  -  Slave sends data byte               *                                                                         
//*                                                                              * 
//*  9) The Receiver Buffer has a one byte delay which requires a data offset    *    
//*     into the SPI Receiver Buffer.                                            *                             
//*                                                                              *                                                                            
//*     The Slave responds with the following sequence of operations:            * 
//*                                                                              *
//*                                                                              *
//*                                                                              *
//*      Future upgrades to this application include: the ability to communicate * 
//* with other SPI slave devices such as Dallas Semiconductor temperature        *
//* sensors, Phillips serial SPI devices and other micro-controllers (both       *
//* master and slave).  It may also be used to communicate as a master or a      *
//* slave for applications requiring that dual capability.  Other upgrades       * 
//* include using SPI to implement the C printf function to be used for a "cheap"*
//* embedded debugger similar to JTAG used by high end micro-processors.         *
//* Explore using communication protocols and error detection/correction         *  
//* algorithms such as parity, CRC and various other codes for robust operation  * 
//* and fault tolerance. TMR and N-Module redundancy could also be investigated  *
//* for fault tolerant systems that work in hazardous environments.  The same    *
//* SPI networking functions may also be used with the asynchronous serial RS-232*
//* interface and the I2C interface.                                             *             
//*                                                                              *
//*                                                                              *
//********************************************************************************
  
  
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
 
//#include <stdio.h>
#include <spi.h>
#include <uart.h>
#include <math.h>
#include <adc12.h>
#include "stdio.h"                                      // USART library functions                                     
#include "timeutil.h"
#include "uartutil.h"
#include "dcmotor.h"                                   // DC Motor Controller functions
#include "my_types.h"

byte FlushSPIBuffers(void);									// Clear the SPI RX and TX buffers

//#define SPI_TEST_1								// Works !!!!
//#define ADC_SUPPORT                               // ADC Support for control POTS

/*     
// Port B definitions for dsPIC30F2010 Motor Controller
#define BUSY_FLAG PORTBbits.RB3					 			//  (0=Not Busy, 1=Busy)
#define BUSY_FLAG_DIR TRISBbits.TRISB3					 	//  Direction (0=output, 1=input)
*/
// Port B definitions for dsPIC30F4011 Motor Controller
#define BUSY_FLAG PORTBbits.RB8					 			//  (0=Not Busy, 1=Busy)
#define BUSY_FLAG_DIR TRISBbits.TRISB8					 	//  Direction (0=output, 1=input)
  

// Left Motor Direction (forward/backward) (Motor 0)
#define MOTOR_0_DIRECTION PORTBbits.RB4					 			//  (1=Forward, 0=Backward)
#define MOTOR_0_DIRECTION_DIR TRISBbits.TRISB4					 	//  Direction (0=output, 1=input)

// Right Motor Direction (forward/backward) (Motor 1)

#define MOTOR_1_DIRECTION PORTBbits.RB5					 			//  (1=Forward, 0=Backward)
#define MOTOR_1_DIRECTION_DIR TRISBbits.TRISB5					 	//  Direction (0=output, 1=input)

// Left Motor Brake (On/Off) (Motor 0)
#define MOTOR_0_BRAKE_FLAG PORTBbits.RB4					 			//  (1=Forward, 0=Backward)
#define MOTOR_0_DIR_FLAG_DIR TRISBbits.TRISB4					 	//  Direction (0=output, 1=input)

// Right Motor Brake (On/Off) (Motor 1)

#define MOTOR_1_DIR_FLAG PORTBbits.RB5					 			//  (1=Forward, 0=Backward)
#define MOTOR_1_DIR_FLAG_DIR TRISBbits.TRISB5					 	//  Direction (0=output, 1=input)
 
// Define General Motor Properties here...

// Raw motor limits (Viewport)
#define MIN_POT_VIEWPORT  		0  			// Minimum POT value range 0..4095 12-Bits  
#define NOM_POT_VIEWPORT  		2047  		// Nominal POT value range 0..4095 12-Bits  
#define MAX_POT_VIEWPORT 		4095  		// Maximum POT value range 0..4095 12-Bits  

#define MIN_MOTOR_VIEWPORT  	0   		// Minimum PWM Duty Motor Command range 0..1023 10-Bits 
#define NOM_MOTOR_VIEWPORT  	511  		// Nominal PWM Duty Motor Command range 0..1023 10-Bits
#define MAX_MOTOR_VIEWPORT 		1023  		// Maximum PWM Duty Motor Command range 0..1023 10-Bits

  
// Scaled motor limits (Window)
#define MIN_PERIOD_WINDOW  		0.0  		// Minimum scaled PWM Period in milliseconds
#define NOM_PERIOD_WINDOW  		0.0  		// Nominal scaled PWM Period in milliseconds
#define MAX_PERIOD_WINDOW 		10000.0  	// Maximum scaled PWM Period in milliseconds

#define MIN_FREQUENCY_WINDOW  	0.0  		// Minimum scaled PWM Frequency in Hz
#define NOM_FREQUENCY_WINDOW  	0.0  		// Nominal scaled PWM Frequency in Hz
#define MAX_FREQUENCY_WINDOW 	10000.0  	// Maximum scaled PWM Frequency in Hz

#define MIN_DUTY_WINDOW 		0.0 		// Minimum scaled PWM Duty Motor Command
#define NOM_DUTY_WINDOW 		511.0 		// Nominal scaled PWM Duty Motor Command 
#define MAX_DUTY_WINDOW 		1023.0		// Maximum scaled PWM Duty Motor Command 

#define MIN_POSITION_WINDOW		0.0			// Minimum scaled motor position in encoder counts (0.0 CPR)
#define NOM_POSITION_WINDOW		50.0		// Minimum scaled motor position in encoder counts (50.0 CPR)
#define MAX_POSITION_WINDOW 	100.0		// Maximum scaled Motor position in encoder counts (100.0 CPR) 

#define MIX_SPEED_WINDOW 		0.0			// Minimum scaled Motor Speed (0.0 RPM)
#define NOM_SPEED_WINDOW 		5000.0		// Minimum scaled Motor Speed (5000.0 RPM)
#define MAX_SPEED_WINDOW 		10000.0		// Maximum scaled Motor Speed (10000..0 RPM)


// General motor commands

#define STOP   0  // Stop motor
#define SLOW 1    // Slow speed
#define MEDIUM 256  // Medium speed
#define FAST 512  // Fast speed

// Conversion from 12-Bit values to volts (0..3.3 Volts)

const float ConvertThreeVolts = 3.3 / 4096.0;

// Conversion from 12-Bit values to volts (0..5 Volts)

const float ConvertFiveVolts = 5.0 / 4096.0;

// Conversion from 12-Bit values to (-90..+90) degrees

const float ConvertToDegrees = 180.0 / 4096.0;

const float ConvertToDegrees8 = 180.0 / 256.0;
 
//--------------------------------------------------------------------------------------------------------------------------

 
// Declare SPI Master Buffer variables here

#ifdef SPI_8
byte SPI_Rx_Buffer[SPI_BUFFER_SIZE];                // SPI Receive Buffer (bytes)
byte SPI_Tx_Buffer[SPI_BUFFER_SIZE];                // SPI Transmit Buffer (bytes)
#endif

#ifdef SPI_16
word SPI_Rx_Buffer[SPI_BUFFER_SIZE];                // SPI Receive Buffer (words)
word SPI_Tx_Buffer[SPI_BUFFER_SIZE];                // SPI Transmit Buffer (words)
#endif

int  SPI_Start_Buffer = 0;                         	// Start pointer for ring buffer
int SPI_End_Buffer = 0;								// SPI End Buffer Index

byte SPI_Rx_Flag = FALSE;                        	// Indicates that data was received into the ring buffer
word SPI_Timeout = 0;                            	// SPI I/O Timeout counter
byte SPI_Timeout_Flag;                           	// SPI Timeout flag
int SPI_Number_Characters = 0;						 // SPI Number of characters in buffer
int SPI_Communications_Error;                    	// SPI I/O error flag
                                                 	// -1 = SPI Input Bufer overrun error
                                                 	// -2 = SPI Communications timeout error
                                                 	// -3 = SPI buffer empty error

int SPI_Interrupt_Count = 0;                   		// Number of MSSP interrupts
int SPI_Data_In = 0;								// Data byte read from the SPI Master
int SPI_Data_Out = 0;								// Data byte sent to the SPI Master
int SPI_Status;										// SPI Status returned from routines
 
int MessageIndex = 0;								// Message ID number

volatile int Msg_Counter = 0;
  
/* Data received at SPI1 */

unsigned int datard ;

// Initialize the DC Motor Commands

byte ControllerState; 		// State of selected slave controller, range (0..255) 

MESSAGE_UNION_TYPE MotorMessage;
word SelectedCommand = 0;


unsigned char arraywr[] = {1,2,3,4,5,6,7,8,9,10,11,
12,13,14,15,16,0};
//24C040/080/160 page write size
unsigned char *wrptr = arraywr;
unsigned char arrayrd[16];
unsigned char *rdptr = arrayrd;
//unsigned char var;
//unsigned char TheCharacterRead;					// Character read
//unsigned char TheCharacterSent;					// Character sent
int SPIStatus;										// Status return from SPI function call
int Duty = 0;										// Current Duty Command  index
longword  DutyCommands[MAX_MOTORS];                  // Output integer duty commands 10-bit PWM range 0..1023
//int OuputDutyCommands[MAX_MOTORS];                  // Output integer duty commands 10-bit PWM range 0..1023
longword InputDutyCommands[MAX_MOTORS];					// Input Duty Commands to be sent 12-Bit read from POTs range 0..4095
float FloatDutyCommands[MAX_MOTORS];				// Scaled float Duty Commands to be sent range 0.0..1023.0

int MotorPeriod = 0;								// Period Command to be sent
int Lines = 0;                                          // Current line of data collected 
longword EncoderCounts[MAX_MOTORS] = {0, 0};							// Encoder counts
//**********************************************************************************
// Define Global variables common to all controllers here
//**********************************************************************************


byte ControllerState; 		// State of selected slave controller, range (0..255) 
											// 		0 => NO_STATES		No States    
											//		1 => INITIALIZED	The selected slave contoller is Initialzed 
											//		2 => DONE 	 	 	The selected slave contoller is done processing current command
											// 		3 => BUSY 	 		The selected slave contoller is busy processing current command 
											// 		4 => ERROR 	 		The selected slave contoller encountered an error processing the current command 

byte ControllerErrorCode; 	// Error code returned by the selected slave controller, range (0..255) 
											// 		0 => NO_ERRORS 		No error codes  
											// 		1 => FAILURE 	 	Failed to complete command
											//  	2 => SUCCESS  	 	Completed command successfully 
 
byte ControllerDiagnosticCode; 	// Diagnostic message code returned by the selected slave controller, range (0..255) 
											// 		0 => NO_DIAGNOSTICS 	No Diagnostic messages
											//  	1 => DIAGNOSTIC 1  	Diagnostic message 2 
											// 		2 => DIAGNOSTIC 2 	Diagnostic message 1


// FUNCTION Prototypes

int main(void);
void ServiceHighInterrupt(void);
void InitializeSPI(void);


// Application specific functions

void set_wren(void);
void busy_polling(void);
unsigned char status_read(void);
void status_write(unsigned char data);
void byte_write(unsigned char addhigh, unsigned char
addlow, unsigned char data);
void page_write(unsigned char addhigh, unsigned char
addlow, unsigned char *wrptr);
void array_read(unsigned char addhigh, unsigned char
addlow, unsigned char *rdptr,
unsigned char count);
unsigned char byte_read(unsigned char addhigh,
unsigned char addlow);
 
//**************************************************************************
//**************************************************************************
//**************************************************************************

void __attribute__((__interrupt__)) _SPI1Interrupt(void)
{
 
	IFS0bits.SPI1IF = 0;
	SPI1STATbits.SPIROV = 0; // Clear SPI1 receive overflow flag if set 
	PORTDbits.RD0 = 0;				// Set LED RD0
}


int main(void)
{

	long int i,j;									// Local Loop Index variables
	unsigned int spi2cfg_1,spi2cfg_2,x,y,z;		// SPI Configuration variables
	unsigned int channel,adc12cfg_1,adc12cfg_2,adc12cfg_3,adc12cfg_p,adc12cfg_s; // ADC Configuration variables
	unsigned int baud,uartmode,uartsta;
 	unsigned int result[20];
	float f1 = 3.1415;
 	unsigned int adc4_in, adc5_in, adc6_in; // ADC Input variables
    int Period_Input, Frequency_Input, Duty_Input, Position_Input, Speed_Input;    // 12-Bit motor POT input values range 0..4095
    int Period, Frequency, Duty, Position, Speed;  // 16-Bit values  
    float Period_Scaled, Frequency_Scaled, Duty_Scaled, Position_Scaled, Speed_Scaled;          // Scaled motor float values  
   	long int Lines = 0;										      // Number of pot motor commands read
    unsigned char Xb,Yb,Zb;				// Data Bytes
    unsigned char Xrb,Yrb,Zrb;				// Reference Data Bytes
	char Xc, Yc, Zc;					// Character preambles 'x', 'y', 'z'
	char Xrc, Yrc, Zrc;					// Character preambles 'u', 'v', 'w'
	long int Number_Of_Samples = 1000;		// Number of encoder values collected

 	long unsigned int Position_Command = 0;		// Position Command (encoder counts)
	long int Speed_Command = 0;			// Speed Command (RPM)

 	byte TriaxData; 					        // Data byte read from POT
	byte c;
	long int Start_Time_1 = 0;					// Start time 1 in millisecods
	long int Stop_Time_1 = 0;				    	// Stop time  1 in millisecods
	long int Elapsed_Time_1 = 0;					// Elapsed time 1  in millisecods
 	long int Start_Time_2 = 0;					// Start time 2 in millisecods
	long int Stop_Time_2 = 0;				    	// Stop time 2 in millisecods
	long int Elapsed_Time_2 = 0;					// Elapsed time 2  in millisecods
   

	/* Holds the information about SPI configuartion */
	unsigned int SPICONValue;

	/* Holds the information about SPI Enable/Disable */
	unsigned int SPISTATValue;

	/*Timeout value during which timer1 is ON */
	int timeout;

 
	Lines = 0;
 	// Configure all analog pins as digital

	//ADPCFG = 0xFFFF;

 	// Configure ADC Port B, AN15:AN0,  analog=0 and digital=1  

	ADPCFG = 0xE38F;  

	 // Initialize PORTs 

    // Configure Port D as outputs
	PORTD = 0x0000; 
	TRISD = 0x0000;  

	// LEDs using for debug
  	LED_1   = 1;					 			// LED 1 (0=On, 1=Off)
  	LED_2   = 1;					 			// LED 2 (0=On, 1=Off)
  	LED_3   = 1;					 			// LED 3 (0=On, 1=Off)
  	LED_4  	= 1;				 	 	 		// LED 4 (0=On, 1=Off)
 
	// Direction bits for LEDs
 	LED_1_DIR  = 0;				 			// LED 1 DIRECTION =0
 	LED_2_DIR  = 0;					 		// LED 2 DIRECTION =0 
 	LED_3_DIR  = 0;					 		// LED 3 DIRECTION =0 
 	LED_4_DIR  = 0; 					 	// LED 4 DIRECTION =0 
 
	TRISDbits.TRISD0 = OUTPUT;
	PORTDbits.RD0 = 1;				// Clear LED RD0


	// Initialize using VDI here...
    //InitializeMasterI2C(); // This I2C configuration from the PIC18C I2C peripheral library works fine!!!!

#ifdef MOTOR_TEST_2
	//**************************************************************************
	//* Setup the 12-Bit Analog to Digital Converter (works)
	//**************************************************************************
      
	ADCON1bits.ADON = 0; // turn off ADC 
    channel = ADC_CH0_POS_SAMPLEA_AN4 &    // Pot 3
              ADC_CH0_POS_SAMPLEA_AN5 &    // Pot 1
              ADC_CH0_POS_SAMPLEA_AN6 &    // Pot 2
              //ADC_CH0_POS_SAMPLEA_AN8 &
              //ADC_CH0_POS_SAMPLEA_AN9 &         
              //ADC_CH0_POS_SAMPLEA_AN10 &    // X   Works!
           	  //ADC_CH0_POS_SAMPLEA_AN11 &    // Y   Works!
              //ADC_CH0_POS_SAMPLEA_AN12 &    // Z   Works!
              //ADC_CH0_POS_SAMPLEA_AN13 &
              ADC_CH0_POS_SAMPLEA_AN15;


    SetChanADC12(channel);
	ConfigIntADC12(ADC_INT_DISABLE);

    adc12cfg_1 = ADC_MODULE_ON &
				 ADC_IDLE_CONTINUE &
				 ADC_FORMAT_INTG &
				 ADC_CLK_AUTO &
				 //ADC_CLK_MANUAL &
				 //ADC_AUTO_SAMPLING_OFF;
				 ADC_AUTO_SAMPLING_ON;
				 //ADC_SAMP_ON;

	adc12cfg_2 = ADC_VREF_AVDD_AVSS &
				 ADC_SCAN_ON &
	             ADC_SAMPLES_PER_INT_16 &
				 ADC_ALT_BUF_OFF &
				 ADC_ALT_INPUT_OFF;

	adc12cfg_3 = //ADC_SAMPLE_TIME_10 &
				 ADC_SAMPLE_TIME_31 &
				 ADC_CONV_CLK_SYSTEM &
                 ADC_CONV_CLK_INTERNAL_RC; // Works!
                 //ADC_CONV_CLK_13Tcy;
				 //ADC_CONV_CLK_32Tcy;  // Works!



	adc12cfg_p = ENABLE_AN4_ANA &    // Pot 3
				 ENABLE_AN5_ANA &    // Pot 1
				 ENABLE_AN6_ANA &    // Pot 2
				 //ENABLE_AN8_ANA &
				 //ENABLE_AN9_ANA &
				 //ENABLE_AN10_ANA &   // X   Works!
				 //ENABLE_AN11_ANA &   // Y   Works!
				 //ENABLE_AN12_ANA &   // Z   Works!
				 //ENABLE_AN13_ANA &
			  	 ENABLE_AN15_ANA,
				(BIT4|BIT5|BIT6|BIT8|BIT15);	//errata requires ch15 ???


	adc12cfg_s = SKIP_SCAN_AN0 &
                 SKIP_SCAN_AN1 &
                 SKIP_SCAN_AN2 &
                 SKIP_SCAN_AN3 &
                 //SKIP_SCAN_AN4 &    // Pot 3
                 //SKIP_SCAN_AN5 &    // Pot 1
                 //SKIP_SCAN_AN6 &    // Pot 2
                 SKIP_SCAN_AN7 &
                 SKIP_SCAN_AN8 &
                 SKIP_SCAN_AN9 &
                 SKIP_SCAN_AN10 &  // X   Works!
                 SKIP_SCAN_AN11 &  // Y   Works!
                 SKIP_SCAN_AN12 &  // Z   Works!
                 SKIP_SCAN_AN13 &
                 SKIP_SCAN_AN14 &
                 SKIP_SCAN_AN15;

 	OpenADC12(adc12cfg_1,adc12cfg_2,adc12cfg_3,adc12cfg_p,adc12cfg_s);


#endif 
 


#ifdef SPI_SUPPORT

	//**************************************************************************
 	//* Configure the SPI module for LCD and interprocessor communications here ...
	//**************************************************************************

	TMR1 = 0 ;
	timeout = 0;

	// Turn off SPI modules 
	CloseSPI1();
 
	// Initialize SPI1 for SPI master here
	CORCONbits.PSV = 1;
    PSVPAG = 0;

    LATBbits.LATB2 = 1;				// SPI1 Slave Select
    TRISBbits.TRISB2 = 0;
  
    LATGbits.LATG9 = 1;				// SPI2 Slave Select
    TRISGbits.TRISG9 = 0;

#ifdef SPI_8
	/* Configure SPI1 module to receive 16 bit timer value in
	slave mode */

	SPICONValue = FRAME_ENABLE_OFF & 
        FRAME_SYNC_OUTPUT &
		ENABLE_SDO_PIN & 
        //SPI_MODE16_ON &
        SPI_MODE16_OFF &
		SPI_SMP_OFF & 
        SPI_CKE_ON &
		SLAVE_ENABLE_OFF &
		CLK_POL_ACTIVE_HIGH &
		MASTER_ENABLE_ON &
		//SEC_PRESCAL_7_1 &
 		//PRI_PRESCAL_64_1;
		SEC_PRESCAL_1_1 &
 		PRI_PRESCAL_16_1;
#endif
 
#ifdef SPI_16
	/* Configure SPI1 module to receive 16 bit timer value in
	slave mode */

	SPICONValue = FRAME_ENABLE_OFF & 
        FRAME_SYNC_OUTPUT &
		ENABLE_SDO_PIN & 
        SPI_MODE16_ON &
		SPI_SMP_OFF & 
        SPI_CKE_ON &
		SLAVE_ENABLE_OFF &
		CLK_POL_ACTIVE_HIGH &
		MASTER_ENABLE_ON &
		//SEC_PRESCAL_7_1 &
 		//PRI_PRESCAL_64_1;
		SEC_PRESCAL_1_1 &
 		PRI_PRESCAL_16_1;
 #endif

	SPISTATValue = SPI_ENABLE & SPI_IDLE_CON & SPI_RX_OVFLOW_CLR;
	OpenSPI1(SPICONValue,SPISTATValue );

    PORTD = 0xFF;

	PORTDbits.RD0 = 1;				// Clear LED RD0

	// Configure SPI1 interrupt for priority 6
	ConfigIntSPI1(SPI_INT_EN & SPI_INT_PRI_6);

	// Initialize SPI1 and SPI2 for general interprocessor communications
/*


   LATGbits.LATG9 = 1;					SPI2 Slave Select 
    TRISGbits.TRISG9 = 0;

	spi2cfg_1 = FRAME_ENABLE_OFF &
		FRAME_SYNC_OUTPUT &
	            ENABLE_SDO_PIN &
	            SPI_MODE16_OFF &
		        SPI_SMP_ON &
		        SPI_CKE_OFF &
	  	SLAVE_ENABLE_OFF &
		        CLK_POL_ACTIVE_HIGH &
		        MASTER_ENABLE_ON &
		SEC_PRESCAL_7_1 &
		        PRI_PRESCAL_64_1;
	spi2cfg_2 = SPI_ENABLE &
	            SPI_IDLE_CON &
	            SPI_RX_OVFLOW_CLR; 
	OpenSPI2(spi2cfg_1,spi2cfg_2); 



	TMR1 = 0 ;
	timeout = 0;

	// Configure SPI2 interrupt for priority 6
	ConfigIntSPI2(SPI_INT_EN & SPI_INT_PRI_6);

	// Configure SPI1 module to transmit 16 bit timer1 value
	// in master mode 
	SPICONValue = FRAME_ENABLE_OFF & FRAME_SYNC_OUTPUT &
		ENABLE_SDO_PIN & SPI_MODE16_ON &
		SPI_SMP_ON & SPI_CKE_OFF &
		SLAVE_ENABLE_OFF &
		CLK_POL_ACTIVE_HIGH &
		MASTER_ENABLE_ON &
		SEC_PRESCAL_7_1 &
		PRI_PRESCAL_64_1;
		SPISTATValue = SPI_ENABLE & SPI_IDLE_CON &
		SPI_RX_OVFLOW_CLR;

	OpenSPI1(SPICONValue,SPISTATValue );

	// Configure SPI2 module to receive 16 bit timer value in
	// slave mode 
	SPICONValue = FRAME_ENABLE_OFF & FRAME_SYNC_OUTPUT &
		ENABLE_SDO_PIN & SPI_MODE16_ON &
		SPI_SMP_OFF & SPI_CKE_OFF &
		SLAVE_ENABLE_OFF &
		CLK_POL_ACTIVE_HIGH &
		MASTER_ENABLE_OFF &
		SEC_PRESCAL_7_1 &
		PRI_PRESCAL_64_1;
		SPISTATValue = SPI_ENABLE & SPI_IDLE_CON &
		PI_RX_OVFLOW_CLR;

	OpenSPI2(SPICONValue,SPISTATValue );
	T1CON = 0X8000;
	while(timeout< 100 )
	{
		timeout = timeout+2 ;
	}
	T1CON = 0;
	WriteSPI1(TMR1);
	while(SPI1STATbits.SPITBF);
	while(!DataRdySPI2());
	datard = ReadSPI2();
	if(datard <= 600)
	{
		PORTDbits.RD0 = 1;
	}
*/

#endif
 

  // Setup timers, configure ports and initialize variables

   SetupTimers();  

   // Configure USART for 20 MHz, 115200 Baud, 8-Bits, No parity, Asynchronous
   
    InitializeUart1(115200);

       // Display message to operator

#ifdef SPI_8
    printf("Master 8-Bit DC Motor Controller Application \r\n");
#endif

#ifdef SPI_16
    printf("Master 16-Bit DC Motor Controller Application \r\n");
#endif
	// Initialization Logic for SPI Slave Buffer Interface goes here ...

	SPI_Start_Buffer=0;                         		// Start pointer for ring buffer
	SPI_End_Buffer = 0;									// SPI End Buffer Index

	SPI_Rx_Flag = FALSE;                        		// Indicates that data was received into the ring buffer

	// Flush the Rx, Tx and message buffers (words)
	for (i=0; i<SPI_BUFFER_SIZE; i++)
	{
 		SPI_Rx_Buffer[i] = 0;                  			// Clear SPI Receive Buffer 
  		SPI_Tx_Buffer[i] = 0;        					// Clear SPI Transmit Buffer   
 	}

 	// Flush the message buffer (bytes)
	for (i=0; i<DATA_BUFFER_SIZE; i++)
	{
		MotorMessage.Message.Data[i] = 0;
 	}

/* 
	for (i=0; i<MAX_MOTORS; i++)
	{
  		MotorMessage.DutyCommand.Duty[i] = 0; 			// Duty Command range (0000 - FFFF) Hex              	
 		MotorMessage.DutyCommand.EncoderCounts[i] = 0; 	// Encoder Counts range (0000 - FFFF) Hex              	
	}		
*/

	// Give the SPI slave a change to initialize
    //pause(1);

 	Beep(1);
	printf("Start of 5 second delay...\r\n");

    printf("Size of SPI_Rx_Buffer = %d\r\n",sizeof(SPI_Rx_Buffer));
    printf("Size of SPI_Tx_Buffer = %d\r\n",sizeof(SPI_Tx_Buffer));
    printf("Size of MotorMessage = %d\r\n", sizeof(MotorMessage));
    printf("Size of MotorMessage.Message = %d\r\n", sizeof(MotorMessage.Message));
    printf("Size of MotorMessage.Message.Data = %d \r\n", sizeof(MotorMessage.Message.Data ));  
	printf("SPI_BUFFER_SIZE = %d \r\n", SPI_BUFFER_SIZE);
	printf("DATA_BUFFER_SIZE = %d \r\n", DATA_BUFFER_SIZE);

    // Give the SPI slave a change to initialize
 
    pause(5000);

    printf("End of 5 second delay!!!\r\n");
    Beep(1);

	BUSY_FLAG_DIR = INPUT;			// Busy line set to input


//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//*                 Send motor commands to the SPI peripheral here...
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************
//********************************************************************************************

//#ifdef POT_TEST
/*
// Define General Motor Properties here...

// Raw motor limits (Viewport)
#define MIN_POT_VIEWPORT  		0  			// Minimum POT value range 0..4095 12-Bits  
#define NOM_POT_VIEWPORT  		2047  		// Nominal POT value range 0..4095 12-Bits  
#define MAX_POT_VIEWPORT 		4095  		// Maximum POT value range 0..4095 12-Bits  

#define MIN_MOTOR_VIEWPORT  	0   		// Minimum PWM Duty Motor Command range 0..1023 10-Bits 
#define NOM_MOTOR_VIEWPORT  	511  		// Nominal PWM Duty Motor Command range 0..1023 10-Bits
#define MAX_MOTOR_VIEWPORT 		1023  		// Maximum PWM Duty Motor Command range 0..1023 10-Bits

  
// Scaled motor limits (Window)
#define MIN_PERIOD_WINDOW  		0.0  		// Minimum scaled PWM Period in milliseconds
#define NOM_PERIOD_WINDOW  		0.0  		// Nominal scaled PWM Period in milliseconds
#define MAX_PERIOD_WINDOW 		10000.0  	// Maximum scaled PWM Period in milliseconds

#define MIN_FREQUENCY_WINDOW  	0.0  		// Minimum scaled PWM Frequency in Hz
#define NOM_FREQUENCY_WINDOW  	0.0  		// Nominal scaled PWM Frequency in Hz
#define MAX_FREQUENCY_WINDOW 	10000.0  	// Maximum scaled PWM Frequency in Hz

#define MIN_DUTY_WINDOW 		0.0 		// Minimum scaled PWM Duty Motor Command
#define NOM_DUTY_WINDOW 		511.0 		// Nominal scaled PWM Duty Motor Command 
#define MAX_DUTY_WINDOW 		1023.0		// Maximum scaled PWM Duty Motor Command 

#define MIN_POSITION_WINDOW		0.0			// Minimum scaled motor position in encoder counts (0.0 CPR)
#define NOM_POSITION_WINDOW		50.0		// Minimum scaled motor position in encoder counts (50.0 CPR)
#define MAX_POSITION_WINDOW 	100.0		// Maximum scaled Motor position in encoder counts (100.0 CPR) 

#define MIX_SPEED_WINDOW 		0.0			// Minimum scaled Motor Speed (0.0 RPM)
#define NOM_SPEED_WINDOW 		5000.0		// Minimum scaled Motor Speed (5000.0 RPM)
#define MAX_SPEED_WINDOW 		10000.0		// Maximum scaled Motor Speed (10000..0 RPM)
 
int Period_Input, Frequency_Input, Duty_Input, Position_Input, Speed_Input;    // 12-Bit motor POT input values range 0..4095
int Period, Frequency, Duty, Position, Speed;  // 16-Bit values  
float Period_Scaled, Frequency_Scaled, Duty_Scaled, Position_Scaled, Speed_Scaled;          // Scaled motor float values  

*/
	// Ramp test mapping POT values range 0..4095 to PWM Duty Commands range 0.0 .. 1023 and Frequency range 0.0 .. 10000.0 

   // Set global viewport coordinates here to scale 12-Bit ADC POT input values to 10-Bit Duty values  

    set_view(MIN_POT_VIEWPORT, MIN_POT_VIEWPORT, MAX_POT_VIEWPORT, MAX_POT_VIEWPORT);

    // set global window coordinates here to scale 12-Bit ADC POT input values to 10-Bit Duty values and 32-Bit Float Frequency values

    //set_window(MIN_DUTY_WINDOW, MIN_FREQUENCY_WINDOW, MAX_DUTY_WINDOW, MAX_FREQUENCY_WINDOW);

    // set global window coordinates here to scale 12-Bit ADC POT input values to 10-Bit Duty values  

    set_window(MIN_DUTY_WINDOW, MIN_DUTY_WINDOW, MAX_DUTY_WINDOW, MAX_DUTY_WINDOW);

/*
// Test scaling functions.  Works!!!
    for (i=0; i<MAX_POT_VIEWPORT; i++)
	{
     	Duty_Input = i;    		// Duty 12-Bit motor POT input values range 0..4095           
     	Frequency_Input = i;   	// Frequency 12-Bit motor POT input values range 0..4095           
    	
		// Scale the motor PWM Duty commands and display them   
 
 	  	map_viewport_to_window (Duty_Input, Frequency_Input, &Duty_Scaled, &Frequency_Scaled);
    	printf("Motor Duty Input=%d, Duty Scaled = %f  Frequency Input=%d, Frequency Scaled = %f\r\n",  
                Duty_Input, Duty_Scaled, Frequency_Input, Frequency_Scaled);
		pause(100);

	}
  
*/

#ifdef MOTOR_TEST_1
	// Test all available message commands here ...
 	printf("Test all available message commands here ... \r\n");
	while(1)
	{
/*
		// Test the calibration command. Works!!!
  		printf("Calling SendCalibrateMotorCommand... \r\n");
 		SendCalibrateMotorCommand(0);  		 
		pause(5000);
*/

		// Test the period command
  		printf("Calling SendPeriodCommand... \r\n");
		j=0x87654321;
  		SendPeriodCommand(j );
  		pause(5000);

		// Test the duty command
  		printf("Calling SendDutyCommand... \r\n");
    
		DutyCommands[0] = 0xABCDEF;
		DutyCommands[1] = 0xFEDCBA;
 		SendDutyCommand(DutyCommands, 2); 
		pause(5000);

		// Test the status command
  		printf("Calling SendGetStatusCommand... \r\n");
   		SendGetStatusCommand();  			 
		pause(5000);

		// Test the number of samples command
  		printf("Calling SendNumberSamplesCommand... \r\n");
  		SendNumberSamplesCommand(0, 0x1234); 	 
		pause(5000);

		// Test the reset command
  		printf("Calling SendResetCommand... \r\n");
 		SendResetCommand();  				 
 		pause(5000);
	}
#endif


#ifdef MOTOR_TEST_2
	// Get motor commands from terminal and send them to the slave motor controller.  Works!!!!
	CharactersToRead = 8;                                    // Number of characters to read into buffer
 
	// Send PWM Period commands here...
//********************************************************************************
//* SetPWMCycles - This function actually computes the PWM1 amd PWM2 period and the duty cycle 
//* factors converting times measured in microseconds into ticks. It's all pretty obvious: 
//*   For modifying purposes, you need to: 
//*    (1) Possibly change to different clock scale factors 
//*    (2) Change the various PWM_PDIS* and PWM_PEN* statements 
//*    (3) Change from free-running mode (which is what I use) to whichever mode you prefer 
//* Note: It uses the TCY_PER_SECOND conversion factor defined above.  
//********************************************************************************
	printf("Enter PWM Period (ms) for motors in hex: ");
   	while (!RX_Data_Received);
  	//j = ReadHexWord();
 	j = ReadHexUnsignedLongInteger();
   	printf("\r\n");

   	printf(" The Period in hex (ms) is: ");
 	hex8(j);
   	printf("\r\n");
	printf("Sending the PWM Period (ms) to motors... ");
  
	
	//j=0x1000;
  	SendPeriodCommand(0, j );
	pause(10);
 	SendPeriodCommand(1, j );
	pause(10);

	FlushBuffer();

	printf("Motor Test 1 - Send Position commands to move motor to a specific position...\r\n ");
	//******************************************************************************************
	// Test 1 - Position command to move motor to a specific position.  Works!!!! 
	//******************************************************************************************
	EncoderCounts[0] = 0;
	EncoderCounts[1] = 0;

	printf("Motor Position Test ...\r\n");
	
	// Get PWM Duty commands ...
//******************************************************************************
//* SetDuty - a new PWM duty to be used for the next PWM DC motor Command, range 
//* (0000..FFFF) microseconds to the selected motor device, range (00..01).
//******************************************************************************

	printf("Enter PWM Duty Command for motors: ");
  	while (!RX_Data_Received);
 	//Duty = ReadHexWord();
 	Duty = ReadHexUnsignedLongInteger();
	printf("\r\n");

   	printf(" The Duty Command in hex is: ");
 	hex8(Duty);
   	printf("\r\n");
 

	// Send Position commands here...
	printf("Enter desired motor position (encoder counts in hex): ");
   	while (!RX_Data_Received);
  	Position_Command = ReadHexUnsignedLongInteger();
   	printf("\r\n");

   	printf(" The selected Position Command in hex is: ");
 	hex8(Position_Command);
   	printf("\r\n");
	printf("Sending the Position Command to motors... ");
  

 	Beep(1);
	printf("Start of 5 second delay...\r\n");

  
    // Give the SPI slave a change to initialize
 
    pause(5000);

    printf("End of 5 second delay!!!\r\n");
    Beep(1);


	// Check status until desired position is reached and then stop the motors at the selected position. Works!!!
	while(1)
	{
		// Send up to MAX_MOTORS number of duty commands
		DutyCommands[0] = Duty;
		DutyCommands[1] = Duty;
		SendDutyCommand(DutyCommands, 2);
 		for (i=0; i<MotorMessage.DutyCommand.NumberOfMotors; i++)
		{
			EncoderCounts[i]+= MotorMessage.DutyCommand.EncoderCounts[i];
    		printf("trace 2: Motor ID = %d, Duty = %d Encoder Count = %ld \r\n",i ,MotorMessage.DutyCommand.Duty[i],
                    EncoderCounts[i]);
              //	MotorMessage.DutyCommand.EncoderCounts[i]);  
			if ( EncoderCounts[i] > Position_Command)
			{
				// Target Position has been reached, stop the motors
				printf("Target Position has been reached, stop the motors!!!\r\n");

 				DutyCommands[0] = STOP;
				DutyCommands[1] = STOP;
				SendDutyCommand(DutyCommands, 2);
				pause(5000);
				goto Exit_Loop;
			}
    	}
		FlushBuffer();
		
	}
Exit_Loop:
#endif

#ifdef MOTOR_TEST_3
	//******************************************************************************************
	// Test 2 - Manual Motor Control 
	//******************************************************************************************
   
	// Initialize motor Data Collection parameters here...
	Lines = 0;
	Number_Of_Samples = 10000;
	LED_1 = OFF;
	printf("Motor Test2 -  Start collecting and processing encoder data from motors  ... \r\n");

	// Start collecting encoder data from motors...

    Start_Time_1 = TimerCount3;
    Start_Time_2 = TimerCount3;

    // Use on-board pots and ADC to vary the PWM frequency and duty 
    while (1)
    {

    	Nop();
		i = 0;
		while( i <16 )
		{
 
   			// Convert the data from the PWM Period and Duty to raw 12-Bit values

			ConvertADC12();
 			while(ADCON1bits.SAMP);
			while(!BusyADC12());
 			//while(BusyADC12());

 			// Read the ADC for selected channels 
			result[i] = ReadADC12(i);
			i++;
		}

		// Collect and Display position, veclocity and acceleration values to the LCD every 0.5 seconds (500 1 ms ticks)
		//if ((TimerCount3 % 1000) == 0) 
		//if ((TimerCount3 % 500) == 0)
		if ((TimerCount3 - Start_Time_1) >= 500) 
		{ 
			Start_Time_1 = TimerCount3;			// Reset timer
			// Voltage values by POT channel Works!!!
    		printf("  Pot 1  = %f",result[5] * ConvertFiveVolts);
     		printf("  Pot 2  = %f",result[6] * ConvertFiveVolts);
    		printf("  Pot 3  = %f Volts\r\n",result[4] * ConvertFiveVolts);
        }

#ifdef NEW_CODE
/*
 			// Clear LCD and home the cursor and display the HEADSET POT Values
 			Lcd_Clear();

			Lcd_Set_Character_Position(1,0);
			Lcd_Printf("X,Y,Z Tilt Angles");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf("X = %3.2f Degrees",X_Axis_Angle);
 			Lcd_Set_Character_Position(1,2);
   	  		Lcd_Printf("Y = %3.2f Degrees",Y_Axis_Angle);			
  			Lcd_Set_Character_Position(1,3);
     		Lcd_Printf("Z = %3.2f Degrees",Z_Axis_Angle);
*/
			// Clear LCD and home the cursor and display the Reference POT Values
 			Lcd_Clear();

			Lcd_Set_Character_Position(1,0);
			Lcd_Printf("X,Y,Z Tilt Angles");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf("X = %3.2f Degrees",X_Axis_Ref_Angle);
 			Lcd_Set_Character_Position(1,2);
   	  		Lcd_Printf("Y = %3.2f Degrees",Y_Axis_Ref_Angle);			
  			Lcd_Set_Character_Position(1,3);
     		Lcd_Printf("Z = %3.2f Degrees",Z_Axis_Ref_Angle);

 
/*
			// 12-Bit values read from the (X,Y,Z) accelerometers.  Works!!!
			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" 12-Bit ADC Values ");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf(" X  = %d Bits",result[X_INDEX]);
 			Lcd_Set_Character_Position(1,2);
    		Lcd_Printf(" Y  = %d Bits",result[Y_INDEX]);			
  			Lcd_Set_Character_Position(1,3);
      		Lcd_Printf(" Z  = %d Bits",result[Z_INDEX]);

			// 8-Bit values read from the POT (X,Y,Z) accelerometers.  Works!!!
			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" 8-Bit ADC Values ");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf(" X  = %d Bits",Xb);
 			Lcd_Set_Character_Position(1,2);
    		Lcd_Printf(" Y  = %d Bits",Yb);			
  			Lcd_Set_Character_Position(1,3);
      		Lcd_Printf(" Z  = %d Bits",Zb;



			// Voltage values read from the (X,Y,Z) accelerometers.  Works!!!
			X_Axis_Volts = result[X_INDEX] * ConvertThreeVolts;
			Y_Axis_Volts = result[Y_INDEX] * ConvertThreeVolts;
			Z_Axis_Volts = result[Z_INDEX] * ConvertFiveVolts;
          
 			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" X,Y,Z Accel. Volts");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf(" X  = %1.2f Volts",X_Axis_Volts);
 			Lcd_Set_Character_Position(1,2);
   	  		Lcd_Printf(" Y  = %1.2f Volts",Y_Axis_Volts);			
  			Lcd_Set_Character_Position(1,3);
     		Lcd_Printf(" Z  = %1.2f Volts",Z_Axis_Volts);


			// Angles values read from the (X,Y,Z) accelerometers.  Works!!!
			X_Axis_Angle = result[X_INDEX] * ConvertToDegrees;
			Y_Axis_Angle = result[Y_INDEX] * ConvertToDegrees;
			Z_Axis_Angle = result[Z_INDEX] * ConvertToDegrees;
          
 			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" X,Y,Z Tilt Angles");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf(" X  = %3.2f Degrees",X_Axis_Angle);
 			Lcd_Set_Character_Position(1,2);
   	  		Lcd_Printf(" Y  = %3.2f Degrees",Y_Axis_Angle);			
  			Lcd_Set_Character_Position(1,3);
     		Lcd_Printf(" Z  = %3.2f Degrees",Z_Axis_Angle);


			// 12-Bit values read from the (X,Y,Z) accelerometers.  Works!!!
			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" 12-Bit ADC Values ");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf(" X  = %d Bits",result[X_INDEX]);
 			Lcd_Set_Character_Position(1,2);
    		Lcd_Printf(" Y  = %d Bits",result[Y_INDEX]);			
  			Lcd_Set_Character_Position(1,3);
      		Lcd_Printf(" Z  = %d Bits",result[Z_INDEX]);

			// Voltage values read by POT channel Works!!!
			Lcd_Set_Character_Position(1,0);
			Lcd_Printf(" Pots 1-3 Volts");
			Lcd_Set_Character_Position(1,1);
    		Lcd_Printf("  Pot 1  = %1.2f",result[5] * ConvertFiveVolts);
 			Lcd_Set_Character_Position(1,2);
     		Lcd_Printf("  Pot 2  = %1.2f",result[6] * ConvertFiveVolts);
  			Lcd_Set_Character_Position(1,3);
    		Lcd_Printf("  Pot 3  = %1.2f Volts\r\n",result[4] * ConvertFiveVolts);
*/
		}


		//if ((TimerCount3 % 100) == 0) 
		if ((TimerCount3 - Start_Time_2) >= 100) 
		{ 
			Start_Time_2 = TimerCount3;			// Reset timer

			// Flash LED 3
			LED_3 = ~ LED_3;


/*
			// Voltage values by POT channel Works!!!
    		printf("  Pot 1  = %f",result[5] * ConvertFiveVolts);
     		printf("  Pot 2  = %f",result[6] * ConvertFiveVolts);
    		printf("  Pot 3  = %f Volts\r\n",result[4] * ConvertFiveVolts);

			// Voltage values from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %f",result[X_INDEX] * ConvertThreeVolts);
    		printf("  Y  = %f",result[Y_INDEX]  * ConvertThreeVolts);			
     		//printf("  Y  = %f Volts\r\n",result[Y_INDEX]  * ConvertThreeVolts);
     		//printf("  Y  = %f",result[Y_INDEX]  * ConvertThreeVolts);
     		printf("  Z  = %f Volts\r\n",result[Z_INDEX] * ConvertFiveVolts);


			// 12-Bit values read from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %d",result[X_INDEX]);
    		printf("  Y  = %d",result[Y_INDEX]);			
     		//printf("  Y  = %d Bits\r\n",result[Y_INDEX]);
     		//printf("  Y  = %d",result[Y_INDEX]);
     		printf("  Z  = %d Bits\r\n",result[Z_INDEX]);
 
			// Voltage values from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %f",result[X_INDEX] * ConvertThreeVolts);
    		printf("  Y  = %f",result[Y_INDEX]  * ConvertThreeVolts);			
     		//printf("  Y  = %f Volts\r\n",result[Y_INDEX]  * ConvertThreeVolts);
     		//printf("  Y  = %f",result[Y_INDEX]  * ConvertThreeVolts);
     		printf("  Z  = %f Volts\r\n",result[Z_INDEX] * ConvertFiveVolts);

			// Voltage values from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %f",result[X_INDEX] * .00122);
    		printf("  Y  = %f",result[Y_INDEX] * .00122);			
     		//printf("  Y  = %f Volts\r\n",result[Y_INDEX] * .00122 );
     		//printf("  Y  = %f",result[Y_INDEX] * .00122);
     		printf("  Z  = %f Volts\r\n",result[Z_INDEX] * .00122);

			// Voltage values from the (X,Y,Z) accelerometers.  Works!!!
    		printf("  X  = %f",result[X_INDEX] * .00122);
    		printf("  Y  = %f",result[Y_INDEX] * .00122);			
     		//printf("  Y  = %f Volts\r\n",result[Y_INDEX] * .00122 );
     		//printf("  Y  = %f",result[Y_INDEX] * .00122);
     		printf("  Z  = %f Volts\r\n",result[Z_INDEX] * .00122);

			// Display timer interrupt counts here...  (This code works!!!)
    		// Note: Timer Interrupts must be enabled using compilation flag                             
         	printf("TimerCount1 = %ld, TimerCount2 = %ld, TimerCount3 = %ld, TimerCount4 = %ld, TimerCount5 = %ld \r\n",
               TimerCount1, TimerCount2, TimerCount3, TimerCount4, TimerCount5 );                         // Display timer  counts
 
      		//Nop();
			//pause(500);
*/
 
  
		}

		// Write the encoder data to a Matlab "m" file for analysis

		//printf("Xb=%d Yb=%d Zb=%d \r\n", Xb, Yb, Zb);
		//fprintf(MatlabOutputFile, "x%dy%dz%d\r\n", Xb, Yb, Zb); // TRIAX Format
		//fprintf(MatlabOutputFile, "%d %d %d \r\n", Lines, Xb, Yb, Zb); // Matlab Format

		// Send data with line count
		printf("%ld %d %d %d \r\n", Lines, Xb, Yb, Zb); // Matlab Format

		// Send data with time stamp in 1 ms tics
		printf("%ld %d %d %d \r\n", TimerCount3, Xb, Yb, Zb); // Matlab Format
 
#endif

 NextValue:
		Lines++;			// Count number of lines read
		//if (Lines > Number_Of_Samples) goto ExitLoop;
/*
   		printf("  Pot 1  = %f",result[5] * ConvertFiveVolts);
     	printf("  Pot 2  = %f",result[6] * ConvertFiveVolts);
    	printf("  Pot 3  = %f Volts\r\n",result[4] * ConvertFiveVolts);
*/

    	
		// Scale the motor PWM Duty commands and display them   
     	//Frequency_Input = i;   	// Frequency 12-Bit motor POT input values range 0..4095           
     	InputDutyCommands[0] = result[5];    		// Input Duty 12-Bit motor 0 POT #1 input values range 0..4095           
   	    InputDutyCommands[1] = result[6];    		// Input Duty 12-Bit motor 1 POT #2 input values range 0..4095           
 
 	  	map_viewport_to_window (InputDutyCommands[0], InputDutyCommands[1], &FloatDutyCommands[0], &FloatDutyCommands[1]);

		// Convert the scaled PWM motor commands to integer
		//OutputDutyCommands[0] = (int) FloatDutyCommands[0];
		//OutputDutyCommands[1] = (int) FloatDutyCommands[1];
		DutyCommands[0] = (int) FloatDutyCommands[0];
		DutyCommands[1] = (int) FloatDutyCommands[1];

    	//printf("Motor Duty Input=%d, Duty Scaled = %f  Frequency Input=%d, Frequency Scaled = %f\r\n",  
        //        Duty_Input, Duty_Scaled, Frequency_Input, Frequency_Scaled);
    	printf("Motor Duty Input=%d, %d  Duty Output = %d  %d\r\n",  
                InputDutyCommands[0], InputDutyCommands[1],  DutyCommands[0], DutyCommands[1]);
    	//printf("Motor Duty Input=%d, %d  Float Duty Output = %f  %f\r\n",  
        //        InputDutyCommands[0], InputDutyCommands[1],  FloatDutyCommands[0], FloatDutyCommands[1]);

		// Send the scaled duty commands to motors
		// Send up to MAX_MOTORS number of duty commands
       	//printf("trace2: Sending DutyCommand = %d\r\n", Duty);

 		SendDutyCommand(DutyCommands, 2);
		pause(500);

	}


ExitLoop:

 	printf("Stopped processing motor commands. \r\n");

#endif 



#ifndef MOTOR_TEST_4
	// Get motor commands from terminal and send them to the slave motor controller.  Works!!!!
	CharactersToRead = 8;                                    // Number of characters to read into buffer
 
	// Send PWM Period commands here...
//********************************************************************************
//* SetPWMCycles - This function actually computes the PWM1 amd PWM2 period and the duty cycle 
//* factors converting times measured in microseconds into ticks. It's all pretty obvious: 
//*   For modifying purposes, you need to: 
//*    (1) Possibly change to different clock scale factors 
//*    (2) Change the various PWM_PDIS* and PWM_PEN* statements 
//*    (3) Change from free-running mode (which is what I use) to whichever mode you prefer 
//* Note: It uses the TCY_PER_SECOND conversion factor defined above.  
//********************************************************************************
	printf("Motor Test 3 - Send   PWM Period and Duty commands (ms) to motors...\r\n ");
	printf("Enter PWM Period (ms) for motors in hex: ");
	// Get motor commands from terminal and send them to the slave motor controller
	CharactersToRead = 8;                                    // Number of characters to read into buffer
   	while (!RX_Data_Received);
  	//j = ReadHexWord();
 	j = ReadHexUnsignedLongInteger();
   	printf("\r\n");

   	printf(" The Period in hex (ms) is: ");
 	hex8(j);
   	printf("\r\n");
	printf("Sending the PWM Period (ms) to motors... ");
  
	
	//j=0x1000;
   	SendPeriodCommand( j );
	pause(10);
  	SendPeriodCommand( j );
	pause(10);
  	SendPeriodCommand( j );
	pause(10);

//	FlushBuffer();

 
/*
	// Send PWM Duty commands ...  Works!!!!
	while(1)
	{

		// Send PWM Duty commands ...

		printf("Enter PWM Duty Command for motors: ");
  		while (!RX_Data_Received);
 		Duty = ReadHexWord();
		printf("\r\n");

    	printf(" The Duty Command in hex is: ");
 		hex4(Duty);
   		printf("\r\n");
		printf("Sending the PWM Duty commands to motors... ");

	   	// Send up to MAX_MOTORS number of duty commands
		DutyCommands[0] = Duty;
		DutyCommands[1] = Duty;
 		SendDutyCommand(DutyCommands, 2);

		FlushBuffer();

	}

*/

	// Test the duty and period command messages using ramps here ...
    while (1)
    { 	

		// Ramp up the motors from 0 to 10000 milliseconds PWM duty
		
 		for (Duty=0; Duty<1024; Duty++)
		{
			// Send up to MAX_MOTORS number of duty commands
			for (j=0; j<2; j++)
			{
		   		DutyCommands[j] = Duty;
			}
        	//printf("trace2: Sending DutyCommand = %d\r\n", Duty);
 			SendDutyCommand(DutyCommands, 2);
			//pause(500);
 			
  		}

		// Test the Period Command
		for (j=1000; j<10000; j+=100)
		{ 			
  			SendPeriodCommand( j );
   
 			//pause(500);
		}

/*		
		// Test the Get Status Motor Command here
		for (j=0; j<6; j++)
		{ 			
			SendGetStatusCommand();
  			//pause(3000);
		}	
   		
  		// Test the Calibrate Motor Command here
		for (j=0; j<6; j++)
		{ 			
  			SendCalibrateMotorCommand(0);  
  			//pause(3000);
  		}
   		
  		// Test the Number Of Samples Command here 
		for (j=0; j<6; j++)
		{ 			
  			SendNumberSamplesCommand(0, 1234);
  			//pause(3000);
		}
  		
 		
 		// Test the Period Command
		for (j=0; j<6; j++)
		{ 			
  			SendPeriodCommand(0, 0x1F );
  			//pause(3000);
		}
  		
 */
			
		// Ramp down the motors from 1000 to 0
		
		for (Duty=1000; Duty>=0; Duty--)
 		{
			// Send up to MAX_MOTORS number of duty commands
			for (j=0; j< 2; j++)
			{
		   		DutyCommands[j] = Duty;
	 		}
 			SendDutyCommand(DutyCommands, 2);
 			//pause(2000);
 			
		}
/*		
  		// Test the Reset Command here
		for (j=0; j<6; j++)
		{ 			
			SendResetCommand();
		}
	}
*/

  }
 
#endif

/*		
		// This is the section where user selected command messages are issued to the Slave 
		// In this case High Level Master DC Motor Controller Commands are issued here ....
		// These functions may be used to test the GearBot and GardenBot motors...

  		Neutral();									// Motor is in Neutral or Stopped
  		Neutral_To_Forward();						// Accelerate from Neutral to maximum Forward  Speed
  		Forward_To_Neutral();						// De-accelerate from maximum Forward speed to Neutral
  		Neutral_To_Reverse();						// Accelerate from Neutral to maximum Reverse speed   
  		Reverse_To_Neutral();						// De-accelerate from maximum Reverse speed to Neutral
      	Left();										// Turn left
      	HardLeft();									// Turn hard left
      	Right();									// Turn right
      	HardRight();								// Turn hard right

*/


		
		
 
#ifdef SPI_TEST_1

	// Send a ramp of data to SPI slave.  This code works!!!

	printf("Sending data to SPI Slave ...\r\n");
  
	while (1)
	{
		printf("trace 1: ****************************************BUSY_FLAG = %d \r\n", BUSY_FLAG);
 
		// Send a sync word of data to the SPI slave
		//WriteSPI1(99);
		//while(SPI1STATbits.SPITBF);
 
		for (i=0; i<SPI_BUFFER_SIZE; i++)
		{
     		LATBbits.LATB2 = 0;				// Enable SPI1 Slave Select

			// Send a word of data to the SPI slave
			WriteSPI1(i*100);
			while(SPI1STATbits.SPITBF);
			pause(1);

			// Read byte of data exchanged by the SPI slave
 			while(!DataRdySPI1());
			datard = ReadSPI1();
		
     		LATBbits.LATB2 = 1;				// Disable SPI1 Slave Select

        	printf("Received i= %d, datard = %d \r\n", i, datard);
            //pause(1);
   		}
		printf("trace 2: ****************************************BUSY_FLAG = %d \r\n", BUSY_FLAG);
     	pause(500);
        printf("\r\n");

		// Wait for SPI Slave peripheral to complete requested command

		while(BUSY_FLAG);
 	}

#endif

	/* Turn off SPI module and clear IF bit */
	CloseSPI1();
 
	return 0;
}

//**********************************************************************************
//* SendSPIMessage - Sends a command message to the SPI Slave and stores the data received 
//* into the SPI_Rx_Buffer so that it may be processed by Reveive Message. 
//**********************************************************************************
byte SendSPIMessage(void)
{
	int i;														// Loop index 		
	byte SendSPIMessageStatus = TRUE;								// Status return
 	word LocalChecksumSent = 0;				// Local message checksum computed from  message sent to peripheral
 	int TempValue = 0;

 
	// Clear Local checksum computed from message sent to peripheral
 	LocalChecksumSent = 0;

 
	// Send a test ramp that spans 0..SPI_BUFFER_SIZE. This code works!!!

	// Compute checksum for message sent to peripheral

	for (i=0; i<MESSAGE_BUFFER_SIZE; i++)
  	{
		// Test SPI Interface by generating a ramp here...
       	//MotorMessage.Message.Data[i] = MESSAGE_BUFFER_SIZE - i;
        //MotorMessage.Message.Data[i] = i+1;   // Generate a ramp 1..33
        //MotorMessage.Message.Data[i] = i;   // Generate a ramp 0..32

 		if (i < (MESSAGE_BUFFER_SIZE-2))
		{ 
			// Compute local checksum sent to peripheral
			LocalChecksumSent += MotorMessage.Message.Data[i];
		}
 	}

	// Save message checksum sent in message to peripheral 
	MotorMessage.Message.Data[CHECKSUM_OFFSET] = LocalChecksumSent;

#ifdef TEST
 
 	printf("SEND: Message Checksum Sent = %04X %d\r\n", LocalChecksumSent, LocalChecksumSent);
	//pause(1000);
 	//printf("Msg_Counter = %d \r\n", Msg_Counter);
  	 	
#endif	
 
 

 	// Copy the Motor Message to the SPI Transmit Buffer so that it can be sent to the SPI peripheral
    //      Destination                   Source
     memcpy( SPI_Tx_Buffer, &MotorMessage.Message, sizeof(MotorMessage.Message));   


 
#ifdef TEST
	// Test code to send a ramp 1..72 to the SPI Slave
 	for (i=0; i<sizeof(SPI_Tx_Buffer); i++)
  	{
       //SPI_Tx_Buffer[i] = i+1;   // Generate a ramp 1..72
       SPI_Tx_Buffer[i] = sizeof(SPI_Tx_Buffer)-i;   // Generate a ramp 72..1
	}

	for (i=0; i<DATA_BUFFER_SIZE; i++)
	//for (i=0; i<DataSize; i++)
	{
       	printf("trace 1: Send: i= %d, MotorMessage.Message.Data[%d]  = %d \r\n", i, i, MotorMessage.Message.Data[i]);
		pause(100);
	}
 		

#endif

	//**********************************************************************************
	// Send an SPI message command here
	//**********************************************************************************
 
	// Send a sync word of data to the SPI slave
	WriteSPI1(99);
	while(SPI1STATbits.SPITBF);
 
	for (i=0; i<SPI_BUFFER_SIZE; i++)
	{
    	LATBbits.LATB2 = 0;				// Enable SPI1 Slave Select

		// Send a word of data to the SPI slave
		WriteSPI1(SPI_Tx_Buffer[i]);
		while(SPI1STATbits.SPITBF);
		pause(1);

		// Read byte of data exchanged by the SPI slave
 		while(!DataRdySPI1());
 
		if (i != 0)
		{
			SPI_Rx_Buffer[i-1] = ReadSPI1();
 		}
		else
		{
			// Throw out first reading
			TempValue = ReadSPI1();
		}
		
   		LATBbits.LATB2 = 1;				// Disable SPI1 Slave Select

   	}
 
		// Wait for SPI Slave peripheral to complete requested command

		while(BUSY_FLAG);
  
#ifdef TEST 		
    printf("\r\n");
#endif

 
#ifdef TEST 		
	

	// Display contents of SPI Buffers 	
			 
	for (i=0; i<SPI_BUFFER_SIZE; i++)
	{
		printf("trace 3: i = %3d,  SPI_Tx_Buffer = %4d,  SPI_Rx_Buffer = %4d \r\n", i, SPI_Tx_Buffer[i], SPI_Rx_Buffer[i]);
 			
		pause(200);
	}
	printf("\r\n\r\n");

 
#endif
		
	return SendSPIMessageStatus;
}

//**********************************************************************************
//* ReceiveSPIMessage - Receives and processes the message that is in the SPI_Rx_Buffer 
//* which was simultaneously sent back from the SPI Slave Controller.  The message may
//* contain data or status information.
//**********************************************************************************
byte ReceiveSPIMessage(void)
{
	int i; 															// Loop index 
 	byte ReceiveSPIMessageStatus = TRUE;							// Status return
 	word LocalChecksumCalculated = 0;				// Local message checksum computed from message received from peripheral

 
	// Clear the old message

	for(i=0; i<sizeof(MotorMessage.Message.Data); i++)
	{
		MotorMessage.Message.Data[i] = 0;
	}

  	// Copy the entire DC Motor Controller message received from the SPI Slave
   	// to local storage for processing.  It may include sensor data, telemetry and status 
   	// information and is received concurrently with each message that the Master SPI controller sends.
    //memcpy(&MotorMessage.Message.Data, SPI_Rx_Buffer, sizeof(MotorMessage.Message.Data));  
#ifdef SPI_8
    memcpy(&MotorMessage.Message.Data, SPI_Rx_Buffer+1, sizeof(SPI_Rx_Buffer));   // Works!!!! 
#endif
 
#ifdef SPI_16
    memcpy(&MotorMessage.Message.Data, SPI_Rx_Buffer, sizeof(MotorMessage.Message.Data));  
#endif



	// Clear Local checksum received from peripheral
 	LocalChecksumCalculated = 0;

	// Compute checksum for message received 

	for (i=0; i<MESSAGE_BUFFER_SIZE; i++)
  	{
 		if (i < (MESSAGE_BUFFER_SIZE-2))
		{ 
			// Compute local checksum from message received from peripheral
			LocalChecksumCalculated  += MotorMessage.Message.Data[i];
		}
 	}


 
#ifdef TEST
	// Message Checksum 
  
 	printf("RECEIVE: Message Checksum Received = %04X\r\n",  MotorMessage.Message.Data[CHECKSUM_OFFSET]);
 	printf("RECEIVE: Message Checksum Calculated = %04X\r\n", LocalChecksumCalculated);
   	pause(1000);

  	for (i=0; i<DATA_BUFFER_SIZE; i++)
	{
		if ((i<10) || (i>30))
		{
       		printf("trace 3: received: i= %d, MotorMessage.Message.Data[%d]  = %04X \r\n", i, i, MotorMessage.Message.Data[i]);
		}
	}
	pause(5000);

#endif
   		
 	//**********************************************************************************************
 	// Display the message received from the SPI Slave here
 
#ifdef TEST	  
      	
    // Generic Message ID received  from the SPI Slave here ...
   	printf(" Motor Message: Message ID = %d \r\n", MotorMessage.SelectedCommand.MessageID);    			 
   	//printf(" Motor Message: Message ID = %d \r\n", MotorMessage.DutyCommand.MessageID);   
   	//printf(" Motor Message: Message ID = %d \r\n", MotorMessage.Message.Data[0]);   
  			 
  	// Generic command received  from the SPI Slave here  
  	printf(" Motor Message: Command = %d \r\n",  MotorMessage.SelectedCommand.Command);
   	//printf(" Motor Message: Command = %d \r\n", MotorMessage.DutyCommand.Command);   
  	//printf(" Motor Message: Command = %d \r\n",  MotorMessage.Message.Data[1]);
 
	// Generic Motor Message  status received  from the SPI Slave here
	printf(" Motor Message: Status = %d \r\n",  MotorMessage.SelectedCommand.CommandStatus);				
	//printf(" Motor Message: Status = %d \r\n",  MotorMessage.Message.Data[2]);				
	pause(4000);
#endif
		  

    // Compare the Message checksum to the local checksum

	if (MotorMessage.Message.Data[CHECKSUM_OFFSET] != LocalChecksumCalculated)
	{
		printf("Error!!! MotorMessage.Message.Data[CHECKSUM_OFFSET] != LocalChecksum\r\n"); 
        //pause(5000);
 	}
	else
 	switch (MotorMessage.SelectedCommand.Command)
  	{	
 	
		case CALIBRATE_MOTOR_COMMAND:
		{
 			// Receive and process the DC Motor Controller Calibrate Command 
 			// response message from the Slave
 			ReceiveCalibrateMotorResponse();
				
			break;
		}
			
		case DUTY_COMMAND:
		{
		
 			// Receive and process the Send PWM duty command response message from the Slave
  
			ReceiveDutyResponse();

 			break;
		}
			
		case GET_POT_COMMAND:
		{
	
  			// Receive and process the Get Pot command response message from the Slave
 
			ReceiveGetPotResponse();

 			break;
		}
			
		case HOLD_POT_COMMAND:
		{

 			// Receive and process the Hold Pot Command Response message
 
			ReceiveHoldPotResponse();
  			break;
		}
			
		case NUMBER_SAMPLES_COMMAND:
		{

			//**********************************************************************************
 			// Receive and process the Get number of samples command response message from the Slave
			//**********************************************************************************
 
			ReceiveNumberSamplesResponse();
	
  			break;
		}

		case PERIOD_COMMAND:
		{
			//**********************************************************************************
 			// Receive and process the Send period command response message from the Slave
			//**********************************************************************************
 
 			ReceivePeriodResponse();
   			break;
		}
		
		case GET_STATUS_COMMAND:
		{
			//**********************************************************************************
 			// Receive and process the Get Status command response message from the Slave
			//**********************************************************************************
 
 			ReceiveGetStatusResponse();
   			break;
		}
 	
		case RESET_COMMAND:
		{
			//**********************************************************************************
 			// Receive and process the Reset Command response message from the Slave
			//**********************************************************************************
 
 			ReceiveResetResponse();
   			break;
		}	
	}	
	
 	//**********************************************************************************************
	
	
	//pause(2000);
	 	
 	// Generate next command 
		
	//SelectedCommand++;   
	//SelectedCommand = SelectedCommand % 6;
 
 

   return ReceiveSPIMessageStatus;
}
 	
//**********************************************************************************
//* CheckControllerState - Check the status of the the current controller to see if
//* the command has been executed correctly.  Check the following Controller state
//* variables: 
//*		ControllerState	{ NO_STATES=0, INITIALIZED=1, DONE=2, BUSY=3, ERROR=4 }
//* 	ControllerErrorCode  { NO_ERRORS=0, FAILURE=1, SUCCESS=2 }	 
//* 	ControllerDiagnosticCode { NO_DIAGNOSTICS=0, DIAGNOSTIC1=1, DIAGNOSTIC2=2 }    
//**********************************************************************************
 byte CheckControllerState(void)
 {

	byte CheckControllerStateStatus = NO_STATES;
	
	// Start out with Controller State set to "NO_STATES" and continue to send and
	// receive Get Status messages until either the controller state changes to "DONE" or
	// gets an error state.
	
    ControllerState =  NO_STATES;
    
	SendGetStatusCommand();
	ReceiveGetStatusResponse(); 
	
	while (ControllerState != DONE)
	{
		  SendGetStatusCommand();
		  ReceiveGetStatusResponse();
#ifndef TEST	  
		  printf("ControllerState = ");
		  dec(ControllerState);
		  printf("  ControllerErrorCode = ");
		  dec(ControllerErrorCode);
		  printf("  ControllerDiagnosticCode = ");
		  dec(ControllerDiagnosticCode);
		  printf("\r\n");
		  
#endif	
	  
		  CheckControllerStateStatus = ControllerState;
		  if (ControllerState == ERROR)
		  {	  
		  		printf("Error # ");
		  		dec(ControllerErrorCode);
		  		printf(" was encountered by the DC Motor Controller!!\r\n");
		  		
		  		break;
		  }
	}
			

#ifndef TEST	  
	if (ControllerState == DONE)
	{
		printf("DC Motor Duty Command completed successfully!\r\n");
	}
#endif 
	return CheckControllerStateStatus;
	
}

//**********************************************************************************
//**********************************************************************************
//**********************************************************************************
//**********************************************************************************

// Send command messages to the SPI Peripheral here...
	 		
//**********************************************************************************
//* SendCalibrateMotorCommand - Send the DC Motor Controller Calibrate Command  
//* message to the Slave SPI Controller.  Return a status byte indicating the 
//* outcome of the calibration process.
//**********************************************************************************
byte SendCalibrateMotorCommand(byte TheMotorID)  
{
 	byte CalibrationStatus = TRUE;								// Status return
 	
 	// Send a message to the DC Motor Controller to start the DC Motor Calibration 
 	// process for the selected motor  ...

 	// Clear the SPI send and receive buffers
	//FlushSPIBuffers();
		
	// Build the DC Motor Controller Calibrate Command
 	
	MotorMessage.CalibrateCommand.MessageID = MessageIndex++;	// Message ID range (0000 - FFFF) Hex
	MotorMessage.CalibrateCommand.Command = CALIBRATE_MOTOR_COMMAND;
	MotorMessage.CalibrateCommand.MotorID = TheMotorID;
	
#ifndef TEST		

    printf("Master: Send Calibrate Motor Command...\r\n");
  	printf("MessageID = ");
   	dec(MotorMessage.CalibrateCommand.MessageID);
   	printf("  Command = CALIBRATE_MOTOR_COMMAND");
   	printf(",  Motor ID = ");
   	dec(MotorMessage.CalibrateCommand.MotorID);
   	printf("\r\n\r\n");
#endif 

	// Send the command message to the SPI Slave Controller
	SendSPIMessage();
	
	// Receive and process the response message from the SPI Slave Controller
	ReceiveSPIMessage();	
 
	return CalibrationStatus;
} 

//**********************************************************************************
//* SendDutyCommand - Send the Duty Command message used to set the PWM Duty to the DC 
//* Motor Controller. Return a status byte indicating if the Duty Command was succesfully
//* applied to the selected motor.
//**********************************************************************************
byte SendDutyCommand(longword *TheDuty, byte NumberOfMotors)
{
 	byte DutyCommandStatus = TRUE;								// Status return
  	byte i;														// Local loop index
 	
 		
	// Build the Duty Command message 
			
	MotorMessage.DutyCommand.MessageID = MessageIndex++;		// Message ID range (0000 - FFFF) Hex 
  	MotorMessage.DutyCommand.Command = DUTY_COMMAND;  			// Duty Command to SPI Slave (see enumeration
               			                 						// declaration above.)                                 	 
 	MotorMessage.DutyCommand.NumberOfMotors = NumberOfMotors; 	// Total number of motors range (0..MAX_MOTORS) 
	MotorMessage.DutyCommand.StatusFlags = 0 ; 	//  Duty status flags = 0 (OK)

	for (i=0; i<NumberOfMotors; i++)
	{
  		MotorMessage.DutyCommand.Duty[i] = TheDuty[i]; 			// Duty Command range (0000000 - FFFFFFFF) Hex              	
	}		
 
#ifndef TEST
    printf("Master: Sending Duty Command: ");
  	printf("  Message ID = %d \r\n", MotorMessage.DutyCommand.MessageID);
 	printf("  NumberOfMotors = %d \r\n", MotorMessage.DutyCommand.NumberOfMotors);
	for (i=0; i<MotorMessage.DutyCommand.NumberOfMotors; i++)
	{
    	printf(", Motor ID = %d, Duty = %ld \r\n",i ,MotorMessage.DutyCommand.Duty[i]);  
    }
#endif

	// Send the command message to the SPI Slave Controller
	SendSPIMessage();
	
	// Receive and process the response message from the SPI Slave Controller
	ReceiveSPIMessage();
	
  	
	return DutyCommandStatus;
}
			
//**********************************************************************************
//* SendGetPotCommand - Send the Get Pot command message that will be used for
//* setting the duty command to the DC Motor Controller. Return a status byte 
//* indicating if the Get Pot Command was succesfully applied to the selected potentiometer.
//**********************************************************************************
byte SendGetPotCommand(byte TheMotorID)
{
 	byte GetPotCommandStatus = TRUE;										// Status return
  				
  	// Build the Get Pot command message
 	
	MotorMessage.GetPotCommand.MessageID = MessageIndex++; 		// Message ID (0000 - FFFF) Hex
	MotorMessage.GetPotCommand.Command = GET_POT_COMMAND;		// Get Pot Command to SPI Slave (see enumeration
        		                        						// declaration above.)  
    MotorMessage.GetPotCommand.MotorID =  TheMotorID;			// Selected motor range (0..1)   		                        						

#ifndef TEST		

    printf("Master: Send Get Pot Command...\r\n");
  	printf("MessageID = ");
    dec(MotorMessage.GetPotCommand.MessageID);
    printf("  Command = GET_POT_COMMAND");
    printf(",  Motor ID = ");
    dec(MotorMessage.GetPotCommand.MotorID);
    printf("\r\n\r\n");
#endif 
	
	// Send the command message to the SPI Slave Controller
	SendSPIMessage();
	
	// Receive and process the response message from the SPI Slave Controller
	ReceiveSPIMessage();
	
   
	return GetPotCommandStatus;
}

//**********************************************************************************
//* SendHoldPotCommand - Send the Hold Pot Command message to the DC Motor Controller.  Save the
//* current Pot value that will be used for setting the duty command.  Return a status byte 
//* indicating if the Hold Pot Command was succesfully applied to the selected 
//* potentiometer.
//**********************************************************************************
byte SendHoldPotCommand(byte TheMotorID)
{
 	byte HoldPotCommandStatus = TRUE;							// Status return
 
  
 	// Build Hold Pot Command message
 		
	MotorMessage.HoldPotCommand.MessageID = MessageIndex++; 	// Message ID (0000 - FFFF) Hex

  	MotorMessage.HoldPotCommand.Command = HOLD_POT_COMMAND;     // Hold current Pot command  
  	     			                           					// (see enumeration declaration above.)  
 		
 	MotorMessage.HoldPotCommand.MotorID = TheMotorID; 			// Selected motor range (0..1)  

#ifndef TEST		

    printf("Master: Send Hold Pot Command...\r\n");
  	printf("MessageID = ");
    dec(MotorMessage.HoldPotCommand.MessageID );
    printf("  Command = HOLD_POT_COMMAND");
    printf(",  Motor ID = ");
    dec(MotorMessage.HoldPotCommand.MotorID);
    printf("\r\n\r\n");
#endif	
		
	// Send the command message to the SPI Slave Controller
	SendSPIMessage();
	
	// Receive and process the response message from the SPI Slave Controller
	ReceiveSPIMessage();
	
 
	return HoldPotCommandStatus;
}
  					
//**********************************************************************************
//* SendNumberSamplesCommand - Send the Get number of samples command message to the 
//* DC Motor Controller.  Return a status byte indicating that the Number of Samples 
//* was accepted.
//**********************************************************************************
byte SendNumberSamplesCommand(byte TheMotorID, word TheNumberOfSamples)
{
 	byte NumberSamplesCommandStatus = TRUE;								// Status return
 				
  	// Build Get number of samples command message
 
 	MotorMessage.SampleCommand.MessageID = MessageIndex++;   			// Message ID (0000 - FFFF) Hex

  	MotorMessage.SampleCommand.Command = NUMBER_SAMPLES_COMMAND;		// Get number of samples  	                                							 			// Command (see enumeration declaration above.)  
  
 	//MotorMessage.SampleCommand.MotorID = TheMotorID; 					// Selected motor range (0..1)  
 	MotorMessage.SampleCommand.NumberOfSamples = TheNumberOfSamples;
					 								                   	// Number of samples
#ifndef TEST		
					
    printf("Master: Send Number of Samples Command...\r\n");
  	printf("MessageID = ");
  	dec(MotorMessage.SampleCommand.MessageID);
    //printf(",  Motor ID = ");
    //dec(MotorMessage.SampleCommand.MotorID);
    printf(", Number of Samples = ");
   	dec(MotorMessage.SampleCommand.NumberOfSamples);  
    printf("\r\n\r\n");
#endif	
													 
	// Send the command message to the SPI Slave Controller
	SendSPIMessage();
	
	// Receive and process the response message from the SPI Slave Controller
	ReceiveSPIMessage();
	
 
	return NumberSamplesCommandStatus;
}
 
//**********************************************************************************
//* SendPeriodCommand - Process the Period Command message used to set the PWM 
//* period. Return a status byte indicating if the Period Command was succesfully
//* applied to the selected motor.
//**********************************************************************************
byte SendPeriodCommand(longword ThePeriod  )
{
 	byte PeriodCommandStatus = TRUE;							// Status return
    int i;

	// Build the Send period command message
 
  	MotorMessage.PeriodCommand.MessageID = MessageIndex++;   	// Message ID range (0000 - FFFF) Hex 
	MotorMessage.PeriodCommand.Command = PERIOD_COMMAND;   		// Send period command to selected motor (see enumeration
                                								// declaration above.)  
 
 
  	MotorMessage.PeriodCommand.Period = ThePeriod; 				// Period range xx (00000000..FFFFFFFF) Hex for all three PWM Channels                  	
			


#ifndef TEST		

    printf("Master: Send Period Command...\r\n");
  	printf("MessageID = ");
    dec(MotorMessage.PeriodCommand.MessageID);
    printf("Sent: Motor ID = %d, Period = %ld \r\n",i ,MotorMessage.PeriodCommand.Period);  
    printf("\r\n\r\n");
#endif

	// Send the command message to the SPI Slave Controller
	SendSPIMessage();
	
	// Receive and process the response message from the SPI Slave Controller
	ReceiveSPIMessage();	
 
	return PeriodCommandStatus;
}

//**********************************************************************************
//* SendResetCommand - Send the Reset the SPI Slave Controller.  This command 
//* resets the selected controller.  It may be used to implement an emergency stop 
//* function using the PIC's watchdog timer. 
//**********************************************************************************
byte SendResetCommand(void)
{ 

	byte ResetCommandStatus;
 	
 
	// Build Reset Command Message
	
	MotorMessage.ResetCommand.MessageID = MessageIndex++;	// Message ID range (0000 - FFFF) Hex
	MotorMessage.ResetCommand.Command = RESET_COMMAND;		// Send Reset Command
   
#ifndef TEST
    printf("Master: Send Reset Command...");
  	printf("MessageID = ");
   	dec(MotorMessage.ResetCommand.MessageID);
  
   	printf("\r\n\r\n");
#endif 


	// Send the command message to the SPI Slave Controller
	SendSPIMessage();
	
	// Receive and process the response message from the SPI Slave Controller
	ReceiveSPIMessage();

 	
	return ResetCommandStatus;
	
}

//**********************************************************************************
//* FlushSPIBuffers - Clear the SPI RX and TX Buffers.                             *
//**********************************************************************************
byte FlushSPIBuffers(void)
{
	int i;

	// Clear the contents of SPI Buffers 	
			 
	for (i=0; i<SPI_BUFFER_SIZE; i++)
	{
 
		SPI_Tx_Buffer[i] = 0;
 		SPI_Rx_Buffer[i] = 0;

	}
	return 0;

}

//**********************************************************************************
//* SendGetStatusCommand - Request to get the latest status from the selected Slave*
//* controller.  The message includes the controller state, any errors detected and*
//* any diagnostic messages reported.                                              *
//**********************************************************************************
byte SendGetStatusCommand(void)
{ 

	byte GetStatusCommandStatus;
 	
 	// Build Reset Command Message
	
	MotorMessage.GetStatusCommand.MessageID = MessageIndex++;	// Message ID range (0000 - FFFF) Hex
	MotorMessage.GetStatusCommand.Command = GET_STATUS_COMMAND;		// Send Get Status Command
   
#ifndef TEST
    printf("Master: Send Get Status Command...");
  	printf("MessageID = ");
   	dec(MotorMessage.GetStatusCommand.MessageID);
  
   	printf("\r\n\r\n");
#endif 

	// Send the command message to the SPI Slave Controller
	SendSPIMessage();
	
	// Receive and process the response message from the SPI Slave Controller
	ReceiveSPIMessage();
	 	

	return GetStatusCommandStatus;
	
}
		
//**********************************************************************************
//**********************************************************************************
//**********************************************************************************
//**********************************************************************************
// Functions used to process responses from the slave DC Motor Controller


// Receive Responses from Slave here...
	
//**********************************************************************************
//* ReceiveCalibrateMotorResponse - Receive the DC Motor Controller Calibrate Command  
//* Response message.  Return a status byte indicating the outcome of the 
//* calibration process.
//**********************************************************************************
byte ReceiveCalibrateMotorResponse(void)  
{
 	byte CalibrationStatus = TRUE;										// Status return
 	
  
#ifndef TEST
    printf("Master: Received Calibration Command Response: ");
  	printf("MessageID = ");
   	dec(MotorMessage.CalibrateCommand.MessageID);
    printf(", Motor ID = ");
   	dec(MotorMessage.CalibrateCommand.MotorID);
   	printf("\r\n\r\n");
#endif
				
	return CalibrationStatus;
}

//**********************************************************************************
//* ReceiveDutyResponse - Receive the Duty Command Response message used to set the PWM Duty.  
//* Return a status byte indicating if the Duty Command was succesfully applied to the selected motor.
//**********************************************************************************
byte ReceiveDutyResponse(void)
{
 	byte DutyCommandStatus = TRUE;								// Status return
 	byte i;														// Local loop index
			
   
#ifndef TEST
/*
    printf("Master: Received Duty Command Response: ");
    printf("  Message ID = %d \r\n", MotorMessage.DutyCommand.MessageID);
	printf("  StatusFlags = %04X \r\n", MotorMessage.DutyCommand.StatusFlags) ; 	//  Duty status flags = 0 (OK)
  	printf("  NumberOfMotors = %d \r\n", MotorMessage.DutyCommand.NumberOfMotors);
	printf("  NumberOfMotors = %d \r\n", MotorMessage.DutyCommand.NumberOfMotors);
 
*/
	for (i=0; i<MotorMessage.DutyCommand.NumberOfMotors; i++)
	{
    	printf("Received: Motor ID = %d, Duty = %0lX Encoder Count = %ld \r\n",i ,MotorMessage.DutyCommand.Duty[i], 
              MotorMessage.DutyCommand.EncoderCounts[i]);  
    }

#endif

	return DutyCommandStatus;
}
			
//**********************************************************************************
//* ReceiveGetPotResponse - Process the Get Pot command response message that will be used for
//* setting the duty command. Return a status byte indicating if the Get Pot Command
//* was succesfully applied to the selected potentiometer.
//**********************************************************************************
byte ReceiveGetPotResponse(void)
{
 	byte GetPotCommandStatus = TRUE;										// Status return
 	
 
#ifndef TEST
    printf("Master: Received Get Pot Command Response: ");
  	printf("MessageID = ");
    dec(MotorMessage.GetPotCommand.MessageID);
    printf(", Motor ID = ");
    dec(MotorMessage.GetPotCommand.MotorID);
    printf("\r\n\r\n");
#endif 

	return GetPotCommandStatus;
}

//**********************************************************************************
//* ReceiveHoldPotResponse - Receive the Hold Pot Command Response message.  Save the
//* current Pot value that will be used for setting the duty command.  Return a status byte 
//* indicating if the Hold Pot Command was succesfully applied to the selected 
//* potentiometer.
//**********************************************************************************
byte ReceiveHoldPotResponse(void)
{
 	byte HoldPotCommandStatus = TRUE;										// Status return
			
  
 	 
#ifndef TEST
    printf("Master: Received Hold Pot Command Response: ");
  	printf("MessageID = ");
    dec(MotorMessage.HoldPotCommand.MessageID );
    printf(", Motor ID = ");
    dec(MotorMessage.HoldPotCommand.MotorID);
    printf("\r\n\r\n");
#endif	


	return HoldPotCommandStatus;
}
  					
//**********************************************************************************
//* ReceiveNumberSamplesResponse - Receive Get number of samples command response message. 
//* Return a status byte indicating that the Number of Samples was accepted.
//**********************************************************************************
byte ReceiveNumberSamplesResponse(void)
{
 	byte NumberSamplesCommandStatus = TRUE;									// Status return
 	static word LocalNumberSamples = 0;
 
 	// Copy Number of Samples to local variable
	LocalNumberSamples = MotorMessage.SampleCommand.NumberOfSamples;
    printf("Local Number of Samples = ");
   	dec(MotorMessage.SampleCommand.NumberOfSamples);  
    printf("\r\n");

#ifndef TEST
    printf("Master: Received Number of Samples Command Response: ");
  	printf("MessageID = ");
  	dec(MotorMessage.SampleCommand.MessageID);
    //printf(",  Motor ID = ");
    //dec(MotorMessage.SampleCommand.MotorID);
    printf(", Number of Samples = ");
   	dec(MotorMessage.SampleCommand.NumberOfSamples);  
    printf("\r\n\r\n");
#endif	

	return NumberSamplesCommandStatus;
}

//**********************************************************************************
//* ReceivePeriodResponse - Receive the Period Command Response message used to set the PWM 
//* period. Return a status byte indicating if the Period Command was succesfully
//* applied to the selected motor.
//**********************************************************************************
byte ReceivePeriodResponse(void)
{
 	byte PeriodCommandStatus = TRUE;									// Status return
  	static byte LocalPeriod = 0;
  
  	LocalPeriod  = MotorMessage.PeriodCommand.Period;
  	printf("Local Period = ");
    dec(LocalPeriod);  
    printf("\r\n"); 
    
    // Send Acknowledgement (ACK) back to the Host PC or Stamp, indicating that the 
    // Period command was received...
	 
#ifndef TEST
    printf("Master: Received Period Command Response: ");
  	printf("MessageID = ");
    dec(MotorMessage.PeriodCommand.MessageID);
    printf("Received: Period = %0lX \r\n", MotorMessage.PeriodCommand.Period);   
    printf("\r\n\r\n");
#endif

	return PeriodCommandStatus;
}

//**********************************************************************************
//* ReceiveResetResponse - Process the Reset the SPI Slave Controller.  This command 
//* resets the selected controller.  It may be used to implement an emergency stop 
//* function using the PIC's watchdog timer. 
//**********************************************************************************
byte ReceiveResetResponse(void)
{
	byte ResetCommandStatus;
	
#ifndef TEST
    printf("Master: Received Reset Command Response:  ");
  	printf("MessageID = %d \r\n", MotorMessage.ResetCommand.MessageID); 
   	printf("\r\n\r\n");
#endif

	// Reset the Controller by allowing the Watchdog Timer to timeout.
	
	return ResetCommandStatus;

}

//**********************************************************************************
//* ReceiveGetStatusResponse - Get the latest status from the selected Slave       *
//* controller.  The message includes the controller state, any errors detected and*
//* any diagnostic messages reported.   The current state is returned from the     *
//* function call, while the error and diagnostics codes are saved globally.       *
//**********************************************************************************
byte ReceiveGetStatusResponse(void)
{ 	
	byte GetStatusCommantStatus;
 	
	//MotorMessage.GetStatusCommand.MessageID = MessageIndex++;			     			// Message ID range (0000 - FFFF) Hex
	//MotorMessage.GetStatusCommand.Command = GET_STATUS_COMMAND;		     			// Send Get Status Command
    ControllerState = MotorMessage.GetStatusCommand.ControllerState; 		 			// Get current Controller State
    ControllerErrorCode = MotorMessage.GetStatusCommand.ControllerErrorCode; 			// Get current Controller Error Code
    ControllerDiagnosticCode = MotorMessage.GetStatusCommand.ControllerDiagnosticCode;  // Get current Diagnostic Code
    
#ifndef TEST
    printf("Master: Received Get Status Command Response:  ");
  	printf("MessageID = %d \r\n", MotorMessage.GetStatusCommand.MessageID);
  	printf("  ControllerState = %0X, ControllerErrorCode =%0X, ControllerDiagnosticCode = %0X \r\n", 
           MotorMessage.GetStatusCommand.ControllerState, MotorMessage.GetStatusCommand.ControllerErrorCode, MotorMessage.GetStatusCommand.ControllerDiagnosticCode);
  
#endif 
 
	return GetStatusCommantStatus;
	
}
		 	
// ************************ High Level DC Motor Controller  Functions **********************************

#ifdef NEW_CODE

//****************************************************************************************
//* Neutral - Wait in Neutral for awhile while keeping pwm going to maintain 2.5V
//****************************************************************************************

void Neutral(void)
{
	int x;														// Local loop variable

#ifdef TEST
   printf("Neutral ...\r\n");
#endif
	// Send the new duty cycle to the selected motor

	SendDutyCommand(LEFT_MOTOR, NEUTRAL);
	SendDutyCommand(RIGHT_MOTOR, NEUTRAL);

}

//****************************************************************************************
//* Neutral_To_Forward - Bring motor speed gradually to full forward, using specified
//* acceleration constant.
//****************************************************************************************
void Neutral_To_Forward(void)
{
	int x;														// Local loop variable

#ifdef TEST 
   printf("Neutral_To_Forward (Forward) ...\r\n");
#endif

	//for(x=NEUTRAL; x<= FULL_FORWARD; x++)
	for(x=NEUTRAL; x> FULL_FORWARD; x--)
	{

#ifdef TEST 
      	printf("x = ");
		dec(x);
      	printf("\r\n");
#endif
 
		// Send the new duty cycle to the selected motor

		SendDutyCommand(LEFT_MOTOR, x);
		SendDutyCommand(RIGHT_MOTOR, x);

		// Delay for specified acceleration

		pause(PAUSE_TIME);
	}
}

//****************************************************************************************
//* Forward_To_Neutral - Bring motor speed gradually to a stop or neutral from full forward
//****************************************************************************************
void Forward_To_Neutral(void)
{
	int x;														// Local loop variable

#ifdef TEST 
   printf("Forward_To_Neutral (Stop) ...\r\n");
#endif

	for(x=FULL_FORWARD; x<=NEUTRAL; x++)
	{

#ifdef TEST 
     	printf("x = ");
		dec(x);
     	printf("\r\n");
#endif

		// Send the new duty cycle to the selected motor

		SendDutyCommand(LEFT_MOTOR, x);
		SendDutyCommand(RIGHT_MOTOR, x);

		// Delay for specified acceleration

		pause(PAUSE_TIME);
	}  
}

//****************************************************************************************
//* Bring motor speed gradually to full reverse.
//****************************************************************************************
void Neutral_To_Reverse(void)
{
	int x;														// Local loop variable
 
#ifdef TEST 
   printf("Neutral_To_Reverse (Reverse) ...\r\n");
#endif

	//for(x=NEUTRAL; x>FULL_REVERSE; x--)
	for(x=NEUTRAL; x<=FULL_REVERSE; x++)
	{

#ifdef TEST 
      	printf("x = ");
		dec(x);
      	printf("\r\n");
#endif

		// Send the new duty cycle to the selected motor

		SendDutyCommand(LEFT_MOTOR, x);
		SendDutyCommand(RIGHT_MOTOR, x);

		// Delay for specified acceleration

		pause(PAUSE_TIME);
	}
}

//****************************************************************************************
//* Reverse_To_Neutral - Bring motor speed  gradually to a stop or neutral from full reverse.
//****************************************************************************************
void Reverse_To_Neutral(void)
{
	int x;														// Local loop variable
  
#ifdef TEST 
   printf("Reverse_To_Neutral (Stop) ...\r\n");
#endif

	//for(x=FULL_REVERSE; x<=NEUTRAL; x++)
	for(x=FULL_REVERSE; x>NEUTRAL; x--)
	{

#ifdef TEST 
      	printf("x = ");
		dec(x);
      	printf("\r\n");
#endif


		// Send the new duty cycle to the selected motor

		SendDutyCommand(LEFT_MOTOR, x);
		SendDutyCommand(RIGHT_MOTOR, x);

		// Delay for specified acceleration

		pause(PAUSE_TIME);
	}
}

//****************************************************************************************
//* Left - Turn left by using right motor only.
//****************************************************************************************
void  Left(void)
{
	int x;														// Local loop variable
  
#ifdef TEST 
   printf("Turn Left ...\r\n");
#endif

	// Send the new duty cycle to the selected motor

	SendDutyCommand(LEFT_MOTOR, NEUTRAL);
	SendDutyCommand(RIGHT_MOTOR, MEDIUM_FORWARD);

	// Delay for specified acceleration

	pause(1000);

}
 
//****************************************************************************************
//* HardLeft - Turn hard left by using left and right motors.
//****************************************************************************************
void  HardLeft(void)
{
	int x;														// Local loop variable
  
#ifdef TEST 
    printf("Turn Hard Left ...\r\n");
#endif

	// Send the new duty cycle to the selected motor

	SendDutyCommand(LEFT_MOTOR, MEDIUM_FORWARD);
	SendDutyCommand(RIGHT_MOTOR, MEDIUM_REVERSE);

	// Delay for specified acceleration

	pause(1000);

}
//****************************************************************************************
//* Right - Turn right by using left motor only.
//****************************************************************************************
void  Right(void)
{
	int x;														// Local loop variable
  
#ifdef TEST 
   printf("Turn Right ...\r\n");
#endif

	// Send the new duty cycle to the selected motor

	SendDutyCommand(LEFT_MOTOR, MEDIUM_FORWARD);
	SendDutyCommand(RIGHT_MOTOR, NEUTRAL);

	// Delay for specified acceleration

	pause(1000);

}
 
//****************************************************************************************
//* HardRight - Turn hard right by using left and right motors.
//****************************************************************************************
void  HardRight(void)
{
	int x;														// Local loop variable
  
#ifdef TEST 
   printf("Turn Hard Right ...\r\n");
#endif

	// Send the new duty cycle to the selected motor

	SendDutyCommand(LEFT_MOTOR, MEDIUM_REVERSE);
	SendDutyCommand(RIGHT_MOTOR, MEDIUM_FORWARD);

	// Delay for specified acceleration

	pause(1000);

}
#endif
