
#include "ariane.h"
#include "io_access.h"
#include "nvme.h"
#include "host_lld.h"
#include "nvme_main.h"
#include "nvme_admin_cmd.h"
#include "nvme_io_cmd.h"
#include "memory_map.h"
#include <stdint.h>

volatile 	NVME_CONTEXT 	g_nvmeTask;
					uint32_t 			table_logic_minus_count;
extern 		uint32_t 			rx_dma_count;
extern 		uint32_t 			tx_dma_count;
// extern uint32_t logical_address;
void nvme_main()
{
	uint32_t exeLlr;
	uint32_t rstCnt = 0;

	cpl_print("!!! Wait until FTL reset complete !!! \r\n");

	//complete logic begin
	uint32_t ENTRY_LIST_TABLE_START_ADDRESS = 0x17FDFF00; 											// varible declaration
	if(cpl_In32(0x17FFFFF0) == 85465){ 																					// value in the 0x17FFFFF0 address is 85465(reset_flag)
		uint32_t list_entry_num = cpl_In32(0x17FFFFF4);														// list_entry_num 을 선헌하며 0x17FFFFF4 에 있는 값을 저장.
		while(list_entry_num){ 																										// if list_entry_num != 0 keep going
			if(cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS + 8) == 1){ 									// start address 부터 8byte를 이동하고, 해당 값이 1이면
				list_entry_num -=1; 																									// list_entry_num 에서 1을 뺌.
				set_auto_nvme_cpl(cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS),0x0,0x0);
				cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS, 0);													// start_address에 0을 write
				cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS + 4, 0);											// start_address에서 4byte를 이동하고, 해당 주소공간에 0을 writ
				cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS + 8, 0); 										// start_address에서 8byte를 이동하고, 해당 주소공간에 0을 write
			}
			ENTRY_LIST_TABLE_START_ADDRESS += 12; 																	// start_address를 12byte 이동
		}																																					// end of while, endless loop before (list_entry_num == 0)
		cpl_Out32(0x17FFFFF4, list_entry_num);																		// 0x17FFFFF4 에 list_entry_num 값을 write(list_entry_num == 0)
	}

	InitFTL(); 																																// initilize ftl components
	cpl_print("Turn on the host PC \r\n");  


	while(1)
	{		
		exeLlr = 1;
/*1. NVME_TASK_WAIT_CC_EN (CC (Controller Configuration) 등록을 기다림)
NVMe 컨트롤러가 초기화되고, 호스트에 의해 활성화될 준비가 되었는지 확인합니다 (check_nvme_cc_en 함수).
CC 등록이 활성화되면 (즉, ccEn == 1), 관리 큐를 활성화하고, 컨트롤러 상태를 준비 완료(NVME_TASK_RUNNING)로 변경합니다.
리셋 플래그 주소에 특정 값을 기록하여 시스템이 리셋되었음을 나타냅니다.
*/
		if(g_nvmeTask.status == NVME_TASK_WAIT_CC_EN){
			uint32_t ccEn;
			ccEn = check_nvme_cc_en(); // status of nvme
			if(ccEn == 1){
				set_nvme_admin_queue(1, 1, 1); // 
				set_nvme_csts_rdy(1);
				g_nvmeTask.status = NVME_TASK_RUNNING; 	// update nvme status to running

				if(cpl_In32(0x17FFFFF0) == 0){ 					// address of reset_flag
					cpl_Out32(0x17FFFFF0, 85465); 				// write 85465 to reset_flag address
				}
				cpl_print("\r\nNVMe ready!!!\r\n");
			}
		}else if(g_nvmeTask.status == NVME_TASK_RUNNING /*&& table_logic_minus_count < 700*/)
		{
			NVME_COMMAND 	nvmeCmd;
			uint32_t 			cmdValid;
			cmdValid = get_nvme_cmd(&nvmeCmd.qID, &nvmeCmd.cmdSlotTag, &nvmeCmd.cmdSeqNum, nvmeCmd.cmdDword);
			// table entry insert logic begin
			NVME_ADMIN_COMMAND *nvmeAdminCmd; // nvmeAdminCmd pointer 변수 선언
			nvmeAdminCmd = (NVME_ADMIN_COMMAND*)nvmeCmd.cmdDword; // nvmeCmd.cmdDword 의 값을 nvmeAdminCmd의 주소로 저장

			uint32_t list_entry_num = cpl_In32(0x17FFFFF4); // list_entry_num 을 선헌하며 0x17FFFFF4 에 있는 값을 저장.
			if((cmdValid == 1) && (nvmeCmd.qID == 1) && ((nvmeAdminCmd->OPC == 0x1) || (nvmeAdminCmd->OPC == 0x2))){ 
				// 해당 조건이 충족되면
					uint32_t ENTRY_LIST_TABLE_START_ADDRESS = 0x17FDFF00; // address 지정
					uint32_t SLOT_TAG_TABLE_START_ADDRESS 	= 0x17FBDF00; // address 지정
					while(1){
						if(cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS + 8) == 0){// start address 부터 8byte를 이동하고, 해당 주소의 값이 0이면
							cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS, nvmeCmd.cmdSlotTag/*nvmeCmd.qID*/);					//qID
							cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS + 4, nvmeAdminCmd->CID);		//CID
							cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS + 8, 1);	//valid
							list_entry_num++;
							cpl_Out32(0x17FFFFF4,list_entry_num);
							break;
						}
						ENTRY_LIST_TABLE_START_ADDRESS += 12;
					}
				}
			if(cmdValid == 1){
				rstCnt = 0;
				if(nvmeCmd.qID == 0){ // if qID is admin queue
					handle_nvme_admin_cmd(&nvmeCmd);
				}else{ 
					handle_nvme_io_cmd(&nvmeCmd);
					ReqTransSliceToLowLevel(&nvmeCmd);
					exeLlr=0;
					NVME_ADMIN_COMMAND *nvmeAdminCmd;
					nvmeAdminCmd = (NVME_ADMIN_COMMAND*)nvmeCmd.cmdDword;

					list_entry_num = cpl_In32(0x17FFFFF4);

					if(list_entry_num > 0){
						if((nvmeAdminCmd->OPC == 0x1) || (nvmeAdminCmd->OPC == 0x2)){ // || nvmeAdminCmd->OPC == 0x44
							uint32_t ENTRY_LIST_TABLE_START_ADDRESS = 0x17FDFF00;
							uint32_t SLOT_TAG_TABLE_START_ADDRESS 	= 0x17FBDF00;
							while(list_entry_num){
								if(cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS + 8) == 1){
									uint32_t temp_slot_tag = cpl_In32(ENTRY_LIST_TABLE_START_ADDRESS);//qID
									set_auto_nvme_cpl(temp_slot_tag, 0x0, 0x0);
									list_entry_num--;
									table_logic_minus_count++;
									cpl_Out32(0x17FFFFF4,list_entry_num); // list_entry_num 을 0x17FFFFF4 에 write
									cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS, 0); 
									cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS + 4, 0);
									cpl_Out32(ENTRY_LIST_TABLE_START_ADDRESS + 8, 0);
									// break;
								}
								ENTRY_LIST_TABLE_START_ADDRESS += 12;
							}
						}
					}
				}	
			}
		}
		else if(g_nvmeTask.status == NVME_TASK_SHUTDOWN) // shutdown
		{
			NVME_STATUS_REG nvmeReg;
			nvmeReg.dword = IO_READ32(NVME_STATUS_REG_ADDR);
			if(nvmeReg.ccShn != 0)
			{
				uint32_t qID;
				set_nvme_csts_shst(1);

				for(qID = 0; qID < 8; qID++)
				{
					set_io_cq(qID, 0, 0, 0, 0, 0, 0);
					set_io_sq(qID, 0, 0, 0, 0, 0);
				}
				set_nvme_admin_queue(0, 0, 0);
				g_nvmeTask.cacheEn = 0;
				set_nvme_csts_shst(2);
				g_nvmeTask.status = NVME_TASK_WAIT_RESET;
				UpdateBadBlockTableForGrownBadBlock(RESERVED_DATA_BUFFER_BASE_ADDR);

				cpl_print("\r\nNVMe shutdown!!!\r\n");
			}
		}
		else if(g_nvmeTask.status == NVME_TASK_WAIT_RESET)
		{
			uint32_t ccEn;
			ccEn = check_nvme_cc_en();
			if(ccEn == 0)
			{
				g_nvmeTask.cacheEn = 0;
				set_nvme_csts_shst(0);
				set_nvme_csts_rdy(0);
				g_nvmeTask.status = NVME_TASK_IDLE;
				cpl_print("\r\nNVMe disable!!!\r\n");
			}
		} 
		else if(g_nvmeTask.status == NVME_TASK_RESET)
		{
			uint32_t qID;
			for(qID = 0; qID < 8; qID++)
			{
				set_io_cq(qID, 0, 0, 0, 0, 0, 0);
				set_io_sq(qID, 0, 0, 0, 0, 0);
			}

			if (rstCnt>= 5){
				pcie_async_reset(rstCnt);
				rstCnt = 0;
				cpl_print("\r\nPcie iink disable!!!\r\n");
				cpl_print("Wait few minute or reconnect the PCIe cable\r\n");
			}
			else
				rstCnt++;

			g_nvmeTask.cacheEn = 0;
			set_nvme_admin_queue(0, 0, 0);
			set_nvme_csts_shst(0);
			set_nvme_csts_rdy(0);
			g_nvmeTask.status = NVME_TASK_IDLE;

			cpl_print("\r\nNVMe reset!!!\r\n");
			// cpl_print("finish nvme_main 5\n");
		}

		if(exeLlr && ((nvmeDmaReqQ.headReq != REQ_SLOT_TAG_NONE) || notCompletedNandReqCnt || blockedReqCnt))
		{	
			CheckDoneNvmeDmaReq();
			SchedulingNandReq();
		}
	}
}

