#pragma GCC push_options
#pragma GCC optimize ("O0")

#include "ariane.h"
#include "memory_map.h"
#include <stdint.h>

uint32_t 				storageCapacity_L;
V2FMCRegisters* chCtlReg[USER_CHANNELS];

void InitFTL()
{
	CheckConfigRestriction();	// in the same file
	InitChCtlReg(); 					// in the same file
	InitReqPool();						// initilize request pool 
	InitDependencyTable();    // initilize dependency table
	InitReqScheduler(); 			// initilize request scheduler
	InitNandArray(); 					// in the same file
	InitAddressMap();					// initilize address map
	InitDataBuf(); 						// initilize data buffer
	InitGcVictimMap(); 				// initilize gc victim map

	storageCapacity_L = (MB_PER_SSD - (MB_PER_MIN_FREE_BLOCK_SPACE + mbPerbadBlockSpace + MB_PER_OVER_PROVISION_BLOCK_SPACE)) * ((1024*1024) / BYTES_PER_NVME_BLOCK);

	cpl_print("[ storage capacity %d MB ]\r\n");// storageCapacity_L / ((1024*1024) / BYTES_PER_NVME_BLOCK));
	cpl_print("[ ftl configuration complete. ]\r\n");
}


void InitChCtlReg()
{
	if(USER_CHANNELS < 1)
		cpl_print("[WARNING] Configuration Error: Channel [WARNING]");
		chCtlReg[0] = (V2FMCRegisters*) NSC_0_BASEADDR;

	if(USER_CHANNELS > 1)
		chCtlReg[1] = (V2FMCRegisters*) NSC_1_BASEADDR;
	if(USER_CHANNELS > 2)
		chCtlReg[2] = (V2FMCRegisters*) NSC_2_BASEADDR;
	if(USER_CHANNELS > 3)
		chCtlReg[3] = (V2FMCRegisters*) NSC_3_BASEADDR;
	if(USER_CHANNELS > 4)
		chCtlReg[4] = (V2FMCRegisters*) NSC_4_BASEADDR;
	if(USER_CHANNELS > 5)
		chCtlReg[5] = (V2FMCRegisters*) NSC_5_BASEADDR;
	if(USER_CHANNELS > 6)
		chCtlReg[6] = (V2FMCRegisters*) NSC_6_BASEADDR;
	if(USER_CHANNELS > 7)
		chCtlReg[7] = (V2FMCRegisters*) NSC_7_BASEADDR;
}



void InitNandArray()
{
	uint32_t chNo, wayNo, reqSlotTag;

	for(chNo=0; chNo<USER_CHANNELS; ++chNo)
		for(wayNo=0; wayNo<USER_WAYS; ++wayNo)
		{
			reqSlotTag = GetFromFreeReqQ();

			reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
			reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_RESET;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr = REQ_OPT_NAND_ADDR_PHY_ORG;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_NONE;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_NONE;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_TOTAL;
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalCh = chNo;
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalWay = wayNo;
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalBlock = 0;	//dummy
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalPage = 0;	//dummy
			reqPoolPtr->reqPool[reqSlotTag].prevBlockingReq = REQ_SLOT_TAG_NONE;

			SelectLowLevelReqQ(reqSlotTag);

			reqSlotTag = GetFromFreeReqQ();

			reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
			reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_SET_FEATURE;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr =  REQ_OPT_NAND_ADDR_PHY_ORG;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_NONE;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_NONE;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_TOTAL;
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalCh = chNo;
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalWay = wayNo;
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalBlock = 0;	//dummy
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalPage = 0;	//dummy
			reqPoolPtr->reqPool[reqSlotTag].prevBlockingReq = REQ_SLOT_TAG_NONE;

			SelectLowLevelReqQ(reqSlotTag);
		} 

	SyncAllLowLevelReqDone();

	cpl_print("[ NAND device reset complete. ]\r\n");
}


void CheckConfigRestriction()
{
	if(USER_CHANNELS > NSC_MAX_CHANNELS)
		cpl_print("[WARNING] Configuration Error: Channel [WARNING]");
	if(USER_WAYS > NSC_MAX_WAYS)
		cpl_print("[WARNING] Configuration Error: WAY [WARNING]");
	if(USER_BLOCKS_PER_LUN > MAIN_BLOCKS_PER_LUN)
		cpl_print("[WARNING] Configuration Error: BLOCK [WARNING]");
	if((BITS_PER_FLASH_CELL != SLC_MODE) && (BITS_PER_FLASH_CELL != MLC_MODE))
		cpl_print("[WARNING] Configuration Error: BIT_PER_FLASH_CELL [WARNING]");
	if(RESERVED_DATA_BUFFER_BASE_ADDR + 0x00200000 > COMPLETE_FLAG_TABLE_ADDR)
		cpl_print("[WARNING] Configuration Error: Data buffer size is too large to be allocated to predefined range [WARNING]");
	if(TEMPORARY_PAY_LOAD_ADDR + 0x00001000 > DATA_BUFFER_MAP_ADDR)
		cpl_print("[WARNING] Configuration Error: Metadata for NAND request completion process is too large to be allocated to predefined range [WARNING]");
	if(FTL_MANAGEMENT_END_ADDR > DRAM_END_ADDR)
		cpl_print("[WARNING] Configuration Error: Metadata of FTL is too large to be allocated to DRAM [WARNING]");
}

#pragma GCC pop_options