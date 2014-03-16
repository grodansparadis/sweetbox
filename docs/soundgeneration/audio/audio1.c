/****************************************************************************
 *
 * Project: Audio Generation 1
 *
 ****************************************************************************
 * Purpose:
 *   
 ****************************************************************************
 * FileName		: audio1.c
 * Dependencies	:
 * Config Bits	: In Code
 * Prefetch   	: On
 * Freq-CPU   	: 80MHz
 * PBCLK      	: 20MHz (1:4)
 * Processor  	: PIC32MX360F512L
 ****************************************************************************
 * Revision		:
 * 0.0 - First Creation - 20,Aug,2008 Abhijit Bose(boseji@yahoo.com)
 ****************************************************************************
 * License		: GPL2
 * Software License Agreement
 *
 * THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ***************************************************************************/
#include <p32xxxx.h>    /* PIC32 Main Library */
#include <plib.h>       /* PIC32 peripheral library */ 
#include "playdata.h"
// Configuration Bit settings
// SYSCLK = 80 MHz (8MHz Crystal/ FPLLIDIV * FPLLMUL / FPLLODIV)
// PBCLK = 20 MHz
// Primary Osc w/PLL (XT+,HS+,EC+PLL)
// WDT OFF
// Other options are don't care
//
#pragma config FPLLMUL = MUL_20, FPLLIDIV = DIV_2, FPLLODIV = DIV_1, FWDTEN = OFF
#pragma config POSCMOD = HS, FNOSC = PRIPLL, FPBDIV = DIV_4

//Interrupt Definition Macros
#define ISR(PRIOR,VECT) void __attribute__ ((interrupt(PRIOR))) __attribute__ ((vector(VECT))) interrupt ## VECT (void)
#define INTERRUPT(PRIOR,VECT,NAME) void __attribute__ ((interrupt(PRIOR))) __attribute__ ((vector(VECT))) NAME (void)

#define SYS_FREQ 				(80000000L)
#define PBCLK                   (20000000L)
#define DESIRED_BAUDRATE    	(115200)      //The desired BaudRate

//System LED
#define SYS_LED_ON()            mPORTDSetBits(BIT_2)
#define SYS_LED_OFF()           mPORTDClearBits(BIT_2)

//Error LED
#define ERR_LED_ON()            mPORTDSetBits(BIT_0)
#define ERR_LED_OFF()           mPORTDClearBits(BIT_0)

//Activity LED
#define ACT_LED_ON()            mPORTDSetBits(BIT_1)
#define ACT_LED_OFF()           mPORTDClearBits(BIT_1)

int ctr;
void SetOC4Freq(int X){ 
    CloseTimer2();
    CloseOC4();
    OpenTimer2(T2_ON|T2_PS_1_1,X);
    OpenOC4(OC_ON|OC_TIMER_MODE16|OC_TIMER2_SRC| OC_CONTINUE_PULSE | OC_LOW_HIGH ,X,(X/2));
}    
INTERRUPT(ipl2,_TIMER_1_VECTOR,Timer1Handler){
    // clear the interrupt flag
    mT1ClearIntFlag();
    if(ctr<PLAY_DATA_SZ)ctr++;
    else ctr = 0;
    SetDCOC4PWM(play_data[ctr]);
}    
        
void main(){
    
	/* Config Prefetch Cache */
    SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    
	/* Init the UART1 port */
	OpenUART1(UART_EN|UART_BRGH_FOUR, 		// Module is ON
			  UART_RX_ENABLE | UART_TX_ENABLE,		// Enable TX & RX
			  PBCLK/4/DESIRED_BAUDRATE-1);	// DESIRED_BAUDRATE bps, 8-N-1
			  
	/* Init LED o/p */
    mPORTDClearBits(BIT_0|BIT_1|BIT_2);         /* Turn off LED0/LED1/LED2 on startup */
    mPORTDSetPinsDigitalOut(BIT_0|BIT_1|BIT_2); /* Make RD0 (LED0)RD1 (LED0)RD2 (LED0) as output */
    
	/* Configure Timer 2 as Clock Source for OC1 */
	SetOC4Freq(256);
	
	/* Config Timer 1 for Controling LED*/
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_1, (PBCLK/(1*8120)));//Prescale by 64
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);//Config interrupt priority for Level2
	ctr = 0;
	
	/* Enable Processor Interrupts */
    INTEnableSystemMultiVectoredInt();
	
	putsUART1("|====================|\r\n");
    putsUART1("|===[ LETS BEGIN ]===|\r\n");
    putsUART1("|====================|\r\n");
    SYS_LED_ON();
    
    while(1)
    {
    }
}
