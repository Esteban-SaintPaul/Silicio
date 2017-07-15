#ifndef __GDT__

#define __GDT__

#define MAX_GDT 200 /*Cantidad de entradas en la gdt*/

typedef struct {  /* descriptor de segmento o sistema */
	unsigned short int lim_l;
	unsigned short int base_l;
	unsigned char  base_m;
	unsigned char  acceso;
	unsigned char  lim_h;
	unsigned char  base_h;
}gdtp;

extern gdtp* gdt_p; /* Apuntador a tabla gdt en asm/boot.S */

#endif
