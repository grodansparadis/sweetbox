//********************************************************************************
//*                                                                              *
//*           Name:  FLAGS.H                                                     *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*     This file includes the global C30 Flags used by C30 C Applications.      *
//*                                                                              * 
//*                                                                              *
//*                                                                              *
//********************************************************************************

#ifndef FLAGS_LOADED  /* prevent multiple loading */
#define FLAGS_LOADED

// General flags used by all applications

// Select processor type

//#define DSPIC30F2010									 // Select the DSPIC30F2010 micro-controller
//#define DSPIC30F4011									 // Select the DSPIC30F4011micro-controller
#define DSPIC30F6014									 // Select the DSPIC30F6014 micro-controller

// Initialize compilation flags here
 
#define BASE_CLOCK 1843200                          // DSPIC internal clock 1.843200(MHz)

// Select clock speed (MIPS) = BASE_CLOCK * MULTIPLIER (4X, 8X, 16X)

//#define XT_4X_PLL                                      // Set system clock for XT x 4 PLL
//#define XT_8X_PLL                                    // Set system clock for XT x 8 PLL         
#define XT_16X_PLL                                   // Set system clock for XT x 16 PLL           


// Select the desired system instruction processing speed (in MIPS) here
#ifdef XT_4X_PLL
#define THE_CLOCK_SPEED (BASE_CLOCK*4)			// MIPS for XT x 4 PLL
#endif

#ifdef XT_8X_PLL
#define THE_CLOCK_SPEED (BASE_CLOCK*8)			// MIPS for XT x 8 PLL
#endif

#ifdef XT_16X_PLL
#define THE_CLOCK_SPEED (BASE_CLOCK*16)			// MIPS for XT x 16 PLL
#endif

#define SPI_SUPPORT					// SPI Support used for LCD, etc
//#define SPI_8										// 8-Bit SPI  Works!!!!
#define SPI_16										// 16-Bit SPI Works!!!!

#define UART_SUPPORT				// UART Support used for serial I/O

// Select the desired UART 1 baud rate here
//#define THE_BAUD_RATE 1200				// 1200 Baud Asynchronous Serial rate
//#define THE_BAUD_RATE 4800				// 4800 Baud Asynchronous Serial rate
//#define THE_BAUD_RATE 9600				// 9600 Baud Asynchronous Serial rate
//#define THE_BAUD_RATE 19200				// 19.2 K Baud Asynchronous Serial rate
//#define THE_BAUD_RATE 38400					// 38.4 K Baud Asynchronous Serial rate
//#define THE_BAUD_RATE 57600				// 56 K Baud Asynchronous Serial rate
#define THE_BAUD_RATE 115200				// 112 K Baud Asynchronous Serial rate

// Select the desired UART 2 baud rate here
//#define THE_BAUD_RATE2 1200				// 1200 Baud Asynchronous Serial rate
//#define THE_BAUD_RATE2 4800				// 4800 Baud Asynchronous Serial rate
//#define THE_BAUD_RATE2 9600				// 9600 Baud Asynchronous Serial rate
//#define THE_BAUD_RATE2 19200				// 19.2 K Baud Asynchronous Serial rate
//#define THE_BAUD_RATE2 38400					// 38.4 K Baud Asynchronous Serial rate
#define THE_BAUD_RATE2 115200				// 112 K Baud Asynchronous Serial rate
	
// Define constants and flags used by the timeutil.c application

//#define USE_TIMER_1                                     // Use 16-Bit Timer 1 for timer utilities
#define USE_TIMER_2_AND_3                                 // Use 32-Bit Timer 2/3 for timer utilities 
//#define USE_TIMER_4_AND_5                               // Use 32-Bit Timer 4/5 for timer utilities

#define WATCHDOG_OFF									  // Watchdog timer off
//#define WATCHDOG_ON                                     // Watchdog timer on 

#define SERIAL_DRIVER                                     // Include serial driver      
//#define KEYPAD_DRIVER                                     // Include Keypad driver
//#define LED_DRIVER                                      // Include Max7219 LED Display
//#define LCD_DRIVER                                      // Include LCD Display 
//#define SERIAL_LCD_DISPLAY							  // Select a serial LCD display driver

//#define I2C_DRIVER                                      // Include I2C driver     
//#define SPI_DRIVER                                      // Include SPI driver     

//#define DEBUG                                        	  // Flag used for MPSIM debugging 
                                                          // (must be commented out for actual use)

#define STDIO_INTERRUPTS                            	  // Include interrupt code  and TX/RX buffers for stdio functions
//#define FULL_DUPLEX                                     // Full duplex serial I/O support
//#define TIMEOUT_SUPPORT                             	  // Timeout support for serial I/O (USART)
//#define NETWORK_SUPPORT                                 // Support for PC to PIC Networking using the USART


//#define TIMER_INTERRUPTS                           	  // Include timer interrupt code
   
// Select the correct module test

//#define SW_USART_TEST                                   // Software UART unit test
//#define UARTUTIL_TEST                                     // UART Utilies unit test
//#define TIMEUTIL_TEST							   	      // Timeutil unit test
//#define PORTUTIL_TEST									  // Portutil unit test
//#define KEYPAD_TEST								 	  // Keypad unit test
//#define IRCONTRL_TEST								 	  // Ircontrl unit test
//#define SONYRC_TEST								 	  // Sonyrc unit test
//#define MASTER_SENSOR_TEST							  // Master Sensor Controller unit test
//#define SLAVE_SENSOR_TEST								  // Slave Sensor Controller unit test
//#define PORTBINT_TEST                                   // Test Port B Change on State
//#define DISPLAY_TEST                  				  // Display unit test
//#define JOYSTICK_TEST               					  // Joystick unit test
//#define ANIMATION_TEST               					  // Animation unit test
//#define RELAYDRV_TEST               					  // Relay Driver unit test
//#define X10_TEST										  // X10 unit test
//#define CLOCK_TEST                                      // NJU6355 real-time clock unit test
//#define TIMERW_TEST                                     // X10 Timer unit test 
//#define MAX7219_TEST                                    // MAX7219 LED display unit test
//#define SONAR_TEST                                      // Polaroid 6500 Sonar Ranger unit test
//#define SW_I2C_MASTER_TEST                              // Software I2C Master test 
//#define I2C_MASTER_TEST                                 // I2C Master test 
//#define I2C_SLAVE_TEST                                  // I2C Slave test 
//#define VIDEOCAM_TEST                                   // VideoCam test 
//#define CALCULATOR_TEST								  // Calculator unit test
//#define MASTER_COPROCESSOR							  // Master co-processor unit test
//#define SLAVE_COPROCESSOR								  // Slave co-processor unit test
//#define LCD_TEST                                        // LCD unit test
//#define DRIVER_ALERT_TEST                                 // Driver Alert Test
#define DC_MOTOR_MASTER_TEST							  // DC motor Controller unit test
//#define DC_MOTOR_SLAVE_TEST							  // DC motor Controller unit test
//#define SPI_MASTER_TEST                                 // SPI Master test
//#define SPI_SLAVE_TEST                                  // SPI Slave Co-Processor test
//#define SPI_ADC_TEST
//#define SPI_TEST                                        // SPI unit test
//#define SERVO_TEST                                      // DC Servo Motor Test 
//#define RC_SERVO_TEST                                   // RC Servo Test 
//#define PWMUTIL_TEST                                    // PWM Utility test
//#define PICTERM_TEST                                    // PIC Terminal test
//#define SILVER1_TEST                                    // PIC I2C Master Serial EEPROM MSSP test (Serial PIC'n)
//#define ORANGE1_TEST                                    // PIC I2C Master Serial EEPROM test Bit-Banged (Serial PIC'n)
//#define ORANGE4_TEST                                    // PIC I2C Master test Bit-Banged (Serial PIC'n)
//#define ORANGE5_TEST                                    // PIC I2C Master test Bit-Banged (Serial PIC'n)
//#define GARDENBOT_TEST                                  // GardenBot robot application
//#define GEARBOT_TEST                                    // GearBot robot application
//#define MATH_TEST                                      // MATHTEST application
//#define JMATH_TEST                                      // JMATH test application



#endif
