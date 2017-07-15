#ifndef __MM__

#define __MM__

#define PAG_OK 0
#define PAG_ERROR 1

#define BYTE		1l
#define KILOBYTES	1024l
#define MEGABYTES	( 1024l * KILOBYTES )
#define PAG_LEN		( 4l * KILOBYTES )

#define PAG_MEM ( 1024 * 1024 )			// Cantidad de páginas de 4k para mapear 4G
#define SEC_MEM 8			// bits por char
#define MAP_LEN ( PAG_MEM / SEC_MEM ) 	// tamaño de mapa = total de páginas / bits por char

unsigned long ini_mm();// inicia el mapa de memoria
unsigned long set_mem(unsigned long pagina); // marca como usada una página
unsigned long unset_mem(unsigned long pagina); // desmarca página usada
unsigned long buscar_mem();// Retorna nro de página libre o 0 si no encuentra
unsigned long pag2addr(unsigned long pag);// retorna la direccion base de una pag
unsigned long addr2pag(unsigned long addr);// retorna la pág que contiene esa direccion
void ini_pag();
unsigned long set_mem_off(unsigned long pagina);
unsigned long *pag_dir();
unsigned long *pag_tab();
#endif

/*
#ifndef __PAG__
#define __PAG__

extern unsigned long *g_dir;
extern unsigned long *g_tab;

#endif
*/
