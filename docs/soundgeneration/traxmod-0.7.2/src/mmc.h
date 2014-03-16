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

#ifndef MMC_H
#define MMC_H

#include "types.h"
#include "file.h"
#include "interfaces/lpc2000_spi.h"

// SSPSR Bit-Definitions
// Transmit FIFO Empty
#define TFE 0

// Transmit FIFO Not Full
#define TNF 1

// Receive FIFO Not Empty
#define RNE 2

// Receive FIFO Full
#define RFF 3

// 0 if SSP module is idle, 1 if currently rx/tx.
#define BSY 4

// MMC/SD Card command codes:
#define	CMDREAD				17
#define	CMDWRITE			24
#define	CMDREADCSD      	9
#define CMDSETBLOCKLEN		16
#define CMDREADMULTI		18
#define CMDSTOPTRANSMISSION	12
#define CMDSENDSTATUS		13

// SSPRIS flags
#define SSP_TXHALFEMPTY		(1<<3)
#define SSP_RXHALFFULL		(1<<2)

euint8 SPIWrite(euint8 outgoing);

void SD_WriteCommand(euint8 cmd, euint16 paramx, euint16 paramy);
euint8 SD_ResponsePing8(void);
esint8 SD_ReadBlock(euint32 address, euint8* buf, euint16 len);
void SD_SetBlockLength(unsigned int blockLength);
esint8 SD_BeginReadBlock(euint32 address, euint8* buf, euint16 len);
esint8 SD_BeginRead(euint32 address, euint8* buf, euint16 len);
void SD_StopTransmission(void);

euint32 file_getAddress(File *file, euint32 offset);

void __attribute__ ((interrupt("IRQ"))) SD_ReadBlock_ISR(void);
void __attribute__ ((interrupt("IRQ"))) SD_SkipBytes_ISR(void);
void __attribute__ ((interrupt("IRQ"))) SD_WaitForStart_ISR(void);
//void __attribute__ ((interrupt("IRQ"))) SD_ReadMulti_ISR(void);

extern int iSD_BlockLength;
extern volatile int iSD_SkipBytes;
extern volatile int iSD_BytesToCRC;
extern volatile int iSD_BlockLengthCRC;
extern volatile int BytesLeftToRead;
extern volatile euint8* pSDReadPtr;

#define SDS_IDLE 		0
#define SDS_READING 	1
#define SDS_WAITSTOP	2
extern volatile int iSD_State;

#endif

