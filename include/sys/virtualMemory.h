#ifndef _virtualMemory_h
#define _virtualMemory_h

#include<sys/defs.h>
#include<sys/task.h>
#define get_PML4_INDEX(x)  (((x) >> 39)&0x1FF)
#define get_PDPT_INDEX(x)  (((x) >> 30)&0x1FF)
#define get_PDT_INDEX(x)  (((x) >> 21)&0x1FF)
#define get_PT_INDEX(x)  (((x) >> 12)&0x1FF)
#define PRESENT 0x1
#define WRITEABLE 0x2
#define MAKERDONLY 0xFFFFFFFFFFFFFFFD
#define USER 0x4
#define FRAME 0xFFFFFFFFFFFFF000
void mapKernelMemory();
void enablePaging();
void mapPage(uint64_t v_addr, uint64_t phy_addr);
void mapPageForUser(uint64_t v_addr, uint64_t phy_addr,uint64_t temp);
void forceMapPage(uint64_t v_addr, uint64_t phy_addr,uint64_t temp,int user);
void identityMapping();
void mapVideoMemory(uint64_t vga_virtual_address);
void* kmalloc();//returns virtual address, to be called only after paging only by kernel.
uint64_t getCr3();
uint64_t get_kernbase();
uint64_t get_stack_top();
uint64_t mapAHCI(uint64_t abar_phys);
void* stackForUser(Task *uthread);

void* getNewPML4ForUser();

extern char kernmem, physbase;

struct PT
{
  uint64_t entries[512];
};

struct PDT
{
  uint64_t entries[512];
};

struct PDPT
{
  uint64_t entries[512];
};

struct PML4
{
  uint64_t entries[512];
};
#endif
