//////////////////////////////////////////////////////////////////////////////////
// request_schedule.h for Cosmos+ OpenSSD
// Copyright (c) 2017 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Jaewook Kwak <jwkwak@enc.hanyang.ac.kr>
//			      Sangjin Lee <sjlee@enc.hanyang.ac.kr>
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
// Module Name: Request Scheduler
// File Name: request_schedule.h
//
// Version: v1.0.0
//
// Description:
//   - define parameters, data structure and functions of request scheduler
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////


#ifndef REQUEST_SCHEDULE_H_
#define REQUEST_SCHEDULE_H_

#include "ftl_config.h"

#define WAY_NONE 			0xF

#define LUN_0_BASE_ADDR		0x00000000
#define LUN_1_BASE_ADDR		0x00200000

#define PSEUDO_BAD_BLOCK_MARK	0

#define RETRY_LIMIT				5	//retry the failed request to the extent that the limit number allows

#define DIE_STATE_IDLE			0
#define DIE_STATE_EXE			1

#define REQ_STATUS_CHECK_OPT_NONE 				0
#define REQ_STATUS_CHECK_OPT_CHECK				1
#define REQ_STATUS_CHECK_OPT_REPORT 			2
#define REQ_STATUS_CHECK_OPT_COMPLETION_FLAG 	3

#define REQ_STATUS_RUNNING	0
#define REQ_STATUS_DONE		1
#define REQ_STATUS_FAIL		2
#define REQ_STATUS_WARNING	3

#define ERROR_INFO_FAIL		0
#define ERROR_INFO_PASS		1
#define ERROR_INFO_WARNING	2


typedef struct _COMPLETE_FLAG_TABLE {
	uint32_t completeFlag[USER_CHANNELS][USER_WAYS];
} COMPLETE_FLAG_TABLE, *P_COMPLETE_FLAG_TABLE;

typedef struct _STATUS_REPORT_TABLE {
	uint32_t statusReport[USER_CHANNELS][USER_WAYS];
} STATUS_REPORT_TABLE, *P_STATUS_REPORT_TABLE;

typedef struct _ERROR_INFO_TABLE {
	uint32_t errorInfo[USER_CHANNELS][USER_WAYS][ERROR_INFO_WORD_COUNT];
} ERROR_INFO_TABLE, *P_ERROR_INFO_TABLE;

typedef struct _RETRY_LIMIT_TABLE {
	int retryLimit[USER_CHANNELS][USER_WAYS];
} RETRY_LIMIT_TABLE, *P_RETRY_LIMIT_TABLE;

typedef struct _DIE_STATE_ENTRY {
	uint32_t dieState	:	8;
	uint32_t reqStatusCheckOpt	:	4;
	uint32_t prevWay	:	4;
	uint32_t nextWay 	:	4;
	uint32_t reserved	:	12;
} DIE_STATE_ENTRY, *P_DIE_STATE_ENTRY;

typedef struct _DIE_STATE_TABLE {
	DIE_STATE_ENTRY dieState[USER_CHANNELS][USER_WAYS];
} DIE_STATE_TABLE, *P_DIE_STATE_TABLE;


typedef struct _WAY_PRIORITY_ENTRY {
	uint32_t idleHead :	4;
	uint32_t idleTail :	4;
	uint32_t statusReportHead	:	4;
	uint32_t statusReportTail 	:	4;
	uint32_t readTriggerHead	:	4;
	uint32_t readTriggerTail	:	4;
	uint32_t writeHead	:	4;
	uint32_t writeTail	:	4;
	uint32_t readTransferHead	:	4;
	uint32_t readTransferTail	:	4;
	uint32_t eraseHead	:	4;
	uint32_t eraseTail	:	4;
	uint32_t statusCheckHead	:	4;
	uint32_t statusCheckTail	:	4;
	uint32_t reserved : 8;
} WAY_PRIORITY_ENTRY, *P_WAY_PRIORITY_ENTRY;

typedef struct _WAY_PRIORITY_TABLE {
	WAY_PRIORITY_ENTRY wayPriority[USER_CHANNELS];
} WAY_PRIORITY_TABLE, *P_WAY_PRIORITY_TABLE;


void InitReqScheduler();

void SyncAllLowLevelReqDone();
void SyncAvailFreeReq();
void SyncReleaseEraseReq(uint32_t chNo, uint32_t wayNo, uint32_t blockNo);
void SchedulingNandReq();
void SchedulingNandReqPerCh(uint32_t chNo);

void PutToNandWayPriorityTable(uint32_t reqSlotTag, uint32_t chNo, uint32_t wayNo);
void PutToNandIdleList(uint32_t chNo, uint32_t wayNo);
void SelectivGetFromNandIdleList(uint32_t chNo, uint32_t wayNo);
void PutToNandStatusReportList(uint32_t chNo, uint32_t wayNo);
void SelectivGetFromNandStatusReportList(uint32_t chNo, uint32_t wayNo);
void PutToNandReadTriggerList(uint32_t chNo, uint32_t wayNo);
void SelectiveGetFromNandReadTriggerList(uint32_t chNo, uint32_t wayNo);
void PutToNandWriteList(uint32_t chNo, uint32_t wayNo);
void SelectiveGetFromNandWriteList(uint32_t chNo, uint32_t wayNo);
void PutToNandReadTransferList(uint32_t chNo, uint32_t wayNo);
void SelectiveGetFromNandReadTransferList(uint32_t chNo, uint32_t wayNo);
void PutToNandEraseList(uint32_t chNo, uint32_t wayNo);
void SelectiveGetFromNandEraseList(uint32_t chNo, uint32_t wayNo);
void PutToNandStatusCheckList(uint32_t chNo, uint32_t wayNo);
void SelectiveGetFromNandStatusCheckList(uint32_t chNo, uint32_t wayNo);

void IssueNandReq(uint32_t chNo, uint32_t wayNo);
uint32_t GenerateNandRowAddr(uint32_t reqSlotTag);
uint32_t GenerateDataBufAddr(uint32_t reqSlotTag);
uint32_t GenerateSpareDataBufAddr(uint32_t reqSlotTag);
uint32_t CheckReqStatus(uint32_t chNo, uint32_t wayNo);
uint32_t CheckEccErrorInfo(uint32_t chNo, uint32_t wayNo);

void ExecuteNandReq(uint32_t chNo, uint32_t wayNo, uint32_t reqStatus);


extern P_COMPLETE_FLAG_TABLE completeFlagTablePtr;
extern P_STATUS_REPORT_TABLE statusReportTablePtr;
extern P_ERROR_INFO_TABLE eccErrorInfoTablePtr;
extern P_RETRY_LIMIT_TABLE retryLimitTablePtr;
extern P_DIE_STATE_TABLE dieStatusTablePtr;
extern P_WAY_PRIORITY_TABLE wayPriorityTablePtr;


#endif /* REQUEST_SCHEDULE_H_ */
