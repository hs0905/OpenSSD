//////////////////////////////////////////////////////////////////////////////////
// address_translation.h for Cosmos+ OpenSSD
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
// Module Name: Address Translator
// File Name: address translation.h
//
// Version: v1.0.0
//
// Description:
//   - define parameters, data structure and functions of address translator
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////


#ifndef ADDRESS_TRANSLATION_H_
#define ADDRESS_TRANSLATION_H_

#include "ftl_config.h"
#include "nvme.h"

#define LSA_NONE	0xffffffff
#define LSA_FAIL	0xffffffff

#define VSA_NONE	0xffffffff
#define VSA_FAIL	0xffffffff

#define PAGE_NONE		0xffff

#define BLOCK_NONE	0xffff
#define BLOCK_FAIL	0xffff

#define DIE_NONE	0xff
#define DIE_FAIL	0xff

#define RESERVED_FREE_BLOCK_COUNT	0x1

#define GET_FREE_BLOCK_NORMAL	0x0
#define GET_FREE_BLOCK_GC		0x1

#define BLOCK_STATE_NORMAL						0
#define BLOCK_STATE_BAD							1

#define DIE_STATE_BAD_BLOCK_TABLE_NOT_EXIST		0
#define DIE_STATE_BAD_BLOCK_TABLE_EXIST			1

#define BAD_BLOCK_TABLE_MAKER_IDLE				0
#define BAD_BLOCK_TABLE_MAKER_TRIGGER			1
#define DIE_STATE_BAD_BLOCK_TABLE_HOLD			2
#define DIE_STATE_BAD_BLOCK_TABLE_UPDATE		3

#define CLEAN_DATA_IN_BYTE						0xff

#define USED_PAGES_FOR_BAD_BLOCK_TABLE_PER_DIE	(TOTAL_BLOCKS_PER_DIE / BYTES_PER_DATA_REGION_OF_PAGE + 1)
#define DATA_SIZE_OF_BAD_BLOCK_TABLE_PER_DIE	(TOTAL_BLOCKS_PER_DIE)
#define START_PAGE_NO_OF_BAD_BLOCK_TABLE_BLOCK	(1)		//bad block table begins at second page for preserving a bad block mark of the block allocated to save bad block table

#define BBT_INFO_GROWN_BAD_UPDATE_NONE			0
#define BBT_INFO_GROWN_BAD_UPDATE_BOOKED		1

// virtual slice address to virtual organization translation
#define Vsa2VdieTranslation(virtualSliceAddr) ((virtualSliceAddr) % (USER_DIES))
#define Vsa2VblockTranslation(virtualSliceAddr) (((virtualSliceAddr) / (USER_DIES)) / (SLICES_PER_BLOCK))
#define Vsa2VpageTranslation(virtualSliceAddr) (((virtualSliceAddr) / (USER_DIES)) % (SLICES_PER_BLOCK))

// virtual organization to virtual slice address translation
#define Vorg2VsaTranslation(dieNo, blockNo, pageNo) ((dieNo) + (USER_DIES)*((blockNo)*(SLICES_PER_BLOCK) + (pageNo)))

// virtual to physical translation
#define Vdie2PchTranslation(dieNo) ((dieNo) % (USER_CHANNELS))
#define Vdie2PwayTranslation(dieNo) ((dieNo) / (USER_CHANNELS))
#define Vblock2PblockOfTbsTranslation(blockNo) (((blockNo) / (USER_BLOCKS_PER_LUN)) * (TOTAL_BLOCKS_PER_LUN) + ((blockNo) % (USER_BLOCKS_PER_LUN))) //Tbs = Total block space
#define Vblock2PblockOfMbsTranslation(blockNo) (((blockNo) / (USER_BLOCKS_PER_LUN)) * (MAIN_BLOCKS_PER_LUN) + ((blockNo) % (USER_BLOCKS_PER_LUN))) //Mbs = Main block space
#define Vpage2PlsbPageTranslation(pageNo) ((pageNo) > (0) ? (2 * (pageNo) - 1): (0))

// physical to virtual translation
#define Pcw2VdieTranslation(chNo, wayNo) ((chNo) + (wayNo) * (USER_CHANNELS))
#define PlsbPage2VpageTranslation(pageNo) ((pageNo) > (0) ? ( ((pageNo) + 1) / 2): (0))

//for logical to virtual translation
typedef struct _LOGICAL_SLICE_ENTRY {
	uint32_t virtualSliceAddr;
} LOGICAL_SLICE_ENTRY, *P_LOGICAL_SLICE_ENTRY;

typedef struct _LOGICAL_SLICE_MAP {
	LOGICAL_SLICE_ENTRY logicalSlice[SLICES_PER_SSD];
} LOGICAL_SLICE_MAP, *P_LOGICAL_SLICE_MAP;


//for virtual to logical  translation
typedef struct _VIRTUAL_SLICE_ENTRY {
	uint32_t logicalSliceAddr;
} VIRTUAL_SLICE_ENTRY, *P_VIRTUAL_SLICE_ENTRY;

typedef struct _VIRTUAL_SLICE_MAP {
	VIRTUAL_SLICE_ENTRY virtualSlice[SLICES_PER_SSD];
} VIRTUAL_SLICE_MAP, *P_VIRTUAL_SLICE_MAP;

typedef struct _VIRTUAL_BLOCK_ENTRY {
	uint32_t bad : 1;
	uint32_t free : 1;
	uint32_t invalidSliceCnt : 16;
	uint32_t reserved0 :10;
	uint32_t currentPage : 16;
	uint32_t eraseCnt : 16;
	uint32_t prevBlock : 16;
	uint32_t nextBlock :16;
} VIRTUAL_BLOCK_ENTRY, *P_VIRTUAL_BLOCK_ENTRY;

typedef struct _VIRTUAL_BLOCK_MAP {
	VIRTUAL_BLOCK_ENTRY block[USER_DIES][USER_BLOCKS_PER_DIE];
} VIRTUAL_BLOCK_MAP, *P_VIRTUAL_BLOCK_MAP;


typedef struct _VIRTUAL_DIE_ENTRY {
	uint32_t currentBlock : 16;
	uint32_t headFreeBlock : 16;
	uint32_t tailFreeBlock : 16;
	uint32_t freeBlockCnt : 16;
	uint32_t prevDie : 8;
	uint32_t nextDie : 8;
	uint32_t reserved0 : 16;
} VIRTUAL_DIE_ENTRY, *P_VIRTUAL_DIE_ENTRY;

typedef struct _VIRTUAL_DIE_MAP {
	VIRTUAL_DIE_ENTRY die[USER_DIES];
} VIRTUAL_DIE_MAP, *P_VIRTUAL_DIE_MAP;

typedef struct _FRRE_BLOCK_ALLOCATION_LIST {	//free block allocation die sequence list
	uint32_t headDie : 8;
	uint32_t tailDie : 8;
	uint32_t reserved0 : 16;
} FRRE_BLOCK_ALLOCATION_LIST, *P_FRRE_BLOCK_ALLOCATION_LIST;

typedef struct _BAD_BLOCK_TABLE_INFO_ENTRY{
	uint32_t phyBlock : 16;
	uint32_t grownBadUpdate : 1;
	uint32_t reserved0 : 15;
} BAD_BLOCK_TABLE_INFO_ENTRY, *P_BAD_BLOCK_TABLE_ENTRY;

typedef struct _BAD_BLOCK_TABLE_INFO_MAP{
	BAD_BLOCK_TABLE_INFO_ENTRY bbtInfo[USER_DIES];
} BAD_BLOCK_TABLE_INFO_MAP, *P_BAD_BLOCK_TABLE_INFO_MAP;

typedef struct _PHY_BLOCK_ENTRY {
	uint32_t remappedPhyBlock : 16;
	uint32_t bad :1;
	uint32_t reserved0 :15;
} PHY_BLOCK_ENTRY, *P_PHY_BLOCK_ENTRY;

typedef struct _PHY_BLOCK_MAP {
	PHY_BLOCK_ENTRY phyBlock[USER_DIES][TOTAL_BLOCKS_PER_DIE];
} PHY_BLOCK_MAP, *P_PHY_BLOCK_MAP;


void InitAddressMap();
void InitSliceMap();
void InitBlockDieMap();

uint32_t AddrTransRead(uint32_t logicalSliceAddr);
uint32_t AddrTransWrite(uint32_t logicalSliceAddr);
uint32_t FindFreeVirtualSlice();
uint32_t FindFreeVirtualSliceForGc(uint32_t copyTargetDieNo, uint32_t victimBlockNo);
uint32_t FindDieForFreeSliceAllocation();

void InvalidateOldVsa(uint32_t logicalSliceAddr);
void EraseBlock(uint32_t dieNo, uint32_t blockNo);

void PutToFbList(uint32_t dieNo, uint32_t blockNo);
uint32_t GetFromFbList(uint32_t dieNo, uint32_t getFreeBlockOption);

void UpdatePhyBlockMapForGrownBadBlock(uint32_t dieNo, uint32_t phyBlockNo);
void UpdateBadBlockTableForGrownBadBlock(uint32_t tempBufAddr);


extern P_LOGICAL_SLICE_MAP logicalSliceMapPtr;
extern P_VIRTUAL_SLICE_MAP virtualSliceMapPtr;
extern P_VIRTUAL_BLOCK_MAP virtualBlockMapPtr;
extern P_VIRTUAL_DIE_MAP virtualDieMapPtr;
extern P_PHY_BLOCK_MAP phyBlockMapPtr;
extern P_BAD_BLOCK_TABLE_INFO_MAP bbtInfoMapPtr;

extern unsigned char sliceAllocationTargetDie;
extern uint32_t mbPerbadBlockSpace;
#endif /* ADDRESS_TRANSLATION_H_ */
