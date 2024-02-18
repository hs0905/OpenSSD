//////////////////////////////////////////////////////////////////////////////////
// fmc_driver.c for Cosmos+ OpenSSD
// Copyright (c) 2016 Hanyang University ENC Lab.
// Contributed by Yong Ho Song <yhsong@enc.hanyang.ac.kr>
//				  Kibin Park <kbpark@enc.hanyang.ac.kr>
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
// Engineer: Kibin Park <kbpark@enc.hanyang.ac.kr>
//
// Project Name: Cosmos+ OpenSSD
// Design Name: Cosmos+ Firmware
// Module Name: NAND Storage Controller Driver
// File Name: nsc_driver.c
//
// Version: v1.1.0
//
// Description:
//   - low level driver for NAND storage controller
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
// Revision History:
//
// * v1.1.0
//   - V2FReadPageTransferAsync needs additional input (rowAddress)
//
// * v1.0.0
//   - First draft
//////////////////////////////////////////////////////////////////////////////////

#include "nsc_driver.h"
#include <stdint.h>

uint32_t __attribute__((optimize("O0"))) V2FIsControllerBusy(V2FMCRegisters* dev)
{
	volatile uint32_t channelBusy = *((volatile uint32_t*)&(dev->channelBusy));

	return channelBusy;
}

void __attribute__((optimize("O0"))) V2FResetSync(V2FMCRegisters* dev, int way)
{
	*((volatile uint32_t*)&(dev->waySelection)) = way;
	*((volatile uint32_t*)&(dev->cmdSelect)) = V2FCommand_Reset;
	while (V2FIsControllerBusy(dev));
}

void __attribute__((optimize("O0"))) V2FSetFeaturesSync(V2FMCRegisters* dev, int way, uint32_t feature0x02, uint32_t feature0x10, uint32_t feature0x01, uint32_t payLoadAddr)
{
	uint32_t* payload = (uint32_t*)payLoadAddr;
	payload[0] = feature0x02;
	payload[1] = feature0x10;
	payload[2] = feature0x01;
	*((volatile uint32_t*)&(dev->waySelection)) = way;
	*((volatile uint32_t*)&(dev->userData)) = (uint32_t)payload;
	*((volatile uint32_t*)&(dev->cmdSelect)) = V2FCommand_SetFeatures;
	while (V2FIsControllerBusy(dev));
}

void __attribute__((optimize("O0"))) V2FGetFeaturesSync(V2FMCRegisters* dev, int way, uint32_t* feature0x01, uint32_t* feature0x02, uint32_t* feature0x10, uint32_t* feature0x30)
{
	volatile uint32_t buffer[4] = {0};
	volatile uint32_t completion = 0;
	*((volatile uint32_t*)&(dev->waySelection)) = way;
	*((volatile uint32_t*)&(dev->userData)) = (uint32_t)buffer;
	*((volatile uint32_t*)&(dev->completionAddress)) = (uint32_t)&completion;
	*((volatile uint32_t*)&(dev->cmdSelect)) = V2FCommand_GetFeatures;
	while (V2FIsControllerBusy(dev));
	while (!(completion & 1));
	*feature0x01 = buffer[0];
	*feature0x02 = buffer[1];
	*feature0x10 = buffer[2];
	*feature0x30 = buffer[3];
}

void __attribute__((optimize("O0"))) V2FReadPageTriggerAsync(V2FMCRegisters* dev, int way, uint32_t rowAddress)
{
	*((volatile uint32_t*)&(dev->waySelection)) = way;
	*((volatile uint32_t*)&(dev->rowAddress)) = rowAddress;
	*((volatile uint32_t*)&(dev->cmdSelect)) = V2FCommand_ReadPageTrigger;
}

void __attribute__((optimize("O0"))) V2FReadPageTransferAsync(V2FMCRegisters* dev, int way, void* pageDataBuffer, void* spareDataBuffer, uint32_t* errorInformation, uint32_t* completion, uint32_t rowAddress)
{
	*((volatile uint32_t*)&(dev->waySelection)) = way;
	*((volatile uint32_t*)&(dev->dataAddress)) = (uint32_t)pageDataBuffer;
	*((volatile uint32_t*)&(dev->spareAddress)) = (uint32_t)spareDataBuffer;
	*((volatile uint32_t*)&(dev->errorCountAddress)) = (uint32_t)errorInformation;
	*((volatile uint32_t*)&(dev->completionAddress)) = (uint32_t)completion;
	*((volatile uint32_t*)&(dev->rowAddress)) = rowAddress;
	*completion = 0;
	*((volatile uint32_t*)&(dev->cmdSelect)) = V2FCommand_ReadPageTransfer;
}

void __attribute__((optimize("O0"))) V2FReadPageTransferRawAsync(V2FMCRegisters* dev, int way, void* pageDataBuffer, uint32_t* completion)
{
	*((volatile uint32_t*)&(dev->waySelection)) = way;
	*((volatile uint32_t*)&(dev->dataAddress)) = (uint32_t)pageDataBuffer;
	*((volatile uint32_t*)&(dev->completionAddress)) = (uint32_t)completion;
	*completion = 0;
	*((volatile uint32_t*)&(dev->cmdSelect)) = V2FCommand_ReadPageTransferRaw;
}


void __attribute__((optimize("O0"))) V2FProgramPageAsync(V2FMCRegisters* dev, int way, uint32_t rowAddress, void* pageDataBuffer, void* spareDataBuffer)
{
	*((volatile uint32_t*)&(dev->waySelection)) = way;
	*((volatile uint32_t*)&(dev->rowAddress)) = rowAddress;
	*((volatile uint32_t*)&(dev->dataAddress)) = (uint32_t)pageDataBuffer;
	*((volatile uint32_t*)&(dev->spareAddress)) = (uint32_t)spareDataBuffer;
	*((volatile uint32_t*)&(dev->cmdSelect)) = V2FCommand_ProgramPage;
}

void __attribute__((optimize("O0"))) V2FEraseBlockAsync(V2FMCRegisters* dev, int way, uint32_t rowAddress)
{
	*((volatile uint32_t*)&(dev->waySelection)) = way;
	*((volatile uint32_t*)&(dev->rowAddress)) = rowAddress;
	*((volatile uint32_t*)&(dev->cmdSelect)) = V2FCommand_BlockErase;
}

void __attribute__((optimize("O0"))) V2FStatusCheckAsync(V2FMCRegisters* dev, int way, uint32_t* statusReport)
{
	*((volatile uint32_t*)&(dev->waySelection)) = way;
	*((volatile uint32_t*)&(dev->completionAddress)) = (uint32_t)statusReport;
	*statusReport = 0;
	*((volatile uint32_t*)&(dev->cmdSelect)) = V2FCommand_StatusCheck;
}

uint32_t __attribute__((optimize("O0"))) V2FStatusCheckSync(V2FMCRegisters* dev, int way)
{
	volatile uint32_t status;
	V2FStatusCheckAsync(dev, way, (uint32_t*)&status);
	while (!(status & 1));
	return (status >> 1);
}

uint32_t __attribute__((optimize("O0"))) V2FReadyBusyAsync(V2FMCRegisters* dev)
{
	volatile uint32_t readyBusy = dev->readyBusy;

	return readyBusy;
}


