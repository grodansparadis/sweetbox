
//********************************************************************************
//*                                                                              *
//*           Name:  my_types.h                                                  *
//* Project Number:  MT2192                                                      *                                                   
//*           Date:  10/15/2007                                                  *                                                  
//*                                                                              *
//*   Description:                                                               * 
//*         This include file contains my basic types for C30 C.                 *
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
typedef unsigned int word;                               // 16 Bit unsigned integer
typedef unsigned long longword;                          // 32 Bit unsigned integer
 
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
  
                	
// LEDs using for debug
#define LED_1 PORTDbits.RD0					 			// LED 1 (0=On, 1=Off)
#define LED_2 PORTDbits.RD1					 			// LED 2 (0=On, 1=Off)
#define LED_3 PORTDbits.RD2					 			// LED 3 (0=On, 1=Off)
#define LED_4 PORTDbits.RD3					 			// LED 4 (0=On, 1=Off)
 
// Direction bits for LEDs
#define LED_1_DIR TRISDbits.TRISD0					 		// LED 1 DIRECTION =0
#define LED_2_DIR TRISDbits.TRISD1					 		// LED 2 DIRECTION =0 
#define LED_3_DIR TRISDbits.TRISD2					 		// LED 3 DIRECTION =0 
#define LED_4_DIR TRISDbits.TRISD3					 		// LED 4 DIRECTION =0 
typedef char boolean;

 
#define MaxWord 0xFFFF

#define UINT8 unsigned char
#define INT8 char
#define UINT16 unsigned int
#define INT16 int
#define UINT32 unsigned long
#define INT32 long

#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)
#define BIT7 (1<<7)
#define BIT8 (1<<8)
#define BIT9 (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)

union uword16 {
	struct {
		UINT8 b0;
		UINT8 b1;
	}bytes;
	UINT16 both;
};

 

#define LED1_ON LATDbits.LATD0=0;
#define LED1_OFF LATDbits.LATD0=1;
#define LED2_ON LATDbits.LATD1=0;
#define LED2_OFF LATDbits.LATD1=1;
#define LED3_ON LATDbits.LATD2=0;
#define LED3_OFF LATDbits.LATD2=1;
#define LED4_ON LATDbits.LATD3=0;
#define LED4_OFF LATDbits.LATD3=1;

 
#endif
