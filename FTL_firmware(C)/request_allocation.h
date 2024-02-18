//////////////////////////////////////////////////////////////////////////////////
// request_allocation.h for Cosmos+ OpenSSD
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
// Module Name: Request Allocator
// File Name: request_allocation.h
//
// Version: v1.0.0
//
// Description:
//   - define parameters, data structure and functions of request allocator
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#ifndef REQUEST_ALLOCATION_H_
#define REQUEST_ALLOCATION_H_

#include "ftl_config.h"
#include "request_format.h"
#include "request_queue.h"

#define	AVAILABLE_OUNTSTANDING_REQ_COUNT			((USER_DIES) * 128)  //regardless of request type

#define REQ_SLOT_TAG_NONE		0xffff
#define REQ_SLOT_TAG_FAIL		0xffff

typedef struct _REQ_POOL
{
	SSD_REQ_FORMAT reqPool[AVAILABLE_OUNTSTANDING_REQ_COUNT];
} REQ_POOL, *P_REQ_POOL;

void InitReqPool();

void PutToFreeReqQ(uint32_t reqSlotTag);
uint32_t GetFromFreeReqQ();

void PutToSliceReqQ(uint32_t reqSlotTag);
uint32_t GetFromSliceReqQ();

void PutToBlockedByBufDepReqQ(uint32_t reqSlotTag);
void SelectiveGetFromBlockedByBufDepReqQ(uint32_t reqSlotTag);

void PutToBlockedByRowAddrDepReqQ(uint32_t reqSlotTag, uint32_t chNo, uint32_t wayNo);
void SelectiveGetFromBlockedByRowAddrDepReqQ(uint32_t reqSlotTag, uint32_t chNo, uint32_t wayNo);

void PutToNvmeDmaReqQ(uint32_t reqSlotTag);
void SelectiveGetFromNvmeDmaReqQ(uint32_t regSlotTag);

void PutToNandReqQ(uint32_t reqSlotTag, unsigned chNo, unsigned wayNo);
void GetFromNandReqQ(uint32_t chNo, uint32_t wayNo, uint32_t reqStatus, uint32_t reqCode);

extern P_REQ_POOL reqPoolPtr;
extern FREE_REQUEST_QUEUE freeReqQ;
extern SLICE_REQUEST_QUEUE sliceReqQ;
extern BLOCKED_BY_BUFFER_DEPENDENCY_REQUEST_QUEUE blockedByBufDepReqQ;
extern BLOCKED_BY_ROW_ADDR_DEPENDENCY_REQUEST_QUEUE blockedByRowAddrDepReqQ[USER_CHANNELS][USER_WAYS];
extern NVME_DMA_REQUEST_QUEUE nvmeDmaReqQ;
extern NAND_REQUEST_QUEUE nandReqQ[USER_CHANNELS][USER_WAYS];

extern uint32_t notCompletedNandReqCnt;
extern uint32_t blockedReqCnt;

#endif /* REQUEST_ALLOCATION_H_ */
