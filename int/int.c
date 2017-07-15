#include <asm.h>
/* Para
	void int0n() */

#include <io.h>
/* Para
	iret() include/io.h   */

#include <stdio.h>
/* Para
	printk() stdio/stdio.h  */

#include <keyb.h>
/* Para
	keyb() keyb/keyb.c    */
#include <int.h>
#include <exec.h>
/*
	int_hard_##()
*/

void ptoshort ( void* func, unsigned short* pl,unsigned short* ph );

void intnula();

// unsigned long reg_int(int n_int, void* funcion);

unsigned char t_int;

int int_up(unsigned char n){
	char byte;
	if(n > 15 && n < 24){
		n -= 16;
		byte = 1 << n;
		t_int &= ~byte;
		outb(0x21, t_int );
		return(0);
	}
	else return(-1);
}

int int_down(unsigned char n){
	char byte;
	if(n > 15 && n < 24){
		n -= 16;
		byte = 1 << n;
		t_int |= byte;
		outb(0x21, t_int );
		return(0);
	}
	else return(-1);
}

void int_set ( idtp* p )
{
	int i;

	for ( i=0 ; i < 256 ; i++ )
	{
		add_int ( i , intnula , p , 0x8e , 0x8 , 0 );
	}
		add_int ( 0 , int_00 , p , 0x8e , 0x8 , 0 );
		add_int ( 1 , int_01 , p , 0x8e , 0x8 , 0 );
		add_int ( 2 , int_02 , p , 0x8e , 0x8 , 0 );
		add_int ( 3 , int_03 , p , 0x8e , 0x8 , 0 );
		add_int ( 4 , int_04 , p , 0x8e , 0x8 , 0 );
		add_int ( 5 , int_05 , p , 0x8e , 0x8 , 0 );
		add_int ( 6 , int_06 , p , 0x8e , 0x8 , 0 );
		add_int ( 7 , int_07 , p , 0x8e , 0x8 , 0 );

		add_int ( 8 , int_08 , p , 0x8e , 0x8 , 0 );
		add_int ( 9 , int_09 , p , 0x8e , 0x8 , 0 );
		add_int ( 10 , int_0a , p , 0x8e , 0x8 , 0 );
		add_int ( 11 , int_0b , p , 0x8e , 0x8 , 0 );
		add_int ( 12 , int_0c , p , 0x8e , 0x8 , 0 );
		add_int ( 13 , int_0d , p , 0x8e , 0x8 , 0 );
		add_int ( 14 , int_0e , p , 0x8e , 0x8 , 0 );
		add_int ( 15 , int_0f , p , 0x8e , 0x8 , 0 );

		add_int ( 17 , int_hard_17 , p , 0x8e , 0x8 , 0 );
		add_int ( 0x41 , int_soft_speed , p , 0xee , 0x8 , 0 );

}

void add_int (int num_int, void* funcion, idtp* p , unsigned char acceso, unsigned short selec, unsigned char cont)
{
	unsigned short pl;
	unsigned short ph;

	ptoshort ( funcion , &pl, &ph );

	p[num_int].desp_l= pl;
	p[num_int].selec = selec;
	p[num_int].cont  = cont;
	p[num_int].acceso=acceso;
	p[num_int].desp_h= ph;
}

void ptoshort ( void* func, unsigned short* pl,unsigned short* ph )
{
	unsigned long pint_l;
	unsigned long pint_h;
	pint_l = (unsigned long) func;
	pint_l = 0x0000ffff & pint_l;
	pint_h = (unsigned long) func;
	pint_h = pint_h >> 16;
	pint_h = 0x0000ffff & pint_h;
	*pl = (unsigned short) pint_l;
	*ph = (unsigned short) pint_h;
}

void intnula()
{
printk("Interrupcion nula\n");
iret();
return;
}

unsigned long reg_int(int n_int, void* funcion){
	add_int ( n_int , funcion , idt_p , 0x8e , 0x8 , 0 );

	return(0);
}

