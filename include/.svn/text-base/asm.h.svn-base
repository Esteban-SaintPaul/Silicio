#include <idt.h>
#include <gdt.h>

#ifndef __ASH__

#define __ASH__

extern char kernel_fin;
#define KERNEL_FIN (unsigned long)&kernel_fin
#define KERNEL_INI 0x100000l

#define DIR_PAG_LEN	1024l
#define	TAB_PAG_LEN	1024l
/* #define CAN_TAB_PAG	8l*/
#define CAN_TAB_PAG	1024l
extern gdtp *ldt_p;
extern gdtp *gdt_p;
extern idtp *idt_p;


void int_00();
void int_01();
void int_02();
void int_03();
void int_04();
void int_05();
void int_06();
void int_07();
void int_08();
void int_09();
void int_0a();
void int_0b();
void int_0c();
void int_0d();
void int_0e();
void int_0f();


#endif

/*
#ifndef __PAG__
extern unsigned long dir_pag[DIR_PAG_LEN];
extern unsigned long tab_pag[CAN_TAB_PAG][TAB_PAG_LEN];
#endif 
*/
