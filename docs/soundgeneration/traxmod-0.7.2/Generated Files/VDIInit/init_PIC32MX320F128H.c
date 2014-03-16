
// Initialization Code for PIC32MX320F128H, Family: General Purpose control, Package: 64-Pin TQFP 64pins

#include <p32xxxx.h>

#include <..\include\proc\P32MX320F128H.h>



// Filter Controls used to Generate Code:

// POR Match Filter ON

// Provisioned Feature Filter ON

// Masks are Ignored and uses UnMasked Register Writes

// ---- Feature=fuses - fuses (DCR) configuration -----
#pragma config ICESEL = ICS_PGx1
#pragma config DEBUG = ON

//#define USEFRC
#define USEECPLL
//#define USEFRCPLL

#ifdef USEFRC
	#pragma config FNOSC = FRC
	#pragma config FPLLIDIV = DIV_6
	#pragma config FPLLMUL = MUL_24
	#pragma config FPLLODIV = DIV_1
	#pragma config POSCMOD = EC
#endif

#ifdef USEFRCPLL
	#pragma config FNOSC = FRCPLL
	#pragma config FPLLIDIV = DIV_12
	#pragma config FPLLMUL = MUL_24
	#pragma config FPLLODIV = DIV_1
	#pragma config POSCMOD = OFF
#endif

#ifdef USEECPLL
	#pragma config FNOSC = PRIPLL
	#pragma config FPLLIDIV = DIV_6
	#pragma config FPLLMUL = MUL_18
	#pragma config FPLLODIV = DIV_1
	#pragma config POSCMOD = EC
#endif 
#pragma config IESO = OFF
#pragma config OSCIOFNC = OFF
#pragma config FPBDIV = DIV_1
#pragma config FCKSM = CSECMD
#pragma config WDTPS = PS1
#pragma config FWDTEN = OFF

//#pragma config FPLLIDIV = DIV_3
//#pragma config FPLLMUL = MUL_21

void VisualInitialization(void)

{

	int tempvar;

// ---- Feature=Interrupts - Disable Interrupts during configuration -----

// ---- Feature=Reset - Reset configuration -----
	RCON = 0x0;
	WDTCON = 0x0;

// ---- Feature=Oscillator - Oscillator configuration -----

	asm ("la $t3, SYSKEY");

	asm ("li $t0,0xaa996655");

	asm ("nor $t1, $0, $t0");

	asm ("sw $t0, 0($t3)");

	asm ("sw $t1, 0($t3)");
	OSCCONbits.FRCDIV = 0x00000007;
	OSCCONbits.SOSCEN = 0x0;
	OSCCONbits.SLPEN = 0x0;
	OSCCONbits.CLKLOCK = 0x0;

	asm ("la $t3, SYSKEY");

	asm ("li $t0,0x33333333");

	asm ("sw $t0, 0($t3)");

// ---- Feature=A2D - Force code -----
                    	// force all A2D ports to digital IO at first
	AD1PCFG = 0x0000FFFF;

// ---- Feature=IOPortB - IO Ports configuration -----
	PORTB = 0x00000010;                    	// enable
	TRISB = 0x0000C1E3;                    	// direction in=1

// ---- Feature=IOPortC - IO Ports configuration -----
	PORTC = 0x0;                           	// enable
	TRISC = 0x000076FF;                    	// direction in=1

// ---- Feature=IOPortD - IO Ports configuration -----
	PORTD = 0x0000000F;                    	// enable
	TRISD = 0x0000C3F0;                    	// direction in=1

// ---- Feature=IOPortE - IO Ports configuration -----
	PORTE = 0x00000080;                    	// enable
	TRISE = 0x0000C60C;                    	// direction in=1

// ---- Feature=IOPortF - IO Ports configuration -----
	PORTF = 0x0;                           	// enable
	TRISF = 0x0000C6FC;                    	// direction in=1

// ---- Feature=IOPortG - IO Ports configuration -----
	PORTG = 0x0;                           	// enable
	TRISG = 0x0000C6F3;                    	// direction in=1

// ---- Feature=CN1 - Input Change Notification configuration -----
	CNPUE = 0x0001F803 | (1<<9);                    	// enable pullup change notification

// ---- Feature=SPI1 - SPI configuration -----
	tempvar = SPI1BUF;
	SPI1CON = 0x40008800;
	SPI1BRG = 0x00000001;                  	// SPI baud rate generator

// ---- Feature=SPI2 - SPI configuration -----
	tempvar = SPI2BUF;
	SPI2CON = 0x00000020;
	SPI2BRG = 0x00000001;                  	// SPI baud rate generator

// ---- Feature=UART2 - UART 2 configuration -----
	U2BRG = 0x000000AC;                    	// UART2 baud rate generator
	U2MODE = 0x8000;                       	// enabling UART flushes buffers
	U2MODE = 0x00008818;

// ---- Feature=RTC - RTCC configuration -----

// ---- Feature=PMP - PMP/PSP configuration -----

// ---- Feature=required - Interrupt flags cleared and interrupt configuration -----

// ---- Feature=Interrupts - enable interrupts -----

	return;

}
