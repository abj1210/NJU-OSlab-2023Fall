#include "./include/common.h"
#include "./include/x86.h"
#include "./include/device.h"

void kEntry(void) {

	// Interruption is disabled in bootloader
	log("Environment initialization start.\n");
	initSerial();// initialize serial port
	initIdt();// initialize idt
	initIntr();// iniialize 8259a
	initSeg();// initialize gdt, tss
	initVga(); // initialize vga device
	initKeyTable();// initialize keyboard device
	log("Environment initialization over.\n");
	loadUMain(); // load user program, enter user space

	while(1);
	assert(0);
}
