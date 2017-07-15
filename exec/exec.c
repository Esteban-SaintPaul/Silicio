#include <int.h>
#include <asm.h>
#include <io.h>
#include <stdio.h>
#include <gdt.h>
#include <exec.h>
#include <string.h>
#include <mm.h>
#include <syscall.h>
#include <vbe.h>

extern s_vbe_header vbe_header;

/* ljmp(x) alta a tarea en referencia de la gdt, posición x  */
#define ljmp(x) __asm__ __volatile__ ("	movw	%w0,%%ax;	\n\t \
					movw	%%ax,(t);	\n\t \
					.byte	0xea	;	\n\t \
					.long	0x0	;	\n\t \
				t:	.word	0x0	;	\n\t "\
					::"g" (x):"eax", "memory")

#define ljmpd(x) __asm__ __volatile__ ("	movw	%w0,%%ax;	\n\t \
					movw	%%ax,(d);	\n\t \
					.byte	0xea	;	\n\t \
					.long	0x0	;	\n\t \
				d:	.word	0x0	;	\n\t "\
					::"g" (x):"eax", "memory")

#define ljmp_call(x) __asm__ __volatile__ ("	movw	%w0,%%ax;	\n\t \
					movw	%%ax,(s);	\n\t \
					.byte	0xea	;	\n\t \
					.long	0x0	;	\n\t \
				s:	.word	0x0	;	\n\t "\
					::"g" (x):"eax", "memory")

#define ljmp_control(x) __asm__ __volatile__ ("	movw	%w0,%%ax; \n\t \
					movw	%%ax,(c);	\n\t \
					.byte	0xea	;	\n\t \
					.long	0x0	;	\n\t \
				c:	.word	0x0	;	\n\t "\
					::"g" (x):"eax", "memory")

typedef struct {
	char name[1024];
	unsigned short task;
	tss *task_tss;
	unsigned long type;
	unsigned long status;
}run;

typedef struct{
	char arch[1024];	//Nombre de archivo virtual
	unsigned long id_drv;	//id de proceso
	char *buf;		//buffer del dispositivo
	char *buf_ret;		//buffer de aplicacion
	unsigned long size;	//tamano del buffer
	unsigned long id_ret;	//proceso a retornar llamada
	unsigned long call;	// llamada hecha por el proceso
	sys_param* sys;		//estuctura de valores para drv
}drv;

run r_tareas[MAX_TASK];		// registro de estado de tareas
drv r_drv[MAX_DRIVERS];		// Registro de drivers
tss tss_p[MAX_TASK];		// tabla de tss
char int_pila[MAX_TASK][200];	// Pila para interrupciones
struct {
	unsigned long tarea;
	unsigned long func;
	unsigned long cr3;
} r_int[256];	// registro de interrupciones usadas

unsigned char r_port[TSS_PORT];
unsigned long r_pipe[256];	// registro para transferencia pipe

tss* set_tss(	void *tarea,		\
		unsigned short code,	\
		unsigned short data,	\
		unsigned long esp,	\
		unsigned long eflag,	\
		unsigned long esp0,	\
		unsigned short data0	);

unsigned long set_desc_tss(tss *tssp);
void set_desc_ldt(unsigned long i);
void clear_tss(tss* t);
unsigned long tarea_libre();
unsigned long driver_libre();
extern unsigned long driver_off;
unsigned long t_indice;	//Variable que guarda el indice a la tarea 
			// que se encuentra corriendo
unsigned long check_addr(unsigned long addr,run *run_t);

void sys_wait();

int sys_mem(unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  );

int sys_read(unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  );

int sys_open(	unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  );

int sys_write(unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  );

int sys_reg(	unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  );

int sys_ret(	unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  );

int sys_seek(	unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  );

int set_port(unsigned short p);
int unset_port(unsigned short p);


/*****************************************************************/
/* Funcion que trata todas las llamadas a sistema*/

void sys_call(){
	unsigned long eax, ebx, ecx, edx;
	int ret;
	do{
		eax = r_tareas[t_indice].task_tss->eax;
		ebx = r_tareas[t_indice].task_tss->ebx;
		ecx = r_tareas[t_indice].task_tss->ecx;
		edx = r_tareas[t_indice].task_tss->edx;
		switch (eax){
			case SYS_OPEN :
				ret = sys_open(eax,ebx,ecx,edx);
				r_tareas[t_indice].task_tss->eax = ret;
				//printk("Open = %d\n",ret);
				break;
			case SYS_READ :
				ret = sys_read(eax,ebx,ecx,edx);
				//r_tareas[t_indice].task_tss->eax = ret;
				//printk("Read = %d\n",ret);
				break;
			case SYS_WRITE :
//				printk("Write = eax %d, ebx %d, ecx %d, edx %d\n",eax,ebx,ecx,edx);
				ret = sys_write(eax,ebx,ecx,edx);
				break;
			case SYS_REG :
				ret = sys_reg(eax,ebx,ecx,edx);
				r_tareas[t_indice].task_tss->eax = ret;
				//printk("Reg = %d\n",ret);
				break;
			case SYS_RET :
				ret = sys_ret(eax,ebx,ecx,edx);
				//printk("Ret = %d\n",ret);
				break;
			case SYS_WAIT :
				sys_wait();
				//printk("Wait\n");
				break;
			case SYS_MEM :
				ret = sys_mem(eax,ebx,ecx,edx);
				//printk("sys_mem %d\n",ret);
				break;
			case SYS_SEEK :
				ret = sys_seek(eax,ebx,ecx,edx);
//				printk("sys_seek %d\n",ret);
				break;
			default:
				printk("Syscall: Proceso %s, Pid %d, no existe \
llamada %d\n", r_tareas[t_indice].name,t_indice,eax);
		}
		/* Salto al administrador de tareas */
		ljmp_call(0x20);	
	}while(1);
	return;
}

int sys_ret(	unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  ){
	int i;
	unsigned long id,aux;
	sys_param *sys;
	unsigned long count;

	for(i=0; i < MAX_DRIVERS ; i++){
		if(r_drv[i].id_drv == t_indice) break; // busco el registro del driver
	}
	if(i == MAX_DRIVERS) return (-1);
	
	// copio el identificador del proceso de retorno
	id = r_drv[i].id_ret;

	aux=check_addr(	ebx, &r_tareas[t_indice]);
	if(aux == 0) return(-2);
	// copio el puntero
	sys = (sys_param*)aux;

	// le copio el valor en el retorno ( registro eax )
	r_tareas[id].task_tss->eax = sys->ret;

	if(r_drv[i].call == SYS_READ){
		for(count=0; count < r_drv[i].sys->count; count++){
			r_drv[i].buf_ret[count]=r_drv[i].buf[count];
		}
	}

	// despierto el proceso dormido
	r_tareas[id].status = RUNNING;	
	
	// Duermo el driver
	r_tareas[t_indice].status = WAITING;

	return(0);	
}

int sys_mem(unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  ){

	tss *t;
	unsigned long *p;

	// si ebx es 0 es que quiere un pagina al azar
	if(ebx == 0 ){
		ebx=buscar_mem();
	}
	// si no encontramos espacio en memoria retornamos con error
	if(ebx == 0)return(-1);

	// limitamos a los primeros 4Mb
	if( (ebx >> 22) != 0)return(-2);

	// vemos si esta libre la pagina
	if( 0 == set_mem(ebx) ){
		t= r_tareas[t_indice].task_tss;
		p=(unsigned long*) (t->cr3 & 0xfffff000l);
		p=(unsigned long*) (p[0] & 0xfffff000l);
		p[ebx] |= 7;
		return(0);
	}
	// si no estaba libre la pagin retornamos error
	return(-3);
}

void sys_wait(){
	r_tareas[t_indice].status = WAITING;
}


int sys_reg(	unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  ){

	unsigned long aux;
	int i;
	sys_param *sys;
	char *arch;
	char *buf;

	for(i=3; i < MAX_DRIVERS;i++){
		if(r_drv[i].id_drv == t_indice) break;
	}
	if( i < MAX_DRIVERS) return(-1);

	// El sistema registrara el 0, 1 y 2 como dispositivos estandard 
	// indicado desde linea de comandos o algo asi, veremos ??? 
	for(i=3; i < MAX_DRIVERS;i++){
		if(r_drv[i].id_drv == 0) break;
	}
	if( i == MAX_DRIVERS) return(-2);

	//Chequeamos que direccione dentro de su memoria
	// puntero a estuctura
	aux = check_addr( ebx, &r_tareas[t_indice]);
	if( aux == 0 ) return(-3);
	sys = (sys_param*)aux;

	// puntero a cadena
	aux = (unsigned long)sys->arch;
	aux = check_addr( aux, &r_tareas[t_indice]);
	if( aux == 0 ) return(-4);
	arch = (char*)aux;

	// buffer
	aux = (unsigned long) sys->buf;
	aux = check_addr( aux, &r_tareas[t_indice]);
	if( aux == 0 ) return(-5);
	buf = (char*)aux;

	// buffer + size
	if( sys->size > 4096)return(-6);
	aux =  (unsigned long) sys->buf + sys->size - 1;
	aux = check_addr( aux, &r_tareas[t_indice]);
	if( aux == 0 ) return(-7);
	strcpy("/dev/",r_drv[i].arch);
	strcat( r_drv[i].arch, arch);
	r_drv[i].id_drv = t_indice;
	r_drv[i].buf = buf;	
	r_drv[i].size = sys->size;	
	r_drv[i].id_ret = 0;	
	r_drv[i].sys = sys;	
	r_tareas[t_indice].status = WAITING;
	return(i);
}
//------------------------------------------------------------------
int sys_read(unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  ){
	unsigned long aux, id_drv;
	char *p_data;
	int ret, max;

	// Vamos a retornar siempre negativos si hay error
	// 0 = ningun dato leido
	max = 0;
	ret = max;

	// Es un numero rezonable?
	if( ebx >= MAX_DRIVERS) return(-1);

	// Es un dispositivo registrado?
	id_drv = r_drv[ebx].id_drv;
	if( id_drv == 0) return(-2);

	// Esta esperando
	if( r_tareas[id_drv].status != WAITING ) return(-3);

	//checkeo las direcciones enviadas por la tarea:
	// estara direccionando dentro de su epacio?

	aux=check_addr( ecx, &r_tareas[t_indice]);
	if(aux == 0) return(-4);
	p_data = (char*)aux;
	
	if(edx > 4096) return(-5);
	if(edx == 0) return(0);
	aux=check_addr(	ecx+edx-1, &r_tareas[t_indice]);
	if(aux == 0) return(-6);
	// Si, esta direccionando bien, ¿que pidio?
	
	// ponemos a esperar al proceso llamante
	r_tareas[t_indice].status = WAITING;
	
	// despertamos la tarea del dispositivo
	r_tareas[id_drv].status = RUNNING;


	// datos a transferir
	if(r_drv[ebx].size > edx){ max = edx; }
	else{ max = r_drv[ebx].size; }

	// registamos el buffer de retorno
	r_drv[ebx].buf_ret = p_data;

	// registamos llamada
	r_drv[ebx].call = SYS_READ;

//	for(count = 0; count < max; count++){
//		p_buf[count] = p_data[count];
//	}

	// avisamos la cantidad transferida
	r_drv[ebx].sys->count = max;

	// avisamos cual fue la llamada
	r_drv[ebx].sys->call = eax;

	// guardamos a quien debemos contestar
	r_drv[ebx].id_ret = t_indice;
	
	return(ret);
}
//------------------------------------------------------------------

int sys_seek(unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  ){
	unsigned long  id_drv;
	int ret, max;

	// Vamos a retornar siempre negativos si hay error
	// 0 = ningun dato leido
	max = 0;

	// Es un numero rezonable?
	if( ebx >= MAX_DRIVERS) return(-1);

	// Es un dispositivo registrado?
	id_drv = r_drv[ebx].id_drv;
	if( id_drv == 0) return(-2);

	// Esta esperando
	if( r_tareas[id_drv].status != WAITING ) return(-3);

	// ponemos a esperar al proceso llamante
	r_tareas[t_indice].status = WAITING;
	
	// despertamos la tarea del dispositivo
	r_tareas[id_drv].status = RUNNING;

	// registamos llamada
	r_drv[ebx].call = SYS_SEEK;

	// avisamos cual fue la llamada
	r_drv[ebx].sys->call = eax;

	// guardamos a quien debemos contestar
	r_drv[ebx].id_ret = t_indice;
	
	ret = max;
	return(ret);
}


int sys_write(unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  ){
	unsigned long aux, id_drv;
	char *p_data,*p_buf;
	int ret, count, max;

	// Vamos a retornar siempre negativos si hay error
	// 0 = ningun dato escrito
	max = 0;
	ret = max;

	// Es un numero rezonable?
	if( ebx >= MAX_DRIVERS) return(-1);

	// Es un dispositivo registrado?
	id_drv = r_drv[ebx].id_drv;
	if( id_drv == 0) return(-2);

	// Esta esperando
	if( r_tareas[id_drv].status != WAITING ) return(-3);

	//checkeo las direcciones enviadas por la tarea:
	// estara direccionando dentro de su epacio?

	aux=check_addr( ecx, &r_tareas[t_indice]);
	if(aux == 0) return(-4);
	p_data = (char*)aux;
	
	if(edx > 4096) return(-5);
	if(edx == 0) return(0);
	aux=check_addr(	ecx+edx-1, &r_tareas[t_indice]);
	if(aux == 0) return(-6);
	// Si, esta direccionando bien, ¿que pidio?
	
	// ponemos a esperar al proceso llamante
	r_tareas[t_indice].status = WAITING;
	
	// despertamos la tarea del dispositivo
	r_tareas[id_drv].status = RUNNING;

	// datos a transferir
	if(r_drv[ebx].size > edx){ max = edx; }
	else{ max = r_drv[ebx].size; }

	// llenamos el buffer del dispositivo
	p_buf = (char*) r_drv[ebx].buf;
	for(count = 0; count < max; count++){
		p_buf[count] = p_data[count];
	}

	// avisamos la cantidad transferida
	r_drv[ebx].sys->count = max;

	// avisamos cual fue la llamada
	r_drv[ebx].sys->call = eax;

	// avisamos cual fue la llamada
	r_drv[ebx].call = SYS_WRITE;

	// guardamos a quien debemos contestar
	r_drv[ebx].id_ret = t_indice;
	
	return(ret);
}

int sys_open(	unsigned long eax, \
			unsigned long ebx, \
			unsigned long ecx, \
			unsigned long edx  ){
	unsigned long aux, var1, var2;
	int i;

	//checkeo las direcciones enviadas por la tarea:
	// esara direccionando dentro de su epacio?

	var1=check_addr( ebx, &r_tareas[t_indice]);
	if(var1 == 0) return(-1);
	aux=strlen((char*)var1);
	if(aux == 0) return(-2);
	aux=check_addr(	ebx+aux, &r_tareas[t_indice]);
	if(aux == 0) return(-3);
	var2=check_addr( ecx, &r_tareas[t_indice]);
	if(var2 == 0) return(-4);
	aux=strlen((char*)var2);
	if(aux == 0) return(-5);
	aux=check_addr( ecx+aux, &r_tareas[t_indice]);
	if(aux == 0) return(-6);
	// Si, esta direccionando bien, ¿que pidio?
	for(i=3;i < MAX_DRIVERS;i++){
		if(0 == strcmp(r_drv[i].arch,(char*)var1) )break;
	}
	// Como no tenemos driver de sistema de archivos retornamos aca
	// sin no encontramos el dispositivo buscado.
	if(i == MAX_DRIVERS ) return(-7);	
	//sys_send(); esto es para llamar al fs cuando este.
	return(i);
}

unsigned long check_addr(unsigned long addr,run *run_t){
	unsigned long acceso=0;
	unsigned long aux;
	unsigned long *cr3,*tabla;
	unsigned long id;

	aux = run_t->task_tss->cr3;
	aux &= 0xfffff000;
	cr3 = (unsigned long*) aux;
	id = addr >> 22;
	aux = cr3[id];
	aux &= 0xfffff000;
	tabla = (unsigned long*) aux;
	id = addr >> 12;
	id &= 0x000003ff;
	acceso = tabla[id];

	if( (acceso & 0x7) != 0x7 ) return(0);
	acceso &= 0xfffff000;
	acceso += (addr & 0xfff); 
	return(acceso);
}

/********************************************************************/
/* Función que maneja la ejecución de las tareas                    */

void task_control(void)
{
	unsigned short des;
	unsigned short tr;
	unsigned char acceso;
	unsigned long seguir;
	
	// inicio banderas de video desocupado
	unset_mem(0xb8);
	unset_mem(0xb9);

	outb(0x21,0xdc); // Mascara de interrupciones 11011100b

	t_indice=0;
	while(1){
		seguir=0;
		do{
			t_indice++;
			// Ejecutamos la tarea 1 si llegamos a la ultima
			if(t_indice == MAX_TASK){
				t_indice = 1;
			}
			else{
			// si llegamos aca es que no llegmos al final
			// de la tabla, si el registro no es una tarea
			// en memoria, volvemos a la 1

				if(r_tareas[t_indice].task==0){t_indice=1;}
			}
			// por ultimo, si es una tarea en memoria y 
			// esta en estado de ejecucion salimos de la
			// busqueda. La encontramos! 
			if(r_tareas[t_indice].status == RUNNING){
				seguir = 1;
			}
			if(r_tareas[t_indice].status == STARTING){
				seguir = 1;
			}

		}while( seguir == 0 );

		des = r_tareas[t_indice].task;
		tr=gdt_p[des/8].base_l;		/* desc (gdt) de tss */
		acceso=gdt_p[tr/8].acceso;	/* guardo tss.acceso */

		reset_8259();			/* Reset del 8259 */
		ljmp(des);			/* salto a tarea */
		gdt_p[tr/8].acceso=acceso;	/* retorno tss.acceso */
	}

	return;
}


/*********************************************************************/
/* clear_tss() Función que borra un tss pasado como puntero          */

void clear_tss(tss* t)
{
	unsigned long i;

	t->b_link=0; /* Coloco ceros entodos el registro */
	t->n=0;
	t->esp0=0;
	t->ss0=0;
	t->n0=0;
	t->esp1=0;
	t->ss1=0;
	t->n1=0;
	t->esp2=0;
	t->ss2=0;
	t->n2=0;
	t->cr3=0;
	t->eip=0;
	t->eflag=0;
	t->eax=0;
	t->ecx=0;
	t->edx=0;
	t->ebx=0;
	t->esp=0;
	t->ebp=0;
	t->esi=0;
	t->edi=0;
	t->es=0;
	t->n3=0;
	t->cs=0;
	t->n4=0;
	t->ss=0;
	t->n5=0;
	t->ds=0;
	t->n6=0;
	t->fs=0;
	t->n7=0;
	t->gs=0;
	t->n8=0;
	t->ldt=0;
	t->n9=0;
	t->n10=0;
	t->bit_map=0;
	for (i=0; i<MAX_PAG_DRIVER ;i++)
	{
		t->pag_mem[i]=0;
	}
	for (i=0; i<MAX_PAG_VIRTUAL ;i++)
	{
		t->pag_virtual[i]=0;
	}
	t->pag_stack=0;
	for (i=0; i < TSS_PORT;i++)
	{
		t->iobp[i]=0xFF;
//		t->iobp[i]=0;
	}
	t->fin=0xff;
	return;
}

unsigned long execs( void* start, void* pila ,char* name )
{
	tss* t;
	unsigned long i,id;
	t=set_tss(	(void*) start,			\
			KERNEL_CS,			\
			KERNEL_DS,			\
			(unsigned long) pila,		\
			0,				\
			(unsigned long) pila,		\
			KERNEL_DS);

	t->cr3 = (unsigned long) pag_dir();
	i=set_desc_tss(t);
	id=tarea_libre();
	r_tareas[id].task = i*8;
	r_tareas[id].task_tss = t;
	strcpy( name, r_tareas[id].name);
	r_tareas[id].status = SLEEPING;

	return(r_tareas[id].task);
}


/**********************************************************************/
/* exec() Función que inicia una tarea                                */

unsigned long exec(unsigned long start, unsigned long end, char* name)
{
	tss* t;
	unsigned long id,pag,cont;
	unsigned long pag_dir_user, pag_tabla_user, pag_tabla_stack;
	unsigned long *p_dir, *k_dir, *k_tab;

	id=tarea_libre(); //busco registro libre 
	pag = (end-start)/ PAG_LEN ; //calculo las paginas usadas
	if (((end-start) % PAG_LEN) != 0){pag++;}
	if(id != 0 && pag <= MAX_PAG_DRIVER){ //si esta todo bien
		t=set_tss( (void*) (V_USER * 4096l),\
			USER_CS,\
			USER_DS,\
			(V_STACK * 4096L) + 0xfff ,\
			0x200,\
			(unsigned long) &int_pila[id][199],\
			KERNEL_DS); // cargo el tss de la tarea

		//marco las paginas como ocupadas
		for(cont=0;cont < pag;cont++){ 
			set_mem((start / PAG_LEN)+cont);
			//cargo en el tss las paginas usadas
			t->pag_mem[cont]=(start / PAG_LEN)+cont;
		}

		//registro las paginas para setear la memoria paginada (virtual)
		pag_dir_user = buscar_mem();
		set_mem(pag_dir_user);
		pag_tabla_user = buscar_mem();
		set_mem(pag_tabla_user);
		pag_tabla_stack = buscar_mem();
		set_mem(pag_tabla_stack);


		//registro el tss en la gdt y cargo el registro de drivers
		r_tareas[id].task=8 * set_desc_tss(t);
		r_tareas[id].task_tss= t;
		strcpy( name, r_tareas[id].name);
		r_tareas[id].status = RUNNING;

		t->pag_virtual[0] = pag_dir_user;
		t->pag_virtual[1] = pag_tabla_user;
		t->pag_stack = pag_tabla_stack;

 // Cargo la tabla de paginas con una copia de la del kernel
		p_dir = (unsigned long *) (4096 * pag_tabla_user);
		k_tab = pag_tab();
		for(cont=0; cont < TAB_PAG_LEN ;cont++){
			p_dir[cont] = k_tab[cont];
		}

 // modifico la tabla para que virtualice las direcciones
		for(cont = 0; cont < pag  ; cont++){
			p_dir[cont+V_USER] = (t->pag_mem[cont] * 4096) + 7 ;
		}
		p_dir[V_STACK] = (t->pag_stack * 4096) + 7 ;

 // Cargo la tabla directorio del paginas con una copia de la del kernel
		p_dir = (unsigned long *) (4096 * pag_dir_user);
		k_dir = pag_dir();
		for(cont=0; cont < DIR_PAG_LEN ;cont++){
			p_dir[cont] = k_dir[cont];
		}

 // Solo será necesario redireccionar la primer tabla
		p_dir[0] = pag_tabla_user * 4096 + 7;
 // cargo el regisro cr3 del tss apuntando al directorio de tablas
		t->cr3=(unsigned long) p_dir;
	}
	return((unsigned long) id);
}

/**********************************************************************/
/* execd() Función que inicia tareas en modo kernel                  */

unsigned long execd(unsigned long start, unsigned long end, char* name)
{
	tss* t;
	unsigned long id,pag,cont;
	unsigned long pag_dir_user, pag_tabla_user, pag_tabla_stack;
	unsigned long *p_dir, *k_dir, *k_tab;

	id=tarea_libre(); //busco registro libre 
	pag = (end-start)/ PAG_LEN ; //calculo las paginas usadas
	if (((end-start) % PAG_LEN) != 0){pag++;}
	if(id != 0 && pag <= MAX_PAG_DRIVER){ //si esta todo bien
		t=set_tss( (void*) (V_USER * 4096l),	\
			KERNEL_CS,			\
			KERNEL_DS,			\
			(V_STACK * 4096L) + 0xfff ,	\
			0x200,				\
			(unsigned long) &int_pila[id][199],\
			KERNEL_DS);

		//marco las paginas como ocupadas
		for(cont=0;cont < pag;cont++){ 
			set_mem((start / PAG_LEN)+cont);
			//cargo en el tss las paginas usadas
			t->pag_mem[cont]=(start / PAG_LEN)+cont;
		}

		//registro las paginas para setear la memoria paginada (virtual)
		pag_dir_user = buscar_mem();
		set_mem(pag_dir_user);
		pag_tabla_user = buscar_mem();
		set_mem(pag_tabla_user);
		pag_tabla_stack = buscar_mem();
		set_mem(pag_tabla_stack);


		//registro el tss en la gdt y cargo el registro de drivers
		r_tareas[id].task=8 * set_desc_tss(t);
		r_tareas[id].task_tss= t;
		strcpy( name, r_tareas[id].name);
		r_tareas[id].status = RUNNING;

		t->pag_virtual[0] = pag_dir_user;
		t->pag_virtual[1] = pag_tabla_user;
		t->pag_stack = pag_tabla_stack;

 // Cargo la tabla de paginas con una copia de la del kernel
		p_dir = (unsigned long *) (4096 * pag_tabla_user);
		k_tab = pag_tab();
		for(cont=0; cont < TAB_PAG_LEN ;cont++){
			p_dir[cont] = k_tab[cont];
		}

 // modifico la tabla para que virtualice las direcciones
		for(cont = 0; cont < pag  ; cont++){
			p_dir[cont+V_USER] = (t->pag_mem[cont] * 4096) + 7 ;
		}
		p_dir[V_STACK] = (t->pag_stack * 4096) + 7 ;

 // Cargo la tabla directorio del paginas con una copia de la del kernel
		p_dir = (unsigned long *) (4096 * pag_dir_user);
		k_dir = pag_dir();
		for(cont=0; cont < DIR_PAG_LEN ;cont++){
			p_dir[cont] = k_dir[cont];
		}

 // Solo será necesario redireccionar la primer tabla
		p_dir[0] = pag_tabla_user * 4096 + 7;
 // cargo el regisro cr3 del tss apuntando al directorio de tablas
		t->cr3=(unsigned long) p_dir;
	}
	return((unsigned long) id);
}

/************************************************************/
/* set_desc_tss() Agrega un tss a un descriptor de tarea    */
/* Retorna descriptor de tarea de la gdt                    */

unsigned long set_desc_tss(tss *tssp)
{
	gdtp* d;
	unsigned long m;
	unsigned long i;
	unsigned long f;

	d=gdt_p;

	/* Busco una entrada de la gdt libre*/
	i=4; // hasta la 3 estan ocupadas 
	while( d[i].acceso!=0 && i<MAX_GDT )
	{
		i++;
	}

	if(i==MAX_GDT)
	{
		return 0; /*Salimos si no encontramos una gdt libre*/
	}


	/* Direcciono el tss desde el desciptor gdt        */

	m=sizeof(tss) & 0x0000ffff; /*limite bajo tamaño short*/
	d[i].lim_l=(unsigned short) m ;	/*lim_l*/

	m=(unsigned long)tssp;/*base bajo tamaño short*/
	m=m & 0x0000ffff;
	d[i].base_l=(unsigned short)m;	/*base_l*/

	m=(unsigned long)tssp;
	m=m >> 16;
	m=m & 0x000000ff;
	d[i].base_m=(unsigned char)m;	/*base_m*/

	d[i].acceso=0x89; /* acceso (descriptor de tss)*/

	m=sizeof(tss); /*limite alto tamaño medio char + 0xc0*/
	m=m>>16;
	m=m & 0x0000000f;
	m=m+0xc0;
	d[i].lim_h=(unsigned char)m;	/*lim_h*/

	m=(unsigned long)tssp;
	m=m >> 24;
	m=m & 0x000000ff;
	d[i].base_h=(unsigned char)m;	/*base_h*/

	/* Inseraré código para crear la puerta de tarea */
	/* creare la puerta un descriptor mas abajo      */
	
	f=i; /* Asigno la posicion en f      */
	f=f*8; /* paso la posicion a bite    */

	i++; /* Direcciono el siguiente regidtro */

	m=(unsigned long)f;             /*base bajo tamaño short*/
	m=m & 0x0000ffff;
	d[i].base_l=(unsigned short)m;	/*base_l*/

	m=f;
	m=m >> 16;
	m=m & 0x000000ff;
	d[i].base_m=(unsigned char)m;	/*base_m*/

	m=f;
	m=m >> 24;
	m=m & 0x000000ff;
	d[i].base_h=(unsigned char)m;	/*base_h*/

	d[i].acceso=0x85;  /*acceso (descriptor de tarea) */

	return (i);
}


/******************************************************************/
/* set_tss() Busca un tss vacio, lo completa y retorna un puntero a este 
*/

tss* set_tss(void *tarea, unsigned short code, unsigned short data, unsigned long esp, unsigned long eflag, unsigned long esp0, unsigned short data0 )
{

	unsigned long var;
	unsigned long i;
	tss *n;
	tss *t;

	n=tss_p;
	t=0;

	for(i=0; i<MAX_TASK && t==0 ;i++) /*Recorro la tabla tss*/
	{
		if( n[i].cs==0 )  /*Busco con segmento codigo nulo*/
		{
			t=&n[i]; /*guardo la dirección en t*/
		}
	}
	if(i==MAX_TASK) /*Si llegué al final retorno 0 "no encontré"*/
	{
		return 0;
	}
	t->n=0;  /*Cargo el registro con los datos de la tarea*/
	t->n0=0;
	t->n1=0;
	t->n2=0;
	t->n3=0;
	t->n4=0;
	t->n5=0;
	t->n6=0;
	t->n7=0;
	t->n8=0;
	t->n9=0;
	t->n10=0;
	t->eip=(unsigned long) tarea;
	t->cs=code;
	t->ds=data;
	t->es=data;
	t->fs=data;
	t->gs=data;
	t->ss=data;
	t->ss0=data0;
	t->ss1=data0;
	t->ss2=data0;
	t->esp=esp;
	t->esp0=esp0;
	t->esp1=esp0;
	t->esp2=esp0;
	t->ldt=0x18;
	t->eflag=(unsigned long) eflag;
	var = (unsigned long) t;
	var = (unsigned long) t->iobp - var;
	t->bit_map=(unsigned short) var;
	t->fin=(unsigned char) 0xff;
	return t;
}


/*****************************************************************/
/* set_desc_ldt() Agrega un ldt a un descriptor de la gdt        */

void set_desc_ldt(unsigned long i)
{
	gdtp* d;
	unsigned long m;

	d=gdt_p;

	m= (TAM_LDT -1) & 0x0000ffff; /*limite bajo tamaño short*/
	d[i].lim_l=(unsigned short) m ;	/*lim_l*/

	m=(unsigned long)ldt_p;/*base bajo tamaño short*/
	m=m & 0x0000ffff;
	d[i].base_l=(unsigned short)m;	/*base_l*/

	m=(unsigned long)ldt_p;
	m=m >> 16;
	m=m & 0x000000ff;
	d[i].base_m=(unsigned char)m;	/*base_m*/

	d[i].acceso=0x82;	/*acceso 0x82 */

	m= (TAM_LDT -1) ; /*limite alto tamaño medio char + 0xc0*/
	m=m>>16;
	m=m & 0x0000000f;
	m=m+0xc0;
	d[i].lim_h=(unsigned char)m;	/*lim_h*/

	m=(unsigned long)ldt_p;
	m=m >> 24;
	m=m & 0x000000ff;
	d[i].base_h=(unsigned char)m;	/*base_h*/
	return;
}


/********************************************************************/
// Esta funcion es llamada antes que el task_control(), se inicializa la
// tabla de tss, el registro de drivers y el registro de interrupciones

void ini_tss(){
	unsigned long i;

	// inicializa la tabla de tss
	for(i=0; i< MAX_TASK ;i++){
		clear_tss(&tss_p[i]);
		r_tareas[i].task=0;
	}

	// inicializa el registro de drivers
	for(i=0; i<MAX_DRIVERS ;i++){
		r_drv[i].id_drv=0;
	}
	
	// inicializa el registro de interrupciones
	// reservo de 0 a 15, son internas del procesador
	for(i=0; i<16 ;i++){
		r_int[i].tarea = 1;
	}

	// inicializa el registro de interrupciones
	// de 16 a 255 para los drivers y usuarios
	for(i=16; i<256 ;i++){
		r_int[i].tarea = 0;
	}

	// rservar interrupcion de reloj para el Task_control()
	r_int[16].tarea = 1;

	for ( i=0; i < TSS_PORT; i++)
	{
		r_port[i]=0xFF;
	}
		
	return;
}


void ini_ldt(){
	set_desc_ldt(0x18 /8);
	return;	
}

unsigned long tarea_libre(){
	unsigned long cont=0,ret=0;
	while(cont < MAX_TASK){
		if(r_tareas[cont].task==0){
			ret=cont;
			cont=MAX_TASK;
		}
		else {
			cont++;
		}
	}
	return(ret);
}

unsigned long driver_libre(){
	unsigned long cont=1,ret=0;
	while(cont < MAX_TASK){
		if(r_tareas[cont].task==0){
			ret=cont;
			cont=MAX_TASK;
		}
		else {
			cont++;
		}
	}
	return(ret);
}

tss* gdt2tss(gdtp* gdt){
	tss* t;

	gdt--;//el descriptor gdt apunta a un descriptor de tss
		// resulta que siempre es el anterior asi que 
		// nos paramos en el anterior

	t=0;
	return(t);
}


void sys_control(){
	do{
		ljmp_control(0x20);	
	}while(1);
	return;
}

void int_hard_17(){
	unsigned long aux_cr3, orig_cr3;
	void (*fun)(void);

//printk("fun %d\n",r_int[17].func);

	// guardo el registro cr3
	__asm__ __volatile__(" 	movl	%%cr3,%%eax;	\
				movl	%%eax,%0;	"
				 :"=g"(aux_cr3):: "%eax" );


	// cargo el valor para esta llamada
	orig_cr3 = r_int[17].cr3;
	__asm__ __volatile__(" 	movl	%0,%%eax;	\
				movl	%%eax,%%cr3;	"
				 ::"g"(orig_cr3));


	// ejecuto la funcion registrada
	fun = (void *) r_int[17].func;
	fun();

	// restauro el registro cr3
	__asm__ __volatile__(" 	movl	%0,%%eax;	\
				movl	%%eax,%%cr3;	"
				 :"=g"(aux_cr3):: "%eax" );
/*
int c;
c = inb(0x60);
printk("int 17 %d\n",c);
*/

/*	__asm__ __volatile__("	popl	%edx;");
	__asm__ __volatile__("	popl	%ecx;");
	__asm__ __volatile__("	popl	%ebx;");
	__asm__ __volatile__("	popl	%eax;");

*/
	iret();
	return;
}

void int_soft_speed(){
	unsigned long vara,varb, varc,vard;
	int i;

	__asm__ __volatile__ ("	movl	%%eax,	%0;" : "=g" (vara));
	__asm__ __volatile__ ("	movl	%%ebx,	%0;" : "=g" (varb));
	__asm__ __volatile__ ("	movl	%%ecx,	%0;" : "=g" (varc));
	__asm__ __volatile__ ("	movl	%%edx,	%0;" : "=g" (vard));

	switch(vara){
	case SYS_PIPER:
		printk("piper \[%d]: 0x%x\n",t_indice, varb);
		__asm__ __volatile__ ("movl $0,%%eax;"
					::"g"(r_pipe[varb]) );		
		break;
	case SYS_PIPEW:
		r_pipe[varb]=varc;
		printk("pipew r_pipe\[%d]= 0x%x\n",varb, r_pipe[varb]);
		break;
	case SYS_DEBUG:
		printk("debug %s \[%d]: 0x%x %d\n", r_tareas[t_indice].name, t_indice, varb, varb);
		break;
	case SYS_RINT:
		if( r_int[varb].tarea == 0){
			//cargo la estructura r_int
			r_int[varb].tarea= t_indice;
			r_int[varb].func= varc;
			r_int[varb].cr3= r_tareas[t_indice].task_tss->cr3;
			//habilito la interrupcion hard
			int_up(varb);
			i=0;
		} else {
			i=-1;
		}


		break;
	case SYS_RPORT:
		i = set_port( (unsigned short) varb );
		__asm__ __volatile__ ("movl %0, %%eax;"
					::"g"(i));
		break;
	case SYS_URPORT:
		i = unset_port( (unsigned short) varb );
		__asm__ __volatile__ ("movl %0, %%eax;"
					::"g"(i));
		break;
	case SYS_VBE_ADDR:
		i=(unsigned long)vbe_header.addr;
		__asm__ __volatile__ ("movl %0, %%eax;"
					::"g"(i));
		break;
	case SYS_VBE_X:
		i=(unsigned long)vbe_header.x;
		__asm__ __volatile__ ("movl %0, %%eax;"
					::"g"(i));
		break;
	case SYS_VBE_Y:
		i=(unsigned long)vbe_header.y;
		__asm__ __volatile__ ("movl %0, %%eax;"
					::"g"(i));
		break;
	case SYS_VBE_COLOR:
		i=(unsigned long)vbe_header.color;
		__asm__ __volatile__ ("movl %0, %%eax;"
					::"g"(i));
		break;
	default:
				printk("Syscall Speed: Proceso %s, Pid %d, no \
existe llamada %d\n", r_tareas[t_indice].name,t_indice,vara);
	}

	__asm__ __volatile__ ("	movl	%0, %%eax;" :: "g" (i));
	__asm__ __volatile__ ("	movl	%0, %%ebx;" :: "g" (varb));
	__asm__ __volatile__ ("	movl	%0, %%ecx;" :: "g" (varc));
	__asm__ __volatile__ ("	movl	%0, %%edx;" :: "g" (vard));

	leave();
	iretd();
	return;
}

int set_port(unsigned short p){
	unsigned short aux,id;
	unsigned char var, *io_map;
	tss *t;

	aux = p / 8;		// calculo el byte
	id = p % 8;		// calculo el bit
	var = r_port[aux] && ( 1 << id ); 
	if(var == 0) return(-1);	// verifico si ya esta seleccionado
	t= r_tareas[t_indice].task_tss;
	io_map = t->iobp; // obtengo el mapa de bit del proceso
	r_port[aux] &= ~( 1 << id );// lo registro en r_port
	io_map[aux] &= ~( 1 << id );// habilito el puerto para ese proceso
	return(0);
}

int unset_port(unsigned short p){
	unsigned short aux,id;
	unsigned char var, *io_map;
	tss *t;

	aux = p / 8;		// calculo el byte
	id = p % 8;		// calculo el bit
	t= r_tareas[t_indice].task_tss;
	io_map = t->iobp; // obtengo el mapa de bit del proceso
	var = ~(io_map[aux]) && ( 1 << id ); 
	if(var == 0) return(-1);	// verifico si ya esta seleccionado
	printk("rport r_port\[%d]= 0x%x\n", aux, r_port[aux]);
	printk("rport io_map\[%d]= 0x%x\n", aux, io_map[aux]);
	r_port[aux] |= ( 1 << id );// lo registro en r_port
	io_map[aux] |= ( 1 << id );// habilito el puerto para ese proceso
	printk("rport r_port\[%d]= 0x%x\n", aux, r_port[aux]);
	printk("rport io_map\[%d]= 0x%x\n", aux, io_map[aux]);
	return(0);
}

/********************************************************************/
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

