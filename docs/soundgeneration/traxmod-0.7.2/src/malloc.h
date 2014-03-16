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

#ifndef __MALLOC_H
#define __MALLOC_H

#include "efsl/types.h"

void printHeap(char* sText);
euint16 freeSpace(char* p);
euint16 freeHeapSize(void);
//_VOID free(_PTR ptr);
//_PTR malloc(size_t __size);

#endif // __MALLOC_H

