#include <multiboot.h>
/*
	struct multiboot;
	struct vbe_mode;
*/

#include <stdio.h>
/*
	cls();
	printk(char * string, ... );
*/

#include <string.h>
/*
	strcmp();
*/

#include <exec.h>
/*
	ini_ldt();
	ini_tss();
	execk();
	exec();
*/

#include <mm.h>
/*
	ini_mm();
*/
#include <version.h>
/*
	VERSION
	version();
*/

#include <int.h>
/*
	int_set(idt* idt_p);
*/

#include <io.h>
/*
	sti();
	cli();
	set_8259();
*/

#include <asm.h>
/* Para
	idtp *idt_p  */


#include <fd.h>
/*
	int fd_ini(void);   */

#include <bmp.h>
/*
	s_bmp_header;
	s_bmp_pixel;		*/

#include <vbe.h>
/*
	s_vbe_header;		*/

#include <font.h>
/*
	s_font;			*/

/*funcio que carga una fuente para modo grafico*/
void open_mod_font(multiboot *data, s_font *f);

/*para comenzar a crear caracteres*/
/* Tabla de fuentes bmp 8x16 bits, 256 caracteres representables*/
char f[16 * 256];
s_font font;

#define MODE_KERNEL 0	// En '1' start.bin correra en modo kernel
			// '0' en modo usuario.

/*Estructura leida por exec/exec.c para informar sobre el video en el 
syscall */

s_vbe_header vbe_header;


void _main( multiboot* multiboot_data )
{

	modules_info *mod_info, *start;
	int mod_count;
	unsigned long aux;
	unsigned char stack_call[200], stack_control[200];

/*----------------------------------------------------*/
/* Definiciones para manipular archivo bmp		*/

	s_bmp_header bmp_header;

/*---------------------------------------------------*/
/* definicion de fuente	de letra*/

	open_mod_font(multiboot_data, &font);

/*----------------------------------------------------*/
/* Inicio el modo de video para el kernel		*/

	open_mod_vbe(multiboot_data, &vbe_header);


/*----------------------------------------------------*/
	aux = open_mod_bmp(multiboot_data, &bmp_header);
	if(aux ==0){
		bmp2vbe(&bmp_header, &vbe_header);
	}else{
		cls();
	}
/*---------------------------------------------------*/
	printk("                Silicio SO - %s \n\n", version());
/*
printk("abcdefghijklmnopqrstuvwxyz\n");
printk("1234567890\n");
printk("[]{}:=\"-.\\/?()_\n");
printk("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
while(1);
*/
/*	Seteo desde que lugar de la memoria puedo comenzar a pedir páginas sin
	pisar ni el kernel ni los drivers
*/
	if (multiboot_data->mods_count > 0){
		mod_info=(modules_info *) multiboot_data->mods_addr;
		for (	mod_count=1; \
			mod_count < multiboot_data->mods_count ;\
			mod_count++){
			mod_info++;
		}
		set_mem_off( addr2pag(mod_info->mod_end) + 1);
	}else{
		printk("main() : No se encontro ningun modulo\n");
		return;
	}

	aux = ini_mm();
	printk("ini_mm()      : Memoria del sistema %d Mb\n",aux * 4096 / (1024*1024) );

	int_set(idt_p);		/*desde la 0 a la 255*/
	printk("ini_set()     : Interrupciones seteadas\n");

	set_8259();	/* Inicio chip de interrupciones */
	printk("set_8259()    : Controlador de interrupciones hard iniciado\n");

	ini_ldt(); /*seteo desc de ldt en gdt[3]*/
	printk("ini_ldt()     : LDT seteada\n");

	ini_tss();
	printk("ini_tss()     : Tabla TSS inicializada\n");

	ini_pag();
	printk("ini_pag()     : Paginacion iniciada\n");


	aux=execs( task_control,0,"/boot/task_control.sys");
	ltr(aux-8);
	printk("execs()       : Tarea task_contol cargada \[0x%x]\n",aux);

	aux=execs( sys_call, &stack_call[199], "/boot/sys_call.sys");
	printk("execs()       : Tarea sys_call cargada \[0x%x]\n",aux);
	add_int(	0x40,		\
			0,		\
			idt_p,		\
			0xe5,		\
			aux - 8,		\
			0	);

	aux=execs( sys_control, &stack_control[199], "/boot/sys_control.sys");
	printk("execs()       : Tarea sys_control cargada \[0x%x]\n",aux);
	add_int(	16,		\
			0,		\
			idt_p,		\
			0x85,		\
			aux - 8,		\
			0	);



	if (multiboot_data->mods_count > 0)
	{
		printk("Estandar Multiboot :\n");
		mod_info=(modules_info *) multiboot_data->mods_addr;
		for (	mod_count=0, start=0; \
			mod_count < multiboot_data->mods_count ; \
			mod_count++,mod_info++){
			if(0 == strcmp((char *)mod_info->string,"/boot/start.bin")){
				start=mod_info;
			}else{
				if(0 == strcmp((char *)mod_info->string,"/boot/background.bmp")){
//					background=mod_info;
				}
				else{
					aux=exec(mod_info->mod_start,mod_info->mod_end, (char*)mod_info->string);
					printk("    exec(): Modulo %s \[%d]\n",mod_info->string,aux);
				}
			}
		}
	}
	if(start!=0){
		if(MODE_KERNEL){
			aux=execd(	start->mod_start,	\
					start->mod_end,		\
					(char*) start->string);

			printk(		"    execd(): Modulo %s \[%d]\n",\
					start->string, aux);
		} else{
			aux=exec(	start->mod_start,	\
					start->mod_end,		\
					(char*) start->string);

			printk(		"    exec(): Modulo %s \[%d]\n",\
					start->string, aux);
		}
	} else {
		printk("main() : No se encontro el modulo \"/boot/start.bin\"\n");
		return;
	}

	/*---------------------------------------------------------*/
	//fd_ini();

//while(1);
	printk("task_control(): Activo Administrador de Tareas\n");
	task_control();

	return;
}

unsigned long open_mod_vbe(multiboot *multi, s_vbe_header *vbe){

	vbe_mode *vbe_mode_info;

	vbe->modo = multi->vbe_mode;
	if(multi->vbe_mode != 0){
		vbe_mode_info = (vbe_mode*)multi->vbe_mode_info;

		vbe->addr = (char*)vbe_mode_info->phys_base;
		vbe->x= vbe_mode_info->x_resolution;
		vbe->y= vbe_mode_info->y_resolution;
		vbe->xy=vbe->x * vbe->y ;
		vbe->color=vbe_mode_info->bits_per_pixel;
	}else {
		vbe->addr= (char*)0xb8000;
		vbe->x= 80;
		vbe->y= 25;
		vbe->xy = 80 * 25 ;
		vbe->color=0;
	}

	return(multi->vbe_mode);
}

void open_mod_font(multiboot *data, s_font *fo){
	unsigned long i;
	fo->addr = f;
	fo->x= 8;
	fo->y=16;

	for( i=0; i< (256*16); i+=16 ){
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x0;
		f[i+6]=0x0;
		f[i+7]=0x0;
		f[i+8]=0x3f;
		f[i+9]=0x0;
		f[i+10]=0x0;
		f[i+11]=0x0;
		f[i+12]=0x0;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	}
	i=16 * 'a';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x1e;
		f[i+6]=0x21;
		f[i+7]=0x20;
		f[i+8]=0x2e;
		f[i+9]=0x31;
		f[i+10]=0x21;
		f[i+11]=0x31;
		f[i+12]=0x2e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'b';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x01;
		f[i+4]=0x01;
		f[i+5]=0x1d;
		f[i+6]=0x23;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x23;
		f[i+12]=0x1d;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'c';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x1e;
		f[i+6]=0x21;
		f[i+7]=0x01;
		f[i+8]=0x01;
		f[i+9]=0x01;
		f[i+10]=0x01;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'd';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x20;
		f[i+4]=0x20;
		f[i+5]=0x2e;
		f[i+6]=0x31;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x31;
		f[i+12]=0x2e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'e';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x1e;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x3f;
		f[i+9]=0x01;
		f[i+10]=0x01;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'f';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x18;
		f[i+4]=0x24;
		f[i+5]=0x04;
		f[i+6]=0x0e;
		f[i+7]=0x04;
		f[i+8]=0x04;
		f[i+9]=0x04;
		f[i+10]=0x04;
		f[i+11]=0x04;
		f[i+12]=0x04;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'g';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x2e;
		f[i+6]=0x31;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x31;
		f[i+12]=0x2e;
		f[i+13]=0x20;
		f[i+14]=0x21;
		f[i+15]=0x1e;
	i=16 * 'h';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x01;
		f[i+4]=0x01;
		f[i+5]=0x1d;
		f[i+6]=0x23;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;

	i=16 * 'i';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x8;
		f[i+4]=0x0;
		f[i+5]=0x0c;
		f[i+6]=0x8;
		f[i+7]=0x8;
		f[i+8]=0x8;
		f[i+9]=0x8;
		f[i+10]=0x8;
		f[i+11]=0x8;
		f[i+12]=0x1c;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'j';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x20;
		f[i+4]=0x0;
		f[i+5]=0x30;
		f[i+6]=0x20;
		f[i+7]=0x20;
		f[i+8]=0x20;
		f[i+9]=0x20;
		f[i+10]=0x20;
		f[i+11]=0x20;
		f[i+12]=0x20;
		f[i+13]=0x21;
		f[i+14]=0x21;
		f[i+15]=0x1e;
	i=16 * 'k';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x1;
		f[i+4]=0x1;
		f[i+5]=0x9;
		f[i+6]=0x5;
		f[i+7]=0x3;
		f[i+8]=0x3;
		f[i+9]=0x5;
		f[i+10]=0x09;
		f[i+11]=0x11;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'l';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x2;
		f[i+4]=0x2;
		f[i+5]=0x2;
		f[i+6]=0x2;
		f[i+7]=0x2;
		f[i+8]=0x2;
		f[i+9]=0x2;
		f[i+10]=0x2;
		f[i+11]=0x2;
		f[i+12]=0x1c;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'm';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x1d;
		f[i+6]=0x2a;
		f[i+7]=0x2a;
		f[i+8]=0x2a;
		f[i+9]=0x2a;
		f[i+10]=0x2a;
		f[i+11]=0x2a;
		f[i+12]=0x2a;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'n';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x1d;
		f[i+6]=0x22;
		f[i+7]=0x22;
		f[i+8]=0x22;
		f[i+9]=0x22;
		f[i+10]=0x22;
		f[i+11]=0x22;
		f[i+12]=0x22;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'o';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x1e;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'p';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x1d;
		f[i+6]=0x23;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x23;
		f[i+12]=0x1d;
		f[i+13]=0x1;
		f[i+14]=0x1;
		f[i+15]=0x1;
	i=16 * 'q';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x2e;
		f[i+6]=0x31;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x31;
		f[i+12]=0x2e;
		f[i+13]=0x20;
		f[i+14]=0x20;
		f[i+15]=0x20;
	i=16 * 'r';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x1d;
		f[i+6]=0x23;
		f[i+7]=0x01;
		f[i+8]=0x01;
		f[i+9]=0x01;
		f[i+10]=0x01;
		f[i+11]=0x01;
		f[i+12]=0x01;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 's';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x1e;
		f[i+6]=0x21;
		f[i+7]=0x01;
		f[i+8]=0x06;
		f[i+9]=0x18;
		f[i+10]=0x20;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 't';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x04;
		f[i+4]=0x04;
		f[i+5]=0x3f;
		f[i+6]=0x04;
		f[i+7]=0x04;
		f[i+8]=0x04;
		f[i+9]=0x04;
		f[i+10]=0x04;
		f[i+11]=0x04;
		f[i+12]=0x18;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'u';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'v';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x12;
		f[i+12]=0x0c;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'w';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x2d;
		f[i+11]=0x2d;
		f[i+12]=0x12;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'x';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x12;
		f[i+8]=0x0c;
		f[i+9]=0x0c;
		f[i+10]=0x12;
		f[i+11]=0x21;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'y';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x31;
		f[i+12]=0x2e;
		f[i+13]=0x20;
		f[i+14]=0x21;
		f[i+15]=0x1e;
	i=16 * 'z';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x3f;
		f[i+6]=0x20;
		f[i+7]=0x10;
		f[i+8]=0x08;
		f[i+9]=0x04;
		f[i+10]=0x02;
		f[i+11]=0x01;
		f[i+12]=0x3f;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'A';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0c;
		f[i+3]=0x12;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x3f;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'B';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1f;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x1f;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x1f;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'C';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x01;
		f[i+5]=0x01;
		f[i+6]=0x01;
		f[i+7]=0x01;
		f[i+8]=0x01;
		f[i+9]=0x01;
		f[i+10]=0x01;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'D';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1f;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x1f;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'E';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x3f;
		f[i+3]=0x01;
		f[i+4]=0x01;
		f[i+5]=0x01;
		f[i+6]=0x01;
		f[i+7]=0x0f;
		f[i+8]=0x01;
		f[i+9]=0x01;
		f[i+10]=0x01;
		f[i+11]=0x01;
		f[i+12]=0x3f;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'F';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x3f;
		f[i+3]=0x01;
		f[i+4]=0x01;
		f[i+5]=0x01;
		f[i+6]=0x01;
		f[i+7]=0x0f;
		f[i+8]=0x01;
		f[i+9]=0x01;
		f[i+10]=0x01;
		f[i+11]=0x01;
		f[i+12]=0x01;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'G';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x01;
		f[i+5]=0x01;
		f[i+6]=0x01;
		f[i+7]=0x39;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x3e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'H';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x3f;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'I';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0e;
		f[i+3]=0x04;
		f[i+4]=0x04;
		f[i+5]=0x04;
		f[i+6]=0x04;
		f[i+7]=0x04;
		f[i+8]=0x04;
		f[i+9]=0x04;
		f[i+10]=0x04;
		f[i+11]=0x04;
		f[i+12]=0x0e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'J';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x3f;
		f[i+3]=0x21;
		f[i+4]=0x20;
		f[i+5]=0x20;
		f[i+6]=0x20;
		f[i+7]=0x20;
		f[i+8]=0x20;
		f[i+9]=0x20;
		f[i+10]=0x20;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'K';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x11;
		f[i+4]=0x09;
		f[i+5]=0x05;
		f[i+6]=0x03;
		f[i+7]=0x01;
		f[i+8]=0x03;
		f[i+9]=0x05;
		f[i+10]=0x09;
		f[i+11]=0x11;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'L';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x01;
		f[i+3]=0x01;
		f[i+4]=0x01;
		f[i+5]=0x01;
		f[i+6]=0x01;
		f[i+7]=0x01;
		f[i+8]=0x01;
		f[i+9]=0x01;
		f[i+10]=0x01;
		f[i+11]=0x01;
		f[i+12]=0x3f;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'M';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x33;
		f[i+4]=0x2d;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'N';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x23;
		f[i+4]=0x23;
		f[i+5]=0x25;
		f[i+6]=0x25;
		f[i+7]=0x29;
		f[i+8]=0x29;
		f[i+9]=0x031;
		f[i+10]=0x31;
		f[i+11]=0x21;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'O';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x01e;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'P';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1f;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x1f;
		f[i+8]=0x01;
		f[i+9]=0x01;
		f[i+10]=0x01;
		f[i+11]=0x01;
		f[i+12]=0x01;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'Q';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x29;
		f[i+11]=0x11;
		f[i+12]=0x2e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'R';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1f;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x1f;
		f[i+8]=0x03;
		f[i+9]=0x05;
		f[i+10]=0x09;
		f[i+11]=0x11;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'S';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x01;
		f[i+5]=0x01;
		f[i+6]=0x01;
		f[i+7]=0x1e;
		f[i+8]=0x20;
		f[i+9]=0x20;
		f[i+10]=0x20;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'T';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x3f;
		f[i+3]=0x04;
		f[i+4]=0x04;
		f[i+5]=0x04;
		f[i+6]=0x04;
		f[i+7]=0x04;
		f[i+8]=0x04;
		f[i+9]=0x04;
		f[i+10]=0x04;
		f[i+11]=0x04;
		f[i+12]=0x04;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'U';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'V';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x12;
		f[i+12]=0x0c;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'X';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x21;
		f[i+4]=0x12;
		f[i+5]=0x12;
		f[i+6]=0x0a;
		f[i+7]=0x0c;
		f[i+8]=0x14;
		f[i+9]=0x12;
		f[i+10]=0x12;
		f[i+11]=0x21;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'Y';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x31;
		f[i+9]=0x2e;
		f[i+10]=0x20;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'Z';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x3f;
		f[i+3]=0x20;
		f[i+4]=0x10;
		f[i+5]=0x10;
		f[i+6]=0x08;
		f[i+7]=0x08;
		f[i+8]=0x04;
		f[i+9]=0x04;
		f[i+10]=0x02;
		f[i+11]=0x01;
		f[i+12]=0x3f;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * 'W';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x2d;
		f[i+11]=0x33;
		f[i+12]=0x21;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;

	i=16 * '1';//numeros
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x08;
		f[i+3]=0x0c;
		f[i+4]=0x0a;
		f[i+5]=0x09;
		f[i+6]=0x08;
		f[i+7]=0x08;
		f[i+8]=0x08;
		f[i+9]=0x08;
		f[i+10]=0x08;
		f[i+11]=0x08;
		f[i+12]=0x3f;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '2';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x20;
		f[i+5]=0x20;
		f[i+6]=0x10;
		f[i+7]=0x08;
		f[i+8]=0x04;
		f[i+9]=0x02;
		f[i+10]=0x01;
		f[i+11]=0x01;
		f[i+12]=0x3f;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '3';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x20;
		f[i+5]=0x20;
		f[i+6]=0x10;
		f[i+7]=0x0c;
		f[i+8]=0x10;
		f[i+9]=0x20;
		f[i+10]=0x20;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '4';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x21;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x21;
		f[i+8]=0x3f;
		f[i+9]=0x20;
		f[i+10]=0x20;
		f[i+11]=0x20;
		f[i+12]=0x20;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '5';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1f;
		f[i+3]=0x01;
		f[i+4]=0x01;
		f[i+5]=0x1d;
		f[i+6]=0x23;
		f[i+7]=0x20;
		f[i+8]=0x20;
		f[i+9]=0x20;
		f[i+10]=0x20;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '6';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x01;
		f[i+5]=0x1d;
		f[i+6]=0x23;
		f[i+7]=0x21;
		f[i+8]=0x21;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '7';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x3f;
		f[i+3]=0x21;
		f[i+4]=0x10;
		f[i+5]=0x10;
		f[i+6]=0x08;
		f[i+7]=0x08;
		f[i+8]=0x08;
		f[i+9]=0x04;
		f[i+10]=0x04;
		f[i+11]=0x04;
		f[i+12]=0x04;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '8';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x12;
		f[i+7]=0x0c;
		f[i+8]=0x12;
		f[i+9]=0x21;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '9';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x21;
		f[i+7]=0x31;
		f[i+8]=0x2e;
		f[i+9]=0x20;
		f[i+10]=0x20;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '0';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x21;
		f[i+4]=0x21;
		f[i+5]=0x21;
		f[i+6]=0x31;
		f[i+7]=0x29;
		f[i+8]=0x25;
		f[i+9]=0x23;
		f[i+10]=0x21;
		f[i+11]=0x21;
		f[i+12]=0x1e;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;

	i=16 * ' ';//espacio
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x0;
		f[i+6]=0x0;
		f[i+7]=0x0;
		f[i+8]=0x0;
		f[i+9]=0x0;
		f[i+10]=0x0;
		f[i+11]=0x0;
		f[i+12]=0x0;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '[';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x02;
		f[i+4]=0x02;
		f[i+5]=0x02;
		f[i+6]=0x02;
		f[i+7]=0x02;
		f[i+8]=0x02;
		f[i+9]=0x02;
		f[i+10]=0x02;
		f[i+11]=0x02;
		f[i+12]=0x02;
		f[i+13]=0x02;
		f[i+14]=0x02;
		f[i+15]=0x1e;
	i=16 * ']';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x1e;
		f[i+3]=0x10;
		f[i+4]=0x10;
		f[i+5]=0x10;
		f[i+6]=0x10;
		f[i+7]=0x10;
		f[i+8]=0x10;
		f[i+9]=0x10;
		f[i+10]=0x10;
		f[i+11]=0x10;
		f[i+12]=0x10;
		f[i+13]=0x10;
		f[i+14]=0x10;
		f[i+15]=0x1e;
	i=16 * '-';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x0;
		f[i+6]=0x0;
		f[i+7]=0x1e;
		f[i+8]=0x0;
		f[i+9]=0x0;
		f[i+10]=0x0;
		f[i+11]=0x0;
		f[i+12]=0x0;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * ':';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x08;
		f[i+6]=0x0;
		f[i+7]=0x0;
		f[i+8]=0x0;
		f[i+9]=0x0;
		f[i+10]=0x0;
		f[i+11]=0x0;
		f[i+12]=0x08;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '=';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x0;
		f[i+6]=0x0;
		f[i+7]=0x3f;
		f[i+8]=0x0;
		f[i+9]=0x0;
		f[i+10]=0x3f;
		f[i+11]=0x0;
		f[i+12]=0x0;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '"';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x12;
		f[i+3]=0x12;
		f[i+4]=0x12;
		f[i+5]=0x0;
		f[i+6]=0x0;
		f[i+7]=0x0;
		f[i+8]=0x0;
		f[i+9]=0x0;
		f[i+10]=0x0;
		f[i+11]=0x0;
		f[i+12]=0x0;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '.';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x0;
		f[i+6]=0x0;
		f[i+7]=0x0;
		f[i+8]=0x0;
		f[i+9]=0x0;
		f[i+10]=0x0;
		f[i+11]=0x0;
		f[i+12]=0x08;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '/';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x20;
		f[i+3]=0x20;
		f[i+4]=0x10;
		f[i+5]=0x10;
		f[i+6]=0x08;
		f[i+7]=0x08;
		f[i+8]=0x04;
		f[i+9]=0x04;
		f[i+10]=0x02;
		f[i+11]=0x02;
		f[i+12]=0x01;
		f[i+13]=0x01;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '\\';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x01;
		f[i+3]=0x01;
		f[i+4]=0x02;
		f[i+5]=0x02;
		f[i+6]=0x04;
		f[i+7]=0x04;
		f[i+8]=0x08;
		f[i+9]=0x08;
		f[i+10]=0x10;
		f[i+11]=0x10;
		f[i+12]=0x20;
		f[i+13]=0x20;
		f[i+14]=0x0;
		f[i+15]=0x0;
	i=16 * '\(';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x18;
		f[i+3]=0x04;
		f[i+4]=0x02;
		f[i+5]=0x02;
		f[i+6]=0x02;
		f[i+7]=0x02;
		f[i+8]=0x02;
		f[i+9]=0x02;
		f[i+10]=0x02;
		f[i+11]=0x02;
		f[i+12]=0x02;
		f[i+13]=0x02;
		f[i+14]=0x04;
		f[i+15]=0x18;
	i=16 * ')';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x06;
		f[i+3]=0x08;
		f[i+4]=0x10;
		f[i+5]=0x10;
		f[i+6]=0x10;
		f[i+7]=0x10;
		f[i+8]=0x10;
		f[i+9]=0x10;
		f[i+10]=0x10;
		f[i+11]=0x10;
		f[i+12]=0x10;
		f[i+13]=0x10;
		f[i+14]=0x08;
		f[i+15]=0x06;
	i=16 * '\?';
		f[i]=0x1e;
		f[i+1]=0x21;
		f[i+2]=0x21;
		f[i+3]=0x20;
		f[i+4]=0x10;
		f[i+5]=0x10;
		f[i+6]=0x08;
		f[i+7]=0x08;
		f[i+8]=0x04;
		f[i+9]=0x04;
		f[i+10]=0x00;
		f[i+11]=0x04;
		f[i+12]=0x00;
		f[i+13]=0x00;
		f[i+14]=0x00;
		f[i+15]=0x00;

	i=16 * '{';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x18;
		f[i+3]=0x04;
		f[i+4]=0x04;
		f[i+5]=0x04;
		f[i+6]=0x08;
		f[i+7]=0x08;
		f[i+8]=0x06;
		f[i+9]=0x06;
		f[i+10]=0x08;
		f[i+11]=0x08;
		f[i+12]=0x04;
		f[i+13]=0x04;
		f[i+14]=0x04;
		f[i+15]=0x18;
	i=16 * '}';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x06;
		f[i+3]=0x08;
		f[i+4]=0x08;
		f[i+5]=0x08;
		f[i+6]=0x04;
		f[i+7]=0x04;
		f[i+8]=0x18;
		f[i+9]=0x18;
		f[i+10]=0x04;
		f[i+11]=0x04;
		f[i+12]=0x08;
		f[i+13]=0x08;
		f[i+14]=0x08;
		f[i+15]=0x06;

	i=16 * '_';
		f[i]=0x0;
		f[i+1]=0x0;
		f[i+2]=0x0;
		f[i+3]=0x0;
		f[i+4]=0x0;
		f[i+5]=0x0;
		f[i+6]=0x0;
		f[i+7]=0x0;
		f[i+8]=0x0;
		f[i+9]=0x0;
		f[i+10]=0x0;
		f[i+11]=0x0;
		f[i+12]=0x3f;
		f[i+13]=0x0;
		f[i+14]=0x0;
		f[i+15]=0x0;

}
