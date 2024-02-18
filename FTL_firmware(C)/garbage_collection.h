//////////////////////////////////////////////////////////////////////////////////
// garbage_collection.h for Cosmos+ OpenSSD
// Copyright (c) 2017 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//
// This file is part of Cosmos+ OpenSSD.
//
// Cosmos+ OpenSSD is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// Cosmos+ OpenSSD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Cosmos+ OpenSSD; see the file COPYING.
// If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Company: ENC Lab. <http://enc.hanyang.ac.kr>
// Engineer: Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: Garbage Collector
// File Name: garbage_collection.h
//
// Version: v1.0.0
//
// Description:
//   - define parameters, data structure and functions of garbage collector
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////


#ifndef GARBAGE_COLLECTION_H_
#define GARBAGE_COLLECTION_H_

#include "ftl_config.h"
#include <stdint.h>

typedef struct _GC_VICTIM_LIST_ENTRY {
	uint32_t headBlock : 16;
	uint32_t tailBlock : 16;
} GC_VICTIM_LIST_ENTRY, *P_GC_VICTIM_LIST_ENTRY;

typedef struct _GC_VICTIM_MAP {
	GC_VICTIM_LIST_ENTRY gcVictimList[USER_DIES][SLICES_PER_BLOCK + 1];
} GC_VICTIM_MAP, *P_GC_VICTIM_MAP;

void InitGcVictimMap();
void GarbageCollection(uint32_t dieNo);

void PutToGcVictimList(uint32_t dieNo, uint32_t blockNo, uint32_t invalidSliceCnt);
uint32_t GetFromGcVictimList(uint32_t dieNo);
void SelectiveGetFromGcVictimList(uint32_t dieNo, uint32_t blockNo);

extern P_GC_VICTIM_MAP gcVictimMapPtr;
extern uint32_t gcTriggered;
extern uint32_t copyCnt;

#endif /* GARBAGE_COLLECTION_H_ */
