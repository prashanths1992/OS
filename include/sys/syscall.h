#ifndef _SYSCALL_H
#define _SYSCALL_H
#include<sys/defs.h>

void* syscall5(void* syscall_number,void* param1,void* param2,void* param3,void* param4,void* param5);

extern uint64_t* userRSP;
extern uint64_t* kernelRSP;
extern uint64_t* userRIP;
extern uint64_t userRax;
extern uint64_t userRbx;
extern uint64_t userRcx;
extern uint64_t userRdx;
extern uint64_t userRsi;
extern uint64_t userRdi;
extern uint64_t userRbp;
extern uint64_t userRflags;


extern int mutex;
void initSyscalls();

uint64_t readMSR(uint32_t msrAddr);
void writeMSR(uint64_t value,uint32_t msrAddr);

int getMutex();
void setMutex();

void* syscall(void* syscallNum,void* param1,void* param2,void* param3,void* param4,void* param5,void* param6);

#endif
