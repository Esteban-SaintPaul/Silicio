#include <string.h>
#include <stdio.h>
#include <multiboot.h>
/*
	struct multiboot;
	struct vbe_mode;
*/

#include <bmp.h>
/*
	s_bmp_header;
	s_bmp_pixel;		*/

#include <vbe.h>
/*
	s_vbe_header;		*/

#define BMP_CENTER_H 100 * 3
#define BMP_CENTER_V 30 * 3

/*-----------------------------------------------*/
/* Funciones que abren los modulos vbe y bmp */

//#unsigned long open_mod_bmp(multiboot *m_boot, s_bmp_header *bmp_h);
//#void bmp2vbe(s_bmp_header *b, s_vbe_header *v);

void bmp2vbe(s_bmp_header *b, s_vbe_header *v){

//	rgb24 *pixel24;
	rgb32 *pixel32;
	s_bmp_pixel *bmp_pixel;
	unsigned long x_scr;
	unsigned long y_scr;
	unsigned long x_aux;
	unsigned long y_aux;
	char *pixel;
	char *bmp_color;

	/*Fijo los limites para no escribir mas alla de la pantalla */
	if(v->x < b->x){x_scr=v->x;} else {x_scr=b->x;}
	if(v->y < b->y){y_scr=v->y;} else {y_scr=b->y;}

	if(v->color == 24){
		for(y_aux=0; y_aux < y_scr ; y_aux++){
			pixel=(char *)v->addr + (y_aux * (v->x) * 3)+ BMP_CENTER_H + (v->x * BMP_CENTER_V);
			bmp_color=(char *)b->init_img + (( b->y - 1 - y_aux) * ((b->x*3)+2) ) ;
			for(x_aux=0; x_aux < x_scr ; x_aux++){
				*pixel = *bmp_color;
				pixel++;
				bmp_color++;
				*pixel = *bmp_color;
				pixel++;
				bmp_color++;
				*pixel = *bmp_color;
				pixel++;
				bmp_color++;

			}
//			bmp_color++;
//			bmp_color++;
		}

	} else{
		if(v->color == 32){
			for(y_aux=0; y_aux < y_scr; y_aux++){
				pixel32=(rgb32 *)v->addr + (y_aux * v->x);
				bmp_pixel=(s_bmp_pixel *)b->init_img -(y_aux * b->x) + b->xy; 
				for(x_aux=0; x_aux < x_scr; x_aux++){
					pixel32->blue=bmp_pixel->blue;
					pixel32->green=bmp_pixel->green;
					pixel32->red=bmp_pixel->red;
					pixel32++;
					bmp_pixel--;
				}
			}
		}
	}
	return;
}


unsigned long open_mod_bmp(multiboot *multiboot_data, s_bmp_header *bmp_header){

	modules_info *background=0, *mod_info;
	unsigned long mod_count;

	//----------------------------------------------------------
	// busco el modulo con la imagen bmp para el fondo de pantalla

	if (multiboot_data->mods_count > 0)
	{
		mod_info=(modules_info *) multiboot_data->mods_addr;
		for (	mod_count=0, background=0; \
			mod_count < multiboot_data->mods_count ; \
			mod_count++,mod_info++){

			if(0 == strcmp((char *)mod_info->string,"/boot/background.bmp")){
				background=mod_info;
			}
		}
	}
	// Retorno con 1 si no encontre archivo "/boot/background.bmp"
	if(background == 0) return(1);
	//----------------------------------------------------------
	unsigned long *plong;
	unsigned char *pchar;
	unsigned short *pshort;

	pchar =(unsigned char *) (background->mod_start + BMP_CHAR0);
	bmp_header->char0 = *pchar;

	pchar =(unsigned char *) (background->mod_start + BMP_CHAR1);
	bmp_header->char1 = *pchar;

	plong =(unsigned long *) (background->mod_start + BMP_OFFSET);
	bmp_header->offset = *plong;

	pshort =(unsigned short *) (background->mod_start + BMP_BIT_PER_PIXEL);
	bmp_header->bit_per_pixel = *pshort;

	bmp_header->init_img = background->mod_start + bmp_header->offset - 3;
	plong =(unsigned long *) (background->mod_start + BMP_SIZE_FILE);
	bmp_header->size_file = *plong;

	plong =(unsigned long *) (background->mod_start + BMP_SIZE_X);
	bmp_header->x = *plong;

	plong =(unsigned long *) (background->mod_start + BMP_SIZE_Y);
	bmp_header->y = *plong;

	bmp_header->xy = bmp_header->x * bmp_header->y;

/*Realizar los controles por si no es un bmp y 800x600x24*/
/*
printk("bmp_header.char0 = %c\n",bmp_header->char0);
printk("bmp_header.char1 = %c\n",bmp_header->char1);
printk("bmp_header.offset = %d\n",bmp_header->offset);
printk("bmp_header.init_img = 0x%x\n",bmp_header->init_img);
printk("bmp_header.bit_per_pixel = %d\n",bmp_header->bit_per_pixel);
printk("bmp_header.size_x = %d\n",bmp_header->x);
printk("bmp_header.size_y = %d\n",bmp_header->y);
*/
	return(0);
}
