#include "../include/x86.h"
#include "../include/device.h"
#include "../include/common.h"

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead;
extern int bufferTail;


void GProtectFaultHandle(struct TrapFrame *tf);

void KeyboardHandle(struct TrapFrame *tf);

void syscallHandle(struct TrapFrame *tf);
void syscallWrite(struct TrapFrame *tf);
void syscallPrint(struct TrapFrame *tf);
void syscallRead(struct TrapFrame *tf);
void syscallGetChar(struct TrapFrame *tf);
void syscallGetStr(struct TrapFrame *tf);


void irqHandle(struct TrapFrame *tf) { // pointer tf = esp
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	log("irq.\n");
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%es"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%fs"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%gs"::"a"(KSEL(SEG_KDATA)));
	switch(tf->irq) {
		// TODO: 填好中断处理程序的调用
		case -1:{
			log("empty.\n");
			break;
		}
		case 0xd:{
			log("GPF.\n");
			GProtectFaultHandle(tf);
			break;
		}
		case 0x21:{
			log("keyboard.\n");
			KeyboardHandle(tf);
			break;
		}
		case 0x80:{
			log("syscall.\n");
			syscallHandle(tf);
			break;
		}
		default:{
			log("unkown.\n");
			assert(0);
		}
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}

void printChar(char c)
{
	uint16_t data = c | (0x0c << 8);
	int pos = (80 * displayRow + displayCol) * 2;
	asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
}

void KeyboardHandle(struct TrapFrame *tf){
	uint32_t code = getKeyCode();
	if(code == 0xe){ // 退格符
		// TODO: 要求只能退格用户键盘输入的字符串，且最多退到当行行首
		if(bufferHead!=bufferTail){
			bufferTail--;
			displayCol--;
			printChar(' ');
		}
	}else if(code == 0x1c){ // 回车符
		// TODO: 处理回车情况
		displayRow++;
		displayCol=0;
		keyBuffer[bufferTail]='\n';
		bufferTail=(bufferTail+1)%MAX_KEYBUFFER_SIZE;
		if(bufferTail==bufferHead)bufferHead=(bufferHead+1)%MAX_KEYBUFFER_SIZE;
	}else if(code < 0x81 && ((code > 1 && code < 0xe )||( code > 0xf && code != 0x1d && code != 0x2a && code != 0x36 && code != 0x38 && code != 0x3a && code < 0x45))){ // 正常字符
		// TODO: 注意输入的大小写的实现、不可打印字符的处理
		printChar(getChar(code));
		displayCol++;
		keyBuffer[bufferTail]=code;
		bufferTail=(bufferTail+1)%MAX_KEYBUFFER_SIZE;
		if(bufferTail==bufferHead)bufferHead=(bufferHead+1)%MAX_KEYBUFFER_SIZE;
	}

	if(displayCol==80){
		displayRow++;
		displayCol=0;
	}
	if(displayRow==25) {
		displayRow=24;
		displayCol=0;
		scrollScreen();
	}
	updateCursor(displayRow, displayCol);
}

void syscallHandle(struct TrapFrame *tf) {
	
	switch(tf->eax) { // syscall number
		case 0:
			syscallWrite(tf);
			break; // for SYS_WRITE
		case 1:
			syscallRead(tf);
			break; // for SYS_READ
		default:break;
	}
}

void syscallWrite(struct TrapFrame *tf) {
	switch(tf->ecx) { // file descriptor
		case 0:
			syscallPrint(tf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallPrint(struct TrapFrame *tf) {
	log("syscall print.\n");
	int sel = USEL(SEG_UDATA); //TODO: segment selector for user data, need further modification
	char *str = (char*)tf->edx;
	int size = tf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		// TODO: 完成光标的维护和打印到显存
		if(character=='\n') {
			displayRow++;
			displayCol=0;
		}
		else {
			data = character | (0x0c << 8);
			pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
			displayCol++;
		}
		if(displayCol==80){
			displayRow++;
			displayCol=0;
		}
		if(displayRow==25) {
			displayRow=24;
			displayCol=0;
			scrollScreen();
		}
	}
	
	updateCursor(displayRow, displayCol);
}

void syscallRead(struct TrapFrame *tf){
	switch(tf->ecx){ //file descriptor
		case 0:
			syscallGetChar(tf);
			break; // for STD_IN
		case 1:
			syscallGetStr(tf);
			break; // for STD_STR
		default:break;
	}
}

void syscallGetChar(struct TrapFrame *tf){
	// TODO: 自由实现
	log("syscall getchar.\n");
	asm volatile("sti");
	while(bufferHead==bufferTail||keyBuffer[bufferTail-1]!='\n')waitForInterrupt();
	asm volatile("cli");
	tf->eax=getChar(keyBuffer[bufferHead]);
	bufferHead=bufferTail;
}

void syscallGetStr(struct TrapFrame *tf){
	// TODO: 自由实现
	log("syscall getstr.\n");
	int sel = USEL(SEG_UDATA); 
	char *str = (char*)tf->edx;
	int size = tf->ebx;
	int i = 0;
	char c=0;
	asm volatile("sti");
	while(bufferHead==bufferTail||keyBuffer[bufferTail-1]!='\n')waitForInterrupt();
	asm volatile("cli");
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		if(bufferHead==bufferTail)break;
		if(keyBuffer[bufferHead]=='\n')break;
		c=getChar(keyBuffer[bufferHead]);
		if(c!=0)asm volatile("movb %0, %%es:(%1)"::"r"(c),"r"(str+i));
		bufferHead=(bufferHead+1)%MAX_KEYBUFFER_SIZE;
	}
	asm volatile("movb $0x00, %%es:(%0)"::"r"(str+i));
	bufferHead=bufferTail;
}
