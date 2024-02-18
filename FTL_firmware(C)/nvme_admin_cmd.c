//////////////////////////////////////////////////////////////////////////////////
// nvme_admin_cmd.c for Cosmos+ OpenSSD
// Copyright (c) 2016 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Youngjin Jo <yjjo@enc.hanyang.ac.kr>
//				  Sangjin Lee <sjlee@enc.hanyang.ac.kr>
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
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: NVMe Admin Command Handler
// File Name: nvme_admin_cmd.c
//
// Version: v1.0.0
//
// Description:
//   - handles NVMe admin command
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////


//#include "cpl_print.h"
#include "ariane.h"
// #include "string.h"
#include "io_access.h"

#include "nvme.h"
#include "host_lld.h"
#include "nvme_identify.h"
#include "nvme_admin_cmd.h"
#include <stdint.h>

extern NVME_CONTEXT g_nvmeTask;

uint32_t get_num_of_queue(uint32_t dword11)
{
	ADMIN_SET_FEATURES_NUMBER_OF_QUEUES_DW11 requested;
	ADMIN_SET_FEATURES_NUMBER_OF_QUEUES_COMPLETE allocated;

	requested.dword = dword11;
	// cpl_print("Number of IO Submission Queues Requested (NSQR, zero-based):\n");// requested.NSQR);
	// cpl_printint(requested.NSQR);
	// cpl_print("Number of IO Completion Queues Requested (NCQR, zero-based):\n");// requested.NCQR);
	// cpl_printint(requested.NCQR);

	//IO submission queue allocating
	if(requested.NSQR >= MAX_NUM_OF_IO_SQ)
		g_nvmeTask.numOfIOSubmissionQueuesAllocated = MAX_NUM_OF_IO_SQ;
	else
		g_nvmeTask.numOfIOSubmissionQueuesAllocated = requested.NSQR + 1;//zero-based -> non zero-based

	allocated.NSQA = g_nvmeTask.numOfIOSubmissionQueuesAllocated - 1;//non zero-based -> zero-based


	//IO completion queue allocating
	if(requested.NCQR >= MAX_NUM_OF_IO_CQ)
		g_nvmeTask.numOfIOCompletionQueuesAllocated = MAX_NUM_OF_IO_CQ;
	else
		g_nvmeTask.numOfIOCompletionQueuesAllocated = requested.NCQR + 1;//zero-based -> non zero-based

	allocated.NCQA = g_nvmeTask.numOfIOCompletionQueuesAllocated - 1;//non zero-based -> zero-based

	// cpl_print("Number of IO Submission Queues Allocated (NSQA, zero-based):\n");// allocated.NSQA);
	// cpl_printint(allocated.NSQA);
	// cpl_print("Number of IO Completion Queues Allocated (NCQA, zero-based):\n");// allocated.NCQA);
	// cpl_printint(allocated.NCQA);

	return allocated.dword;
}

void handle_set_features(NVME_ADMIN_COMMAND *nvmeAdminCmd, NVME_COMPLETION *nvmeCPL)
{
	ADMIN_SET_FEATURES_DW10 features;

	features.dword = nvmeAdminCmd->dword10;

	switch(features.FID)
	{
		case NUMBER_OF_QUEUES:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = get_num_of_queue(nvmeAdminCmd->dword11);
			break;
		}
		case INTERRUPT_COALESCING:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = 0x0;
			break;
		}
		case ARBITRATION:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = 0x0;
			break;
		}
		case ASYNCHRONOUS_EVENT_CONFIGURATION:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = 0x0;
			break;
		}
		case VOLATILE_WRITE_CACHE:
		{
			cpl_print("Set VWC: %X\r\n");// nvmeAdminCmd->dword11);
			g_nvmeTask.cacheEn = (nvmeAdminCmd->dword11 & 0x1);
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = 0x0;
			break;
		}
		case POWER_MANAGEMENT:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = 0x0;
			break;
		}
		case Timestamp:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = 0x0;
			break;
		}
		default:
		{
			cpl_print("Not Support FID (Set): %X\r\n");// features.FID);
			if(!0){
				cpl_print("handle_set_features error\n");
				while(1);
			}
			break;
		}
	}
	cpl_print("Set Feature FID:\n");// features.FID);
	cpl_printint(features.FID);
}

void handle_get_features(NVME_ADMIN_COMMAND *nvmeAdminCmd, NVME_COMPLETION *nvmeCPL)
{
	ADMIN_GET_FEATURES_DW10 features;
	NVME_COMPLETION cpl;

	features.dword = nvmeAdminCmd->dword10;

	switch(features.FID)
	{
		case LBA_RANGE_TYPE:
		{
			if(!(nvmeAdminCmd->NSID == 1)){
				cpl_print("handle_get_features error1\n");
				while(1);
			}

			cpl.dword[0] = 0x0;
			cpl.statusField.SC = SC_INVALID_FIELD_IN_COMMAND;
			nvmeCPL->dword[0] = cpl.dword[0];
			nvmeCPL->specific = 0x0;
			break;
		}
		case TEMPERATURE_THRESHOLD:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = nvmeAdminCmd->dword11;
			break;
		}
		case VOLATILE_WRITE_CACHE:
		{
			
			cpl_print("Get VWC: %X\r\n");// g_nvmeTask.cacheEn);
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = g_nvmeTask.cacheEn;
			break;
		}
		case POWER_MANAGEMENT:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = 0x0;
			break;
		}
		case Power_State_Transition:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = 0x0;
			break;
		}
		case 0xD0:
		{
			nvmeCPL->dword[0] = 0x0;
			nvmeCPL->specific = 0x0;
			break;
		}
		default:
		{
			cpl_print("Not Support FID (Get): %X\r\n");// features.FID);
			if(!0){
				cpl_print("handle_get_features error2\n");
				while(1);
			}
			break;
		}
	}
	cpl_print("Get Feature FID:%X\r\n");// features.FID);
}

void handle_create_io_sq(NVME_ADMIN_COMMAND *nvmeAdminCmd, NVME_COMPLETION *nvmeCPL)
{
	ADMIN_CREATE_IO_SQ_DW10 sqInfo10;
	ADMIN_CREATE_IO_SQ_DW11 sqInfo11;
	NVME_IO_SQ_STATUS *ioSqStatus;
	uint32_t ioSqIdx;

	sqInfo10.dword = nvmeAdminCmd->dword10;
	sqInfo11.dword = nvmeAdminCmd->dword11;

	// cpl_print("create sq: 0x%08X, 0x%08X\r\n");// sqInfo11.dword, sqInfo10.dword);
	/*cpl_print("QID : 0x%08X, QSIZE : 0x%08X\r\n");// sqInfo10.QID, sqInfo10.QSIZE);
	cpl_print("PC : 0x%08X, QPRIO : 0x%08X, CQID : 0x%08X\r\n");// sqInfo11.PC, sqInfo11.QPRIO,sqInfo11.CQID);
	cpl_print("pcieBaseAddrL : 0x%08X, pcieBaseAddrH : 0x%08X\r\n");// ioSqStatus->pcieBaseAddrL, ioSqStatus->pcieBaseAddrH);*/

	if(!((nvmeAdminCmd->PRP1[0] & 0x3) == 0 && nvmeAdminCmd->PRP1[1] < 0x10000)){
		cpl_print("handle_create_io_sq error1\n");
		while(1);
	}
	if(!(0 < sqInfo10.QID && sqInfo10.QID <= 8 && sqInfo10.QSIZE < 0x100 && 0 < sqInfo11.CQID && sqInfo11.CQID <= 8)){
		cpl_print("handle_create_io_sq error2\n");
		while(1);
	}

	ioSqIdx = sqInfo10.QID - 1;
	ioSqStatus = g_nvmeTask.ioSqInfo + ioSqIdx;

	ioSqStatus->valid = 1;
	ioSqStatus->qSzie = sqInfo10.QSIZE;
	ioSqStatus->cqVector = sqInfo11.CQID;
	ioSqStatus->pcieBaseAddrL = nvmeAdminCmd->PRP1[0];
	ioSqStatus->pcieBaseAddrH = nvmeAdminCmd->PRP1[1];

	set_io_sq(ioSqIdx, ioSqStatus->valid, ioSqStatus->cqVector, ioSqStatus->qSzie, ioSqStatus->pcieBaseAddrL, ioSqStatus->pcieBaseAddrH);

	nvmeCPL->dword[0] = 0;
	nvmeCPL->specific = 0x0;

}

void handle_delete_io_sq(NVME_ADMIN_COMMAND *nvmeAdminCmd, NVME_COMPLETION *nvmeCPL)
{
	ADMIN_DELETE_IO_SQ_DW10 sqInfo10;
	NVME_IO_SQ_STATUS *ioSqStatus;
	uint32_t ioSqIdx;

	sqInfo10.dword = nvmeAdminCmd->dword10;

	cpl_print("delete sq: 0x%08X\r\n");// sqInfo10.dword);

	ioSqIdx = (uint32_t)sqInfo10.QID - 1;
	ioSqStatus = g_nvmeTask.ioSqInfo + ioSqIdx;

	ioSqStatus->valid = 0;
	ioSqStatus->cqVector = 0;
	ioSqStatus->qSzie = 0;
	ioSqStatus->pcieBaseAddrL = 0;
	ioSqStatus->pcieBaseAddrH = 0;

	set_io_sq(ioSqIdx, 0, 0, 0, 0, 0);

	nvmeCPL->dword[0] = 0;
	nvmeCPL->specific = 0x0;
}


void handle_create_io_cq(NVME_ADMIN_COMMAND *nvmeAdminCmd, NVME_COMPLETION *nvmeCPL)
{
	ADMIN_CREATE_IO_CQ_DW10 cqInfo10;
	ADMIN_CREATE_IO_CQ_DW11 cqInfo11;
	NVME_IO_CQ_STATUS *ioCqStatus;
	uint32_t ioCqIdx;

	cqInfo10.dword = nvmeAdminCmd->dword10;
	cqInfo11.dword = nvmeAdminCmd->dword11;

	// cpl_print("create cq: 0x%08X, 0x%08X\r\n");// cqInfo11.dword, cqInfo10.dword);

	if(!(((nvmeAdminCmd->PRP1[0] & 0x3) == 0) && (nvmeAdminCmd->PRP1[1] < 0x10000))){
		cpl_print("handle_create_io_cq error1\n");
		while(1);
	}
	if(!(cqInfo11.IV < 8 && cqInfo10.QSIZE < 0x100 && 0 < cqInfo10.QID && cqInfo10.QID <= 8)){
		cpl_print("handle_create_io_cq error2\n");
		while(1);
	}

	ioCqIdx = cqInfo10.QID - 1;
	ioCqStatus = g_nvmeTask.ioCqInfo + ioCqIdx;

	ioCqStatus->valid = 1;
	ioCqStatus->qSzie = cqInfo10.QSIZE;
	ioCqStatus->irqEn = cqInfo11.IEN;
	ioCqStatus->irqVector = cqInfo11.IV;
	ioCqStatus->pcieBaseAddrL = nvmeAdminCmd->PRP1[0];
	ioCqStatus->pcieBaseAddrH = nvmeAdminCmd->PRP1[1];

	set_io_cq(ioCqIdx, ioCqStatus->valid, ioCqStatus->irqEn, ioCqStatus->irqVector, ioCqStatus->qSzie, ioCqStatus->pcieBaseAddrL, ioCqStatus->pcieBaseAddrH);

	nvmeCPL->dword[0] = 0;
	nvmeCPL->specific = 0x0;
}

void handle_delete_io_cq(NVME_ADMIN_COMMAND *nvmeAdminCmd, NVME_COMPLETION *nvmeCPL)
{
	ADMIN_DELETE_IO_CQ_DW10 cqInfo10;
	NVME_IO_CQ_STATUS *ioCqStatus;
	uint32_t ioCqIdx;

	cqInfo10.dword = nvmeAdminCmd->dword10;

	cpl_print("delete cq: 0x%08X\r\n");// cqInfo10.dword);

	ioCqIdx = (uint32_t)cqInfo10.QID - 1;
	ioCqStatus = g_nvmeTask.ioCqInfo + ioCqIdx;

	ioCqStatus->valid = 0;
	ioCqStatus->irqVector = 0;
	ioCqStatus->qSzie = 0;
	ioCqStatus->pcieBaseAddrL = 0;
	ioCqStatus->pcieBaseAddrH = 0;
	
	set_io_cq(ioCqIdx, 0, 0, 0, 0, 0, 0);

	nvmeCPL->dword[0] = 0;
	nvmeCPL->specific = 0x0;
}

void handle_identify(NVME_ADMIN_COMMAND *nvmeAdminCmd, NVME_COMPLETION *nvmeCPL)
{
	ADMIN_IDENTIFY_COMMAND_DW10 identifyInfo;
	uint32_t pIdentifyData = ADMIN_CMD_DRAM_DATA_BUFFER;
	uint32_t prp[2];
	uint32_t prpLen;

	identifyInfo.dword = nvmeAdminCmd->dword10;

	// cpl_print("handle_identify0");
	if(identifyInfo.CNS == 1)
	{
		// cpl_print("handle_identify1");
		if((nvmeAdminCmd->PRP1[0] & 0x3) != 0 || (nvmeAdminCmd->PRP2[0] & 0x3) != 0)
			cpl_print("CI: %X, %X, %X, %X\r\n");// nvmeAdminCmd->PRP1[1], nvmeAdminCmd->PRP1[0], nvmeAdminCmd->PRP2[1], nvmeAdminCmd->PRP2[0]);

		if(!((nvmeAdminCmd->PRP1[0] & 0x3) == 0 && (nvmeAdminCmd->PRP2[0] & 0x3) == 0)){
		cpl_print("handle_identify error1\n");
		while(1);
		}
		
		identify_controller(pIdentifyData);
		// cpl_print("handle_identify2");
	}
	else if(identifyInfo.CNS == 0)
	{
		// cpl_print("handle_identify3");
		if((nvmeAdminCmd->PRP1[0] & 0x3) != 0 || (nvmeAdminCmd->PRP2[0] & 0x3) != 0)
			cpl_print("NI: %X, %X, %X, %X\r\n");// nvmeAdminCmd->PRP1[1], nvmeAdminCmd->PRP1[0], nvmeAdminCmd->PRP2[1], nvmeAdminCmd->PRP2[0]);
		//ASSERT(nvmeAdminCmd->NSID == 1);
		if(!((nvmeAdminCmd->PRP1[0] & 0x3) == 0 && (nvmeAdminCmd->PRP2[0] & 0x3) == 0)){
			cpl_print("handle_identify error2\n");
			while(1);
		}
		identify_namespace(pIdentifyData);
		// cpl_print("handle_identify4");
	}
	else
		if(!0){
			cpl_print("handle_identify error3\n");
			while(1);
		}
	
	prp[0] = nvmeAdminCmd->PRP1[0];
	prp[1] = nvmeAdminCmd->PRP1[1];

	// cpl_print("nvmeAdminCmd->PRP1[0]:\n");
	// cpl_printint(nvmeAdminCmd->PRP1[0]);

	// cpl_print("nvmeAdminCmd->PRP1[1]:\n");
	// cpl_printint(nvmeAdminCmd->PRP1[1]);

	// cpl_print("nvmeAdminCmd->PRP2[0]:\n");
	// cpl_printint(nvmeAdminCmd->PRP2[0]);

	// cpl_print("nvmeAdminCmd->PRP2[1]:\n");
	// cpl_printint(nvmeAdminCmd->PRP2[1]);

	// cpl_print("fifoHead A ");
	// cpl_printint(g_hostDmaStatus.fifoHead.directDmaTx);
	// cpl_print("fifoTail A ");
	// cpl_printint(g_hostDmaStatus.fifoTail.directDmaTx);

	prpLen = 0x1000 - (prp[0] & 0xFFF);
//	cpl_print("prpLen = %X, prp[1] = %X, prp[0] = %X\r\n");//prpLen, prp[1], prp[0]);
	// cpl_print("handle_identify5");
	set_direct_tx_dma(pIdentifyData, prp[1], prp[0], prpLen);
	// cpl_print("handle_identify6");

	// cpl_print("fifoHead B ");
	// cpl_printint(g_hostDmaStatus.fifoHead.directDmaTx);
	// cpl_print("fifoTail B ");
	// cpl_printint(g_hostDmaStatus.fifoTail.directDmaTx);

	if(prpLen != 0x1000)
	{
		pIdentifyData = pIdentifyData + prpLen;
		prpLen = 0x1000 - prpLen;
		prp[0] = nvmeAdminCmd->PRP2[0];
		prp[1] = nvmeAdminCmd->PRP2[1];

//		ASSERT((prp[1] & 0xFFF) == 0);
//		cpl_print("prpLen = %X, prp[1] = %X, prp[0] = %X\r\n");//prpLen, prp[1], prp[0]);
		// cpl_print("handle_identify7");
		set_direct_tx_dma(pIdentifyData, prp[1], prp[0], prpLen);
//		cpl_print("handle_identify8");

		// cpl_print("fifoHead C ");
		// cpl_printint(g_hostDmaStatus.fifoHead.directDmaTx);
		// cpl_print("fifoTail C ");
		// cpl_printint(g_hostDmaStatus.fifoTail.directDmaTx);

	}

//	cpl_print("handle_identify9");
	check_direct_tx_dma_done();
//	cpl_print("handle_identify10");
	nvmeCPL->dword[0] = 0;
	nvmeCPL->specific = 0x0;
}

void handle_get_log_page(NVME_ADMIN_COMMAND *nvmeAdminCmd, NVME_COMPLETION *nvmeCPL)
{
	//ADMIN_GET_LOG_PAGE_DW10 getLogPageInfo;

	//uint32_t prp1[2];
	//uint32_t prp2[2];
	//uint32_t prpLen;

	//getLogPageInfo.dword = nvmeAdminCmd->dword10;

	//prp1[0] = nvmeAdminCmd->PRP1[0];
	//prp1[1] = nvmeAdminCmd->PRP1[1];
	//prpLen = 0x1000 - (prp1[0] & 0xFFF);

	//prp2[0] = nvmeAdminCmd->PRP2[0];
	//prp2[1] = nvmeAdminCmd->PRP2[1];

	//cpl_print("ADMIN GET LOG PAGE\n");

	//LID
	//Mandatory//1-Error information, 2-SMART/Health information, 3-Firmware Slot information
	//Optional//4-ChangedNamespaceList, 5-Command Effects Log
	//cpl_print("LID: 0x%X, NUMD: 0x%X \r\n");// getLogPageInfo.LID, getLogPageInfo.NUMD);

	//cpl_print("PRP1[63:32] = 0x%X, PRP1[31:0] = 0x%X", prp1[1], prp1[0]);
	//cpl_print("PRP2[63:32] = 0x%X, PRP2[31:0] = 0x%X", prp2[1], prp2[0]);

	nvmeCPL->dword[0] = 0;
	nvmeCPL->specific = 0x9;//invalid log page
}

void handle_nvme_admin_cmd(NVME_COMMAND *nvmeCmd)
{	NVME_ADMIN_COMMAND *nvmeAdminCmd;
	NVME_COMPLETION 		nvmeCPL;
	uint32_t 						opc;
	uint32_t 						needCpl;
	uint32_t 						needSlotRelease;

	nvmeAdminCmd = (NVME_ADMIN_COMMAND*)nvmeCmd->cmdDword; // nvmeAdminCmd == cmdDword의 주소
	opc = (uint32_t)nvmeAdminCmd->OPC;


	needCpl = 1;
	needSlotRelease = 0;

	switch(opc)
	{
		case ADMIN_SET_FEATURES:
		{
			handle_set_features(nvmeAdminCmd, &nvmeCPL);
			break;
		}
		case ADMIN_CREATE_IO_CQ:
		{
			handle_create_io_cq(nvmeAdminCmd, &nvmeCPL);
			break;
		}
		case ADMIN_CREATE_IO_SQ:
		{
			handle_create_io_sq(nvmeAdminCmd, &nvmeCPL);
			break;
		}
		case ADMIN_IDENTIFY:
		{
			handle_identify(nvmeAdminCmd, &nvmeCPL);
			break;
		}
		case ADMIN_GET_FEATURES:
		{
			handle_get_features(nvmeAdminCmd, &nvmeCPL);
			break;
		}
		case ADMIN_DELETE_IO_CQ:
		{
			handle_delete_io_cq(nvmeAdminCmd, &nvmeCPL);
			break;
		}
		case ADMIN_DELETE_IO_SQ:
		{
			handle_delete_io_sq(nvmeAdminCmd, &nvmeCPL);
			break;
		}
		case ADMIN_ASYNCHRONOUS_EVENT_REQUEST:
		{
			needCpl = 0;
			needSlotRelease = 1;
			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			break;
		}
		case ADMIN_GET_LOG_PAGE:
		{
			handle_get_log_page(nvmeAdminCmd, &nvmeCPL);
			break;
		}
		case ADMIN_SECURITY_RECEIVE:
		{
			needCpl = 0;
			needSlotRelease = 0;
			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			break;
		}
		case ADMIN_DOORBELL_BUFFER_CONFIG:
		{
			needCpl = 0;
			needSlotRelease = 0;
			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			break;
		}
		case 0x08: // ADMIN_ABORT
		{
			needCpl = 1;
			needSlotRelease = 0;
			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			break;
		}
		default:
		{
			cpl_print("Not Support Admin Command OPC: \n");// opc);
			cpl_printint(opc);
			if(!0){
				cpl_print("handle_nvme_admin_cmd error\n");
				while(1); //
			}
			break;
		}
	}

	if(needCpl == 1)
		set_auto_nvme_cpl(nvmeCmd->cmdSlotTag, nvmeCPL.specific, nvmeCPL.statusFieldWord);
	else if(needSlotRelease == 1)
		set_nvme_slot_release(nvmeCmd->cmdSlotTag);
	else
		set_nvme_cpl(nvmeCmd->qID, nvmeAdminCmd->CID, nvmeCPL.specific, nvmeCPL.statusFieldWord);

	// cpl_print("Done Admin Command OPC:\n");// opc);
	cpl_printint(opc);

}

