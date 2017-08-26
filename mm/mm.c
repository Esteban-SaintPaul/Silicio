#define __PAG__


#include <stdio.h>
#include <mm.h>
#include <asm.h>

unsigned long *g_dir;
unsigned long *g_tab;

unsigned long testmem(void);
void ini_map(unsigned char map[],unsigned long tope);
unsigned char mem_map[MAP_LEN]; //mapa de bits de memoria
unsigned long mem_max;
unsigned long mem_off; // pagina desde donde buscar desocupadas

unsigned long ini_mm(){
	unsigned long pag_grub,pag_kernel,pag_modulos;
unsigned long pag_video;

	mem_max=testmem();
	ini_map(mem_map,mem_max);

	for(pag_grub=0; (pag_grub * PAG_LEN) < MEGABYTES; pag_grub++ ){
		set_mem(pag_grub); // ocupado el primer mega de memoria
	}

/* Dejo a disposición de algún proceso las páginas de la mem de video */
	pag_video= 0xFC000l / PAG_LEN;
	unset_mem(pag_video);
	pag_video++;
	unset_mem(pag_video);
	pag_video++;
	unset_mem(pag_video);
	pag_video++;
	unset_mem(pag_video);
	pag_video++;
	unset_mem(pag_video);
	pag_video++;
	unset_mem(pag_video);
	pag_video++;
	unset_mem(pag_video);
	pag_video++;
	unset_mem(pag_video);

	for(	pag_kernel = KERNEL_INI / PAG_LEN ;    \
		pag_kernel <= KERNEL_FIN / PAG_LEN ; \
		pag_kernel++ ){
		set_mem(pag_kernel);
	}
	for( pag_modulos = pag_kernel ; pag_modulos < mem_off ; pag_modulos++ ){
		set_mem(pag_modulos);
	}
	return(mem_max);
}

unsigned long testmem(void)
{
	char *i,aux,ciclo;
	unsigned long salto;

	i= (char*) KERNEL_FIN;

	for(ciclo=0;ciclo < 3;ciclo++){
		if(ciclo==0){
			salto=MEGABYTES;
		}
		else {
			if(ciclo==1) {
				salto=KILOBYTES;
			}
			else {
				salto=BYTE;
			}
		}

		aux=*i;
		*i=1;
		while(*i == 1){
			*i=aux;
			i+=salto;
			aux=*i;
			*i=1;
		}
		i-=salto;
	}
	i+=1;
	return ((unsigned long)i / PAG_LEN);
}


void ini_map(unsigned char map[],unsigned long tope){
	unsigned long i,aux;
	aux=tope/8;
	for(i=0; i < MAP_LEN; i++){
		if(i<aux){
			map[i]=0;
		}
		else{
			map[i]=0; // tengo que liberar la memoria de video y queda muy arriba, mas allá de la RAM física
//			map[i]=0xff;
		}
	}
	return;
}

unsigned long set_mem(unsigned long pagina){
	unsigned long sector;
	char bits;

	sector=pagina/8;
	bits=1 << (pagina%8);
	if( 0 == (mem_map[sector] & bits)){
		mem_map[sector]= mem_map[sector] | bits;
		return(PAG_OK);
	}
	return(PAG_ERROR);
}
unsigned long unset_mem(unsigned long pagina){
	unsigned long sector;
	char bits;

	sector=pagina/8;
	bits=1 << (pagina%8);
	if( 0 != (mem_map[sector] & bits)){
		mem_map[sector]= mem_map[sector] & ~bits;
		return(PAG_OK);
	}
	return(PAG_ERROR);
}

unsigned long buscar_mem(){
	unsigned long i,fin;
	unsigned char mask;

	fin = mem_max / 8;
	for(i = (mem_off - 10) / 8 ; \
		i<fin ;	i++){
		if(mem_map[i]!=0xff){break;}
	}
	if(i == fin)return(0);
	for(mask=0; mask < 8 ; mask++){
		if( (mem_map[i] & (1 << mask)) == 0 ){break;}
	}
	return((i*8)+mask);
}

unsigned long pag2addr(unsigned long pag){
	return(pag * PAG_LEN);
}

unsigned long addr2pag(unsigned long addr){
	return(addr / PAG_LEN);
}

unsigned long set_mem_off(unsigned long pag){
	mem_off = pag;
	return(0);
}


void ini_pag(){
	unsigned long tab,pag,p,i;
	unsigned long *aux_d, *aux_t;

	/*Reservo memoria para la tabla de directorio de paginas*/
	p= buscar_mem();
	set_mem(p);
	g_dir= (unsigned long *) (p << 12);
	aux_d= (unsigned long *) (p << 12);

	/*Reservo memoria para la tabla de paginas*/
	p= buscar_mem();
	set_mem(p);
	g_tab= (unsigned long *) (p << 12);
	aux_t= (unsigned long *) (p << 12);

	/*Cargo la tabla directorio de paginas*/
	*aux_d= (unsigned long) aux_t + 1;
	aux_d++;

	/*Las 1023 tablas restantes*/
	for(i=1; i < CAN_TAB_PAG; i++){
		p= buscar_mem();
		set_mem(p);
		p= (unsigned long ) (p << 12);
		*aux_d= (unsigned long) p + 1;
		aux_d++;
	}


	for(tab=0; tab < CAN_TAB_PAG  ; tab++){
		for(pag=0; pag < TAB_PAG_LEN; pag++){
			*aux_t = (unsigned long)(pag * (1 << 12))+ (tab *(1 << 22) + 1 );
			aux_t++; 
		}
	}

	__asm__ __volatile__ ("	pushl %%eax		\n\t	\
				movl %0,%%eax		\n\t 	\
				movl %%eax,%%cr3	\n\t	\
				movl %%cr0,%%eax	\n\t	\
				or  $0x80000000,%%eax	\n\t	\
				movl %%eax,%%cr0	\n\t	\
				popl %%eax		\n\t	\
				"	\
				::"g" (g_dir));

	return;
}

unsigned long *pag_dir(){
	return(g_dir);
}

unsigned long *pag_tab(){
	return(g_tab);
}
