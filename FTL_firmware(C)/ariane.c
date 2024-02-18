#include <stdint.h>
// #include "ariane.h"
#define STDIN_BASEADDRESS       0xE0001000
#define STDOUT_BASEADDRESS      0xE0001000
#define XUARTPS_FIFO_OFFSET	0x0030U  /**< FIFO [7:0] */
#define XUARTPS_SR_OFFSET	0x002CU  /**< Channel Status [14:0] */
#define XUARTPS_SR_TXFULL	0x00000010U /**< TX FIFO full */
#define CMD_DONE_ADDRESS 0x43C80000
#define CMD_ADDRESS 0x43C80004
#define DATA_ADDRESS_REGNUM 0x43C80008
#define DATA_ADDRESS_BITNUM 0x43C8000C

static __inline uint32_t Xil_In32(uintptr_t Addr)
{
	return *(volatile uint32_t *)Addr;												// Addr을 받아와서 해당 주소에 있는 값을 읽어 반환
}

static __inline void Xil_Out32(uintptr_t Addr, uint32_t Value)
{ 
	volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr; // 포인터 변수 LocalAddr에 Addr을 저장
	*LocalAddr = Value; 																			// LocalAddr의 주소 공간에 Value를 저장
}

// macro function, BaseAddress와Offset을 받아와서 Full 여부를 비교 연산자를 통해 확인
#define XUartPs_IsTransmitFull(BaseAddress)			 \
	((Xil_In32((BaseAddress) + XUARTPS_SR_OFFSET) & 	\
	 (uint32_t)XUARTPS_SR_TXFULL) == (uint32_t)XUARTPS_SR_TXFULL)

// macro function, BaseAddress와 RegOffset, RegisterValue를 받아와서 해당 주소에 RegisterValue를 write
#define XUartPs_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	Xil_Out32((BaseAddress) + (uint32_t)(RegOffset), (uint32_t)(RegisterValue))

void XUartPs_SendByte(uint32_t BaseAddress, uint8_t Data)
{
	/* Wait until there is space in TX FIFO */ // do nothing { ; }
	while (XUartPs_IsTransmitFull(BaseAddress)) { ; }
	/* Write the byte into the TX FIFO */
	XUartPs_WriteReg(BaseAddress, XUARTPS_FIFO_OFFSET, (uint32_t)Data);
}

void outbyte(char c) {
	XUartPs_SendByte(STDOUT_BASEADDRESS, c);
}

void cpl_print(const char *ptr)
{
	while (*ptr != (char)0) {
		outbyte(*ptr);
		ptr++;
	}
	// asm("fence");
}

void cpl_printint(const uint32_t val)
{
	char buffer[16];
	uint32_t val1 = val;
	int digits = 0;
	while(val1) {	  
	  val1 /= 10;
	  digits++;
	}	
	if(digits==0) {
		cpl_print("0\n");
		return;
	}

	uint32_t val2 = val;
	int digits_idx = 0;
	while(val2) {
	  buffer[digits-digits_idx-1] = val2%10 + '0';
	  digits_idx++;
	  val2 /= 10;
	}

	buffer[digits] = '\n';
	buffer[digits+1] = '\0';
	cpl_print(buffer);
	return;

}

void cpl_printint_no_enter(const uint32_t val)
{
	char buffer[16];
	uint32_t val1 = val;
	int digits = 0;
	while(val1) {	  
	  val1 /= 10;
	  digits++;
	}	
	if(digits==0) {
		cpl_print("0\t");
		return;
	}

	uint32_t val2 = val;
	int digits_idx = 0;
	while(val2) {
	  buffer[digits-digits_idx-1] = val2%10 + '0';
	  digits_idx++;
	  val2 /= 10;
	}

	buffer[digits] = '\t';
	buffer[digits+1] = '\0';
	cpl_print(buffer);
	return;

}

void cpl_Out32(uintptr_t Addr, uint32_t Value)
{
	volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
	*LocalAddr = Value;
}

uint32_t cpl_In32(uintptr_t Addr)
{
	volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
	return *LocalAddr;
}

// void ICacheDisable(void){
//     __asm__("csrw 0x7C0, 0");
//     __asm__("fence");
// }

// void DCacheDisable(){
//     __asm__("csrw 0x7C1, 0");
//     __asm__("fence");
// }

// void DisableMMU(){

// }

// void ICacheDisable(void){
//     __asm__("csrw 0x7C0, 1");
//     __asm__("fence");
// }

// void DCacheDisable(){
//     __asm__("csrw 0x7C1, 1");
//     __asm__("fence");
// }

// void DisableMMU(){

// }

// void SetTlbAttributes(){

// }
