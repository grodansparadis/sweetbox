/*****************************************************************************\
*              efs - General purpose Embedded Filesystem library              *
*          --------------------- -----------------------------------          *
*                                                                             *
* Filename : lpc2000_spi.c                                                    *
* Description : This file contains the functions needed to use efs for        *
*               accessing files on an SD-card connected to an LPC2xxx.        *
*                                                                             *
* This program is free software; you can redistribute it and/or               *
* modify it under the terms of the GNU General Public License                 *
* as published by the Free Software Foundation; version 2                     *
* of the License.                                                             *
                                                                              *
* This program is distributed in the hope that it will be useful,             *
* but WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
* GNU General Public License for more details.                                *
*                                                                             *
* As a special exception, if other files instantiate templates or             *
* use macros or inline functions from this file, or you compile this          *
* file and link it with other works to produce a work based on this file,     *
* this file does not by itself cause the resulting work to be covered         *
* by the GNU General Public License. However the source code for this         *
* file must still be made available in accordance with section (3) of         *
* the GNU General Public License.                                             *
*                                                                             *
* This exception does not invalidate any other reasons why a work based       *
* on this file might be covered by the GNU General Public License.            *
*                                                                             *
*                                                    (c)2005 Martin Thomas    *
\*****************************************************************************/

/*****************************************************************************/
#include "lpc2000_spi.h"
#include "sd.h"
#include "config.h"
/*****************************************************************************/

#ifndef HW_ENDPOINT_LPC2000_SPINUM
#error "HW_ENDPOINT_LPC2000_SPINUM has to be defined in config.h"
#endif

// LPC2103/2/1 ### SSP ### ("SPI1")

// SSPCR0  Bit-Definitions
#define CPOL    6
#define CPHA    7
// SSPCR1  Bit-Defintions
#define SSE     1
#define MS      2
#define SCR     8
// SSPSR  Bit-Definitions
#define TNF     1
#define RNE     2
#define BSY		4

#define SPI_SCK_PIN    14   /* Clock       P0.14  out */
#define SPI_MISO_PIN   19   /* from Card   P0.19  in  */
#define SPI_MOSI_PIN   20   /* to Card     P0.20  out */
/* Card-Select P0.20 - GPIO out during startup
   Function 03 during normal operation */
#define SPI_SS_PIN	   21   

#define SPI_PINSEL0    PINSEL0
#define SPI_SCK_FUNCBIT   28

#define SPI_PINSEL     PINSEL1
#define SPI_MISO_FUNCBIT  6
#define SPI_MOSI_FUNCBIT  8
#define SPI_SS_FUNCBIT    10

#define SPI_PRESCALE_REG  SSPCPSR

// Note: SD Cards are only specified to operate up to 25MHz.
//
// If processor clock is 70MHz, then prescaler of 4 gives us:
// 70MHz / 4 = 17.4MHz, which is as close as we can get.
// 70MHz / 4 = 17.4MHz, which is as close as we can get.
#define SPI_PRESCALE_MIN  4 /* must be an even numbered value */
//#define SPI_PRESCALE_MIN  2 /* must be an even numbered value */

esint8 if_initInterface(hwInterface* file, eint8* opts)
{
	euint32 sc;

	if_spiInit(); /* init at low speed */
	
	if(sd_Init() < 0)
	{
		DBG((TXT("Card failed to init, breaking up...\n")));
		return(-1);
	}

	if(sd_State() < 0)
	{
		DBG((TXT("Card didn't return the ready state, breaking up...\n")));
		return(-2);
	}
	
	// file->sectorCount=4; /* FIXME ASAP!! */
	
	sd_getDriveSize(&sc);
	file->sectorCount = sc/512;
	if( (sc%512) != 0) {
		file->sectorCount--;
	}
	DBG((TXT("Drive Size is %lu Bytes (%lu Sectors)\n"), sc, file->sectorCount));

	 // increase speed after init
	SSPCR0 = ((8-1)<<0) | (0<<CPOL);
	if_spiSetSpeed(SPI_PRESCALE_MIN);
	// if_spiSetSpeed(100); // debug - slower
	
	DBG((TXT("Init done...\n")));
	return(0);
}
/*****************************************************************************/ 

esint8 if_readBuf(euint32 address, euint8* buf)
{
	return(sd_readSector(address, buf, 512));
}
/*****************************************************************************/

esint8 if_writeBuf(euint32 address, euint8* buf)
{
	return(sd_writeSector(address, buf));
}

/*****************************************************************************/ 

/*****************************************************************************/ 

void if_spiInit(void)
{
	euint8 i; 

	// setup GPIO
	SPI_IODIR |= (1<<SPI_SCK_PIN)|(1<<SPI_MOSI_PIN)|(1<<SPI_SS_PIN);
	SPI_IODIR &= ~(1<<SPI_MISO_PIN);
	
	// set Chip-Select high - unselect card
	UNSELECT_CARD();

	// reset Pin-Functions	
	PINSEL0 &= ~( (3<<SPI_SCK_FUNCBIT) );
	PINSEL1 &= ~( (3<<SPI_MISO_FUNCBIT) | (3<<SPI_MOSI_FUNCBIT) | (3<<SPI_SS_FUNCBIT) );

	DBG((TXT("spiInit for SSP/SPI1\n")));

	// setup Pin-Functions - keep automatic CS disabled during init
	PINSEL0 |= ( 2<<SPI_SCK_FUNCBIT );
	PINSEL1 |= (1<<SPI_MISO_FUNCBIT) | (1<<SPI_MOSI_FUNCBIT) | (0<<SPI_SS_FUNCBIT);

	// enable SPI-Master - slowest speed
	SSPCR0 = ((8-1)<<0) | (0<<CPOL) | (0x20<<SCR); //  (0xff<<SCR);
	SSPCR1 = (1<<SSE);
	
	// low speed during init
	if_spiSetSpeed(254); 

	/* Send 20 spi commands with card not selected */
	for(i=0;i<21;i++)
	{
		if_spiSend(0xff);
	}

	// enable automatic slave CS for SSP
	SSPCR1 &= ~(1<<SSE); // disable interface
	PINSEL0 |= (2<<SPI_SCK_FUNCBIT);
	PINSEL1 |= ( (1<<SPI_MISO_FUNCBIT) | (1<<SPI_MOSI_FUNCBIT) | (1<<SPI_SS_FUNCBIT) );
	SSPCR1 |= (1<<SSE); // enable interface	
}
/*****************************************************************************/

void if_spiSetSpeed(euint8 speed)
{
	speed &= 0xFE;
	if ( speed < SPI_PRESCALE_MIN  ) speed = SPI_PRESCALE_MIN ;
	SPI_PRESCALE_REG = speed;
}

/*****************************************************************************/

euint8 if_spiSend(euint8 outgoing)
{
	euint8 incoming;

	while( !(SSPSR & (1<<TNF)) ) ;
	SSPDR = outgoing;
	while( !(SSPSR & (1<<RNE)) ) ;
	incoming = SSPDR;

	return(incoming);
}
/*****************************************************************************/

