/*
# TODO: This is lab1.1
# Real Mode Hello World 
 .code16
 .global start
 start:
 	movw %cs, %ax
 	movw %ax, %ds
 	movw %ax, %es
 	movw %ax, %ss
 	movw $0x7d00, %ax
 	movw %ax, %sp # setting stack pointer to 0x7d00
 	pushw $13 # pushing the size to print into stack
 	pushw $message # pushing the address of message into stack
 	callw displayStr # calling the display function
 loop:
 	jmp loop

 message:
 	.string "Hello, World!\n\0"

 displayStr:
 	pushw %bp
 	movw 4(%esp), %ax
 	movw %ax, %bp
 	movw 6(%esp), %cx
 	movw $0x1301, %ax
 	movw $0x000c, %bx
 	movw $0x0000, %dx
 	int $0x10
 	popw %bp
 	ret
*/


/*
# TODO: This is lab1.2
# Protected Mode Hello World 
  .code16
  .global start
  start:
	movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	cli
	                             #关闭中断
    in $0x92, %al  
	or $0x2, %al   
	out %al, $0x92 
                                  #启动A20总线
    
    
    data32 addr32 lgdt gdtDesc      #加载GDTR
                                     #启动保护模式
    
    movl %cr0, %eax
	or $0x1, %eax
	movl %eax, %cr0                 #设置CR0的PE位（第0位）为1

    data32 ljmp $0x08, $start32     #长跳转切换至保护模式

  .code32
  start32:
                                 #初始化DS ES FS GS SS 初始化栈顶指针ESP
	movw $0x0010, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x0018, %ax
	movw %ax, %gs
	movl $0x8000, %esp


  	pushl $13 # pushing the size to print into stack
  	pushl $message # pushing the address of message into stack
  	calll displayStr # calling the display function
 loop:
  	jmp loop

 message:
 	.string "Hello, World!\n\0"

 displayStr:
 	movl 4(%esp), %ebx
 	movl 8(%esp), %ecx
 	movl $((80*5+0)*2), %edi
 	movb $0x0c, %ah
 nextChar:
 	movb (%ebx), %al
 	movw %ax, %gs:(%edi)
 	addl $2, %edi
 	incl %ebx
 	loopnz nextChar # loopnz decrease ecx by 1
 	ret

 gdt:
     .word 0x0000                           #GDT第一个表项必须为空
	 .word 0x0000
	 .word 0x0000
	 .word 0x0000
 
     .word 0xffff                    #代码段描述符
	 .word 0x0000
	 .byte 0x00
	 .byte 0x9a
	 .byte 0xcf
     .byte 0x00
        
     .word 0xffff                    #数据段描述符
	 .word 0x0000
	 .byte 0x00
	 .byte 0x92
	 .byte 0xcf
     .byte 0x00

	 .word 0xffff                           #视频段描述符
     .word 0x8000 
	 .byte 0x0b
	 .byte 0x92
	 .byte 0xcf
     .byte 0x00

 gdtDesc:
     .word (gdtDesc - gdt -1)
     .long gdt
*/



# TODO: This is lab1.3
# Protected Mode Loading Hello World APP 
  .code16
  .global start
  start:
    movw %cs, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	cli
	                             #关闭中断
    in $0x92, %al  
	or $0x2, %al   
	out %al, $0x92 
                                  #启动A20总线
    
    
    data32 addr32 lgdt gdtDesc      #加载GDTR
                                     #启动保护模式
    
    movl %cr0, %eax
	or $0x1, %eax
	movl %eax, %cr0                 #设置CR0的PE位（第0位）为1

    data32 ljmp $0x08, $start32     #长跳转切换至保护模式

  .code32
  start32:
                                 #初始化DS ES FS GS SS 初始化栈顶指针ESP
	movw $0x0010, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %ss
	movw $0x0018, %ax
	movw %ax, %gs
	movl $0x8000, %esp
	
 	jmp bootMain # jump to bootMain in boot.c

 gdt:
     .word 0x0000                           #GDT第一个表项必须为空
	 .word 0x0000
	 .word 0x0000
	 .word 0x0000
 
     .word 0xffff                    #代码段描述符
	 .word 0x0000
	 .byte 0x00
	 .byte 0x9a
	 .byte 0xcf
     .byte 0x00
        
     .word 0xffff                    #数据段描述符
	 .word 0x0000
	 .byte 0x00
	 .byte 0x92
	 .byte 0xcf
     .byte 0x00

	 .word 0xffff                           #视频段描述符
     .word 0x8000 
	 .byte 0x0b
	 .byte 0x92
	 .byte 0xcf
     .byte 0x00

 gdtDesc:
     .word (gdtDesc - gdt -1)
     .long gdt
