//////////////////////////////////////////////////////////////////////////////////
// address_translation.c for Cosmos+ OpenSSD
// Copyright (c) 2017 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
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
// Engineer: Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: Address Translator
// File Name: address translation.c
//
// Version: v1.0.0
//
// Description:
//   - translate address between address space of host system and address space of NAND device
//   - manage bad blocks in NAND device
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#pragma GCC push_options
#pragma GCC optimize ("O0")


#include "ariane.h"
#include "memory_map.h"
//#include "cpl_print.h"

P_LOGICAL_SLICE_MAP logicalSliceMapPtr;
P_VIRTUAL_SLICE_MAP virtualSliceMapPtr;
P_VIRTUAL_BLOCK_MAP virtualBlockMapPtr;
P_VIRTUAL_DIE_MAP virtualDieMapPtr;
P_PHY_BLOCK_MAP phyBlockMapPtr;
P_BAD_BLOCK_TABLE_INFO_MAP bbtInfoMapPtr;

unsigned char sliceAllocationTargetDie;
uint32_t mbPerbadBlockSpace;

volatile unsigned char targetCh;
volatile unsigned char targetWay;

void InitAddressMap()
{
	uint32_t blockNo, dieNo;

	logicalSliceMapPtr = (P_LOGICAL_SLICE_MAP ) LOGICAL_SLICE_MAP_ADDR;
	virtualSliceMapPtr = (P_VIRTUAL_SLICE_MAP) VIRTUAL_SLICE_MAP_ADDR;
	virtualBlockMapPtr = (P_VIRTUAL_BLOCK_MAP) VIRTUAL_BLOCK_MAP_ADDR;
	virtualDieMapPtr = (P_VIRTUAL_DIE_MAP) VIRTUAL_DIE_MAP_ADDR;
	phyBlockMapPtr = (P_PHY_BLOCK_MAP) PHY_BLOCK_MAP_ADDR;
	bbtInfoMapPtr = (P_BAD_BLOCK_TABLE_INFO_MAP) BAD_BLOCK_TABLE_INFO_MAP_ADDR;

	//init phyblockMap
	for(dieNo=0 ; dieNo<USER_DIES ; dieNo++)
	{
		for(blockNo=0 ; blockNo<TOTAL_BLOCKS_PER_DIE ; blockNo++)
			phyBlockMapPtr->phyBlock[dieNo][blockNo].remappedPhyBlock = blockNo;

		bbtInfoMapPtr->bbtInfo[dieNo].phyBlock = 0;
		bbtInfoMapPtr->bbtInfo[dieNo].grownBadUpdate = BBT_INFO_GROWN_BAD_UPDATE_NONE;
	}
	
	// for(int i=0;i<64;i++){
	// 	for(int j=0;j<4096*2;j++){
	// 		virtualBlockMapPtr->block[i][j].currentPage=0;
	// 	}
	// }

	sliceAllocationTargetDie = FindDieForFreeSliceAllocation();

	InitSliceMap();
	//cpl_print("finish InitSliceMap\n");
	InitBlockDieMap();
	//cpl_print("finish InitBlockDieMap\n");
}

void InitSliceMap()
{
	int sliceAddr;
	for(sliceAddr=0; sliceAddr<SLICES_PER_SSD ; sliceAddr++)
	{
		logicalSliceMapPtr->logicalSlice[sliceAddr].virtualSliceAddr = VSA_NONE;
		virtualSliceMapPtr->virtualSlice[sliceAddr].logicalSliceAddr = LSA_NONE;
	}
}

void RemapBadBlock()
{
	uint32_t blockNo, dieNo, remapFlag, maxBadBlockCount;
	uint32_t reservedBlockOfLun0[USER_DIES];
	uint32_t reservedBlockOfLun1[USER_DIES];
	uint32_t badBlockCount[USER_DIES];

	cpl_print("Bad block remapping start...\r\n");

	for(dieNo=0 ; dieNo<USER_DIES ; dieNo++)
	{
		reservedBlockOfLun0[dieNo] = USER_BLOCKS_PER_LUN;
		reservedBlockOfLun1[dieNo] = TOTAL_BLOCKS_PER_LUN + USER_BLOCKS_PER_LUN;
		badBlockCount[dieNo] = 0;
	}


	for(blockNo=0 ; blockNo<USER_BLOCKS_PER_LUN ; blockNo++)
	{
		for(dieNo=0 ; dieNo<USER_DIES ; dieNo++)
		{
			//lun0
			if(phyBlockMapPtr->phyBlock[dieNo][blockNo].bad)
			{
				if(reservedBlockOfLun0[dieNo] < TOTAL_BLOCKS_PER_LUN)
				{
					remapFlag = 1;
					while(phyBlockMapPtr->phyBlock[dieNo][reservedBlockOfLun0[dieNo]].bad)
					{
						reservedBlockOfLun0[dieNo]++;
						if(reservedBlockOfLun0[dieNo] >= TOTAL_BLOCKS_PER_LUN)
						{
							remapFlag = 0;
							break;
						}
					}

					if(remapFlag)
					{
						phyBlockMapPtr->phyBlock[dieNo][blockNo].remappedPhyBlock  = reservedBlockOfLun0[dieNo];
						reservedBlockOfLun0[dieNo]++;
					}
					else
					{
						// cpl_print("No reserved block - Ch %d Way %d virtualBlock %d is bad block \r\n");// Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo), blockNo);
						badBlockCount[dieNo]++;
					}
				}
				else
				{
					// cpl_print("No reserved block - Ch %d Way %d virtualBlock %d is bad block \r\n");// Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo), blockNo);
					badBlockCount[dieNo]++;
				}
			}

			//lun1
			if(phyBlockMapPtr->phyBlock[dieNo][blockNo+TOTAL_BLOCKS_PER_LUN].bad)
			{
				if(reservedBlockOfLun1[dieNo] < TOTAL_BLOCKS_PER_DIE)
				{
					remapFlag = 1;
					while(phyBlockMapPtr->phyBlock[dieNo][reservedBlockOfLun1[dieNo]].bad)
					{
						reservedBlockOfLun1[dieNo]++;
						if(reservedBlockOfLun1[dieNo] >= TOTAL_BLOCKS_PER_DIE)
						{
							remapFlag = 0;
							break;
						}
					}

					if(remapFlag)
					{
						phyBlockMapPtr->phyBlock[dieNo][blockNo+TOTAL_BLOCKS_PER_LUN].remappedPhyBlock  = reservedBlockOfLun1[dieNo];
						reservedBlockOfLun1[dieNo]++;
					}
					else
					{
						// cpl_print("No reserved block - Ch %x Way %x virtualBlock %d is bad block \r\n");//  Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo), blockNo+USER_BLOCKS_PER_LUN);
						badBlockCount[dieNo]++;
					}
				}
				else
				{
					// cpl_print("No reserved block - Ch %x Way %x virtualBlock %d is bad block \r\n");//  Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo), blockNo+USER_BLOCKS_PER_LUN);
					badBlockCount[dieNo]++;
				}
			}
		}
	}

	cpl_print("Bad block remapping end\r\n");


	maxBadBlockCount = 0;
	for(dieNo=0; dieNo < USER_DIES; dieNo++)
	{
		if(maxBadBlockCount < badBlockCount[dieNo])
			maxBadBlockCount = badBlockCount[dieNo];
	}

	mbPerbadBlockSpace = maxBadBlockCount * USER_DIES * MB_PER_BLOCK;
}

void InitDieMap()
{
	uint32_t dieNo;

	for(dieNo=0 ; dieNo<USER_DIES ; dieNo++)
	{
		virtualDieMapPtr->die[dieNo].headFreeBlock = BLOCK_NONE;
		virtualDieMapPtr->die[dieNo].tailFreeBlock = BLOCK_NONE;
		virtualDieMapPtr->die[dieNo].freeBlockCnt = 0;
	}
}

void InitBlockMap()
{
	uint32_t dieNo, phyBlockNo, virtualBlockNo, remappedPhyBlock;

	for(dieNo=0 ; dieNo<USER_DIES ; dieNo++)
	{
		for(virtualBlockNo=0; virtualBlockNo<USER_BLOCKS_PER_DIE ; virtualBlockNo++)
		{
			phyBlockNo = Vblock2PblockOfTbsTranslation(virtualBlockNo);
			remappedPhyBlock = phyBlockMapPtr->phyBlock[dieNo][phyBlockNo].remappedPhyBlock;
			virtualBlockMapPtr->block[dieNo][virtualBlockNo].bad = phyBlockMapPtr->phyBlock[dieNo][remappedPhyBlock].bad;

			virtualBlockMapPtr->block[dieNo][virtualBlockNo].free = 1;
			virtualBlockMapPtr->block[dieNo][virtualBlockNo].invalidSliceCnt = 0;
			virtualBlockMapPtr->block[dieNo][virtualBlockNo].currentPage = 0;
			virtualBlockMapPtr->block[dieNo][virtualBlockNo].eraseCnt = 0;

			if(virtualBlockMapPtr->block[dieNo][virtualBlockNo].bad)
			{
				virtualBlockMapPtr->block[dieNo][virtualBlockNo].prevBlock = BLOCK_NONE;
				virtualBlockMapPtr->block[dieNo][virtualBlockNo].nextBlock = BLOCK_NONE;
			}
			else
				PutToFbList(dieNo, virtualBlockNo);
		}
	}
}

void InitCurrentBlockOfDieMap()
{
	uint32_t dieNo;

	for(dieNo=0 ; dieNo<USER_DIES ; dieNo++)
	{
		virtualDieMapPtr->die[dieNo].currentBlock = GetFromFbList(dieNo, GET_FREE_BLOCK_NORMAL);
		if(virtualDieMapPtr->die[dieNo].currentBlock == BLOCK_FAIL)
			cpl_print("[WARNING] There is no free block [WARNING]");
	}
}

void ReadBadBlockTable(uint32_t tempBbtBufAddr[], uint32_t tempBbtBufEntrySize)
{
	uint32_t tempPage, reqSlotTag, dieNo;
	int loop, dataSize;

	loop = 0;
	dataSize = DATA_SIZE_OF_BAD_BLOCK_TABLE_PER_DIE;
	tempPage = PlsbPage2VpageTranslation(START_PAGE_NO_OF_BAD_BLOCK_TABLE_BLOCK); 	//bad block table is saved at lsb pages

	while(dataSize>0)
	{
		for(dieNo = 0; dieNo < USER_DIES; dieNo++)
		{
			reqSlotTag = GetFromFreeReqQ();

			reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
			reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_READ;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_ADDR;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr = REQ_OPT_NAND_ADDR_PHY_ORG;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandEcc = REQ_OPT_NAND_ECC_ON;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandEccWarning = REQ_OPT_NAND_ECC_WARNING_OFF;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_NONE;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_TOTAL;

			reqPoolPtr->reqPool[reqSlotTag].dataBufInfo.addr = tempBbtBufAddr[dieNo] + loop * tempBbtBufEntrySize;

			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalCh = Vdie2PchTranslation(dieNo);
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalWay = Vdie2PwayTranslation(dieNo);
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalBlock = bbtInfoMapPtr->bbtInfo[dieNo].phyBlock;
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalPage = Vpage2PlsbPageTranslation(tempPage);

			SelectLowLevelReqQ(reqSlotTag);
		}

		tempPage++;
		loop++;
		dataSize -= BYTES_PER_DATA_REGION_OF_PAGE;
	}
	SyncAllLowLevelReqDone();
}

void FindBadBlock(unsigned char dieState[], uint32_t tempBbtBufAddr[], uint32_t tempBbtBufEntrySize, uint32_t tempReadBufAddr[], uint32_t tempReadBufEntrySize)
{
	uint32_t phyBlockNo, dieNo, reqSlotTag;
	unsigned char blockChecker[USER_DIES];
	unsigned char* markPointer0;
	unsigned char* markPointer1;
	unsigned char* bbtUpdater;
	cpl_print("FindBadBlock1\n");
	//check bad block mark of each block
	for(phyBlockNo = 0; phyBlockNo < TOTAL_BLOCKS_PER_DIE; phyBlockNo++)
	// for(phyBlockNo = 0; phyBlockNo < 8; phyBlockNo++)
	{
		for(dieNo=0; dieNo < USER_DIES; dieNo++)
			if(!dieState[dieNo])
			{
				blockChecker[dieNo] = BLOCK_STATE_NORMAL;

				reqSlotTag = GetFromFreeReqQ();

				reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
				reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_READ;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_ADDR;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr = REQ_OPT_NAND_ADDR_PHY_ORG;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandEcc = REQ_OPT_NAND_ECC_OFF;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandEccWarning = REQ_OPT_NAND_ECC_WARNING_OFF;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_NONE;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_TOTAL;

				reqPoolPtr->reqPool[reqSlotTag].dataBufInfo.addr = tempReadBufAddr[dieNo];

				reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalCh = Vdie2PchTranslation(dieNo);
				reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalWay = Vdie2PwayTranslation(dieNo);
				reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalBlock = phyBlockNo;
				reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalPage = BAD_BLOCK_MARK_PAGE0;

				SelectLowLevelReqQ(reqSlotTag);
			}
		SyncAllLowLevelReqDone();
		

		for(dieNo=0; dieNo < USER_DIES; dieNo++)
			if(!dieState[dieNo])
			{
				//
				markPointer0 = (unsigned char*)(tempReadBufAddr[dieNo] + BAD_BLOCK_MARK_BYTE0);
				markPointer1 = (unsigned char*)(tempReadBufAddr[dieNo] + BAD_BLOCK_MARK_BYTE1);

				if((*markPointer0 == CLEAN_DATA_IN_BYTE) && (*markPointer1 == CLEAN_DATA_IN_BYTE))
				{	//
					reqSlotTag = GetFromFreeReqQ();

					reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
					reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_READ;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_ADDR;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr = REQ_OPT_NAND_ADDR_PHY_ORG;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandEcc = REQ_OPT_NAND_ECC_OFF;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandEccWarning = REQ_OPT_NAND_ECC_WARNING_OFF;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_NONE;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_TOTAL;

					reqPoolPtr->reqPool[reqSlotTag].dataBufInfo.addr = tempReadBufAddr[dieNo];

					reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalCh = Vdie2PchTranslation(dieNo);
					reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalWay = Vdie2PwayTranslation(dieNo);
					reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalBlock = phyBlockNo;
					reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalPage = BAD_BLOCK_MARK_PAGE1;

					SelectLowLevelReqQ(reqSlotTag);
				}
				else
				{	//
					cpl_print("	bad block is detected: Ch %d Way %d phyBlock %d \r\n");//Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo), phyBlockNo);

					blockChecker[dieNo] = BLOCK_STATE_BAD;
				}
			}
		SyncAllLowLevelReqDone();
		

		for(dieNo=0; dieNo < USER_DIES; dieNo++)
			if(!dieState[dieNo])
			{	
				//
				markPointer0 = (unsigned char*)(tempReadBufAddr[dieNo] + BAD_BLOCK_MARK_BYTE0);
				markPointer1 = (unsigned char*)(tempReadBufAddr[dieNo] + BAD_BLOCK_MARK_BYTE1);

				if(!((*markPointer0 == CLEAN_DATA_IN_BYTE) && (*markPointer1 == CLEAN_DATA_IN_BYTE)))
					if(blockChecker[dieNo] == BLOCK_STATE_NORMAL)
					{
						cpl_print("	bad block is detected: Ch %d Way %d phyBlock %d \r\n");//Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo), phyBlockNo);

						blockChecker[dieNo] = BLOCK_STATE_BAD;

					}

				bbtUpdater= (unsigned char*)(tempBbtBufAddr[dieNo] + phyBlockNo);
				*bbtUpdater = blockChecker[dieNo];
				phyBlockMapPtr->phyBlock[dieNo][phyBlockNo].bad = blockChecker[dieNo];
			}
	}
}


void SaveBadBlockTable(unsigned char dieState[], uint32_t tempBbtBufAddr[], uint32_t tempBbtBufEntrySize)
{
	uint32_t dieNo, reqSlotTag;
	int loop, dataSize, tempPage;

	loop = 0;
	dataSize = DATA_SIZE_OF_BAD_BLOCK_TABLE_PER_DIE;
	tempPage = PlsbPage2VpageTranslation(START_PAGE_NO_OF_BAD_BLOCK_TABLE_BLOCK);	//bad block table is saved at lsb pages

	while(dataSize>0)
	{
		for(dieNo = 0; dieNo < USER_DIES; dieNo++)
			if((dieState[dieNo] == DIE_STATE_BAD_BLOCK_TABLE_NOT_EXIST) || (dieState[dieNo] == DIE_STATE_BAD_BLOCK_TABLE_UPDATE))
			{
				if(loop == 0)
				{
					reqSlotTag = GetFromFreeReqQ();

					reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
					reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_ERASE;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr = REQ_OPT_NAND_ADDR_PHY_ORG;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_NONE;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_NONE;
					reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_TOTAL;

					reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalCh = Vdie2PchTranslation(dieNo);
					reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalWay = Vdie2PwayTranslation(dieNo);
					reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalBlock = bbtInfoMapPtr->bbtInfo[dieNo].phyBlock;
					reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalPage = 0;	//dummy

					SelectLowLevelReqQ(reqSlotTag);
				}

				reqSlotTag = GetFromFreeReqQ();

				reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
				reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_WRITE;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_ADDR;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr = REQ_OPT_NAND_ADDR_PHY_ORG;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandEcc = REQ_OPT_NAND_ECC_ON;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandEccWarning = REQ_OPT_NAND_ECC_WARNING_OFF;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_NONE;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_TOTAL;

				reqPoolPtr->reqPool[reqSlotTag].dataBufInfo.addr = tempBbtBufAddr[dieNo] + loop * tempBbtBufEntrySize;

				reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalCh = Vdie2PchTranslation(dieNo);
				reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalWay = Vdie2PwayTranslation(dieNo);
				reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalBlock = bbtInfoMapPtr->bbtInfo[dieNo].phyBlock;
				reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalPage =  Vpage2PlsbPageTranslation(tempPage);

				SelectLowLevelReqQ(reqSlotTag);
			}

		loop++;
		tempPage++;
		dataSize -= BYTES_PER_DATA_REGION_OF_PAGE;
	}
	SyncAllLowLevelReqDone();
	//cpl_print("finish save bad block table SyncAllLowLevelReqDone\n");

	for(dieNo=0; dieNo < USER_DIES; dieNo++)
		if(dieState[dieNo] == DIE_STATE_BAD_BLOCK_TABLE_NOT_EXIST)
			cpl_print("[ bad block table of Ch %d Way %d is saved. ]\r\n");// dieNo%USER_CHANNELS, dieNo/USER_CHANNELS);

}


void RecoverBadBlockTable(uint32_t tempBufAddr)
{
	uint32_t dieNo, phyBlockNo, bbtMaker, tempBbtBufBaseAddr, tempBbtBufEntrySize, tempReadBufBaseAddr, tempReadBufEntrySize;
	uint32_t tempBbtBufAddr[USER_DIES];
	uint32_t tempReadBufAddr[USER_DIES];
	unsigned char dieState[USER_DIES];
	unsigned char* bbtTableChecker;

	//data buffer allocation
	tempBbtBufBaseAddr = tempBufAddr;
	tempBbtBufEntrySize = BYTES_PER_DATA_REGION_OF_PAGE + BYTES_PER_SPARE_REGION_OF_PAGE;
	tempReadBufBaseAddr = tempBbtBufBaseAddr + USER_DIES * USED_PAGES_FOR_BAD_BLOCK_TABLE_PER_DIE * tempBbtBufEntrySize;
	tempReadBufEntrySize = BYTES_PER_NAND_ROW;
	for(dieNo = 0; dieNo < USER_DIES; dieNo++)
	{
		tempBbtBufAddr[dieNo] = tempBbtBufBaseAddr + dieNo * USED_PAGES_FOR_BAD_BLOCK_TABLE_PER_DIE * tempBbtBufEntrySize;
		tempReadBufAddr[dieNo] = tempReadBufBaseAddr + dieNo * tempReadBufEntrySize;
	}

	//read bad block tables
	ReadBadBlockTable(tempBbtBufAddr, tempBbtBufEntrySize);

	//check bad block tables
	bbtMaker = BAD_BLOCK_TABLE_MAKER_IDLE;
	for(dieNo=0; dieNo<USER_DIES; dieNo++)
	{
		bbtTableChecker = (unsigned char*)(tempBbtBufAddr[dieNo]);

		if((*bbtTableChecker == BLOCK_STATE_NORMAL)||(*bbtTableChecker == BLOCK_STATE_BAD))
		{
			// cpl_print("[ bad block table of ch %d way %d exists.]\r\n");//Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo));

			dieState[dieNo] = DIE_STATE_BAD_BLOCK_TABLE_EXIST;
			for(phyBlockNo=0; phyBlockNo<TOTAL_BLOCKS_PER_DIE; phyBlockNo++)
			{
				bbtTableChecker = (unsigned char*)(tempBbtBufAddr[dieNo] + phyBlockNo);

				phyBlockMapPtr->phyBlock[dieNo][phyBlockNo].bad = *bbtTableChecker;
				// if(phyBlockMapPtr->phyBlock[dieNo][phyBlockNo].bad == BLOCK_STATE_BAD)
				// 	cpl_print("	bad block: ch %d way %d phyBlock %d  \r\n");// Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo), phyBlockNo);
			}

			// cpl_print("[ bad blocks of ch %d way %d are checked. ]\r\n");//Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo));
		}
		else
		{
			cpl_print("[ bad block table of ch %d way %d does not exist.]\r\n");//Vdie2PchTranslation(dieNo), Vdie2PwayTranslation(dieNo));
			dieState[dieNo] = DIE_STATE_BAD_BLOCK_TABLE_NOT_EXIST;
			bbtMaker = BAD_BLOCK_TABLE_MAKER_TRIGGER;
		}
	}
	//if bad block table does not exist in some dies, make new bad block table for each die having no bad block table
	if(bbtMaker == BAD_BLOCK_TABLE_MAKER_TRIGGER)
	{
		FindBadBlock(dieState, tempBbtBufAddr, tempBbtBufEntrySize, tempReadBufAddr, tempReadBufEntrySize);
		// cpl_print("finish save FindBadBlock\n");

		SaveBadBlockTable(dieState, tempBbtBufAddr, tempBbtBufEntrySize);
		// cpl_print("finish save SaveBadBlockTable\n");
	}

	//grown bad update flag initialization
	for(dieNo=0; dieNo<USER_DIES; dieNo++)
		bbtInfoMapPtr->bbtInfo[dieNo].grownBadUpdate = BBT_INFO_GROWN_BAD_UPDATE_NONE;
	
}

void EraseTotalBlockSpace()
{
	uint32_t blockNo, dieNo, reqSlotTag;

	cpl_print("Erase total block space...wait for a minute...\r\n");
	for(blockNo=0 ; blockNo<TOTAL_BLOCKS_PER_DIE ; blockNo++)// ?????
	// for(blockNo=0 ; blockNo<8 ; blockNo++)
		for(dieNo=0 ; dieNo<USER_DIES ; dieNo++)
		{
			reqSlotTag = GetFromFreeReqQ();

			reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
			reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_ERASE;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr = REQ_OPT_NAND_ADDR_PHY_ORG;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_NONE;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_NONE;
			reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_TOTAL;

			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalCh = Vdie2PchTranslation(dieNo);
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalWay = Vdie2PwayTranslation(dieNo);
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalBlock = blockNo;
			reqPoolPtr->reqPool[reqSlotTag].nandInfo.physicalPage = 0;

			SelectLowLevelReqQ(reqSlotTag);
		}

	SyncAllLowLevelReqDone();
	cpl_print("Done.\r\n");
}


void EraseUserBlockSpace()
{
	uint32_t blockNo, dieNo, reqSlotTag;

	cpl_print("Erase User block space...wait for a minute...\r\n");
	for(blockNo=0 ; blockNo<USER_BLOCKS_PER_DIE ; blockNo++)// ?????
	// for(blockNo=0 ; blockNo<8 ; blockNo++)
		for(dieNo=0 ; dieNo<USER_DIES ; dieNo++)
			if(!virtualBlockMapPtr->block[dieNo][blockNo].bad)
			{
				reqSlotTag = GetFromFreeReqQ();

				reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
				reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_ERASE;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr = REQ_OPT_NAND_ADDR_VSA;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_NONE;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_NONE;
				reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_MAIN;

				reqPoolPtr->reqPool[reqSlotTag].nandInfo.virtualSliceAddr = Vorg2VsaTranslation(dieNo, blockNo, 0);

				SelectLowLevelReqQ(reqSlotTag);
			}
	SyncAllLowLevelReqDone();
	cpl_print("Done.\r\n");
}


void InitBlockDieMap()
{
	uint32_t dieNo;
	unsigned char eraseFlag = 1;

	cpl_print("Press 'X' to re-make the bad block table.\r\n");
	// if (inbyte() == 'X')
	// {
	// EraseTotalBlockSpace();
	// eraseFlag = 0;
	// }

	InitDieMap();

	//make bad block table
	RecoverBadBlockTable(RESERVED_DATA_BUFFER_BASE_ADDR);
	// cpl_print("finish RecoverBadBlockTable\n");

	//to prevent accessing bbtBlock by host
	for(dieNo=0 ; dieNo<USER_DIES ; dieNo++)
		phyBlockMapPtr->phyBlock[dieNo][bbtInfoMapPtr->bbtInfo[dieNo].phyBlock].bad = 1;

	RemapBadBlock();
	// cpl_print("finish RemapBadBlock\n");
	
	InitBlockMap();
	// cpl_print("finish InitBlockMap\n");

	// for(int i=0 ; i<USER_DIES ; i++)
	// {
	// 	for(int j=0; j<USER_BLOCKS_PER_DIE ; j++)
	// 	{
	// 		cpl_printint(virtualBlockMapPtr->block[i][j].currentPage);
	// 	}
	// }

	if(eraseFlag /*&& cpl_In32(0x17FFFFF0) != 85465*/)
		EraseUserBlockSpace();

	InitCurrentBlockOfDieMap();
	// cpl_print("finish InitCurrentBlockOfDieMap\n");
}

uint32_t AddrTransRead(uint32_t logicalSliceAddr)
{
	uint32_t virtualSliceAddr;

	if(logicalSliceAddr < SLICES_PER_SSD)
	{
		virtualSliceAddr = logicalSliceMapPtr->logicalSlice[logicalSliceAddr].virtualSliceAddr;

		if(virtualSliceAddr != VSA_NONE)
			return virtualSliceAddr;
		else
			return VSA_FAIL;
	}
	else
		cpl_print("[WARNING] Logical address is larger than maximum logical address served by SSD [WARNING]");
}

uint32_t AddrTransWrite(uint32_t logicalSliceAddr)
{
	uint32_t virtualSliceAddr;

	if(logicalSliceAddr < SLICES_PER_SSD)
	{
		InvalidateOldVsa(logicalSliceAddr);

		virtualSliceAddr = FindFreeVirtualSlice();
		// cpl_print("finish FindFreeVirtualSlice\n");
		logicalSliceMapPtr->logicalSlice[logicalSliceAddr].virtualSliceAddr = virtualSliceAddr;
		virtualSliceMapPtr->virtualSlice[virtualSliceAddr].logicalSliceAddr = logicalSliceAddr;

		return virtualSliceAddr;
	}
	else
		cpl_print("[WARNING] Logical address is larger than maximum logical address served by SSD [WARNING]\n");
}


uint32_t FindFreeVirtualSlice()
{
	uint32_t currentBlock, virtualSliceAddr, dieNo;

	dieNo = sliceAllocationTargetDie;
	currentBlock = virtualDieMapPtr->die[dieNo].currentBlock;

	//cpl_print("dieNo: ");          
	// cpl_printint(dieNo);
	//cpl_print("currentBlock: ");
	//cpl_printint(currentBlock);    

	// cpl_print("FindFreeVirtualSlice current page: ");
	// cpl_printint(virtualBlockMapPtr->block[dieNo][currentBlock].currentPage);

	if(virtualBlockMapPtr->block[dieNo][currentBlock].currentPage == USER_PAGES_PER_BLOCK)
	{
		currentBlock = GetFromFbList(dieNo, GET_FREE_BLOCK_NORMAL);

		if(currentBlock != BLOCK_FAIL)
			virtualDieMapPtr->die[dieNo].currentBlock = currentBlock;
		else
		{
			GarbageCollection(dieNo);
			currentBlock = virtualDieMapPtr->die[dieNo].currentBlock;

			if(virtualBlockMapPtr->block[dieNo][currentBlock].currentPage == USER_PAGES_PER_BLOCK)
			{
				currentBlock = GetFromFbList(dieNo, GET_FREE_BLOCK_NORMAL);
				if(currentBlock != BLOCK_FAIL)
					virtualDieMapPtr->die[dieNo].currentBlock = currentBlock;
				else
					cpl_print("[WARNING] There is no available block [WARNING]\n");
			}
			else if(virtualBlockMapPtr->block[dieNo][currentBlock].currentPage > USER_PAGES_PER_BLOCK)
				cpl_print("[WARNING] Current page management fail [WARNING]\n");
		}
	}
	else if(virtualBlockMapPtr->block[dieNo][currentBlock].currentPage > USER_PAGES_PER_BLOCK)
		cpl_print("[WARNING] Current page management fail [WARNING]\n");


	virtualSliceAddr = Vorg2VsaTranslation(dieNo, currentBlock, virtualBlockMapPtr->block[dieNo][currentBlock].currentPage);
	virtualBlockMapPtr->block[dieNo][currentBlock].currentPage++;
	sliceAllocationTargetDie = FindDieForFreeSliceAllocation();
	dieNo = sliceAllocationTargetDie;
	return virtualSliceAddr;
}


uint32_t FindFreeVirtualSliceForGc(uint32_t copyTargetDieNo, uint32_t victimBlockNo)
{
	uint32_t currentBlock, virtualSliceAddr, dieNo;

	dieNo = copyTargetDieNo;
	if(victimBlockNo == virtualDieMapPtr->die[dieNo].currentBlock)
	{
		virtualDieMapPtr->die[dieNo].currentBlock = GetFromFbList(dieNo, GET_FREE_BLOCK_GC);
		if(virtualDieMapPtr->die[dieNo].currentBlock == BLOCK_FAIL)
			cpl_print("[WARNING] There is no available block [WARNING]");
	}
	currentBlock = virtualDieMapPtr->die[dieNo].currentBlock;

	if(virtualBlockMapPtr->block[dieNo][currentBlock].currentPage == USER_PAGES_PER_BLOCK)
	{

		currentBlock = GetFromFbList(dieNo, GET_FREE_BLOCK_GC);

		if(currentBlock != BLOCK_FAIL)
			virtualDieMapPtr->die[dieNo].currentBlock = currentBlock;
		else
			cpl_print("[WARNING] There is no available block [WARNING]");
	}
	else if(virtualBlockMapPtr->block[dieNo][currentBlock].currentPage > USER_PAGES_PER_BLOCK)
		cpl_print("[WARNING] Current page management fail [WARNING]");
		


	virtualSliceAddr = Vorg2VsaTranslation(dieNo, currentBlock, virtualBlockMapPtr->block[dieNo][currentBlock].currentPage);
	virtualBlockMapPtr->block[dieNo][currentBlock].currentPage++;
	return virtualSliceAddr;
}


uint32_t FindDieForFreeSliceAllocation()
{
	// static unsigned char targetCh = 0;
	// static unsigned char targetWay = 0;
	uint32_t targetDie;

	targetDie = Pcw2VdieTranslation(targetCh, targetWay);

	// cpl_print("FindDieForFreeSliceAllocation USER_CHANNELS: ");
	// cpl_printint(USER_CHANNELS);

	//cpl_print("FindDieForFreeSliceAllocation USER_WAYS: ");
	//cpl_printint(USER_WAYS);

	//cpl_print("FindDieForFreeSliceAllocation targetCh: ");
	//cpl_printint(targetCh);

	//cpl_print("FindDieForFreeSliceAllocation targetWay: ");
	//cpl_printint(targetWay);


	if(targetCh != (USER_CHANNELS - 1))
		targetCh = targetCh + 1;
	else
	{
		targetCh = 0;
		targetWay = (targetWay + 1) % USER_WAYS;
	}

	//cpl_print("FindDieForFreeSliceAllocation targetDie: ");
	//cpl_printint(targetDie);
	return targetDie;
}

void InvalidateOldVsa(uint32_t logicalSliceAddr)
{
	uint32_t virtualSliceAddr, dieNo, blockNo;

	virtualSliceAddr = logicalSliceMapPtr->logicalSlice[logicalSliceAddr].virtualSliceAddr;

	if(virtualSliceAddr != VSA_NONE)
	{
		if(virtualSliceMapPtr->virtualSlice[virtualSliceAddr].logicalSliceAddr != logicalSliceAddr)
			return;

		dieNo = Vsa2VdieTranslation(virtualSliceAddr);
		blockNo = Vsa2VblockTranslation(virtualSliceAddr);

		// unlink
		SelectiveGetFromGcVictimList(dieNo, blockNo);
		virtualBlockMapPtr->block[dieNo][blockNo].invalidSliceCnt++;
		logicalSliceMapPtr->logicalSlice[logicalSliceAddr].virtualSliceAddr = VSA_NONE;

		PutToGcVictimList(dieNo, blockNo, virtualBlockMapPtr->block[dieNo][blockNo].invalidSliceCnt);
	}

}


void EraseBlock(uint32_t dieNo, uint32_t blockNo)
{
	uint32_t pageNo, virtualSliceAddr, reqSlotTag;

	reqSlotTag = GetFromFreeReqQ();

	reqPoolPtr->reqPool[reqSlotTag].reqType = REQ_TYPE_NAND;
	reqPoolPtr->reqPool[reqSlotTag].reqCode = REQ_CODE_ERASE;
	reqPoolPtr->reqPool[reqSlotTag].reqOpt.nandAddr = REQ_OPT_NAND_ADDR_VSA;
	reqPoolPtr->reqPool[reqSlotTag].reqOpt.dataBufFormat = REQ_OPT_DATA_BUF_NONE;
	reqPoolPtr->reqPool[reqSlotTag].reqOpt.rowAddrDependencyCheck = REQ_OPT_ROW_ADDR_DEPENDENCY_CHECK;
	reqPoolPtr->reqPool[reqSlotTag].reqOpt.blockSpace = REQ_OPT_BLOCK_SPACE_MAIN;
	reqPoolPtr->reqPool[reqSlotTag].nandInfo.virtualSliceAddr = Vorg2VsaTranslation(dieNo, blockNo, 0);
	reqPoolPtr->reqPool[reqSlotTag].nandInfo.programmedPageCnt = virtualBlockMapPtr->block[dieNo][blockNo].currentPage;

	SelectLowLevelReqQ(reqSlotTag);

	// block map indicated blockNo initialization
	virtualBlockMapPtr->block[dieNo][blockNo].free = 1;
	virtualBlockMapPtr->block[dieNo][blockNo].eraseCnt++;
	virtualBlockMapPtr->block[dieNo][blockNo].invalidSliceCnt = 0;
	virtualBlockMapPtr->block[dieNo][blockNo].currentPage = 0;

	PutToFbList(dieNo, blockNo);

	for(pageNo=0; pageNo<USER_PAGES_PER_BLOCK; pageNo++)
	{
		virtualSliceAddr = Vorg2VsaTranslation(dieNo, blockNo, pageNo);
		virtualSliceMapPtr->virtualSlice[virtualSliceAddr].logicalSliceAddr = LSA_NONE;
	}
}

void PutToFbList(uint32_t dieNo, uint32_t blockNo) //fb means free block
{
	if(virtualDieMapPtr->die[dieNo].tailFreeBlock != BLOCK_NONE)
	{
		virtualBlockMapPtr->block[dieNo][blockNo].prevBlock = virtualDieMapPtr->die[dieNo].tailFreeBlock;
		virtualBlockMapPtr->block[dieNo][blockNo].nextBlock = BLOCK_NONE;
		virtualBlockMapPtr->block[dieNo][virtualDieMapPtr->die[dieNo].tailFreeBlock].nextBlock = blockNo;
		virtualDieMapPtr->die[dieNo].tailFreeBlock = blockNo;
	}
	else
	{
		virtualBlockMapPtr->block[dieNo][blockNo].prevBlock = BLOCK_NONE;
		virtualBlockMapPtr->block[dieNo][blockNo].nextBlock = BLOCK_NONE;
		virtualDieMapPtr->die[dieNo].headFreeBlock = blockNo;
		virtualDieMapPtr->die[dieNo].tailFreeBlock = blockNo;
	}

	virtualDieMapPtr->die[dieNo].freeBlockCnt++;
}

uint32_t GetFromFbList(uint32_t dieNo, uint32_t getFreeBlockOption) //fb means free block
{
	uint32_t evictedBlockNo;

	evictedBlockNo = virtualDieMapPtr->die[dieNo].headFreeBlock;

	if(getFreeBlockOption == GET_FREE_BLOCK_NORMAL)
	{
		if(virtualDieMapPtr->die[dieNo].freeBlockCnt <= RESERVED_FREE_BLOCK_COUNT)
			return BLOCK_FAIL;
	}
	else if(getFreeBlockOption == GET_FREE_BLOCK_GC)
	{
		if(evictedBlockNo == BLOCK_NONE)
			return BLOCK_FAIL;
	}
	else
		cpl_print("[WARNING] Wrong getFreeBlockOption [WARNING]");

	if(virtualBlockMapPtr->block[dieNo][evictedBlockNo].nextBlock != BLOCK_NONE)
	{
		virtualDieMapPtr->die[dieNo].headFreeBlock = virtualBlockMapPtr->block[dieNo][evictedBlockNo].nextBlock;
		virtualBlockMapPtr->block[dieNo][virtualBlockMapPtr->block[dieNo][evictedBlockNo].nextBlock].prevBlock = BLOCK_NONE;
	}
	else
	{
		virtualDieMapPtr->die[dieNo].headFreeBlock = BLOCK_NONE;
		virtualDieMapPtr->die[dieNo].tailFreeBlock = BLOCK_NONE;
	}

	virtualBlockMapPtr->block[dieNo][evictedBlockNo].free = 0;
	virtualDieMapPtr->die[dieNo].freeBlockCnt--;

	virtualBlockMapPtr->block[dieNo][evictedBlockNo].nextBlock = BLOCK_NONE;
	virtualBlockMapPtr->block[dieNo][evictedBlockNo].prevBlock = BLOCK_NONE;

	return evictedBlockNo;
}


void UpdatePhyBlockMapForGrownBadBlock(uint32_t dieNo, uint32_t phyBlockNo)
{
	phyBlockMapPtr->phyBlock[dieNo][phyBlockNo].bad = BLOCK_STATE_BAD;

	bbtInfoMapPtr->bbtInfo[dieNo].grownBadUpdate = BBT_INFO_GROWN_BAD_UPDATE_BOOKED;
}


void UpdateBadBlockTableForGrownBadBlock(uint32_t tempBufAddr)
{
	uint32_t dieNo, phyBlockNo, tempBbtBufBaseAddr, tempBbtBufEntrySize;
	uint32_t tempBbtBufAddr[USER_DIES];
	unsigned char dieState[USER_DIES];
	unsigned char* bbtUpdater;

	//data buffer allocation
	tempBbtBufBaseAddr = tempBufAddr;
	tempBbtBufEntrySize = BYTES_PER_DATA_REGION_OF_PAGE + BYTES_PER_SPARE_REGION_OF_PAGE;
	for(dieNo = 0; dieNo < USER_DIES; dieNo++)
		tempBbtBufAddr[dieNo] = tempBbtBufBaseAddr + dieNo * USED_PAGES_FOR_BAD_BLOCK_TABLE_PER_DIE * tempBbtBufEntrySize;

	//create new bad block table
	for(dieNo = 0; dieNo < USER_DIES; dieNo++)
	{
		if(bbtInfoMapPtr->bbtInfo[dieNo].grownBadUpdate == BBT_INFO_GROWN_BAD_UPDATE_BOOKED)
		{
			for(phyBlockNo = 0; phyBlockNo < TOTAL_BLOCKS_PER_DIE; phyBlockNo++)
			{
				bbtUpdater = (unsigned char*)(tempBbtBufAddr[dieNo] + phyBlockNo);

				if(phyBlockNo != bbtInfoMapPtr->bbtInfo[dieNo].phyBlock)
					*bbtUpdater = phyBlockMapPtr->phyBlock[dieNo][phyBlockNo].bad;
				else
					*bbtUpdater = BLOCK_STATE_NORMAL;
			}

			dieState[dieNo] = DIE_STATE_BAD_BLOCK_TABLE_UPDATE;
		}
		else
			dieState[dieNo] = DIE_STATE_BAD_BLOCK_TABLE_HOLD;
	}

	//update bad block tables in flash
	SaveBadBlockTable(dieState, tempBbtBufAddr, tempBbtBufEntrySize);
}

#pragma GCC pop_options