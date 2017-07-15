#include <idt.h>

#ifndef __INT__

#define __INT__

void int_set ( idtp* p );
void add_int (	int num_int,		\
		void* funcion,		\
		idtp* p,		\
		unsigned char acceso,	\
		unsigned short selec,	\
		unsigned char cont	);
int int_up(unsigned char n);
int int_down(unsigned char n);
unsigned long reg_int(int n_int, void* func);

#endif
