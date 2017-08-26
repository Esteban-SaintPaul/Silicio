#ifndef __TSS__

#define __TSS__

#define KERNEL_DS	0x10
#define KERNEL_CS	0x08
#define USER_CS		0x17
#define USER_DS		0x1f
#define MAX_TASK	100
#define LDT		0x18
#define TAM_LDT		5*8

#define V_DRIVER	0x200
#define V_USER		0x300
#define V_STACK		0x3ff
#define MAX_PAG_DRIVER	8	//original 5
#define MAX_DRIVERS	10
#define MAX_PAG_VIRTUAL	6 // original 2

#define TSS_PORT	256

typedef struct tss{
	unsigned short b_link, n;
	unsigned long esp0;
	unsigned short ss0, n0;
	unsigned long esp1;
	unsigned short ss1, n1;
	unsigned long esp2;
	unsigned short ss2, n2;
	unsigned long cr3;
	unsigned long eip;
	unsigned long eflag;
	unsigned long eax;
	unsigned long ecx;
	unsigned long edx;
	unsigned long ebx;
	unsigned long esp;
	unsigned long ebp;
	unsigned long esi;
	unsigned long edi;
	unsigned short es, n3;
	unsigned short cs, n4;
	unsigned short ss, n5;
	unsigned short ds, n6;
	unsigned short fs, n7;
	unsigned short gs, n8;
	unsigned short ldt, n9, n10;
	unsigned short bit_map;
	unsigned long pag_mem[MAX_PAG_DRIVER];
	unsigned long pag_virtual[MAX_PAG_VIRTUAL];
	unsigned long pag_stack;
	unsigned char iobp[TSS_PORT];
	unsigned char fin;
}tss;



#endif
