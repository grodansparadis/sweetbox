 //********************************************************************************
//*                                                                              *
//* Description:                                                                 *
//*                                                                              *
//*     Name:  network.h                                                         *
//*   Author:  Daniel F. Ramirez                                                 *
//*     Date:  01/30/2003                                                        *
//*                                                                              *
//*     This application implements an SPI master and slave command, message IDs *
//* used. by spimast.c, spislave.c, dcmotor.c, sensor.c, animate.c, sonar.c and  * 
//* other networked applications. This file is included by each of these         *
//* applications so that they may communicate over the SPI bus using common      *
//* Master and Slave message formats                                             *         
//*                                                                              *                                                         
//*      A communication interface is provided that allows the host SPI or       *
//* I2C Master to request individual sensor readings, average sensor readings,   *
//* calibrating a sensor or perform a sensor processing algorithm using command  *
//* message packets.  The commands available are described below.  This header   *
//* file provides the common Master and Slave message formats and descriptors.   *                                            
//*                                                                              *
//* in order to  give any PC the capability to serve as an SPI controller.       * 
//* It can also be used for robotic applications where multiple controllers and  *
//* SPI devices are used, with a minimum of data and control lines needed to     *
//* connect them.  Each SPI device has its own unique address and only listens   *
//* to commands associated with it.                                              *
//*                                                                              *
//*     Future upgrades to this application include: Implementing large          *
//* CRC and Checksum capabilities along with large message definitions for       *
//* various applications.                                                        *
//*                                                                              *
//********************************************************************************
 
#ifndef NETWORK_LOADED
#define NETWORK LOADED								/* prevent multiple loading */

#include "flags.h"                              	// Global flags used for MPSIM debugging 
#include "my_types.h"
//#include "stdio.h"                              	// I/O support functions  
 
//#define SPI_BYTE_INTERFACE   						// SPI Byte I/O
//#define SPI_NETWORK_INTERFACE  					// SPI Message Passing I/O (using SPI interrupt)

#define SPI_BUFFER_INTERFACE  						// SPI Buffered I/O (using SPI interrupt)
//#define PARALLEL_PORT_INTERFACE					// PSP Parallel Slave Port Interface
//#define USART_BUFFER_INTERFACE  					// Serial Buffered I/O (using USART interrupt)


// Common Constants

#define INPUT 1										// Pin input 
#define OUTPUT 0									// Pin output

#define ENABLE 0									// Enable the Slave device
#define DISABLE 1									// Disable the Slave device
#define TRUE  1                                 	// Boolean TRUE value
#define FALSE 0                                 	// Boolean FALSE value

//#define ON  1                                   	// read operation
//#define OFF 0                                  		// SPI write operation

//#define READ  1                                 	// SPI read operation
//#define WRITE 0                                 	// SPI write operation

// Message synchronization 
#define SYNC_BYTE	255								// USART Sync Byte
#define ACK_BYTE	254								// USART Acknowledge Byte

#define PSP_BUFFER_SIZE 34							// Size of PSP Tx and Rx Buffers
//#define USART_BUFFER_SIZE 32						// Size of SPI Tx and Rx Buffers
#define SPI_BUFFER_SIZE 34							// Size of SPI Tx and Rx Buffers
#define DATA_BUFFER_SIZE 32							// Size of SPI Tx and Rx Message
#define TX_DATA_OFFSET 1							// Transmit Buffer Offset
#define RX_DATA_OFFSET 2							// Receive Buffer Offset
 
 // Offsets for Slave SPI Receive Buffer

#define SYNC_INDEX 0								// Sync byte index
#define COMMAND_INDEX 3								// Command code index
#define STATE_INDEX 4								// Controller State index
#define ERROR_CODE_INDEX 5							// Controller Error code index
#define DIAGNOSTIC_CODE_INDEX 6					    // Diagnostic code index


//******************************************************************************
// Define error, diagnostic and status enumerations here.
//******************************************************************************
 
//enum CONTROLLER_STATE_TYPE { NO_STATES=0, INITIALIZED=1, DONE=2, BUSY=3, ERROR=4}; 	// Status of selected slave controller, range (0..255) 
													// 		0 => NO_STATES	    No State    
													//		1 => INITIALIZED 	The selected slave contoller is Initialzed 
													//		2 => DONE 	 		The selected slave contoller is done processing current command
													// 		3 => BUSY 	 		The selected slave contoller is busy processing current command 
													// 		4 => ERROR 	 		The selected slave contoller encountered an error processing the current command 

//enum CONTROLLER_ERROR_TYPE { NO_ERRORS=0, FAILURE=1, SUCCESS=2 }; 	// Error code returned by the selected slave controller, range (0..255) 
													// 		0 => NO_ERRORS 		No error codes  
													// 		1 => FAILURE 	 	Failed to complete command
													//  	2 => SUCCESS  	 	Completed command successfully 
//enum DIAGNOSTIC_MESSAGE_TYPE { NO_DIAGNOSTICS=0, DIAGNOSTIC1=1, DIAGNOSTIC2=2 };
													// 		0 => NO_DIAGNOSTICS No Diagnostic messages
													// 		1 => DIAGNOSTIC 1 	Diagnostic message 1
													//  	2 => DIAGNOSTIC 2  	Diagnostic message 2 

//******************************************************************************
//* Define message IDs for each type of command that is sent by the SPI Master 
//* applications and recognized by the selected SPI Slave applications. Up to 255 
//* different commands can be defined.
//******************************************************************************
typedef enum command_type
{
	//******************************************************************************
	// DC Motor Controller Commands Message
	//******************************************************************************

	CALIBRATE_MOTOR_COMMAND = 0,					// Send calibrate command to the selected, motor
	DUTY_COMMAND = 1, 								// Send duty command to the selected motor
	GET_POT_COMMAND =2, 							// Send get pot command to the selected motor
	HOLD_POT_COMMAND =3,							// Send hold pot command to the selected motor


	//******************************************************************************
	// Sensor Controller Commands
	//******************************************************************************

	CALIBRATE_SENSOR_COMMAND =4, 					// Calibrate the selected sensor 
	GET_SENSOR_CALIBRATION_COMMAND =5,				// Get sensor calibration data from the Sensor Controller
	SET_SENSOR_WINDOW_COMMAND =6,					// Set the sensor window used to scale data for the Sensor
	GET_RAW_SENSOR_DATA_COMMAND =7, 				// Get raw sensor data from the Sensor Controller
	GET_SCALED_SENSOR_DATA_COMMAND =8,				// Get scaled sensor data from the Sensor Controller
	SELECT_ALGORITHM_COMMAND=9,						// Select the algorithm to be used for processing the sensors

	//******************************************************************************
	// Animation Controller Commands
	//******************************************************************************

	CALIBRATE_JOYSTICK_COMMAND =10, 				// Calibrate the selected joystick
	GET_JOYSTICK_CALIBRATION_COMMAND =11,			// Get joystick calibration data from the Animation Controller
	SET_JOYSTICK_WINDOW_COMMAND =12,				// Set the joystick window used to scale data for the Animation Controller
	GET_RAW_JOYSTICK_DATA_COMMAND =13, 				// Get raw joystick data from the Animation Controller
	GET_SCALED_JOYSTICK_DATA_COMMAND =14,			// Get scaled joystick data from the Animation Controller

	//******************************************************************************
	// Math Co-processor Commands
	//******************************************************************************

	CALCULATE_COMMAND =15,							// Calculate command evaluates RPN expression
	GET_ANSWERS_COMMAND =16,						// Gets the floating point answers
	CLEAR_MEMORY_COMMAND =17,						// Clears the floating point scratch registers
	STORE_COMMAND =18,								// Stores a floating point value to a selected float register
	RECALL_COMMAND =19,								// Retrieves a floating point value from a selected float register	
	FILTER_COMMAND =20, 							// Performs selected filter operation (FIR, IFR)
	FFT_COMMAND =21, 								// Performs an FFT operation
	IFFT_COMMAND =22, 								// Performs an inverse FFT operation
	KALMAN_FILTER_COMMAND =23, 						// Performs a Kalman filter operation
	MEDIAN_FILTER_COMMAND =24, 						// Performs a Median filter operation
	HISTOGRAM_COMMAND =25, 							// Performs a Histogram operation
	SORT_COMMAND =26, 								// Performs a Sort operation
	
	//******************************************************************************
	// Commonly used (Overloaded) Commands
	//******************************************************************************

	READ_JOYSTICK_COMMAND=241,                      // Read the latest joystick positions and button states
	RECORD_COMMAND =242,                            // Record sensor data or animation script
	PLAY_COMMAND =243,								// Play back sensor data or animation script
	NUMBER_SAMPLES_COMMAND =244,					// Send number of samples to the selected controller
	PERIOD_COMMAND =245,							// Send period to the selected controller
 	DELAY_COMMAND =246, 							// Send delay to the selected controller 
 	READ_PORT_COMMAND =247, 						// Read from a specified port address on the slave 
 	WRITE_PORT_COMMAND =248, 						// Write to a specified port address on the slave 
 	GET_INTERRUPT_COUNTS_COMMAND =249, 				// Get latest interrupt counts from the slave
 	SELECT_FILTER_COMMAND =250, 					// Specify a filter
 	TEST_COMMAND =251,								// Test a specific function
  	SEND_TELEMETRY_COMMAND =252, 					// Send Telemetry using specified formats
	GET_STATUS_COMMAND =253,						// Get the status of the selected slave controller
  	RESET_COMMAND =254,								// Resets the selected slave controller

	END_OF_MESSAGES =255 							// End of command messages



} COMMAND_TYPE;

/******************************************************************************
 * Define Message Type used to hold the unformmated command messages 
 ******************************************************************************/
typedef struct message_type
{
  	byte Data[DATA_BUFFER_SIZE];    					// Byte buffer used for to hold a message
} MESSAGE_TYPE;

/******************************************************************************
 * Define Generic Command Message Type to be used to hold the selected 
 * command for processing by the SPI SLave Controller. 
 ******************************************************************************/

typedef struct command_message_type
{
   	word MessageID;    				// Message ID range (0000 .. FFFF) Hex

  	COMMAND_TYPE Command;         	// Generic command to SPI Slave  
    byte Filler[29];				// Padding 

} COMMAND_MESSAGE_TYPE;

//**********************************************************************************
// Define Global variables common to all controllers here
//**********************************************************************************

extern byte ControllerState; 		// State of selected slave controller, range (0..255) 
											// 		0 => NO_STATES		No States    
											//		1 => INITIALIZED	The selected slave contoller is Initialzed 
											//		2 => DONE 	 	 	The selected slave contoller is done processing current command
											// 		3 => BUSY 	 		The selected slave contoller is busy processing current command 
											// 		4 => ERROR 	 		The selected slave contoller encountered an error processing the current command 

extern byte ControllerErrorCode; 	// Error code returned by the selected slave controller, range (0..255) 
											// 		0 => NO_ERRORS 		No error codes  
											// 		1 => FAILURE 	 	Failed to complete command
											//  	2 => SUCCESS  	 	Completed command successfully 
 
extern byte ControllerDiagnosticCode; 	// Diagnostic message code returned by the selected slave controller, range (0..255) 
											// 		0 => NO_DIAGNOSTICS 	No Diagnostic messages
											//  	1 => DIAGNOSTIC 1  	Diagnostic message 2 
											// 		2 => DIAGNOSTIC 2 	Diagnostic message 1
 
//**********************************************************************************
// Define message commands common to all controllers.  Thes messages are sent by 
// the SPI Master and  recognized by the selected SPI Slave.
//**********************************************************************************

//**********************************************************************************
//* GetStatus - Get the latest status from the selected Slave controller.  The     *
//* message includes the controller state, any errors detected and any diagnostic  * 
//* messages reported.                                                             *
//**********************************************************************************
typedef struct get_status_command_type
{
  	word MessageID;    				// Message ID range (0000 .. FFFF) Hex 

  	COMMAND_TYPE Command;         	// Send the get status command to selected slave controller
  	byte ControllerState; 			// State of selected slave controller, range (0..255) 
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

    byte Filler[26];				// Padding 

} GET_STATUS_COMMAND_TYPE;

//**********************************************************************************
//*                                                                                *
//*     Nxxxx     - Accept total number of samples message.  Allows up to XXXX     *
//*                 samples to be read from selected Potentiometer, where xxxx is  *
//*                 in 4 digit hex byte format  range (00..FFFF).                  *
//**********************************************************************************
typedef struct sample_command_type
{
  	word MessageID;    				// Message ID range (0000 .. FFFF) Hex

  	COMMAND_TYPE Command;         	// Get new motor commands from the current POT readings
  	                                // Command (see enumeration declaration above.)  
 
 	word NumberOfSamples;			// Number of samples 
    byte Filler[27];				// Padding 
    
} SAMPLE_COMMAND_TYPE;

//**********************************************************************************
//*                                                                                *
//* 	Pmm,xx    - Send PWM period command range xx (00..FF) to motor             *
//*                 mm range (00..01). This command is used to vary the            *
//*                 PWM update frequency.                                          *
//**********************************************************************************
typedef struct period_command_type
{
  	word MessageID;    				// Message ID range (0000 .. FFFF) Hex 

  	COMMAND_TYPE Command;         	// Send period command to selected motor (see enumeration
                                	// declaration above.)  
 
 	byte MotorID; 					// Selected motor range (0..1) Hex
 	byte Period; 					// Period range xx (00..FF) Hex              	
    byte Filler[27];				// Padding 

} PERIOD_COMMAND_TYPE;

//**********************************************************************************
//* 	Z         - Reset the SPI Slave Controller.  This command resets the       *
//*                 selected controller.  It may be used to implement an emergency *
//*                 stop function using the PIC's watchdog timer                   *                                                      *
//**********************************************************************************
typedef struct reset_command_type
{
  	word MessageID;    				// Message ID range (0000 .. FFFF) Hex 

  	COMMAND_TYPE Command;         	// Send period command to selected motor (see enumeration
                                	// declaration above.)  
  	byte MotorID; 					// Selected motor range (0..1)  

    byte Filler[28];				// Padding 

} RESET_COMMAND_TYPE;

#endif
