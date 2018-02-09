
#include <stdlib.h>


void* syscall(void* syscallNum,void* param1,void* param2,void* param3,void* param4,void* param5,void* param6) {
    void* returnValue;
    __asm__ volatile ("movq %1, %%rax; movq %2, %%rdi; movq %3, %%rsi; movq %4, %%rdx; movq %5, %%r10; movq %6, %%r8;movq %7, %%r9;syscall;movq %%rax, %0;":"=g"(returnValue):"g"(syscallNum),"g"(param1),"g"(param2),"g"(param3),"g"(param4),"g"(param5),"g"(param6):"%rax","%rdi","%rsi","%rdx","%r10","%r8","%r9","memory");//clobber list
    return returnValue;
}

uint64_t toInteger(char *s){
    uint64_t i=0;
    uint64_t num=0;
    while(*(s+i)!= '\0'){
        num=num*10+(*(s+i)-'0');
        i+=1;
    }
    return num;
}

int main(int argc, char *argv[], char *envp[]){
    
    if(argc>=3){
        syscall((void*)199,(void*)(uint64_t)toInteger(argv[2]),(void*)0,(void*)3,(void*)4,(void*)5,(void*)0);
    }
    if(argc==2){
        syscall((void*)199,(void*)(uint64_t)toInteger(argv[1]),(void*)0,(void*)3,(void*)4,(void*)5,(void*)0);
    }
    return 0;
}
