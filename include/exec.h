#ifndef __EXEC__

#define __EXEC__

#include <tss.h>

#define ltr(x) __asm__ __volatile__ (" movw %w0,%%ax; \n\t \
                                        ltr %%ax; \n\t" \
                                        ::"g" (x):"ax", "memory")

#define RUNNING		1
#define STARTING	2
#define SLEEPING	3
#define WAITING		4

unsigned long exec(unsigned long start, unsigned long end, char* name);
unsigned long execd(unsigned long start, unsigned long end, char* name);
void task_control();
void sys_call();
void sys_control();
unsigned long execs( void* start, void* stack, char* name);

void ini_tss();
void ini_ldt();

void int_hard_17();
void int_soft_speed();

#endif
