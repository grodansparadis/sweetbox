// -*- tab-width: 4 -*-
//
// Heap memory allocation routines for embedded systems.
//
// Copyright (c) 2006-2008, K9spud LLC.
// http://www.k9spud.com/
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

#include <stdlib.h>

#include "malloc.h"
#include "main.h"

#define MEM_INUSE (1<<15)
//#define DEBUG_MALLOC 1

//extern char endHeap[]; // address provided by linker
extern char _stack[];
extern const int _min_stack_size;
#define endHeap (_stack + _min_stack_size)
extern char _heap[]; // address provided by linker
char* pHeapBegin = NULL; // beginning of heap memory (usually address of "end," but word aligned)
char* pFreeHeap = NULL;  // next available free heap memory block

euint16 freeHeapSize(void)
{
	return ((euint16*)pFreeHeap)[0];
}

void* malloc(size_t __size)
{
	char* p;
	char* pAlloc = NULL;
	euint16 iSmallestMatch;
	
	if(pFreeHeap == NULL)
	{
		pFreeHeap = _heap;
		euint16 iBlockSize = (euint16)(((euint32)(endHeap) - (euint32)pFreeHeap));
		while((((euint32)pFreeHeap) & 3) != 2)
		{
			iBlockSize--; // round down for block alignment.
			pFreeHeap++;
		}
		pHeapBegin = pFreeHeap;
		((euint16*)pFreeHeap)[0] = iBlockSize;
	}

#ifdef DEBUG_MALLOC
	printHeap("pre-malloc");
#endif
	
	p = pFreeHeap;
	iSmallestMatch = 0xFFFF;

	euint16 size;
	euint16 sizeAlloc = 0;
	while(p < endHeap)
	{
		euint16 iBlockSize = ((euint16*)p)[0];
		if(iBlockSize & (1<<16))
		{
#ifdef DEBUG_MALLOC
			rprintf("Can't use block at: %X, %u bytes already in use.\n", (unsigned long)p, iBlockSize & (~MEM_INUSE));
#endif
			// this block is already in use, can't use it.
			p += iBlockSize & (~MEM_INUSE);
			continue;
		}
		
		size = __size;
		while((((euint32)(p) + sizeof(euint16) + size) & 3) != 2)
		{
			size++;
		}
		if( (iBlockSize >= (size + sizeof(euint16))) && 
			iBlockSize < iSmallestMatch )
		{
#ifdef DEBUG_MALLOC
			rprintf("Usable block at: %X, %u bytes available, %u needed.\n", (unsigned long)p, iBlockSize, __size);
#endif
			iSmallestMatch = iBlockSize;
			pAlloc = p + sizeof(euint16);
			sizeAlloc = size;
		}

		p += iBlockSize;
	}
	
	if(pAlloc == NULL)
	{
		rprintf("Out of memory.\n");
	}
	else
	{
		pFreeHeap = pAlloc + sizeAlloc;
		((euint16*)pAlloc)[-1] = (sizeAlloc + sizeof(euint16)) | MEM_INUSE;
		if(pFreeHeap < endHeap)
		{
			((euint16*)pFreeHeap)[0] = iSmallestMatch - (sizeAlloc + sizeof(euint16));
		}
	}

#ifdef DEBUG_MALLOC
	printHeap("post-malloc");
#endif

	return pAlloc;
}

void free(void* ptr)
{
	char* p;
	euint16 iSize = ((euint16*)ptr)[-1] & (~MEM_INUSE);

#ifdef DEBUG_MALLOC
	printHeap("pre-free");
#endif
	
	// free the block by writing back iSize without the MSB bit
	// set.
	((euint16*)ptr)[-1] = iSize;

#ifdef DEBUG_MALLOC
	printHeap("pre-defrag");
#endif

	// now we need to defragment the free memory
	p = pHeapBegin;
	while(p < endHeap)
	{
		iSize = freeSpace(p);
		if(iSize)
		{
			// Write back new free space size.
			((euint16*)p)[0] = iSize;
			if(p < pFreeHeap)
			{
				pFreeHeap = p;
			}
		}
		else
		{
			// Block is in use, find size of used block to
			// jump to next block.
			iSize = ((euint16*)p)[0] & (~MEM_INUSE); // strip off "in-use" flag.
		}
		p += iSize;
	}

#ifdef DEBUG_MALLOC
	printHeap("post-defrag");
#endif
}

/**
 * Recursively determines the maximize free memory size of
 * the specified memory block. 
 *
 * If the memory block is in use, returns 0.
 *
 * Example:
 * | Block 0, 32 bytes (FREE) | Block 32, 64 bytes (FREE) | Block 96, 10 bytes (IN-USE) |
 * freeSpace(0)  will return 32 + 64 = 96 bytes.
 * freeSpace(32) will return 64 bytes.
 * freeSpace(96) will return 0 bytes.
 *
 * @author edwards
 * @date 5/7/2006
 */
euint16 freeSpace(char* p)
{
	euint16 iSize;
	if(p >= endHeap)
	{
		return 0;
	}

	iSize = ((euint16*)p)[0];
	if(iSize & MEM_INUSE)
	{
		// this memory block is in use
		return 0;
	}
	else
	{
		char* pNext = p + iSize;
		return iSize + freeSpace(pNext);
	}
}

void printHeap(char* sText)
{
	char* p = pHeapBegin;
	
	if(sText != NULL)
	{
		rprintf("%s ", sText);
	}
	
	rprintf("End of RAM: %X Heap Size: %u Beginning of Heap: %X Free Heap: %X\n", 
		(unsigned long)endHeap, (unsigned long)endHeap - (unsigned long)startHeap, (unsigned long) p, pFreeHeap);

	while(p < endHeap)
	{
		euint16 iSize = ((euint16*)p)[0];
		rprintf("  Heap Address: %X Size: ", (unsigned long)p);
		if(iSize & MEM_INUSE)
		{
			iSize &= ~MEM_INUSE;
			rprintf("%u (in-use)\n", iSize);
		}
		else
		{
			rprintf("%u (free)\n", iSize);
		}
		
		p += iSize;
	}
}
