#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <vbe.h>
#include <font.h>

extern s_vbe_header vbe_header;
extern s_font font;

#define MAX_STRING 1024
#define VIDEO 0xb8000

static int xpos = 0;
int ypos = 0;
static volatile unsigned char *video = (unsigned char *) VIDEO;

/* Exportadas en stdio.h */
//void cls ( void );
//void putc ( int c );
//void itoa ( char *buf, int base, int d );
//void printk ( char *format , ... );

/* exportadas en string.h */
//unsigned long strlen(char *str);
//unsigned long strcmp(char* buf, char* str);

/* exportadas en io.h */
void set_8259(void);
unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char val);

void bmp2graf(int c, char *tab, unsigned long x,unsigned long \
		y,unsigned long r, unsigned long g, unsigned long b);


void printk ( char *format , ... )
{
	int c;
	char *p;
	int d;
	char buf[20];
	va_list parg;
	char *s;

	va_start(parg,format);

	while ((c = *format++) != 0)
	{
		if (c != '%')
			putc (c);
		else
		{
			c = *format++;
			switch (c)
			{
				case 'c':
					d = va_arg(parg,char);
					putc ( d );
					break;
				case 'd':
				case 'u':
				case 'x':
					d = va_arg(parg,int);
					itoa (buf, c , d );
					p = buf;
					while ( (d = *p++) != 0 )
					{
						putc ( d );
					}
					break;
				case 's':
					s = va_arg(parg,char *);
					while ( *s != 0 )
					{
						putc ( *s );
						s++;
					}
					break;
				default:
					break;
			}
		}
	}
	va_end(parg);
	return;
}

void itoa (char *buf, int base, int d)
{
	char a[20];
	char *p;
	unsigned long ud; /* unsigned long original */
	int divisor = 10;
	int resto;
	int i;

	ud = d;
	p = buf;
	i = 0;

	if (base == 'd' && d < 0) /* cambio el signo */
	{
		ud = -d;
	}
	else if (base == 'x')
	{
		divisor = 16;
	}

	do
	{
		resto = ud % divisor;
		ud = ud / divisor;
		if (resto < 10)
		{
			 resto = resto + '0';
		}
		else
		{
			 resto = resto + 'a' - 10;
		}
		a[i] = resto;
		i++;
	}while ( ud != 0);
	if (base == 'd' && d < 0) /* cambio el signo */
	{
		a[i] = '-';
		i++;
	}
	while (i != 0)
	{
		i--;
		*p = a[i];
		p++;
	}
	*p = 0;
}

void cls ( void )
{
	int i;
	rgb24 *p24;
	rgb32 *p32;
	char *video;

	if(vbe_header.color == 0){
		video =  vbe_header.addr;
		for ( i=0; i< vbe_header.xy * 2;i++ )
		{
			*video = 0;
			video++;
		}
	}
	if(vbe_header.color == 24){
		p24 = (rgb24 *) vbe_header.addr;
		for ( i=0; i< vbe_header.xy ;i++ )
		{
			p24->red = 0;
			p24->green = 0;
			p24->blue = 0;
			p24++;
		}
	}
	if(vbe_header.color == 32){
		p32 = (rgb32 *) vbe_header.addr;
		for ( i=0; i< vbe_header.xy ;i++ )
		{
			p32->red = 0;
			p32->green = 0;
			p32->blue = 0;
			p32++;
		}
	}
	xpos = 0;
	ypos = 0;
}

void tputc ( int c )
{
	if ( c == '\n' )
	{
		newline:
			xpos = 0;
			ypos++;
			if ( ypos >= 24 )
				ypos = 0;
		return;
	}
	if ( c == '\r' )
	{
			xpos = 0;
		return;
	}
	*( video + (xpos + ypos * 80) * 2 ) = c & 0xff;
	*( video + (xpos + ypos * 80) * 2 +1 ) = 0x7;
	xpos++;
	if (xpos >= 80)
		goto newline;

}

void set_8259(void)
{
	outb(0x20,0x11); // ICW1 maestro
	outb(0xa0,0x11);

	outb(0x21,0x10); // ICW2 maestro
	outb(0xa1,0x18); // ICW2 esclavo

	outb(0x21,0x04); // ICW3 maestro
	outb(0xa1,0x02); // ICW3 esclavo

	outb(0x21,0x01); // ICW4 maestro
	outb(0xa1,0x01); // ICW4 esclavo

	outb(0x21,0xff); // inhabilito maestro
	outb(0xa1,0xff); // inhabilito esclavo

	outb(0x21,0xfd); // 11111100
	outb(0x64,0xae); // habilito chip de teclado
return;
}

unsigned char inb(unsigned short port)
{
unsigned char val; 
	__asm__ __volatile__ (" inb %w1,%b0 " :"=a"(val): "Nd" (port));
return(val);
}

void outb(unsigned short port, unsigned char val)
{
	__asm__ __volatile__ (" outb %b0,%w1 " ::"a"(val),"Nd" (port));
return;
}

unsigned long strcmp(char* buf, char* str)
{
	unsigned long ret,lb,ls,i;

	lb = strlen( buf );
	ls = strlen( str );
	if ( lb != 0 && ls != 0 )
	{
		if ( lb == ls )
		{
			i = 0;
			while(buf[i] != 0 && (buf[i] - str[i]) == 0)
			{
				i++;
			}
			if ( i == lb ) ret = 0; 
		}
		else
		{
			ret = 1;
		}
	}
	else
	{
		ret = 1;
	}
	return ret;
}


/* strlen() funcion que devuelve el largo de una cadena o cero en caso \
		de error o cadena nula */

unsigned long strlen(char *str)
{
	unsigned long cont=0;
	while( str[cont] != 0 && cont < 0xffffffff )
	{
		cont++;
	}
	if(cont == 0xffffffff) cont = 0;
	return (cont);
}

unsigned long strcpy( char* origen, char* destino){
	unsigned long largo;
	unsigned long i;

	largo=strlen(origen);
	if(largo == 0) return(0);
	if(largo > MAX_STRING) return(0);
	for( i=0; i < largo; i++){
		destino[i] = origen[i];
	}
	return(largo);
}

unsigned long strcat( char* base, char* agr){
	unsigned long a,b;
	a=strlen(base);
	b=strlen(agr);
	if( (a+b) > MAX_STRING) return(1);
	strcpy( agr, &base[a]);
	return(0);
}


void putc ( int c )
{
	unsigned long x, y, z, a, b;

	if(vbe_header.color != 0){
		x = vbe_header.x / font.x;
		y = vbe_header.y / font.y;
	}else{
		x = vbe_header.x;
		y = vbe_header.y;
	}
	if ( c == '\n' )
	{
		newline:
			xpos = 0;
			ypos++;
			if ( ypos >= y ){
				ypos--;

			    if(vbe_header.color != 0){
				a= (unsigned long) vbe_header.addr;
				b= font.y;
				c= vbe_header.x * vbe_header.color / 8;
				z= a + (c * b);
				b= vbe_header.y - font.y;
				memcpy((void *)z , vbe_header.addr ,( b * c));

				z= a + (b * c);
				b= font.y * vbe_header.x;
				c= vbe_header.color / 8;
				a= b * c;
				memset( (char *)z , a,	0);
			    }
			    else{
				a= (unsigned long) vbe_header.addr;
				b= vbe_header.x * 2;
				z= a + b;
				c= vbe_header.y - 1;
				memcpy((void *)z , vbe_header.addr ,( b * c * 2));
			    }
		}
		return;
	}
	if ( c == '\r' )
	{
			xpos = 0;
		return;
	}
	if(vbe_header.color == 0){
	*( vbe_header.addr + (xpos + ypos * x) * 2 ) = c & 0xff;
	*( vbe_header.addr + (xpos + ypos * x) * 2 +1 ) = 0x7;
	}else{
		bmp2graf( c, font.addr , (xpos * font.x), (ypos * font.y),\
				0xf0,0xf0,0x80 );
	}
	xpos++;
	if (xpos >= x)
		goto newline;

}

void bmp2graf(int c, char *tab, unsigned long x,unsigned long \
		y,unsigned long r, unsigned long g, unsigned long b){

	unsigned long y_aux, x_aux, cor;
	char *let;
	rgb24 *pixel24;
	rgb32 *pixel32;

	let = &tab[c * font.y];
	cor = (y * vbe_header.x) + x;

	if(vbe_header.color == 24){
		for(y_aux=0; y_aux < font.y; y_aux++){
			pixel24=(rgb24 *)vbe_header.addr + \
				(y_aux * vbe_header.x) + cor;
			for(x_aux=0;x_aux < font.x;x_aux++){
				if( ((let[y_aux]>> x_aux) & 0x1) == 0x1){
					pixel24->blue=b;
					pixel24->green=g;
					pixel24->red=r;
				}
				pixel24++;;
			}
		}
	}else if(vbe_header.color == 32){
		for(y_aux=0; y_aux < font.y; y_aux++){
			pixel32=(rgb32 *)vbe_header.addr + \
				(y_aux * vbe_header.x) + cor;
			for(x_aux=0;x_aux < font.x;x_aux++){
				if( ((let[y_aux]>> x_aux) & 0x1) == 0x1){
					pixel32->blue=b;
					pixel32->green=g;
					pixel32->red=r;
				}
				pixel32++;;
			}
		}
	}
}


/*
    Ejemplo de como cear asembler in line en Gcc

__asm__  __volatile__ ("0:			;	\n\t \
			movw	%w2,%%ax	;	\n\t \
			movw	%%ax,(t)		;	\n\t \
				.byte	0xea		;	\n\t \
				.long	0x0		;	\n\t \
			t:	.word	0x0		;	\n\t \
			movb	$0x20,%%al	;	\n\t \
			outb	%%al,$0x20	;	\n\t \
			movb	$0x89,%%al	;	\n\t \
			movb	%%al,(%0)	;	\n\t \
			ljmp	$0x30,$0	;	\n\t \
			movb	$0x20,%%al	;	\n\t \
			outb	%%al,$0x20	;	\n\t \
			movb	$0x89,%%al	;	\n\t \
			movb	%%al,(%1)	;	\n\t \
			jmp	0b		;	\n\t  "
			::"r" (p),"r" (d),"g" (tr):"ax", "memory" );
*/

/*
		for(y_aux=0; y_aux < 16; y_aux++){
			pixel32=(rgb32 *)vbe_header.addr+ (y_aux * vbe_header.x);
			for(x_aux=0;x_aux < 8;x_aux++){
				if( ((let[y_aux]>> x_aux) & 0x1) == 0x1){	
					pixel32->blue=0x0f;
					pixel32->green=0xff;
					pixel32->red=0x00;
				}
				pixel32++;;
			}
		}
*/

void memcpy(char *orig, char *des, unsigned long cant){
	while(cant > 0){
		*des = *orig;
		cant--;
		orig++;
		des++;
	}
}

void memset(char *des,unsigned long cant, char dat){
	while(cant > 0){
		*des = dat;
		des++;
		cant--;
	}
}
