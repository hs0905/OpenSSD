#include <stdint.h>
// #define HOST_IP_ADDR						0x83C00000

// #define XPAR_TIGER4NSC_0_BASEADDR           0x43C00000
// #define XPAR_TIGER4NSC_1_BASEADDR           0x43C10000
// #define XPAR_TIGER4NSC_2_BASEADDR           0x43C20000       
// #define XPAR_TIGER4NSC_3_BASEADDR           0x43C30000
// #define XPAR_TIGER4NSC_4_BASEADDR           0x43C40000
// #define XPAR_TIGER4NSC_5_BASEADDR           0x43C50000
// #define XPAR_TIGER4NSC_6_BASEADDR           0x43C60000
// #define XPAR_TIGER4NSC_7_BASEADDR           0x43C70000

// void ICacheDisable(void);
// void DCacheDisable(void);
// void DisableMMU(void);
// void ICacheEnable(void);
// void DCacheEnable(void);
// void EnableMMU(void);
// void SetTlbAttributes(void);

#define CMD_DONE_ADDRESS    0x43C80000
#define CMD_ADDRESS         0x43C80004
#define DATA_ADDRESS_REGNUM 0x43C80008
#define DATA_ADDRESS_BITNUM 0x43C8000C
#define DATA_ADDRESS_COUNT  0x43C80010

void      cpl_print(const char *);
void      cpl_printint(const uint32_t val);
void      cpl_printint_no_enter(const uint32_t val);
void      cpl_Out32(uintptr_t Addr, uint32_t Value);
uint32_t  cpl_In32(uintptr_t Addr);

