
//********************************************************************************
//*                                                                              *
//* Description:                                                                 *
//*                                                                              *
//*     Name:  my_types.h                                                        *
//*   Author:  Daniel F. Ramirez                                                 *
//*     Date:  01/17/2007                                                        *
//*                                                                              *
//* Description:                                                                 *
//*         This include file contains my basic types for Stellaris LM3S811 ARM. *
//*                                                                              *
//*                                                                              *                        
//*                                                                              *
//********************************************************************************
   
#ifndef MY_TYPES_LOADED  /* prevent multiple loading */
#define MY_TYPES_LOADED
       
//*******************************************************************************
// Define typedef structures
//*******************************************************************************
  
typedef unsigned char bit;                               // 1 Bit 
typedef unsigned char nib;                               // 4 Bit unsigned integer
typedef unsigned char byte;                              // 8 Bit unsigned integer
typedef unsigned short int word;                         // 16 Bit unsigned integer	(32-Bit ARM)
typedef unsigned int longword;                          // 32 Bit unsigned integer	(32-Bit ARM)
 
typedef byte HexByte_T[3];                         		// Hex byte string type range (00..FF)
typedef byte HexWord_T[5];                              // Hex word string type range (0000..FFFF)
typedef byte HexLongWord_T[9];           				// Hex word string type range (00000000..FFFFFFFF)   
 
 
 
//*******************************************************************************
// Define port bit definitions here
//*******************************************************************************
 
#define TRUE 1                                          // Logical True value
#define FALSE 0                                         // Logical False value
#define INPUT    1                                  	// Input pin direction
#define OUTPUT   0                                  	// Output pin direction
#define READ  1                                     	// I2C read operation
#define WRITE 0                                     	// I2C write operation
#define ON   0											// Turn on LED
#define OFF  1											// Turn off LED

//*******************************************************************************
// Bit positions 
//*******************************************************************************
// Define bit positions as powers of 2

#define BIT_0 1 		// Position of bit 0
#define BIT_1 2 		// Position of bit 1
#define BIT_2 4 		// Position of bit 2
#define BIT_3 8 		// Position of bit 3
#define BIT_4 16 		// Position of bit 4
#define BIT_5 32 		// Position of bit 5
#define BIT_6 64		// Position of bit 6
#define BIT_7 128		// Position of bit 7
  
 /*             	
// LEDs using for debug using port D
#define LED_1 PORTDbits.RD0					 			// LED 1 (0=On, 1=Off)
#define LED_2 PORTDbits.RD1					 			// LED 2 (0=On, 1=Off)
#define LED_3 PORTDbits.RD2					 			// LED 3 (0=On, 1=Off)
#define LED_4 PORTDbits.RD3					 			// LED 4 (0=On, 1=Off)
 
// Direction bits for LEDs
#define LED_1_DIR TRISDbits.TRISD0					 		// LED 1 DIRECTION =0
#define LED_2_DIR TRISDbits.TRISD1					 		// LED 2 DIRECTION =0 
#define LED_3_DIR TRISDbits.TRISD2					 		// LED 3 DIRECTION =0 
#define LED_4_DIR TRISDbits.TRISD3					 		// LED 4 DIRECTION =0 
 
 // LEDs using for debug using port E
#define LED_1 PORTEbits.RE0					 			// LED 1 (0=On, 1=Off)
#define LED_2 PORTEbits.RE1					 			// LED 2 (0=On, 1=Off)
#define LED_3 PORTEbits.RE2					 			// LED 3 (0=On, 1=Off)
#define LED_4 PORTEbits.RE3					 			// LED 4 (0=On, 1=Off)
 
// Direction bits for LEDs
#define LED_1_DIR TRISEbits.TRISE0					 		// LED 1 DIRECTION =0
#define LED_2_DIR TRISEbits.TRISE1					 		// LED 2 DIRECTION =0 
#define LED_3_DIR TRISEbits.TRISE2					 		// LED 3 DIRECTION =0 
#define LED_4_DIR TRISEbits.TRISE3					 		// LED 4 DIRECTION =0 
 */
#endif
