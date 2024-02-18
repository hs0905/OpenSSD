//////////////////////////////////////////////////////////////////////////////////
// data_buffer.h for Cosmos+ OpenSSD
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
// Module Name: Data Buffer Manager
// File Name: data_buffer.h
//
// Version: v1.0.0
//
// Description:
//   - define parameters, data structure and functions of data buffer manager
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////


#ifndef DATA_BUFFER_H_
#define DATA_BUFFER_H_

#include "ftl_config.h"
#include <stdint.h>

#define AVAILABLE_DATA_BUFFER_ENTRY_COUNT				(16 * USER_DIES)
#define AVAILABLE_TEMPORARY_DATA_BUFFER_ENTRY_COUNT		(USER_DIES)

#define DATA_BUF_NONE	0xffff
#define DATA_BUF_FAIL	0xffff
#define DATA_BUF_DIRTY	1
#define DATA_BUF_CLEAN	0

#define FindDataBufHashTableEntry(logicalSliceAddr) ((logicalSliceAddr) % AVAILABLE_DATA_BUFFER_ENTRY_COUNT)


typedef struct _DATA_BUF_ENTRY {
	uint32_t logicalSliceAddr;
	uint32_t prevEntry : 16;
	uint32_t nextEntry : 16;
	uint32_t blockingReqTail : 16;
	uint32_t hashPrevEntry : 16;
	uint32_t hashNextEntry : 16;
	uint32_t dirty : 1;
	uint32_t reserved0 : 15;
} DATA_BUF_ENTRY, *P_DATA_BUF_ENTRY;

typedef struct _DATA_BUF_MAP{
	DATA_BUF_ENTRY dataBuf[AVAILABLE_DATA_BUFFER_ENTRY_COUNT];
} DATA_BUF_MAP, *P_DATA_BUF_MAP;

typedef struct _DATA_BUF_LRU_LIST {
	uint32_t headEntry : 16;
	uint32_t tailEntry : 16;
} DATA_BUF_LRU_LIST, *P_DATA_BUF_LRU_LIST;

typedef struct _DATA_BUF_HASH_ENTRY{
	uint32_t headEntry : 16;
	uint32_t tailEntry : 16;
} DATA_BUF_HASH_ENTRY, *P_DATA_BUF_HASH_ENTRY;


typedef struct _DATA_BUF_HASH_TABLE{
	DATA_BUF_HASH_ENTRY dataBufHash[AVAILABLE_DATA_BUFFER_ENTRY_COUNT];
} DATA_BUF_HASH_TABLE, *P_DATA_BUF_HASH_TABLE;


typedef struct _TEMPORARY_DATA_BUF_ENTRY {
	uint32_t blockingReqTail : 16;
	uint32_t reserved0 : 16;
} TEMPORARY_DATA_BUF_ENTRY, *P_TEMPORARY_DATA_BUF_ENTRY;

typedef struct _TEMPORARY_DATA_BUF_MAP{
	TEMPORARY_DATA_BUF_ENTRY tempDataBuf[AVAILABLE_TEMPORARY_DATA_BUFFER_ENTRY_COUNT];
} TEMPORARY_DATA_BUF_MAP, *P_TEMPORARY_DATA_BUF_MAP;

void InitDataBuf();
uint32_t CheckDataBufHit(uint32_t reqSlotTag);
uint32_t AllocateDataBuf();
void UpdateDataBufEntryInfoBlockingReq(uint32_t bufEntry, uint32_t reqSlotTag);

uint32_t AllocateTempDataBuf(uint32_t dieNo);
void UpdateTempDataBufEntryInfoBlockingReq(uint32_t bufEntry, uint32_t reqSlotTag);

void PutToDataBufHashList(uint32_t bufEntry);
void SelectiveGetFromDataBufHashList(uint32_t bufEntry);

extern P_DATA_BUF_MAP dataBufMapPtr;
extern DATA_BUF_LRU_LIST dataBufLruList;
extern P_DATA_BUF_HASH_TABLE dataBufHashTable;
extern P_TEMPORARY_DATA_BUF_MAP tempDataBufMapPtr;

#endif /* DATA_BUFFER_H_ */
