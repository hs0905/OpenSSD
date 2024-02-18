//////////////////////////////////////////////////////////////////////////////////
// host_lld.h for Cosmos+ OpenSSD
// Copyright (c) 2016 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Youngjin Jo <yjjo@enc.hanyang.ac.kr>
//				  Sangjin Lee <sjlee@enc.hanyang.ac.kr>
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
// Engineer: Sangjin Lee <sjlee@enc.hanyang.ac.kr>
//			 Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: NVMe Low Level Driver
// File Name: host_lld.h
//
// Version: v1.1.0
//
// Description:
//   - defines parameters and data structures of the NVMe low level driver
//   - declares functions of the NVMe low level driver
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.1.0
//   - new DMA status type is added (HOST_DMA_ASSIST_STATUS)
//	 - DMA partial done check functions are added
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>

#ifndef __HOST_LLD_H_
#define __HOST_LLD_H_


#define HOST_IP_ADDR						0x83C00000

#define DEV_IRQ_MASK_REG_ADDR				(HOST_IP_ADDR + 0x4)
#define DEV_IRQ_CLEAR_REG_ADDR				(HOST_IP_ADDR + 0x8)
#define DEV_IRQ_STATUS_REG_ADDR				(HOST_IP_ADDR + 0xC)

#define PCIE_STATUS_REG_ADDR				(HOST_IP_ADDR + 0x100)
#define PCIE_FUNC_REG_ADDR					(HOST_IP_ADDR + 0x104)

#define NVME_STATUS_REG_ADDR				(HOST_IP_ADDR + 0x200)
#define HOST_DMA_FIFO_CNT_REG_ADDR			(HOST_IP_ADDR + 0x204)
#define NVME_ADMIN_QUEUE_SET_REG_ADDR		(HOST_IP_ADDR + 0x21C)
#define NVME_IO_SQ_SET_REG_ADDR				(HOST_IP_ADDR + 0x220)
#define NVME_IO_CQ_SET_REG_ADDR				(HOST_IP_ADDR + 0x260)

#define NVME_CMD_FIFO_REG_ADDR				(HOST_IP_ADDR + 0x300)
#define NVME_CPL_FIFO_REG_ADDR				(HOST_IP_ADDR + 0x304)
#define HOST_DMA_CMD_FIFO_REG_ADDR			(HOST_IP_ADDR + 0x310)

#define NVME_CMD_SRAM_ADDR					(HOST_IP_ADDR + 0x10000)




#define HOST_DMA_DIRECT_TYPE				(1)
#define HOST_DMA_AUTO_TYPE					(0)

#define HOST_DMA_TX_DIRECTION				(1)
#define HOST_DMA_RX_DIRECTION				(0)

#define ONLY_CPL_TYPE						(0)
#define AUTO_CPL_TYPE						(1)
#define CMD_SLOT_RELEASE_TYPE				(2)
#define P_SLOT_TAG_WIDTH					(10) //slot_modified

#pragma pack(push, 1)

typedef struct _DEV_IRQ_REG
{
	union {
		uint32_t dword;
		struct {
			uint32_t pcieLink			:1;
			uint32_t busMaster			:1;
			uint32_t pcieIrq			:1;
			uint32_t pcieMsi			:1;
			uint32_t pcieMsix			:1;
			uint32_t nvmeCcEn			:1;
			uint32_t nvmeCcShn			:1;
			uint32_t mAxiWriteErr		:1;
			uint32_t mAxiReadErr		:1;
			uint32_t pcieMreqErr		:1;
			uint32_t pcieCpldErr		:1;
			uint32_t pcieCpldLenErr		:1;
			uint32_t reserved0			:20;
		};
	};
} DEV_IRQ_REG;

typedef struct _PCIE_STATUS_REG
{
	union {
		uint32_t dword;
		struct {
			uint32_t ltssm				:6;
			uint32_t reserved0			:2;
			uint32_t pcieLinkUp			:1;
			uint32_t reserved1			:23;
		};
	};
} PCIE_STATUS_REG;

typedef struct _PCIE_FUNC_REG
{
	union {
		uint32_t dword;
		struct {
			uint32_t busMaster			:1;
			uint32_t msiEnable			:1;
			uint32_t msixEnable			:1;
			uint32_t irqDisable			:1;
			uint32_t msiVecNum			:3;
			uint32_t reserved0			:25;
		};
	};
} PCIE_FUNC_REG;

//offset: 0x00000200, size: 4
typedef struct _NVME_STATUS_REG
{
	union {
		uint32_t dword;
		struct {
			uint32_t ccEn				:1;
			uint32_t ccShn				:2;
			uint32_t reserved0			:1;
			uint32_t cstsRdy			:1;
			uint32_t cstsShst			:2;
			uint32_t rstCnt				:4;
			uint32_t linkNum			:2;
			uint32_t linkEn				:2;
			uint32_t reserved1			:17;
		};
	};
} NVME_STATUS_REG;

//offset: 0x00000300, size: 4
typedef struct _NVME_CMD_FIFO_REG
{
	union {
		uint32_t dword;
		struct {
			uint32_t qID				:4;
			uint32_t reserved0			:1;//slot_modified
			uint32_t cmdSlotTag			:P_SLOT_TAG_WIDTH; //slot_modified
			uint32_t reserved2			:1;//slot_modified
			uint32_t cmdSeqNum			:8;
			uint32_t reserved3			:(17-P_SLOT_TAG_WIDTH);//slot_modified
			uint32_t cmdValid				:1;
		};
	};
} NVME_CMD_FIFO_REG;

//offset: 0x00000304, size: 8
typedef struct _NVME_CPL_FIFO_REG
{
	union {
		uint32_t dword[3];
		struct {
			struct 
			{
				uint32_t 				cid					:16;
				uint32_t 				sqId				:4;
				uint32_t 				reserved0		:12;
			};
				uint32_t 				specific;
				unsigned short 	cmdSlotTag	:P_SLOT_TAG_WIDTH; //slot_modified
				unsigned short 	reserved1		:(14- P_SLOT_TAG_WIDTH); //slot_modified
				unsigned short 	cplType			:2;

			union {
				unsigned short statusFieldWord;
				struct 
				{
					unsigned short reserved0	:1;
					unsigned short SC			:8;
					unsigned short SCT			:3;
					unsigned short reserved1	:2;
					unsigned short MORE			:1;
					unsigned short DNR			:1;
				}statusField;
			};
		};
	};
} NVME_CPL_FIFO_REG;

//offset: 0x0000021C, size: 4
typedef struct _NVME_ADMIN_QUEUE_SET_REG
{
	union {
		uint32_t dword;
		struct {
			uint32_t cqValid			:1;
			uint32_t sqValid			:1;
			uint32_t cqIrqEn			:1;
			uint32_t reserved0			:29;
		};
	};
} NVME_ADMIN_QUEUE_SET_REG;

//offset: 0x00000220, size: 8
typedef struct _NVME_IO_SQ_SET_REG
{
	union {
		uint32_t dword[2];
		struct {
			uint32_t pcieBaseAddrL;
			uint32_t pcieBaseAddrH		:16;//modified
			uint32_t valid				:1;
			uint32_t cqVector			:4;
			uint32_t reserved1			:3;
			uint32_t sqSize				:8;
		};
	};
} NVME_IO_SQ_SET_REG;


//offset: 0x00000260, size: 8
typedef struct _NVME_IO_CQ_SET_REG
{
	union {
		uint32_t dword[2];
		struct {
			uint32_t pcieBaseAddrL;
			uint32_t pcieBaseAddrH		:16;//modified
			uint32_t valid				:1;
			uint32_t irqVector			:3;
			uint32_t irqEn				:1;
			uint32_t reserved1			:3;
			uint32_t cqSize				:8;
		};
	};
} NVME_IO_CQ_SET_REG;

//offset: 0x00000204, size: 4
typedef struct _HOST_DMA_FIFO_CNT_REG
{
	union {
		uint32_t dword;
		struct 
		{
			unsigned char directDmaRx;
			unsigned char directDmaTx;
			unsigned char autoDmaRx;
			unsigned char autoDmaTx;
		};
	};
} HOST_DMA_FIFO_CNT_REG;

//offset: 0x0000030C, size: 16
typedef struct _HOST_DMA_CMD_FIFO_REG
{
	union {
		uint32_t dword[5];//slot_modified
		struct 
		{
			uint32_t devAddr;
			uint32_t pcieAddrH;
			uint32_t pcieAddrL;			
			struct 
			{
				uint32_t dmaLen				:13;
				uint32_t autoCompletion		:1;
				uint32_t cmd4KBOffset		:9;
				uint32_t reserved0			:7;//slot_modified
				uint32_t dmaDirection		:1;
				uint32_t dmaType			:1;
			};
			uint32_t cmdSlotTag;//slot_modified
		};
	};
} HOST_DMA_CMD_FIFO_REG;

//offset: 0x00002000, size: 64 * 128
typedef struct _NVME_CMD_SRAM
{
	uint32_t dword[128][16];
} _NVME_CMD_SRAM;

#pragma pack(pop)


typedef struct _HOST_DMA_STATUS
{
	HOST_DMA_FIFO_CNT_REG fifoHead;
	HOST_DMA_FIFO_CNT_REG fifoTail;
	uint32_t directDmaTxCnt;
	uint32_t directDmaRxCnt;
	uint32_t autoDmaTxCnt;
	uint32_t autoDmaRxCnt;
} HOST_DMA_STATUS;


typedef struct _HOST_DMA_ASSIST_STATUS
{
	uint32_t autoDmaTxOverFlowCnt;
	uint32_t autoDmaRxOverFlowCnt;
} HOST_DMA_ASSIST_STATUS;

void dev_irq_init();

void dev_irq_handler();

uint32_t check_nvme_cc_en();

void set_nvme_csts_rdy();

void set_nvme_csts_shst(uint32_t shst);

void set_nvme_admin_queue(uint32_t sqValid, uint32_t cqValid, uint32_t cqIrqEn);

uint32_t get_nvme_cmd(unsigned short *qID, unsigned short *cmdSlotTag, uint32_t *cmdSeqNum, uint32_t *cmdDword);

void set_auto_nvme_cpl(uint32_t cmdSlotTag, uint32_t specific, uint32_t statusFieldWord);

void set_nvme_slot_release(uint32_t cmdSlotTag);

void set_nvme_cpl(uint32_t sqId, uint32_t cid, uint32_t specific, uint32_t statusFieldWord);

void set_io_sq(uint32_t ioSqIdx, uint32_t valid, uint32_t cqVector, uint32_t qSzie, uint32_t pcieBaseAddrL, uint32_t pcieBaseAddrH);

void set_io_cq(uint32_t ioCqIdx, uint32_t valid, uint32_t irqEn, uint32_t irqVector, uint32_t qSzie, uint32_t pcieBaseAddrL, uint32_t pcieBaseAddrH);

void set_direct_tx_dma(uint32_t devAddr, uint32_t pcieAddrH, uint32_t pcieAddrL, uint32_t len);

void set_direct_rx_dma(uint32_t devAddr, uint32_t pcieAddrH, uint32_t pcieAddrL, uint32_t len);

void set_auto_tx_dma(uint32_t cmdSlotTag, uint32_t cmd4KBOffset, uint32_t devAddr, uint32_t autoCompletion);

void set_auto_rx_dma(uint32_t cmdSlotTag, uint32_t cmd4KBOffset, uint32_t devAddr, uint32_t autoCompletion);

void set_link_width(uint32_t linkNum);

void pcie_async_reset(uint32_t rstCnt);

void check_direct_tx_dma_done();

void check_direct_rx_dma_done();

void check_auto_tx_dma_done();

void check_auto_rx_dma_done();

uint32_t check_auto_tx_dma_partial_done(uint32_t tailIndex, uint32_t tailAssistIndex);

uint32_t check_auto_rx_dma_partial_done(uint32_t tailIndex, uint32_t tailAssistIndex);

extern HOST_DMA_STATUS g_hostDmaStatus;
extern HOST_DMA_ASSIST_STATUS g_hostDmaAssistStatus;


#endif	//__HOST_LLD_H_
