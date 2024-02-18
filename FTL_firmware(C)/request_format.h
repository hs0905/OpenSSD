//////////////////////////////////////////////////////////////////////////////////
// request_format.h for Cosmos+ OpenSSD
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
// File Name: request_format.h
//
// Version: v1.0.0
//
// Description:
//   - define parameters, data structure of request
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#ifndef REQUEST_FORMAT_H_
#define REQUEST_FORMAT_H_

#include "nvme.h"

#define REQ_TYPE_SLICE			0x0
#define REQ_TYPE_NAND			0x1
#define REQ_TYPE_NVME_DMA		0x2

#define REQ_QUEUE_TYPE_NONE							0x0
#define REQ_QUEUE_TYPE_FREE							0x1
#define REQ_QUEUE_TYPE_SLICE						0x2
#define REQ_QUEUE_TYPE_BLOCKED_BY_BUF_DEP			0x3
#define REQ_QUEUE_TYPE_BLOCKED_BY_ROW_ADDR_DEP		0x4
#define REQ_QUEUE_TYPE_NVME_DMA						0x5
#define REQ_QUEUE_TYPE_NAND							0x6

#define REQ_CODE_WRITE				0x00
#define REQ_CODE_READ				0x08
#define REQ_CODE_READ_TRANSFER		0x09
#define REQ_CODE_ERASE				0x0C
#define REQ_CODE_RESET				0x0D
#define REQ_CODE_SET_FEATURE		0x0E
#define REQ_CODE_FLUSH				0x0F
#define REQ_CODE_RxDMA				0x10
#define REQ_CODE_TxDMA				0x20

#define REQ_CODE_OCSSD_PHY_TYPE_BASE	0xA0
#define REQ_CODE_OCSSD_PHY_WRITE		0xA0
#define REQ_CODE_OCSSD_PHY_READ			0xA8
#define REQ_CODE_OCSSD_PHY_ERASE		0xAC


#define REQ_OPT_DATA_BUF_ENTRY		0
#define REQ_OPT_DATA_BUF_TEMP_ENTRY	1
#define REQ_OPT_DATA_BUF_ADDR		2
#define REQ_OPT_DATA_BUF_NONE		3

#define REQ_OPT_NAND_ADDR_VSA		0
#define REQ_OPT_NAND_ADDR_PHY_ORG	1

#define REQ_OPT_NAND_ECC_OFF		0
#define REQ_OPT_NAND_ECC_ON			1

#define REQ_OPT_NAND_ECC_WARNING_OFF		0
#define REQ_OPT_NAND_ECC_WARNING_ON			1

#define REQ_OPT_WRAPPING_NONE		0
#define REQ_OPT_WRAPPING_REQ		1

#define REQ_OPT_ROW_ADDR_DEPENDENCY_NONE	0
#define REQ_OPT_ROW_ADDR_DEPENDENCY_CHECK 	1

#define REQ_OPT_BLOCK_SPACE_MAIN	0
#define REQ_OPT_BLOCK_SPACE_TOTAL 	1

#define LOGICAL_SLICE_ADDR_NONE 	0xffffffff

typedef struct _DATA_BUF_INFO{
	union {
		uint32_t addr;
		uint32_t entry;
	};
} DATA_BUF_INFO, *P_DATA_BUF_INFO;


typedef struct _NVME_DMA_INFO{
	uint32_t startIndex : 16;
	uint32_t nvmeBlockOffset : 16;
	uint32_t numOfNvmeBlock : 16;
	uint32_t reqTail	: 8;
	uint32_t reserved0 : 8;
	uint32_t overFlowCnt;
} NVME_DMA_INFO, *P_NVME_DMA_INFO;


typedef struct _NAND_INFO{
	union {
		uint32_t virtualSliceAddr;
		struct {
			uint32_t physicalCh : 4;
			uint32_t physicalWay : 4;
			uint32_t physicalBlock : 16;
			uint32_t phyReserved0 : 8;
		};
	};
	union {
		uint32_t programmedPageCnt;
		struct {
			uint32_t physicalPage : 16;
			uint32_t phyReserved1 : 16;
		};
	};
} NAND_INFO, *P_NAND_INFO;


typedef struct _REQ_OPTION{
	uint32_t dataBufFormat : 2;
	uint32_t nandAddr : 2;
	uint32_t nandEcc : 1;
	uint32_t nandEccWarning : 1;
	uint32_t rowAddrDependencyCheck : 1;
	uint32_t blockSpace : 1;
	uint32_t reserved0 : 24;
} REQ_OPTION, *P_REQ_OPTION;


typedef struct _SSD_REQ_FORMAT
{
	uint32_t reqType : 4;
	uint32_t reqQueueType : 4;
	uint32_t reqCode : 8;
	uint32_t nvmeCmdSlotTag : 16;
	
	uint32_t qID;
	uint32_t cID;

	uint32_t logicalSliceAddr;

	REQ_OPTION reqOpt;			//4byte
	DATA_BUF_INFO dataBufInfo;	//8byte
	NVME_DMA_INFO nvmeDmaInfo;	//24byte
	NAND_INFO nandInfo;			//12byte

	uint32_t prevReq : 16;
	uint32_t nextReq : 16;
	uint32_t prevBlockingReq : 16;
	uint32_t nextBlockingReq : 16;

} SSD_REQ_FORMAT, *P_SSD_REQ_FORMAT;

#endif /* REQUEST_FORMAT_H_ */
