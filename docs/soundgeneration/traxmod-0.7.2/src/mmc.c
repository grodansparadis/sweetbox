// -*- tab-width: 4 -*-
// MMC/SD Card Interface
//
// Copyright (c) 2006-2008, K9spud LLC.
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

#include "main.h"
#include "mmc.h"
#include "modplay/mixer.h"
#include "interfaces/sd.h"

#include "interfaces/lpc2000_dbg_printf.h"
#define rprintf lpc2000_debug_printf

int iSD_BlockLength = 512;

euint8 SPIWrite(euint8 outgoing)
{
	euint8 incoming;

	// Wait until TX FIFO not full
	while( !(SSPSR & (1<<TNF)) )
	{
		rprintf("tx fifo full\n");
	}
	
	SSPDR = outgoing;

	// Wait until RX FIFO not empty
	while( !(SSPSR & (1<<RNE)) );
	incoming = SSPDR;

	return incoming;
}

void SD_WriteCommand(euint8 cmd, euint16 paramx, euint16 paramy)
{
	SPIWrite(0xff);

	SPIWrite(0x40 | cmd);
	SPIWrite((euint8) (paramx >> 8)); 	// MSB of parameter x
	SPIWrite((euint8) (paramx)); 		// LSB of parameter x
	SPIWrite((euint8) (paramy >> 8)); 	// MSB of parameter y
	SPIWrite((euint8) (paramy)); 		// LSB of parameter y

	SPIWrite(0x95); // Checksum (should be only valid for first command (0)

	// Following any command, the SD Card needs 8 clocks to finish up its work.
	// (from SanDisk SD Card Product Manual v1.9 section 5.1.8)
	SPIWrite(0xff);
}

euint8 SD_ResponsePing8(void)
{
	euint8 i;
	euint8 resp;
	
	// Response will come after 1 - 8 pings
	for(i = 0; i < 8; i++)
	{
		resp = SPIWrite(0xff);
		if(resp != 0xff)
		{
			return(resp);
		}
	}
		
	return(resp);
}

esint8 SD_ReadBlock(euint32 address, euint8* buf, euint16 len)
{
	euint8 cardresp;
	euint8 firstblock;
	euint8 c;
	euint16 fb_timeout = 0xffff;
	euint32 i;
	euint32 place;

	place = 512 * address;
	SD_WriteCommand(CMDREAD, (euint16) (place >> 16), (euint16) place);
	
	cardresp = SD_ResponsePing8();

	// Wait for startblock
	do
	{
		firstblock = SD_ResponsePing8(); 
	} while(firstblock == 0xff && fb_timeout--);

	if(cardresp != 0x00 || firstblock != 0xfe)
	{
		sd_Resp8bError(firstblock);
		return(-1);
	}
	
	for(i = 0; i < 512; i++)
	{
		c = SPIWrite(0xff);
		if(i < len)
			buf[i] = c;
	}

	// Checksum (2 byte) - ignore for now 
	SPIWrite(0xff);
	SPIWrite(0xff);

	return(0);
}

void SD_SetBlockLength(unsigned int blockLength)
{
	if(SSPIMSC)
	{
//		rprintf("waiting for read block...\n");
		while(SSPIMSC);
	}

	SD_WriteCommand(CMDSETBLOCKLEN, (euint16) (blockLength >> 16), (euint16) blockLength);

	euint8 cardresp = SD_ResponsePing8();
	if(cardresp == 0)
	{
		iSD_BlockLength	= blockLength;
	}
	rprintf("card response to setblocklen(%d): %d\n", blockLength, cardresp);
}

void SD_StopTransmission(void)
{
	if(SSPIMSC)
	{
		rprintf("waiting for read block...\n");
		while(SSPIMSC);
	}

	while(SSPSR & (1<<RNE))
	{
		// Receive FIFO is not empty, read junk data until it goes empty.		
		junkRead = SSPDR;
	}

	SD_WriteCommand(CMDSTOPTRANSMISSION, 0, 0);
	
	// wait for card to go into IDLE state
	while(SD_ResponsePing8() == 0);
}


// Reads multiple blocks starting from address to length. If address is
// not block aligned, this routine will use iSD_SkipBytes to skip storing
// initial bytes received in order to read on block boundary.
esint8 SD_BeginRead(euint32 address, euint8* buf, euint16 len)
{
	if(SSPIMSC)
	{
		rprintf("waiting for read block...\n");
		while(SSPIMSC);
	}

	pSDReadPtr = buf;
	BytesLeftToRead = len;
	iSD_BytesToCRC = iSD_BlockLengthCRC;
	iSD_SkipBytes = address & 0x1FF;

	if(readingData != NULL)
	{
		readingData->Length = 0;	
	}

	if(iSD_SkipBytes)
	{
//		rprintf("addr: %X align: %X finalAddr: ", address, iSD_SkipBytes);
		address -= iSD_SkipBytes;
//		rprintf("%X\n", address);
//		iSD_SkipBytes++;
	}
	VICVectAddr5 = (unsigned long) SD_WaitForStart_ISR;
	
	SSPDR = 0x40 | CMDREADMULTI; // command
	SSPDR = address >> 24; // Address
	SSPDR = address >> 16;
	SSPDR = address >> 8;
	SSPDR = address;
	SSPDR = 0x95; // CRC 
	SSPDR = 0xFF; // dummy data to drive SPI clock
	SSPDR = 0xFF;	

	SSPIMSC = (1<<2) | (1<<1);		// enable interrupt on RX FIFO half full or not empty for 32 bit times
	VICIntEnable = (1<<11); // re-enable SSP interrupt		

	return(0);
}


/**
 * Takes an EFSL file handle and offset, figures out the real MMC/SD Card physical
 * address of the given file+offset, and returns the physical address.
 */
euint32 file_getAddress(File *file, euint32 offset)
{
	euint32 coffset = offset;
	euint32 cclus, csec, cbyte;
	euint32 rclus, rsec;
		
	if(!file_getAttr(file, FILE_STATUS_OPEN))
	{
		return(0);
	}
	
	if(offset >= file->FileSize)
	{
		return(0);
	}
	
	cclus = coffset / (512 * file->fs->volumeId.SectorsPerCluster);
	csec = (coffset / (512)) % file->fs->volumeId.SectorsPerCluster;
	cbyte = coffset % 512;
		
	if((fat_LogicToDiscCluster(file->fs, &(file->Cache), cclus)) != 0)
	{
		return(0);
	}

	rclus = file->Cache.DiscCluster;
	rsec = fs_clusterToSector(file->fs, rclus);
		
	return part_getRealLBA(file->fs->part, rsec+csec);
}
