// -*- tab-width: 4 -*-
// TRAXMOD Digital Audio Player
//
// Copyright (c) 2008, K9spud LLC.
// http://www.k9spud.com/traxmod/
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <p32xxxx.h>

#include "main.h"
#include "picspi.h"
#include "efsl/sd.h"

inline void UNSELECT_MMC(void)
{
	PORTESET = MMC_SS_PIN;
}

inline void SELECT_MMC(void)
{
	PORTECLR = MMC_SS_PIN;
}

// Note: SD Cards are only specified to operate up to 25MHz.
//
// If peripheral bus clock is 80MHz, then SPIBRG of 1 gives us:
// 80MHz / ( 2 * (SPIBRG(1) + 1) ) = 20MHz, which is as close as we can get.
#define SPI_RATE_FAST  0
#define SPI_RATE_SLOW  0xFE

esint8 if_initInterface(hwInterface* file, eint8* opts)
{
	euint32 sc;

	if_spiInit(); /* init at low speed */
	
	if(sd_Init() < 0)
	{
		DBG((TXT("Card failed to init, breaking up...\n")));
		PORTD = LED3|LED4;
		return(-1);
	}

	if(sd_State() < 0)
	{
		DBG((TXT("Card didn't return the ready state, breaking up...\n")));
		PORTD = LED2|LED4;
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
	if_spiSetSpeed(SPI_RATE_FAST);
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
#define SPI_ENABLED (1<<15)
#define SPI_MSTEN (1<<5)
#define SPI_SMP (1<<9)
#define SPI_CKP (1<<6)
void if_spiInit(void)
{
	euint8 i; 

	SPI2CON = 0;
	PORTGSET = MMC_SCK_PIN;

	// set Chip-Select high - unselect card
	UNSELECT_MMC();

//	PORTGCLR = MMC_SCK_PIN;
//	SPI2STATCLR = 0x40;

	i = SPI2BUF; //clear receive buffer
	if_spiSetSpeed(SPI_RATE_SLOW);
	SPI2STAT = 0;	// clear overflow flag

	// SPICKE totally doesn't work, with CKP or without CKP
	// SPICKP - Idle state for clock line is High.
	SPI2CON = SPI_ENABLED | SPI_MSTEN | SPICKP; //SPI_SMP | 

	DBG((TXT("spiInit for MMC/SD SPI2\n")));

	/* Send about 100 clocks with card not selected to allow SD/MMC Card to boot up */
	for(i = 0; i < 16; i++)
	{
		if_spiSend(0xff);
	}

	SELECT_MMC();
}
/*****************************************************************************/

void if_spiSetSpeed(euint8 speed)
{
	//SPI2BRG = 0; // Peripheral Bus Clock / 2 (fastest speed possible of SPI)
	SPI2BRG = speed; 
}

/*****************************************************************************/

#define SPITBE (1<<3)
#define SPIRBF (1<<0)
euint8 if_spiSend(euint8 outgoing)
{
	int i;
	euint8 incoming;

	while( !(SPI2STAT & SPITBE) )
	{
		// spin while SPI transmit buffer full
//		PORTD ^= LED1;
 	}
//	PORTDCLR = LED1;
/*	if(SPI2STAT & SPIRBF)
	{
		// whoops, already have data waiting to be read in SPI2BUF
		incoming = SPI2BUF;
	}
*/	SPI2STATCLR = SPIROV;
	SPI2BUF = outgoing;

	while(!(SPI2STAT & SPIRBF));
	incoming = SPI2BUF;
	return incoming;
}
/*****************************************************************************/

