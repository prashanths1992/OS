#include <sys/defs.h>
#include <sys/idt.h>
#include <sys/pic.h>
#include <sys/task.h>
#include <sys/keyboard.h>
#include <sys/kprintf.h>
#include<sys/phyMemMapper.h>
#include<sys/virtualMemory.h>
#include<sys/task.h>
#include<sys/syscall.h>
#include<sys/tarfs.h>

uint64_t switchRsp=0;
uint64_t switchRbx=0;
uint64_t switchRcx=0;
uint64_t switchRdx=0;
uint64_t switchRsi=0;
uint64_t switchRdi=0;
uint64_t switchRbp=0;

//int timeCounter = 0;
//int counterEnable =0;
//
//
//void enableCounter()
//{
//    counterEnable =1;
//}
//
//void disableCounter()
//{
//    timeCounter=0;
//    counterEnable =0;
//}
//
//int getTimeCounter()
//{
//    return timeCounter;
//}

uint64_t errorCode;
char *stdStart;
char *writePosition;
char *readPosition;
char *end;
int indicator;
int readline;

static char rtc_second,rtc_hour,rtc_minute;

char getCurSec()
{
    return rtc_second;
}
char getCurHr()
{
    return rtc_hour;
}
char getCurMin()
{
    return rtc_minute;
}

char * get_input_buf(){
    return stdStart;
}

void set_input_buf(uint64_t page){
    stdStart=(char *)page;
    writePosition=(char *)page;
    readPosition=(char *)page;
    end=(char *)(page+0x1000);
    indicator=0;
    readline=0;
}

typedef struct registers
{
    uint64_t ds;                  // Data segment selector
    uint64_t rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax; // Pushed by pusha.
    uint64_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint64_t rip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;


struct idt_descriptor {
    uint16_t id_base_lo;
    uint16_t id_sel;
    uint8_t id_always0;
    uint8_t id_flags;
    uint16_t id_base_middle;
    uint32_t id_base_high;
    uint32_t id_always00;
}__attribute__((packed));


struct idtr_t {
    uint16_t limit;
    uint64_t base;
}__attribute__((packed));


struct idt_descriptor idt[256];
static const struct idtr_t idtp = {(sizeof(struct idt_descriptor)*256)-1,(uint64_t)idt};

void id_set_gate(uint8_t intr_num,uint64_t base_addr, uint8_t sel,uint8_t flags)
{
    idt[intr_num].id_flags = flags;
    idt[intr_num].id_always0 = 0;
    idt[intr_num].id_always00 = 0;
    idt[intr_num].id_sel = sel;
    
    //ADDRESS TO THE INTERRUPT SERVICE ROUTINE METHOD
    
    idt[intr_num].id_base_lo = (base_addr & 0xFFFF);
    idt[intr_num].id_base_middle = (base_addr >> 16) & 0xFFFF;
    idt[intr_num].id_base_high = (base_addr >> 32) & 0xFFFFFFFF;
    
}

static int shift=0,control=0;

void _key_press_handler(registers_t regs){
    
    //kprintf("%d ",regs.int_no);
    unsigned char a;
    
//    uint64_t temp;
//    //kprintf("In User Space1........");
////    __asm__ __volatile__("movq %%rsp, %%rax; movq %%rax, %0;":"=m"(temp)::"%rax");
////    kprintf("%p ", temp);
    
    a = inb(0x60);
    
    //kprintf("\n%d\n",a);
    
    if(a<128) {
        
        switch(a)
        {
            case 42:
                shift=1;break;
            case 54:
                shift=1;break;
            case 29:
                control = 1;break;
            case 58:
                keypress_bar("LAST PRESSED: Caps",14);break;
            case 14:
                keypress_bar("LAST PRESSED: Backspace",14);handleBackspace();break;
            case 28:
                keypress_bar("LAST PRESSED: Enter",14);writeChar('\n',0);kprintf("\n");break;
            case 77:
                keypress_bar("LAST PRESSED: Right Arrow",14);break;
            case 75:
                keypress_bar("LAST PRESSED: Left Arrow",14);break;
            case 72:
                keypress_bar("LAST PRESSED: Up Arrow",14);break;
            case 71:
                keypress_bar("LAST PRESSED: Home",14);break;
            case 70:
                keypress_bar("LAST PRESSED: Scroll Lock",14);break;
            case 73:
                keypress_bar("LAST PRESSED: Page Up",14);break;
            case 81:
                keypress_bar("LAST PRESSED: Page Down",14);break;
            case 82:
                keypress_bar("LAST PRESSED: Insert",14);break;
            case 83:
                keypress_bar("LAST PRESSED: Delete",14);break;
            case 79:
                keypress_bar("LAST PRESSED: End",14);break;
            case 80:
                keypress_bar("LAST PRESSED: Down Arrow",14);break;
            case 57:
                keypress_bar("LAST PRESSED: Space",14);writeChar(0x20,0);kprintf(" ");break;
            case 56:
                keypress_bar("LAST PRESSED: Alt",14);break;
            case 15:
                keypress_bar("LAST PRESSED: Tab",14);break;
            case 1:
                keypress_bar("LAST PRESSED: Esc",14);break;
            case 59:
                keypress_bar("LAST PRESSED: F1",14);break;
            case 60:
                keypress_bar("LAST PRESSED: F2",14);break;
            case 61:
                keypress_bar("LAST PRESSED: F3",14);break;
            case 62:
                keypress_bar("LAST PRESSED: F4",14);break;
            case 63:
                keypress_bar("LAST PRESSED: F5",14);break;
            case 64:
                keypress_bar("LAST PRESSED: F6",14);break;
            case 65:
                keypress_bar("LAST PRESSED: F7",14);break;
            case 66:
                keypress_bar("LAST PRESSED: F8",14);break;
            case 67:
                keypress_bar("LAST PRESSED: F9",14);break;
            case 68:
                keypress_bar("LAST PRESSED: F10",14);break;
            case 87:
                keypress_bar("LAST PRESSED: F11",14);break;
            case 88:
                keypress_bar("LAST PRESSED: F12",14);break;
                
            default:
                if (shift)
                {
                    //shift = 0;
                    char s[] = {'L','A','S','T',' ','P','R','E','S','S','E','D',':',' ','s','h','i','f','t','+','\'',kbdus[a],'\'','=','\'',CAPS_kbdus[a],'\'','\0'};
                    keypress_bar(s,14);
                    kprintf("%c",CAPS_kbdus[a]);
                    writeChar(CAPS_kbdus[a],0);
                }
                else if (control)
                {
                    //control=0;
                    char s[] = {'L','A','S','T',' ','P','R','E','S','S','E','D',':',' ','^',CAPS_kbdus[a],'\0'};
                    keypress_bar(s,14);
                    //kprintf("^%c",CAPS_kbdus[a]);
                    //writeChar(CAPS_kbdus[a],1);
                    if(CAPS_kbdus[a]=='H'){
                      handleBackspace();
                    }
                    if(CAPS_kbdus[a]=='C'){
                      int pid = getforegroundPid();
                      if(pid!=0){
                        setforegroundPid(0);
                        outb(0x20,0x20);
                        outb(0x20,0xA0);
                        exitpid(pid);
                      }
                    }
                }
                else
                {
                    char s[]={'L','A','S','T',' ','P','R','E','S','S','E','D',':',' ',kbdus[a],'\0'};
                    keypress_bar(s,14);
                    kprintf("%c",kbdus[a]);
                    writeChar(kbdus[a],0);
                }
        }
    }
    else{
      switch(a)
      {
        case 170:
          shift=0;
        case 182:
          shift=0;
        case 157:
          control=0;
      }
    }
    
    
    
    outb(0x20,0x20);
    outb(0x20,0xA0);
}

static int i = 0;
static int j=0;
static int hour = 0;
static int minute = 0;
static int seconds = 0;

void _timer_intr_hdlr(){
    
    i++;
    j++;
    
    if(i%19==0) {
        i = 0;
        seconds++;
        
//        if(counterEnable)
//        {
//            timeCounter++;
//        }
        
        Task* temp = getRunningThread();
        
        if(temp->currAlarmCount >= 1)
        {
            temp->currAlarmCount++;
        }
        /*if(temp->currAlarmCount>temp->expectedAlarmCount)
        {
            temp->regs.rip=(uint64_t)temp->functionPointer;
            temp->currAlarmCount=0;
        }*/
        
        if(temp->currSleepCount >= 1)
        {
            temp->currSleepCount++;
        }
        
        temp = getRunningThread()->next;
        
        while(temp!=getRunningThread())
        {
            
            if(temp->currAlarmCount >= 1)
            {
                temp->currAlarmCount++;
            }
            
            if(temp->currSleepCount >= 1)
            {
                temp->currSleepCount++;
            }
            
            /*if(temp->currAlarmCount>temp->expectedAlarmCount)
            {
                temp->regs.rip=(uint64_t)temp->functionPointer;
                temp->currAlarmCount=0;
                temp->expectedAlarmCount=0;
            }*/
            
            temp=temp->next;
        }
        
        
        if(seconds%60==0) {
            seconds%=60;
            minute+=1;
            if(minute%60==0)
            {
                minute%=60;
                hour++;
            }
        }
        
        boot_time_bar(hour,minute,seconds,14);
        
        //send EOI to PIC
//        outb(0x20,0x20);
//        outb(0x20,0xA0);
//
        //runNextTask();
        
    }
    else if (j%3==0)
    {
        j=0;
        outb(0x20,0x20);
        outb(0x20,0xA0);
        
        if(getMutex()!=1)
            runNextTask(); // preemptive multitasking code starts here..
        
    }
    //runNextTask();
    
    
//    //preemptive scheduling // time 1 sec
//    if(i%19==0) {
//        //move on to the next task
//        kprintf("Trying to move to the next task\n");
//        void switchToUserMode();
//        
//    }
//    
    
    
    
    
   
    //send EOI to PIC
    outb(0x20,0x20);
    outb(0x20,0xA0);
}


static int rtc_read_timeout = 0;

void _rtc_intr_hndlr(){
    rtc_read_timeout++;
    
    if(rtc_read_timeout%19==0){
        
        rtc_read_timeout=0;
    
    //rtc_second;
    outb(0x00,0x70);
    rtc_second = inb(0x71);
    rtc_second = ((rtc_second / 16) * 10) + (rtc_second & 0xf);
    
    //rtc_minute;
    outb(0x02,0x70);
    rtc_minute = inb(0x71);
    
    //rtc_hour;
    outb(0x04,0x70);
    rtc_hour = inb(0x71);
    
    unsigned char regb;
    
    outb(0x0B,0x70);
    regb = inb(0x71);
        
    //kprintf("\n%d:%d:%d\n",rtc_hour,rtc_minute,rtc_second);
        
    if (!(regb & 0x04)) {
        //rtc_second=(rtc_second & 0x0F)+((rtc_second/16)*10);
        rtc_minute=(rtc_minute & 0x0F)+ ((rtc_minute/16)*10);
        rtc_hour=((rtc_hour & 0x0F) +(((rtc_hour & 0x70)/16)*10));
    }
    
    if (!(regb & 0x02) && (rtc_hour & 0x80)) {
            rtc_hour = ((rtc_hour & 0x7F) + 12) % 24;
    }
        
    //kprintf("\n%d:%d:%d\n",rtc_hour,rtc_minute,rtc_second);
    //NYC Time   
    rtc_hour=rtc_hour-5<0?rtc_hour-5+24:rtc_hour-5;
    
    time_bar(rtc_hour,rtc_minute,rtc_second,14);
        
    }
    
    outb(0x20,0x20);
    outb(0x20,0xA0);
    
    outb(0x0C,0x70);
    inb(0x71);
   
    
}



void _hndlr_isr0(registers_t regs){
    kprintf("0x0 divide by 0");
    exit(0);
}
void _hndlr_isr1(registers_t regs){
    kprintf("0x01    Single-step interrupt (see trap flag)");
    exit(0);
}
void _hndlr_isr2(registers_t regs){
    kprintf("0x02    NMI");
    exit(0);
}
void _hndlr_isr3(registers_t regs){
    kprintf("0x03    Breakpoint (callable by the special 1-byte instruction 0xCC, used by debuggers)");
    exit(0);
}
void _hndlr_isr4(registers_t regs){
    kprintf("0x04    Overflow");
    exit(0);
}

void _hndlr_isr5(registers_t regs){
    kprintf("0x05    Bounds");
    exit(0);
}

void _hndlr_isr6(registers_t regs){
    kprintf("0x06    Invalid Opcode");
    exit(0);
}

void _hndlr_isr7(registers_t regs){
    kprintf("0x07    Coprocessor not available");
    exit(0);
}

void _hndlr_isr8(registers_t regs){
    kprintf("0x08    Double fault");
    exit(0);
}

void _hndlr_isr9(registers_t regs){
    kprintf("0x09    Coprocessor Segment Overrun (386 or earlier only)");
    exit(0);
}

void _hndlr_isr10(registers_t regs){
    kprintf(" 0x0A    Invalid Task State Segment");
    exit(0);
}

void _hndlr_isr11(registers_t regs){
    kprintf("0x0B    Segment not present");
    exit(0);
}

void _hndlr_isr12(registers_t regs){
    kprintf("0x0C    Stack Fault");
    exit(0);
}

void _hndlr_isr13(){
    kprintf("0x0D    General protection fault %d",errorCode);
    
    exit(0);
}

void _hndlr_isr14(){
    
    //kprintf("\n0x0E    Page fault");
     uint64_t pagefaultAt;
    __asm__ __volatile__("movq %%cr2, %%rax; movq %%rax, %0;":"=m"(pagefaultAt)::"%rax");
    
    if((pagefaultAt&FRAME) == 0xFFFFFF7EFFFF5000)
    {
        kprintf("Reached the stack border: Cannot allocate this page\n");
        exit(0);
    }
   
//    kprintf("\nPAGE FAULT AT : %p Error Code: %d\n",pagefaultAt,errorCode);
    
    int p = errorCode&0x1;
    errorCode>>=1;
    int rw = errorCode&0x1;
    errorCode>>=1;
    int us = errorCode&0x1;
    errorCode>>=1;
    
    //kprintf("%d %d %d",us,rw,p);
    
    //At this stage if the page fault is not zero.. the error is valid error and not be handled
    if(errorCode!=0)
    {
//        kprintf("Kill this bad guy..");
        kprintf("%d",errorCode);
        exit(0);
    }
    if((us&rw&!p) || ((!us)&rw&(!p)))
    {
//        kprintf("Handling page fault ");
        
        if(isPartofCurrentVma(pagefaultAt&FRAME))
        {
            void *ptr=pageAllocator();
            mapPageForUser(pagefaultAt&FRAME,(uint64_t)ptr,(uint64_t)(getRunCr3()+get_kernbase()));
            memset((uint64_t)ptr+get_kernbase());
            
            
//            Task* runningThread = getRunningThread();
//            uint64_t pNum = getPageNumFromAddr(pagefaultAt&FRAME);
//            if(pNum==-1)
//            {
//                kprintf("ERROR");
//                exit(0);
//
//            }
//            makePageCopiesForChilden(pNum,runningThread);
//            markPageAsRW(pagefaultAt&FRAME,(runningThread->regs.cr3+get_kernbase()),0); //0enable write
            
        }
        else if(us&rw&!p){
            //kprintf("%d\n",errorCode);
            kprintf("Segmentation Fault..Unauthorised access to meomry..Killing the process..\n");
            //need to write code here to kill it..
            exit(0);
        } 
        else if((!us)&rw&(!p)){
            //kprintf("%d\n",errorCode);
            kprintf("\nPAGE FAULT AT : %p Error Code: %d\n",pagefaultAt,errorCode);
            kprintf("Segmentation Fault..Please restart\n");
            //need to write code here to kill it..
            //exit(0);
            while(1);
        } 
        //Task *task=getRunningThread();
        //kprintf("\n*** %d ***\n",task->pid_t);
//        kprintf("Handling page fault DONE ");
    }
    else if(us&rw&p)
    {
        Task* runningThread = getRunningThread();
//        uint64_t pNum = getPageNumFromAddr(pagefaultAt&FRAME);
//        if(pNum==-1)
//        {
//            kprintf("ERROR");
//            exit(0);
//
//        }
//        makePageCopiesForChilden(pNum,runningThread);
//        markPageAsRW(pagefaultAt&FRAME,(runningThread->regs.cr3+get_kernbase()),0); //0enable write
        int pageCount=1;
        int64_t phyAddr = getPhysicalPageAddr(pagefaultAt&FRAME,getRunCr3());
        phyAddr&=FRAME;
        Task* temp = runningThread->next;
        while(temp!=runningThread)
        {
            uint64_t tempPhy = getPhysicalPageAddr(pagefaultAt&FRAME,temp->regs.cr3);
            if(tempPhy!=-1 && tempPhy!=0&& (tempPhy&FRAME)==phyAddr)
                pageCount+=1; //how many cr3 contain this phy address,virtual address combo
            temp=temp->next;
        }
        
        if(pageCount==1)
        {
            markPageAsRW(pagefaultAt&FRAME,(getRunCr3()+get_kernbase()),0);
            
        }
        else
        {
            uint64_t newPage = (uint64_t)kmalloc();
            newPage-=get_kernbase();
            phyAddr+=get_kernbase();
            mapPageForUser(pagefaultAt&FRAME,newPage,getRunCr3()+get_kernbase());
            invlpg(pagefaultAt&FRAME);  //do at every change of page table entry for the current tlb
            memcpy((void *)phyAddr,(void *)(pagefaultAt&FRAME),4096);
            
        }
        
    }
    else if(us&!rw&!p){
        kprintf("%d",errorCode);
        kprintf("Segmentation Fault..Unauthorised access to meomry..Killing the process..\n");
        exit(0);
    }
    else if(!us&!rw&!p){
        kprintf("%d",errorCode);
        kprintf("Segmentation Fault..Please Restart.\n");
        while(1);
    }
    else
    {  
        kprintf("%d",errorCode);
        kprintf("Segmentation Fault..\n");
        exit(0);
    }
    
}

void _hndlr_isr15(registers_t regs){
    kprintf("0x0F    reserved");
    exit(0);
}

void _hndlr_isr16(registers_t regs){
    kprintf("0x10    Math Fault");
    exit(0);
}

void _hndlr_isr17(registers_t regs){
    kprintf("0x11    Alignment Check");
    exit(0);
}

void _hndlr_isr18(registers_t regs){
    kprintf("0x12    Machine Check");
    exit(0);
}

void _hndlr_isr19(registers_t regs){
    kprintf("0x13    SIMD Floating-Point Exception");
    exit(0);
}

void _hndlr_isr20(registers_t regs){
    kprintf("0x14    Virtualization Exception");
    exit(0);
}
void _hndlr_isr21(registers_t regs){
    kprintf("0x15 Control Protection Exception");
    exit(0);
}

void _isr0();
void _isr1();
void _isr2();
void _isr3();
void _isr4();
void _isr5();
void _isr6();
void _isr7();
void _isr8();
void _isr9();
void _isr10();
void _isr11();
void _isr12();
void _isr13();
void _isr14();
void _isr15();
void _isr16();
void _isr17();
void _isr18();
void _isr19();
void _isr20();
void _isr21();


void _key_board_intr();


void _timer_intr();

void _rtc_intr();

void init_idt()
{
    id_set_gate(40,(uint64_t)_rtc_intr,8,0x8E);
    id_set_gate(33,(uint64_t)_key_board_intr,8,0x8E);
    id_set_gate(32,(uint64_t)_timer_intr,8,0x8E);
    
    id_set_gate(0x00,(uint64_t)_isr0,8,0x8E);
    id_set_gate(0x01,(uint64_t)_isr1,8,0x8E);
    id_set_gate(0x02,(uint64_t)_isr2,8,0x8E);
    id_set_gate(0x03,(uint64_t)_isr3,8,0x8E);
    id_set_gate(0x04,(uint64_t)_isr4,8,0x8E);
    id_set_gate(0x05,(uint64_t)_isr5,8,0x8E);
    id_set_gate(0x06,(uint64_t)_isr6,8,0x8E);
    id_set_gate(0x07,(uint64_t)_isr7,8,0x8E);
    id_set_gate(0x08,(uint64_t)_isr8,8,0x8E);
    id_set_gate(0x09,(uint64_t)_isr9,8,0x8E);
    id_set_gate(0x0A,(uint64_t)_isr10,8,0x8E);
    id_set_gate(0x0B,(uint64_t)_isr11,8,0x8E);
    id_set_gate(0x0C,(uint64_t)_isr12,8,0x8E);
    id_set_gate(0x0D,(uint64_t)_isr13,8,0x8E);
    id_set_gate(0x0E,(uint64_t)_isr14,8,0x8E);
    id_set_gate(0x0F,(uint64_t)_isr15,8,0x8E);
    id_set_gate(0x10,(uint64_t)_isr16,8,0x8E);
    id_set_gate(0x11,(uint64_t)_isr17,8,0x8E);
    id_set_gate(0x12,(uint64_t)_isr18,8,0x8E);
    id_set_gate(0x13,(uint64_t)_isr19,8,0x8E);
    id_set_gate(0x14,(uint64_t)_isr20,8,0x8E);
    id_set_gate(0x15,(uint64_t)_isr21,8,0x8E);
    
    __asm__ __volatile__("lidt %0" : : "m" (idtp));
    
}

void writeChar(char c,int specialCharacter){//writePosition is place where character will be written
  if(specialCharacter==1){
      *writePosition='^';
      writePosition++;
      if(writePosition==end){
          writePosition=stdStart;
          indicator=1;
      }
  }
  *writePosition=c;
  writePosition++;
  if(writePosition==end){
      readChar();
      if(readPosition!=stdStart){
        writePosition=stdStart;
        indicator=1;
      }
      else{
        kprintf("Warning: Input Buffer Full.. Cant Take more input.. Please Type Enter\n");
      }
  }
  if((writePosition==readPosition-1) && (indicator=1)){//TODO: In this situation if its a correct command execute it, otherwise discard saying wrong command.
    kprintf("Warning: Input Buffer Full..Cant Take more input..Please Type Enter\n");
  }
  if(c=='\n'){
      readline+=1;
  }
}

char readChar(){//readPosition is place where character will be read from
  char c='\0';
  if(((((uint64_t)writePosition>(uint64_t)readPosition) && (indicator==0)) || (((uint64_t)readPosition>(uint64_t)writePosition)&&(indicator==1))) && (readline>0)){
      c=*readPosition;
      if(c=='\n'){
          readline-=1;
      }
      *readPosition='\0';
      readPosition++;
      if(readPosition==end+1){
          readPosition=stdStart;indicator=0;
      }
  }
  else{
      //runNextTask();
      return '\0';
  }
  return c;
}

void handleBackspace(){
    if(((uint64_t)(writePosition)==(uint64_t)(readPosition))){
        *writePosition='\0';
    }
    if(((uint64_t)(writePosition)>(uint64_t)(readPosition) && (indicator==0))&&(*(writePosition-1)!='\n')){
        writePosition--;
        if(writePosition==stdStart-1){
          writePosition=stdStart;
        }
        *writePosition='\0';
        kprintf("\b");
    }
    if((((uint64_t)readPosition>(uint64_t)writePosition)&&(indicator==1)) && (*(writePosition-1)!='\n')){
        writePosition--;
        if(writePosition==stdStart-1){
          writePosition=end;
          indicator=0;
        }
        *writePosition='\0';
        kprintf("\b");
    }
}
