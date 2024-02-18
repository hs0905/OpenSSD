#include "ariane.h"
#include "nvme_identify.h"
#include "nvme.h"
#include "nvme_main.h"
#include "host_lld.h"
#include <stdint.h>

extern char __bss_start; __bss_end;
extern NVME_CONTEXT g_nvmeTask;

int main()
{
	cpl_print("y18\n");

	if(cpl_In32(0x17FFFFF0) != 85465){ 								//0x17FFFFF0 주소의 값이 85465가 아니면
		cpl_memset((void *)0x20fba0/*bss*/, 0, 0x648); 	// bss 영역 초기화
		cpl_memset((void *)0x17FFFFF0, 0, 0x4);
		cpl_memset((void *)0x17FFFFF4, 0, 0x4);
		cpl_memset((void *)0x17FDFF00, 0, 0x20000);
		cpl_memset((void *)0x17FDFE00, 0, 0x100); 			// HOST_DMA_STATUS,  0x17FFFFF8
		cpl_memset((void *)0x17FBFE00, 0, 0x10000); 		// table meta data table
	}
	cpl_print("\r\n Hello COSMOS+ OpenSSD !!! \r\n"); // 해당 문자열 출력

	if(cpl_In32(0x17FFFFF0) != 85465){ 		//0x17FFFFF0 주소의 값이 85465가 아니면
		dev_irq_init(); // 
	}
	
	nvme_main(); 																				// 주요 함수 호출 및 수행

	cpl_print("done\r\n");

	return 0;
}
