//////////////////////////////////////////////////////////////////////////////////
// nvme_io_cmd.c for Cosmos+ OpenSSD
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
// Module Name: NVMe IO Command Handler
// File Name: nvme_io_cmd.c
//
// Version: v1.0.1
//
// Description:
//   - handles NVMe IO command
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.1
//   - header file for buffer is changed from "ia_lru_buffer.h" to "lru_buffer.h"
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#pragma GCC push_options
#pragma GCC optimize ("O0")

//#include "cpl_print.h"
#include "ariane.h"
#include "io_access.h"

#include "nvme.h"
#include "host_lld.h"
#include "nvme_io_cmd.h"

#include "ftl_config.h"
#include "request_transform.h"
#include <stdint.h>

extern uint32_t table_logic_minus_count;

void handle_nvme_io_read(uint32_t cmdSlotTag, NVME_IO_COMMAND *nvmeIOCmd)
{
	IO_READ_COMMAND_DW12 readInfo12;
	//IO_READ_COMMAND_DW13 readInfo13;
	//IO_READ_COMMAND_DW15 readInfo15;
	uint32_t startLba[2];
	uint32_t nlb;

	readInfo12.dword = nvmeIOCmd->dword[12];
	//readInfo13.dword = nvmeIOCmd->dword[13];
	//readInfo15.dword = nvmeIOCmd->dword[15];

	startLba[0] = nvmeIOCmd->dword[10];
	startLba[1] = nvmeIOCmd->dword[11];
	nlb = readInfo12.NLB;

	// if(table_logic_minus_count == 101){
	// 	cpl_printint(startLba[0]);
	// 	// cpl_printint(nlb);
	// 	// cpl_print("\r\n");
	// }

	if(!(startLba[0] < storageCapacity_L && (startLba[1] < STORAGE_CAPACITY_H || startLba[1] == 0))){
		cpl_print("handle_nvme_io_read error1\n");
		while(1);
	}
	//ASSERT(nlb < MAX_NUM_OF_NLB);
	if(!((nvmeIOCmd->PRP1[0] & 0x3) == 0 && (nvmeIOCmd->PRP2[0] & 0x3) == 0)){
		cpl_print("handle_nvme_io_read error2\n");
		while(1);
	} 
	if(!(nvmeIOCmd->PRP1[1] < 0x10000 && nvmeIOCmd->PRP2[1] < 0x10000)){
		cpl_print("handle_nvme_io_read error3\n");
		while(1);
	}
	
	ReqTransNvmeToSlice(cmdSlotTag, startLba[0], nlb, IO_NVM_READ);
}


void handle_nvme_io_write(uint32_t cmdSlotTag, NVME_IO_COMMAND *nvmeIOCmd)
{
	IO_READ_COMMAND_DW12 writeInfo12;
	//IO_READ_COMMAND_DW13 writeInfo13;
	//IO_READ_COMMAND_DW15 writeInfo15;
	uint32_t startLba[2];
	uint32_t nlb;

	writeInfo12.dword = nvmeIOCmd->dword[12];
	//writeInfo13.dword = nvmeIOCmd->dword[13];
	//writeInfo15.dword = nvmeIOCmd->dword[15];

	//if(writeInfo12.FUA == 1)
	//	cpl_print("write FUA\r\n");

	startLba[0] = nvmeIOCmd->dword[10];
	startLba[1] = nvmeIOCmd->dword[11];
	nlb = writeInfo12.NLB;

	if(!(startLba[0] < storageCapacity_L && (startLba[1] < STORAGE_CAPACITY_H || startLba[1] == 0))){
		cpl_print("handle_nvme_io_write error1\n");
		while(1);
	}
	//ASSERT(nlb < MAX_NUM_OF_NLB);
	if(!((nvmeIOCmd->PRP1[0] & 0xF) == 0 && (nvmeIOCmd->PRP2[0] & 0xF) == 0)){
		cpl_print("handle_nvme_io_write error2\n");
		while(1);
	}
	if(!(nvmeIOCmd->PRP1[1] < 0x10000 && nvmeIOCmd->PRP2[1] < 0x10000)){
		cpl_print("handle_nvme_io_write error3\n");
		while(1);
	}
	// cpl_print("2\r\n");
	ReqTransNvmeToSlice(cmdSlotTag, startLba[0], nlb, IO_NVM_WRITE);
}

void handle_nvme_error_insection(uint32_t cmdSlotTag, NVME_IO_COMMAND *nvmeIOCmd)
{
	
	unsigned int reg_num = nvmeIOCmd->dword[10];
	unsigned int bit_num = nvmeIOCmd->dword[11];
	unsigned int count_num = nvmeIOCmd->dword[12];

	cpl_Out32(DATA_ADDRESS_REGNUM, reg_num); // data0 1 ??
	cpl_Out32(DATA_ADDRESS_BITNUM, bit_num); // data1 mod 32 // reg_num
	cpl_Out32(DATA_ADDRESS_COUNT, count_num); // ?
	cpl_Out32(CMD_ADDRESS, 0x1); //command
	cpl_Out32(CMD_DONE_ADDRESS, 0); //CMD_DATA

	// from kjh
	// cpl_Out32(DATA_ADDRESS_REGNUM, count_num); // ?
	// cpl_Out32(CMD_ADDRESS, 0x2); //command
	// cpl_Out32(CMD_DONE_ADDRESS, 0); //CMD_DATA
}

void handle_nvme_io_cmd(NVME_COMMAND *nvmeCmd)
{
	NVME_IO_COMMAND *nvmeIOCmd;
	NVME_COMPLETION nvmeCPL;
	uint32_t opc;
	nvmeIOCmd = (NVME_IO_COMMAND*)nvmeCmd->cmdDword;
	/*		cpl_print("OPC = 0x%X\r\n");// nvmeIOCmd->OPC);
			cpl_print("PRP1[63:32] = 0x%X, PRP1[31:0] = 0x%X\r\n");// nvmeIOCmd->PRP1[1], nvmeIOCmd->PRP1[0]);
			cpl_print("PRP2[63:32] = 0x%X, PRP2[31:0] = 0x%X\r\n");// nvmeIOCmd->PRP2[1], nvmeIOCmd->PRP2[0]);
			cpl_print("dword10 = 0x%X\r\n");// nvmeIOCmd->dword10);
			cpl_print("dword11 = 0x%X\r\n");// nvmeIOCmd->dword11);
			cpl_print("dword12 = 0x%X\r\n");// nvmeIOCmd->dword12);*/


	opc = (uint32_t)nvmeIOCmd->OPC;
	// cpl_print("3\r\n");
	switch(opc)
	{
		case IO_NVM_FLUSH:
		{
			// cpl_print("IO Flush Command\r\n");
			nvmeCPL.dword[0] = 0;
			nvmeCPL.specific = 0x0;
			set_auto_nvme_cpl(nvmeCmd->cmdSlotTag, nvmeCPL.specific, nvmeCPL.statusFieldWord);
			break;
		}
		case IO_NVM_WRITE:
		{
			// cpl_print("IO Write Command\r\n");
			handle_nvme_io_write(nvmeCmd->cmdSlotTag, nvmeIOCmd);
			break;
		}
		case IO_NVM_READ:
		{
			// cpl_print("IO Read Command\r\n");
			handle_nvme_io_read(nvmeCmd->cmdSlotTag, nvmeIOCmd);
			break;
		}
		case ERROR_INJECTION:
		{
			// cpl_print("IO Inject Command\r\n");
			handle_nvme_error_insection(nvmeCmd->cmdSlotTag, nvmeIOCmd);

			// //complete logic
			// uint32_t ENTRY_LIST_TABLE_START_ADDRESS = 0x17FDFF00;
			// uint32_t list_entry_num = cpl_In32(0x17FFFFF4);
			// uint32_t address_tmp = 0x17FDFF00;
			// NVME_ADMIN_COMMAND *nvmeAdminCmd;
			// nvmeAdminCmd = (NVME_ADMIN_COMMAND*)nvmeCmd->cmdDword;
			// while(1){
				
			// 	uint32_t tmp_qID = cpl_In32(address_tmp);
			// 	uint32_t tmp_cID = cpl_In32(address_tmp+4);
			// 	uint32_t tmp_valid = cpl_In32(address_tmp+16);
			// 	// uint32_t tmp_cnt = cpl_In32(address_tmp+20);

			// 	if((tmp_qID == nvmeCmd->qID) && (tmp_cID == nvmeAdminCmd->CID) && (tmp_valid == 1)){
					
			// 		// tmp_cnt +=1;
			// 		// if(tmp_cnt == 4){ //SIZE!! (1MB/4KB)
			// 			list_entry_num -=1;
			// 			cpl_Out32(0x17FFFFF4,list_entry_num);

			// 			cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS,0);
			// 			cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS+4,0);
			// 			cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS+8,0);
			// 			cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS+12,0);
			// 			cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS+16,0);
			// 			// cpl_Out(ENTRY_LIST_TABLE_START_ADDRESS+20,0);
			// 			break;
			// 		// }
			// 		// cpl_Out32(address_tmp+20,tmp_cnt);
			// 		// break;
			// 	}
				
			// 	address_tmp += 20;
			// 	ENTRY_LIST_TABLE_START_ADDRESS += 20;
			// }

			set_auto_nvme_cpl(nvmeCmd->cmdSlotTag, 0, 0);
			break;
		}
		// case ERROR_INJECTION_TIMING:
		// {
		// 			//complete logic
		// 	uint32_t ENTRY_LIST_TABLE_START_ADDRESS = 0x17FDFF00; //nuk nuk
		// 	if(cpl_In32(0x17FFFFF0) == 85465){ //reset flag num
		// 		uint32_t *ENTRY_LIST_TABLE_START_POINTER = 0x17FDFF00;
		// 		uint32_t list_entry_num = cpl_In32(0x17FFFFF4);

		// 		while(list_entry_num){
		// 			if(cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS+16)==1){
		// 				list_entry_num -=1;

		// 				cpl_print("set_nvme_cpl : ");
		// 				cpl_printint_no_enter(cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS));
		// 				cpl_printint_no_enter(cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS+4));
		// 				cpl_print("\r\n");

		// 				set_nvme_cpl(	cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS),
		// 								cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS+4),
		// 								0,
		// 								0
		// 							);

		// 				cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS,0);
		// 				cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS+4,0);
		// 				cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS+8,0);
		// 				cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS+12,0);
		// 				cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS+16,0);
		// 				// cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS+20,0);
		// 			}
		// 			ENTRY_LIST_TABLE_START_ADDRESS+=20;
		// 			ENTRY_LIST_TABLE_START_POINTER = ENTRY_LIST_TABLE_START_ADDRESS;
		// 		}
		// 		cpl_Out32(0x17FFFFF4,list_entry_num);
		// 	}
		// 	break;
		// }
		default:
		{
			cpl_print("Not Support IO Command OPC: %X\r\n");// opc);
			if(!0){
				cpl_print("handle_nvme_io_cmd error\n");
				while(1);
			}
			break;
		}
	}
}

#pragma GCC pop_options