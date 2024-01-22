#include "device.h"
#include "x86.h"

extern ProcessTable pcb[MAX_PCB_NUM];
extern int current;

extern TSS tss;
extern int displayRow;
extern int displayCol;

void GProtectFaultHandle(struct StackFrame *sf);

void syscallHandle(struct StackFrame *sf);
void timerHandle(struct StackFrame *sf);

void syscallWrite(struct StackFrame *sf);
void syscallPrint(struct StackFrame *sf);
void syscallFork(struct StackFrame *sf);
void syscallSleep(struct StackFrame *sf);
void syscallExit(struct StackFrame *sf);

void irqHandle(struct StackFrame *sf) {  // pointer sf = esp
    
    /* Reassign segment register */
    asm volatile("movw %%ax, %%ds" ::"a"(KSEL(SEG_KDATA)));
    /* Save esp to stackTop */
    uint32_t tmpStackTop = pcb[current].stackTop;
    pcb[current].prevStackTop = pcb[current].stackTop;
    pcb[current].stackTop = (uint32_t)sf;
    

    switch (sf->irq) {
        case -1:
            break;
        case 0xd:
            GProtectFaultHandle(sf);
            break;
        case 0x20:
            timerHandle(sf);
            break;
        case 0x80:
            syscallHandle(sf);
            break;
        default:
            assert(0);
    }

    /* Recover stackTop */
    pcb[current].stackTop = tmpStackTop;
}

void GProtectFaultHandle(struct StackFrame *sf) {
    assert(0);
    return;
}

void syscallHandle(struct StackFrame *sf) {
    switch (sf->eax) {  // syscall number
        case 0:
            syscallWrite(sf);
            break;  // for SYS_WRITE (0)
        case 1:
            syscallFork(sf);
            break;
        case 2:
            break;
        case 3:
            syscallSleep(sf);
            break;
        case 4:
            syscallExit(sf);
            break;
        /*TODO Add Fork,Sleep... */
        default:
            break;
    }
}

void shift_proc(struct StackFrame *sf){
    int i=-1;
    for (i = (current+1)%MAX_PCB_NUM; i != current; i=(i+1)%MAX_PCB_NUM){
        if (pcb[i].state == STATE_RUNNABLE)break;
    }
    if (current == i){
        if(pcb[i].state == STATE_RUNNABLE)current = i;
        else current = 0;
    }
    else current = i;
    pcb[current].state = STATE_RUNNING;
    uint32_t tmpStackTop = pcb[current].stackTop;
    tss.esp0 = pcb[current].prevStackTop;
    pcb[current].stackTop = pcb[current].prevStackTop;
	asm volatile("movl %0, %%esp"::"m"(tmpStackTop));
    asm volatile("popl %gs");
	asm volatile("popl %fs");
	asm volatile("popl %es");
	asm volatile("popl %ds");
	asm volatile("popal");
	asm volatile("addl $8, %esp");
	asm volatile("iret");
}

void timerHandle(struct StackFrame *sf) {
    for (int i = 0; i < MAX_PCB_NUM; i++) {
		if(pcb[i].state==STATE_BLOCKED){
            pcb[i].sleepTime--;
            if(pcb[i].sleepTime==0)pcb[i].state=STATE_RUNNABLE;
        }
	}
    if(pcb[current].state==STATE_RUNNING){
        if(pcb[current].timeCount!=MAX_TIME_COUNT)pcb[current].timeCount++;
        if(pcb[current].timeCount==MAX_TIME_COUNT){
            pcb[current].timeCount=0;
            pcb[current].state=STATE_RUNNABLE;
        }
    }
    if(pcb[current].state!=STATE_RUNNING){
        shift_proc(sf);
    }
    return ;
}

void syscallWrite(struct StackFrame *sf) {
    switch (sf->ecx) {  // file descriptor
        case 0:
            syscallPrint(sf);
            break;  // for STD_OUT
        default:
            break;
    }
}

// Attention:
// This is optional homework, because now our kernel can not deal with
// consistency problem in syscallPrint. If you want to handle it, complete this
// function. But if you're not interested in it, don't change anything about it
void syscallPrint(struct StackFrame *sf) {
    int sel = sf->ds;  // TODO segment selector for user data, need further
                       // modification
    char *str = (char *)sf->edx;
    int size = sf->ebx;
    int i = 0;
    int pos = 0;
    char character = 0;
    uint16_t data = 0;
    asm volatile("movw %0, %%es" ::"m"(sel));
    for (i = 0; i < size; i++) {
        asm volatile("movb %%es:(%1), %0" : "=r"(character) : "r"(str + i));
        if (character == '\n') {
            displayRow++;
            displayCol = 0;
            if (displayRow == 25) {
                displayRow = 24;
                displayCol = 0;
                scrollScreen();
            }
        } else {
            data = character | (0x0c << 8);
            pos = (80 * displayRow + displayCol) * 2;
            asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
            displayCol++;
            if (displayCol == 80) {
                displayRow++;
                displayCol = 0;
                if (displayRow == 25) {
                    displayRow = 24;
                    displayCol = 0;
                    scrollScreen();
                }
            }
        }
        // asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
        // asm volatile("int $0x20":::"memory"); //XXX Testing irqTimer during
        // syscall
    }

    updateCursor(displayRow, displayCol);
    // TODO take care of return value
    return;
}

void syscallFork(struct StackFrame *sf) {
    // TODO in lab3
	int pid = -1;
	for (int i = 0; i < MAX_PCB_NUM; i++) 
	{
		if (pcb[i].state == STATE_DEAD){
            pid = i;
			break;
        }
	}
	if (pid != -1)
	{
		for (int j = 0; j < 0x100000; j++) 
		{
			*(uint8_t *)(j + (pid+1)*0x100000) = *(uint8_t *)(j + (current+1)*0x100000);
		}
		pcb[pid].pid=pid;
		pcb[pid].prevStackTop=pcb[current].prevStackTop+(uint32_t)&pcb[pid]-(uint32_t)&pcb[current];
        pcb[pid].stackTop=pcb[current].stackTop + (uint32_t)&pcb[pid]-(uint32_t)&pcb[current];
		pcb[pid].sleepTime=0;		
		pcb[pid].state = STATE_RUNNABLE;
		pcb[pid].timeCount = 0;
		pcb[pid].regs.edi = sf->edi;
		pcb[pid].regs.esi = sf->esi;
		pcb[pid].regs.ebp = sf->ebp;
		pcb[pid].regs.xxx = sf->xxx;
		pcb[pid].regs.ebx = sf->ebx;
		pcb[pid].regs.edx = sf->edx;
		pcb[pid].regs.ecx = sf->ecx;
		pcb[pid].regs.eax = sf->eax;
		pcb[pid].regs.irq = sf->irq;
		pcb[pid].regs.error = sf->error;
		pcb[pid].regs.eip = sf->eip;
		pcb[pid].regs.esp = sf->esp;
		pcb[pid].regs.eflags = sf->eflags;
		pcb[pid].regs.cs = USEL((1 + pid * 2));
		pcb[pid].regs.ss = USEL((2 + pid * 2));
		pcb[pid].regs.ds = USEL((2 + pid * 2));
		pcb[pid].regs.es = USEL((2 + pid * 2));
		pcb[pid].regs.fs = USEL((2 + pid * 2));
		pcb[pid].regs.gs = USEL((2 + pid * 2));
		pcb[pid].regs.eax = 0; 
		pcb[current].regs.eax = pid; 
	}
	else pcb[current].regs.eax = -1; 
	return;
}

void syscallSleep(struct StackFrame *sf) {
    pcb[current].sleepTime = sf->ecx;
    pcb[current].state = STATE_BLOCKED;
    shift_proc(sf);
    sf->eax = pcb[current].sleepTime;
    return ;
}

void syscallExit(struct StackFrame *sf) {
    pcb[current].state = STATE_DEAD;
    shift_proc(sf);
    sf->eax = 0;
    return ;
}
