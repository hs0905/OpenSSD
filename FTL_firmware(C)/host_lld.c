//////////////////////////////////////////////////////////////////////////////////
// host_lld.c for Cosmos+ OpenSSD
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
// File Name: host_lld.c
//
// Version: v1.1.0
//
// Description:
//   - defines functions to control the NVMe controller
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.1.0
//	 - DMA partial done check functions are added
//	 - DMA assist status is added to support DMA partial done check functions
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#pragma GCC push_options
#pragma GCC optimize ("O0")

// #include "stdio.h"
// #include "xil_exception.h"
//#include "cpl_print.h"
#include "ariane.h"
#include "io_access.h"

#include "nvme.h"
#include "host_lld.h"
#include <stdint.h>

extern NVME_CONTEXT g_nvmeTask;
extern uint32_t table_logic_minus_count;
HOST_DMA_STATUS g_hostDmaStatus;
HOST_DMA_ASSIST_STATUS g_hostDmaAssistStatus;
// HOST_DMA_CMD_FIFO_REG hostDmaReg;

void dev_irq_init()
{
	DEV_IRQ_REG devReg;

	devReg.dword = 0;
	devReg.pcieLink = 1;
	devReg.busMaster = 1;
	devReg.pcieIrq = 1;
	devReg.pcieMsi = 1;
	devReg.pcieMsix = 1;
	devReg.nvmeCcEn = 1;
	devReg.nvmeCcShn = 1;
	devReg.mAxiWriteErr = 1;
	devReg.pcieMreqErr = 1;
	devReg.pcieCpldErr = 1;
	devReg.pcieCpldLenErr = 1;

	IO_WRITE32(DEV_IRQ_MASK_REG_ADDR, devReg.dword);
}

#pragma GCC push_options
#pragma GCC optimize ("align-functions=4")
void __attribute__ ((interrupt))
dev_irq_handler()
{
	DEV_IRQ_REG devReg;
	// Xil_ExceptionDisable();

	devReg.dword = IO_READ32(DEV_IRQ_STATUS_REG_ADDR);
	IO_WRITE32(DEV_IRQ_CLEAR_REG_ADDR, devReg.dword);
	// cpl_print("IRQ: ");
	// cpl_printint(devReg.dword);

	if(devReg.pcieLink == 1)
	{
		PCIE_STATUS_REG pcieReg;
		pcieReg.dword = IO_READ32(PCIE_STATUS_REG_ADDR);
		// cpl_print("PCIe Link:\n");// pcieReg.pcieLinkUp);
		// cpl_printint(pcieReg.pcieLinkUp);
		// set_link_width(0) //you can choose pcie lane width 0,2,4,8 mini board -> maximum 4, cosmos+ board -> maximum 8
		if(pcieReg.pcieLinkUp == 0)
			g_nvmeTask.status = NVME_TASK_RESET;
	}

	if(devReg.busMaster == 1)
	{
		PCIE_FUNC_REG pcieReg;
		pcieReg.dword = IO_READ32(PCIE_FUNC_REG_ADDR);
		// cpl_print("PCIe Bus Master:\n");// pcieReg.busMaster);
		// cpl_printint(pcieReg.busMaster);
	}

	if(devReg.pcieIrq == 1)
	{
		PCIE_FUNC_REG pcieReg;
		pcieReg.dword = IO_READ32(PCIE_FUNC_REG_ADDR);
		// cpl_print("PCIe IRQ Disable: %d\r\n");// pcieReg.irqDisable);
		// cpl_printint(pcieReg.irqDisable);
	}

	if(devReg.pcieMsi == 1)
	{
		PCIE_FUNC_REG pcieReg;
		pcieReg.dword = IO_READ32(PCIE_FUNC_REG_ADDR);
		// cpl_print("PCIe MSI Enable: %d, 0x%x\r\n");// pcieReg.msiEnable, pcieReg.msiVecNum);

	}

	if(devReg.pcieMsix == 1)
	{
		PCIE_FUNC_REG pcieReg;
		pcieReg.dword = IO_READ32(PCIE_FUNC_REG_ADDR);
		// cpl_print("PCIe MSI-X Enable:\n");// pcieReg.msixEnable);
		// cpl_printint(pcieReg.msixEnable);
	}

	if(devReg.nvmeCcEn == 1)
	{
		NVME_STATUS_REG nvmeReg;
		nvmeReg.dword = IO_READ32(NVME_STATUS_REG_ADDR);
		// cpl_print("NVME CC.EN:\n");// nvmeReg.ccEn);
		// cpl_printint(nvmeReg.ccEn);
		if(nvmeReg.ccEn == 1)
			g_nvmeTask.status = NVME_TASK_WAIT_CC_EN;
		else
			g_nvmeTask.status = NVME_TASK_RESET;

	}

	if(devReg.nvmeCcShn == 1)
	{
		NVME_STATUS_REG nvmeReg;
		nvmeReg.dword = IO_READ32(NVME_STATUS_REG_ADDR);
		// cpl_print("NVME CC.SHN:\n");
		// cpl_printint(nvmeReg.ccShn);
		if(nvmeReg.ccShn == 1)
			g_nvmeTask.status = NVME_TASK_SHUTDOWN;
	}

	if(devReg.mAxiWriteErr == 1)
	{
		// cpl_print("mAxiWriteErr\r\n");
	}

	if(devReg.pcieMreqErr == 1)
	{
		// cpl_print("pcieMreqErr\r\n");
	}

	if(devReg.pcieCpldErr == 1)
	{
		// cpl_print("pcieCpldErr\r\n");
	}

	if(devReg.pcieCpldLenErr == 1)
	{
		// cpl_print("pcieCpldLenErr\r\n");
	}

	// cpl_print("irq_handler end\n");
//	Xil_ExceptionEnable();
}
#pragma GCC pop_options

uint32_t check_nvme_cc_en()
{
	NVME_STATUS_REG nvmeReg;

	nvmeReg.dword = IO_READ32(NVME_STATUS_REG_ADDR); // HOST_IP_ADDR + 0x200
	
	return (uint32_t)nvmeReg.ccEn;
}

void pcie_async_reset(uint32_t rstCnt)
{
	NVME_STATUS_REG nvmeReg;

	nvmeReg.rstCnt = rstCnt;
	// cpl_print("rstCnt= %X \r\n");//rstCnt);
	IO_WRITE32(NVME_STATUS_REG_ADDR, nvmeReg.dword);

}

void set_link_width(uint32_t linkNum)
{
	NVME_STATUS_REG nvmeReg;

	nvmeReg.linkNum = linkNum;
	nvmeReg.linkEn = 1;
	// cpl_print("linkNum= %X \r\n");//linkNum);
	IO_WRITE32(NVME_STATUS_REG_ADDR, nvmeReg.dword);

}


void set_nvme_csts_rdy(uint32_t rdy)
{
	NVME_STATUS_REG nvmeReg;

	nvmeReg.dword = IO_READ32(NVME_STATUS_REG_ADDR);
	nvmeReg.cstsRdy = rdy;

	IO_WRITE32(NVME_STATUS_REG_ADDR, nvmeReg.dword);
}

void set_nvme_csts_shst(uint32_t shst)
{
	NVME_STATUS_REG nvmeReg;

	nvmeReg.dword = IO_READ32(NVME_STATUS_REG_ADDR);
	nvmeReg.cstsShst = shst;

	IO_WRITE32(NVME_STATUS_REG_ADDR, nvmeReg.dword);
}

void set_nvme_admin_queue(uint32_t sqValid, uint32_t cqValid, uint32_t cqIrqEn)
{
	NVME_ADMIN_QUEUE_SET_REG nvmeReg;

	nvmeReg.dword = IO_READ32(NVME_ADMIN_QUEUE_SET_REG_ADDR);
	nvmeReg.sqValid = sqValid;
	nvmeReg.cqValid = cqValid;
	nvmeReg.cqIrqEn = cqIrqEn;

	IO_WRITE32(NVME_ADMIN_QUEUE_SET_REG_ADDR, nvmeReg.dword);
}


uint32_t get_nvme_cmd(unsigned short *qID, unsigned short *cmdSlotTag, uint32_t *cmdSeqNum, uint32_t *cmdDword)
{
	NVME_CMD_FIFO_REG nvmeReg;
	
	nvmeReg.dword = IO_READ32(NVME_CMD_FIFO_REG_ADDR);

	if(nvmeReg.cmdValid == 1)
	{
		uint32_t addr;
		uint32_t idx;
		*qID = nvmeReg.qID;
		*cmdSlotTag = nvmeReg.cmdSlotTag;
		*cmdSeqNum = nvmeReg.cmdSeqNum;
		//cpl_print("nvmeReg.cmdSlotTag = 0x%X\r\n");// nvmeReg.cmdSlotTag);
		addr = NVME_CMD_SRAM_ADDR + (nvmeReg.cmdSlotTag * 64);
		for(idx = 0; idx < 16; idx++)
			*(cmdDword + idx) = IO_READ32(addr + (idx * 4));
	}
	
	return (uint32_t)nvmeReg.cmdValid;
}

void set_auto_nvme_cpl(uint32_t cmdSlotTag, uint32_t specific, uint32_t statusFieldWord)
{
	NVME_CPL_FIFO_REG nvmeReg;

	nvmeReg.specific 				= specific;
	nvmeReg.cmdSlotTag 			= cmdSlotTag;
	nvmeReg.statusFieldWord = statusFieldWord;
	nvmeReg.cplType 				= AUTO_CPL_TYPE;

	// cpl_print("set_auto_nvme_cpl\r\n");
	// cpl_printint(cmdSlotTag);
	// cpl_printint(specific);
	// cpl_printint(statusFieldWord);
	//IO_WRITE32(NVME_CPL_FIFO_REG_ADDR, nvmeReg.dword[0]);
	IO_WRITE32((NVME_CPL_FIFO_REG_ADDR + 4), nvmeReg.dword[1]);
	IO_WRITE32((NVME_CPL_FIFO_REG_ADDR + 8), nvmeReg.dword[2]);
}

void set_nvme_slot_release(uint32_t cmdSlotTag)
{
	NVME_CPL_FIFO_REG nvmeReg;

	nvmeReg.cmdSlotTag = cmdSlotTag;
	nvmeReg.cplType = CMD_SLOT_RELEASE_TYPE;
	// cpl_print("set_nvme_slot_release\r\n");
	//IO_WRITE32(NVME_CPL_FIFO_REG_ADDR, nvmeReg.dword[0]);
	//IO_WRITE32((NVME_CPL_FIFO_REG_ADDR + 4), nvmeReg.dword[1]);
	IO_WRITE32((NVME_CPL_FIFO_REG_ADDR + 8), nvmeReg.dword[2]);
}

void set_nvme_cpl(uint32_t sqId, uint32_t cid, uint32_t specific, uint32_t statusFieldWord)
{
	NVME_CPL_FIFO_REG nvmeReg;

	nvmeReg.cid = cid;
	nvmeReg.sqId = sqId;
	nvmeReg.specific = specific;
	nvmeReg.statusFieldWord = statusFieldWord;
	nvmeReg.cplType = ONLY_CPL_TYPE;

	// cpl_print("set_nvme_cpl\r\n");
	// cpl_printint(sqId);
	// cpl_printint(cid);
	// cpl_printint(specific);
	// cpl_printint(statusFieldWord);
	IO_WRITE32(NVME_CPL_FIFO_REG_ADDR, nvmeReg.dword[0]);
	IO_WRITE32((NVME_CPL_FIFO_REG_ADDR + 4), nvmeReg.dword[1]);
	IO_WRITE32((NVME_CPL_FIFO_REG_ADDR + 8), nvmeReg.dword[2]);
}

void set_io_sq(uint32_t ioSqIdx, uint32_t valid, uint32_t cqVector, uint32_t qSzie, uint32_t pcieBaseAddrL, uint32_t pcieBaseAddrH)
{
	NVME_IO_SQ_SET_REG nvmeReg;
	uint32_t addr;

	nvmeReg.valid = valid;
	nvmeReg.cqVector = cqVector;
	nvmeReg.sqSize = qSzie;
	nvmeReg.pcieBaseAddrL = pcieBaseAddrL;
	nvmeReg.pcieBaseAddrH = pcieBaseAddrH;

	addr = NVME_IO_SQ_SET_REG_ADDR + (ioSqIdx * 8);
	IO_WRITE32(addr, nvmeReg.dword[0]);
	IO_WRITE32((addr + 4), nvmeReg.dword[1]);
}

void set_io_cq(uint32_t ioCqIdx, uint32_t valid, uint32_t irqEn, uint32_t irqVector, uint32_t qSzie, uint32_t pcieBaseAddrL, uint32_t pcieBaseAddrH)
{
	NVME_IO_CQ_SET_REG nvmeReg;
	uint32_t addr;

	nvmeReg.valid = valid;
	nvmeReg.irqEn = irqEn;
	nvmeReg.irqVector = irqVector;
	nvmeReg.cqSize = qSzie;
	nvmeReg.pcieBaseAddrL = pcieBaseAddrL;
	nvmeReg.pcieBaseAddrH = pcieBaseAddrH;

	addr = NVME_IO_CQ_SET_REG_ADDR + (ioCqIdx * 8);
	IO_WRITE32(addr, nvmeReg.dword[0]);
	IO_WRITE32((addr + 4), nvmeReg.dword[1]);

}


void set_direct_tx_dma(uint32_t devAddr, uint32_t pcieAddrH, uint32_t pcieAddrL, uint32_t len)
{
	HOST_DMA_CMD_FIFO_REG hostDmaReg;

	if(!((len <= 0x1000) && ((pcieAddrL & 0x3) == 0))){
		cpl_print("set_direct_tx_dma error\n");
		while(1);
	} //modified
	
	hostDmaReg.devAddr = devAddr;
	hostDmaReg.pcieAddrL = pcieAddrL;
	hostDmaReg.pcieAddrH = pcieAddrH;
	
	hostDmaReg.dword[3] = 0;
	hostDmaReg.dmaType = HOST_DMA_DIRECT_TYPE;
	hostDmaReg.dmaDirection = HOST_DMA_TX_DIRECTION;
	hostDmaReg.dmaLen = len;

	IO_WRITE32(HOST_DMA_CMD_FIFO_REG_ADDR, hostDmaReg.dword[0]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 4), hostDmaReg.dword[1]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 8), hostDmaReg.dword[2]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 12), hostDmaReg.dword[3]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 16), hostDmaReg.dword[4]);//slot_modified

	g_hostDmaStatus.fifoTail.directDmaTx++;
	g_hostDmaStatus.directDmaTxCnt++;
}

void set_direct_rx_dma(uint32_t devAddr, uint32_t pcieAddrH, uint32_t pcieAddrL, uint32_t len)
{
	HOST_DMA_CMD_FIFO_REG hostDmaReg;

	if(!((len <= 0x1000) && ((pcieAddrL & 0x3) == 0)))
	{
		cpl_print("set_direct_rx_dma error\n");
		while(1);
	} //modified
	
	hostDmaReg.devAddr = devAddr;
	hostDmaReg.pcieAddrH = pcieAddrH;
	hostDmaReg.pcieAddrL = pcieAddrL;

	hostDmaReg.dword[3] = 0;
	hostDmaReg.dmaType = HOST_DMA_DIRECT_TYPE;
	hostDmaReg.dmaDirection = HOST_DMA_RX_DIRECTION;
	hostDmaReg.dmaLen = len;

	IO_WRITE32(HOST_DMA_CMD_FIFO_REG_ADDR, hostDmaReg.dword[0]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 4), hostDmaReg.dword[1]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 8), hostDmaReg.dword[2]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 12), hostDmaReg.dword[3]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 16), hostDmaReg.dword[4]);//slot_modified
	g_hostDmaStatus.fifoTail.directDmaRx++;
	g_hostDmaStatus.directDmaRxCnt++;

}

void set_auto_tx_dma(uint32_t cmdSlotTag, uint32_t cmd4KBOffset, uint32_t devAddr, uint32_t autoCompletion)
{
	HOST_DMA_CMD_FIFO_REG hostDmaReg;
	unsigned char tempTail;

	if(!(cmd4KBOffset < 256)){
		cpl_print("set_auto_tx_dma error\n");
		while(1);
	}
	
	g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);
	while((g_hostDmaStatus.fifoTail.autoDmaTx + 1) % 256 == g_hostDmaStatus.fifoHead.autoDmaTx)
		g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);

	hostDmaReg.devAddr = devAddr;

	hostDmaReg.dword[3] = 0;
	hostDmaReg.dmaType = HOST_DMA_AUTO_TYPE;
	hostDmaReg.dmaDirection = HOST_DMA_TX_DIRECTION;
	hostDmaReg.cmd4KBOffset = cmd4KBOffset;
	hostDmaReg.cmdSlotTag = cmdSlotTag;
	hostDmaReg.autoCompletion = autoCompletion;

	IO_WRITE32(HOST_DMA_CMD_FIFO_REG_ADDR, hostDmaReg.dword[0]);
	//IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 4), hostDmaReg.dword[1]);
	//IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 8), hostDmaReg.dword[2]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 12), hostDmaReg.dword[3]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 16), hostDmaReg.dword[4]);//slot_modified

	tempTail = g_hostDmaStatus.fifoTail.autoDmaTx++;
	if(tempTail > g_hostDmaStatus.fifoTail.autoDmaTx)
		g_hostDmaAssistStatus.autoDmaTxOverFlowCnt++;

	g_hostDmaStatus.autoDmaTxCnt++;
}

void set_auto_rx_dma(uint32_t cmdSlotTag, uint32_t cmd4KBOffset, uint32_t devAddr, uint32_t autoCompletion)
{
	HOST_DMA_CMD_FIFO_REG hostDmaReg;
	unsigned char tempTail;

	if(cmd4KBOffset < 256);
	
	g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);
	while((g_hostDmaStatus.fifoTail.autoDmaRx + 1) % 256 == g_hostDmaStatus.fifoHead.autoDmaRx)
		g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);

	hostDmaReg.devAddr = devAddr;

	hostDmaReg.dword[3] = 0;
	hostDmaReg.dmaType = HOST_DMA_AUTO_TYPE;
	hostDmaReg.dmaDirection = HOST_DMA_RX_DIRECTION;
	hostDmaReg.cmd4KBOffset = cmd4KBOffset;
	hostDmaReg.cmdSlotTag = cmdSlotTag;
	hostDmaReg.autoCompletion = autoCompletion;

	IO_WRITE32(HOST_DMA_CMD_FIFO_REG_ADDR, hostDmaReg.dword[0]);
	//IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 4), hostDmaReg.dword[1]);
	//IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 8), hostDmaReg.dword[2]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 12), hostDmaReg.dword[3]);
	IO_WRITE32((HOST_DMA_CMD_FIFO_REG_ADDR + 16), hostDmaReg.dword[4]);//slot_modified

	tempTail = g_hostDmaStatus.fifoTail.autoDmaRx++;
	if(tempTail > g_hostDmaStatus.fifoTail.autoDmaRx)
		g_hostDmaAssistStatus.autoDmaRxOverFlowCnt++;

	g_hostDmaStatus.autoDmaRxCnt++;
}

void check_direct_tx_dma_done()
{
	// cpl_print("fifoHead ");
	// cpl_printint(g_hostDmaStatus.fifoHead.directDmaTx);
	// cpl_print("fifoTail ");
	// cpl_printint(g_hostDmaStatus.fifoTail.directDmaTx);
	while(g_hostDmaStatus.fifoHead.directDmaTx != g_hostDmaStatus.fifoTail.directDmaTx)
	{
		g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);
	}
}

void check_direct_rx_dma_done()
{
	while(g_hostDmaStatus.fifoHead.directDmaRx != g_hostDmaStatus.fifoTail.directDmaRx)
	{
		g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);
	}
}

void check_auto_tx_dma_done()
{
	while(g_hostDmaStatus.fifoHead.autoDmaTx != g_hostDmaStatus.fifoTail.autoDmaTx)
	{
		g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);
	}
}

void check_auto_rx_dma_done()
{
	while(g_hostDmaStatus.fifoHead.autoDmaRx != g_hostDmaStatus.fifoTail.autoDmaRx)
	{
		g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);
	}
}

uint32_t check_auto_tx_dma_partial_done(uint32_t tailIndex, uint32_t tailAssistIndex)
{
	//cpl_print("check_auto_tx_dma_partial_done \r\n");

	g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);

	if(g_hostDmaStatus.fifoHead.autoDmaTx == g_hostDmaStatus.fifoTail.autoDmaTx)
		return 1;

	if(g_hostDmaStatus.fifoHead.autoDmaTx < tailIndex)
	{
		if(g_hostDmaStatus.fifoTail.autoDmaTx < tailIndex)
		{
			if(g_hostDmaStatus.fifoTail.autoDmaTx > g_hostDmaStatus.fifoHead.autoDmaTx)
				return 1;
			else
				if(g_hostDmaAssistStatus.autoDmaTxOverFlowCnt != (tailAssistIndex + 1))
					return 1;
		}
		else
			if(g_hostDmaAssistStatus.autoDmaTxOverFlowCnt != tailAssistIndex)
				return 1;

	}
	else if(g_hostDmaStatus.fifoHead.autoDmaTx == tailIndex)
		return 1;
	else
	{
		if(g_hostDmaStatus.fifoTail.autoDmaTx < tailIndex)
			return 1;
		else
		{
			if(g_hostDmaStatus.fifoTail.autoDmaTx > g_hostDmaStatus.fifoHead.autoDmaTx)
				return 1;
			else
				if(g_hostDmaAssistStatus.autoDmaTxOverFlowCnt != tailAssistIndex)
					return 1;
		}
	}

	return 0;
}

uint32_t check_auto_rx_dma_partial_done(uint32_t tailIndex, uint32_t tailAssistIndex)
{
	//cpl_print("check_auto_rx_dma_partial_done \r\n");

	g_hostDmaStatus.fifoHead.dword = IO_READ32(HOST_DMA_FIFO_CNT_REG_ADDR);

	if(g_hostDmaStatus.fifoHead.autoDmaRx == g_hostDmaStatus.fifoTail.autoDmaRx)
		return 1;

	if(g_hostDmaStatus.fifoHead.autoDmaRx < tailIndex)
	{
		if(g_hostDmaStatus.fifoTail.autoDmaRx < tailIndex)
		{
			if(g_hostDmaStatus.fifoTail.autoDmaRx > g_hostDmaStatus.fifoHead.autoDmaRx)
				return 1;
			else
				if(g_hostDmaAssistStatus.autoDmaRxOverFlowCnt != (tailAssistIndex + 1))
					return 1;
		}
		else
			if(g_hostDmaAssistStatus.autoDmaRxOverFlowCnt != tailAssistIndex)
				return 1;

	}
	else if(g_hostDmaStatus.fifoHead.autoDmaRx == tailIndex)
		return 1;
	else
	{
		if(g_hostDmaStatus.fifoTail.autoDmaRx < tailIndex)
			return 1;
		else
		{
			if(g_hostDmaStatus.fifoTail.autoDmaRx > g_hostDmaStatus.fifoHead.autoDmaRx)
				return 1;
			else
				if(g_hostDmaAssistStatus.autoDmaRxOverFlowCnt != tailAssistIndex)
					return 1;
		}
	}

	return 0;
}

#pragma GCC pop_options