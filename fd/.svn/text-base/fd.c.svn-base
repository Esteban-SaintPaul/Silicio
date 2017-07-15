#include <stdio.h>
	/* Para usar
		printk()
	*/
#include <io.h>
	/* Para usar
		inb()
		outb()
		cli()
		sti()
		iret()
	*/
#include <fd.h>
	/* Para definir
	int fd_ini();    */

#define FD_CAP_NO_DETEC 2 /* No se que capacidad tiene la unidad */
#define FD_CONT_NO_REC 1  /* No se cual es el controlador        */
#define FD_INI 0          /* Controlodor iniciado sin problemas  */
#define FD_START 1
#define FD_STOP  0

static unsigned char fd_estado=0;

void fd_reset(void);
void fd_out(unsigned char val);
unsigned char fd_in();
void fd_motor(unsigned char estado);
void fd_esperar(void);

int fd_ini()
{
	unsigned char version,tipo;
	unsigned int time;
	printk("Iniciando unidad de disquette \"0\"\n");
	fd_reset();
	fd_out( 0x10 ); /* Orden solicitud de version */
	version = fd_in();  /* Leo la respuesta al pedido de version */
	switch (version)
	{
		case 0x80:
			printk("Controlador de disquette 765A\n");
			break;
		case 0x90:
			printk("Controlador de disquette 765B\n");
			break;
		default:
			printk("Controlador no reconocido\n");
			return(FD_CONT_NO_REC);
	}
	outb(0x70,0x10);
	tipo=inb(0x71);
	if(tipo == 0x40 )
	{
		printk("Disquette de 1.44 Mb \n");
	}
	else return(FD_CAP_NO_DETEC);
	fd_motor(FD_START);
	printk("fd : Motor encendido\n ");
	time=20; /* Retardo 2 segundos */
	do{
		fd_esperar();
		time--;
	}while(time!=0);
	fd_motor(FD_STOP);
	printk("fd : Motor detenido\n ");
	return(FD_INI);
}

void fd_reset()
{
	outb(0x3f2,0x00);/* Borro el registro y genero reset soft */
	outb(0x3f2,0x0c);/* activo el chip en int 6 */
	fd_estado = 0x0c;/* Me guardo el registro para usarlo despues */
	fd_esperar();    /* Retardo para dar tiempo al reset */
	return;
}

void fd_out(unsigned char val)
{
	unsigned int i;
	int c;
	
	i= 1000000;
	do{
		c=inb( 0x3f4 ) & (0x80 | 0x40);/*Leo registro estado*/

		/* Si está activo el bit 7 puedo escribir en
		   los registros, sino no                     */
		if( c == 0x80 ) break;
		i--;
	}while(i);
	if( c == 0 ){
		printk("fd : fd_out() : Tiempo excedido\n");
		return;
	}
	outb(0x3f5,val);/*envio a Registro de datos */
}

unsigned char fd_in()
{
	unsigned int i;
	int c;
	
	i= 1000000;
	do{
		c=inb( 0x3f4 ) & (0x80 | 0x40 | 0x10) ;/*Leo estado*/
		if( c == (0x80 | 0x40 | 0x10) ) break;
		i--;
	}while(i);
	if( c == 0 ){
		printk("kernel : fd : Tiempo excedido\n");
		return(0);
	}
	return (inb(0x3f5));/*leo registro de datos */
}


void fd_motor(unsigned char n)
{
	unsigned int i=1000000;
	if(n==FD_START){
		/* coloco en 1 el bit 4 para activar el motor
		   en fd_estado tengo el valor 0x0c luego del reset */
		fd_estado |= 0x10;
		 outb(0x3f2,fd_estado);
	}
	else
	{
		/*Si el parametro no es el correcto ( FD_STOP ) detengo
		  el motor igual pero muestro un aviso ! */
		/* Borro el bit 5 del registro 0x3f2 */
		fd_estado &= ~(0x10);
		if(n!=FD_STOP)printk("kernel : fd : error de parametro en fd_motor()\n");
		outb(0x3f2,fd_estado);
	}
	do{
		i--;
	}while(i);
}

void fd_esperar()
{
	unsigned int i=1000000;
	do{
		i--;
	}while(i!=0);
	return;
}

int fd_seek(unsigned char cabeza, unsigned char cilindro)
{
	cabeza = cabeza << 2;
	if( cabeza > 1) return(-1);
	fd_out(0x0f);
	fd_out(cabeza);
	fd_out(cilindro);
	return(0);
}
