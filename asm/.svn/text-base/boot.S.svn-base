[BITS 32]
;------------------------------------------------------------
; Declaración de funciones internas y externas al archivo
global boot	; Función donde inicia nuestro kernel
global idt_p	; Apuntador a tabla de interrupciones
global gdt_p	; Apuntador a tabla de descriptores globales
global ldt_p	; Apuntador a tabla de descriptores locales
global kernel_fin ; ultimo byte del kenrnel
;global dir_pag	; Directorio de paginas del kernel
;global tab_pag	; Tabla de paginas del kernel
extern t_indice	; exec/exex.c indice de tarea en curso

;-----------------------------------------------------------
; Excepciones


global int_00	; interrupciones excepciones 
global int_01
global int_02
global int_03
global int_04
global int_05
global int_06
global int_07

global int_08
global int_09
global int_0a
global int_0b
global int_0c
global int_0d
global int_0e
global int_0f

;-----------------------------------------------------------
; Procedimientos y funiones externas al archivo

extern _main	; main/main.c

extern cls	; stdio/stdio.c
extern printk

;------------------------------------------------------------
; Definiciones de constantes con "equ" ( #define en asm )

	KERNEL_STACK	equ	0x4096
	TASK_STACK	equ	0x200
	KERNEL_IDT	equ	8*256
	KERNEL_DS	equ	0x10
	KERNEL_CS	equ	0x08
	KERNEL_VIDEO	equ	0xb8000
	KERNEL_KEY	equ	256
	DIR_PAG_LEN	equ	1024
	CANT_TAB_PAG	equ	1024
	TAB_PAG_LEN	equ	1024*CANT_TAB_PAG
;---------------------------------------------------------
; Segmento de datos - Kernel
;---------------------------------------------------------
SECTION .data
ALIGN 4

idt:
	times KERNEL_IDT db	0
idt_size equ $-idt

gdt:
;------------------------------------------------
; Vector nulo en la gdt

	dw	0
	dw	0
	db	0
	db	0
	db	0
	db	0


;------------------------------------------------
; Creo descriptores de segmento de código y datos en ring 0

	dw	0xffff	;vector 0x08 KERNEL_CS
	dw	0
	db	0
	db	0x9a
	db	0xcf
	db	0

	dw	0xffff	;vector 0x10 KERNEL_DS
	dw	0
	db	0
	db	0x92
	db	0xcf
	db	0

;------------------------------------------------------
;	Descriptor de ldt para las tareas

	dw	0	;vector 0x18 TASK_LDT
	dw	0
	db	0
	db	0
	db	0
	db	0

	times	8*8187	db	0
gdt_size equ $-gdt

;-------------------------------------------------------
; Tabla ldt (descriptores locales

ldt:
	dw	0	;vector 0x00+7=0x07 
	dw	0
	db	0
	db	0
	db	0
	db	0

	dw	0	;vector 0x08+7=0x0f
	dw	0
	db	0
	db	0
	db	0
	db	0

	dw	0xffff	;vector 0x10+7=0x17 USER_CS
	dw	0
	db	0
	db	0xfa
	db	0xcf
	db	0

	dw	0xffff	;vector 0x18+7=0x1f USER_DS
	dw	0
	db	0
	db	0xf2
	db	0xcf
	db	0

	dw	0	;vector 0x20+7=0x27
	dw	0
	db	0
	db	0
	db	0
	db	0

	dw	0	;vector 0x28+7=0x2f
	dw	0
	db	0
	db	0
	db	0
	db	0
ldt_size: equ $-ldt

;---------------------------------------------------
; Apuntadores para registrar las tablas idt y gdt

idtr:
	dw	idt_size
	dd	idt

gdtr:
	dw	gdt_size
	dd	gdt


;--------------------------------------------------------
; Apuntadores de tablas del kernel para manipular en C

idt_p:
	dd	idt
gdt_p:
	dd	gdt
ldt_p:
	dd	ldt


;--------------------------------------------------------------------
; Variable donde guardo el código de segmento en una excepción

interr:	dd	0

;--------------------------------------------------------------------
; Textos de excepciones  que saldrÃ¡n por pantalla antes de bloquearce 

tex_00	db	'Error al dividir',0
tex_01	db	'Paso a paso (depurar)',0
tex_02	db	'MNI Error de mamoria',0
tex_03	db	'Error Punto de rotura',0
tex_04	db	'Error de desbordamiento',0
tex_05	db	'Error de limite',0
tex_06	db	'Instruccion invalida',0
tex_07	db	'Error cooprocesador no disponible',0
tex_08	db	'Error de doble falta',0
tex_09	db	'Desbordamiento de cooprocesador',0
tex_0a	db	'Descriptor de tarea invalido',0
tex_0b	db	'Segmento no presente',0
tex_0c	db	'Error de pila',10,13,0
tex_0d	db	'Error de proteccion general',0
tex_0e	db	'Error de paginacion',0
tex_0f	db	'Error de cooprocesador',0

text_halt: db 'Silicio: Sistema detenido',0
fin:	dd	0

;SECTION .paginacion
;dir_pag:	times	DIR_PAG_LEN	dd	0
;tab_pag:	times	TAB_PAG_LEN	dd	0

;---------------------------------------------------------------
; Sección de código - Kernel
;---------------------------------------------------------------

SECTION .text 
ALIGN 4

; Declaración de variables en espacio de código 
; multiboot_header será leida por grub antes de cargarlo en RAM

multiboot_header:
	dd	0x1badb002 ; mágico (0x1badb002)
	dd	0x00000007 ; ordenes(0x00000003 orig)e4524ffb
	dd	0xe4524ff7 ; -(mágico+configuración-ffffffff-1)
	dd	0x0        ; bit[16] dirección de cabecera
	dd	0x0        ;         dirección de .text
	dd	0x0        ;         dirección de .data
	dd	0x0        ;         dirección fin de .bss
	dd	0x0        ;         dirección inicial boot()
	dd	0x0        ; bit[2] modo video
	dd	0x0        ; 320   ancho en pixel o caracter
	dd	0x0        ; 200    alto en pixel o caracter
	dd	0x0        ; 16 bit    resolución o 0 en texto

;------------------------------------------------------------------------------------
; Función boot() primera en ejecutarce

boot:
	cli                    ; Inhabilito las interrupciones
;	call	A20_ini        ; habilito la linea A20 del 8042

	mov	eax,cr0        ; habilito el bit PE de CR0
	or	al,0x1
	mov	cr0,eax

	lidt	[idtr]         ; Seteo la tabla idt
	lgdt	[gdtr]         ; Seteo la tabla gdt
	jmp	KERNEL_CS:start ; cargo el selector de código nuevo

;------------------------------------------------------------------------------------
; Función start() genera el entorno para main()

start:
	mov	ax,KERNEL_DS   ; Cargo los registros para utilizar la nueva gdt
	mov	ds,ax
	mov	es,ax
	mov	ss,ax
	mov	gs,ax
	mov	fs,ax
	mov	esp,stack+KERNEL_STACK-1 ; Cargo final .bss como pila
	
;------------------------------------------------------------
; Llamamos a main() para que haga todo lo demás

	push	ebx             ; Grub me dejó en ebx la dirección de la estructura module_info
	call	_main            ; llamo a main ( multiboot_info* module_info )
	
;-------------------------------------------------------------
; Ejemplo de llamada a función de C con parametros
;
;	mov	eax,idt
;	push	eax
;	mov	eax,gdt
;	push	eax
;	call	_func ; void _func (unsigned int *gdt ,unsigned int *idt )
;-------------------------------------------------------------	
	push	text_halt
	call	printk
stop:
	jmp	stop; Fin del kernel ciclo infinito
	ret

;---------------------------------------------------------------
; Fin del código de boot
;---------------------------------------------------------------

;---------------------------------------------------------------
; Procedimiento que habilita la linea A20
;--------------------------------------------------------------
;A20_ini: 
;	in	al,0x64 
;	and	al,0x2 
;	jnz	A20_ini 
;	mov	al,0xd1 
;	out	0x64,al 
;A2: 
;	in	al,0x64 
;	and	al,0x2 
;	jnz	A2 
;	mov	al,0xdf 
;	out	0x60,al 
;A3: 
;	in	al,0x64 
;	and	al,0x2 
;	jnz	A3 
;;	xor	cx,cx 
;;A20_retardo: 
;;	inc	ax 
;;	loop	A20_retardo 
;	ret

;---------------------------------------------------------
; Excepciones de procesador int 0x01 a 0x0f

int_00:
	push	eax
	mov	eax,tex_00
	jmp	msj_err

int_01:
	push	eax
	mov	eax,tex_01
	jmp	msj_err

int_02:
	push	eax
	mov	eax,tex_02
	jmp	msj_err

int_03:
	push	eax
	mov	eax,tex_03
	jmp	msj_err

int_04:
	push	eax
	mov	eax,tex_04
	jmp	msj_err

int_05:
	push	eax
	mov	eax,tex_05
	jmp	msj_err

int_06:
	push	eax
	mov	eax,tex_06
	jmp	msj_err

int_07:
	push	eax
	mov	eax,tex_07
	jmp	msj_err


int_08:
	pop dword [interr]
	push	eax
	mov	eax,tex_08
	jmp	msj_err

int_09:
	push	eax
	mov	eax,tex_09
	jmp	msj_err

int_0a:
	pop dword [interr]
	push	eax
	mov	eax,tex_0a
	jmp	msj_err

int_0b:
	pop dword [interr]
	push	eax
	mov	eax,tex_0b
	jmp	msj_err

int_0c:
	pop dword [interr]
	push	eax
	mov	eax,tex_0c
	jmp	msj_err

int_0d:
	pop dword [interr]
	push	eax
	mov	eax,tex_0d
	jmp	msj_err

int_0e:	
	push	eax
	push	ebx
	push	ecx
	mov	eax,[esp+16]
	mov	ebx,[esp+20]
	mov	ecx,[esp+24]

	push	ecx
	push	ebx
	push	eax
	push	t_stak
	call	printk
	pop	eax
	pop	eax
	pop	eax
	pop	eax

	pop	ecx
	pop	ebx
	pop	eax

	push	eax
	push	t_eax
	call	printk

	push	ebx
	push	t_ebx
	call	printk

	push	ecx
	push	t_ecx
	call	printk

	push	edx
	push	t_edx
	call	printk

	mov	eax,cr0
	push	eax
	push	t_cr0
	call	printk

	mov	eax,cr3
	push	eax
	push	t_cr3
	call	printk

;	mov	eax,ltr
;	push	eax
;	push	t_tr
;	call	printk

	push	dword [t_indice]
	push	t_t_indice
	call	printk

	push	tex_0e
	call	printk
	cli
ms:	jmp	ms
;	jmp	msj_err

int_0f:
	push	eax
	mov	eax,tex_0f
	jmp	msj_err

t_stak:	db	'eip = 0x%x , cs= 0x%x, flag=0x%x',10,0
t_eax:	db	'eax = 0x%x',10,0
t_ebx:	db	'ebx = 0x%x',10,0
t_ecx:	db	'ecx = 0x%x',10,0
t_edx:	db	'edx = 0x%x',10,0
t_eip:	db	'eip = 0x%x',10,0
t_tr:	db	'tr = 0x%x',10,0
t_cr0:	db	'cr0 = 0x%x',10,0
t_cr3:	db	'cr3 = 0x%x',10,0
t_t_indice:	db	't_indice = 0x%x',10,0
;------------------------------------------------------------
;	msj_err : Imprime escepciones y bloquea el kernel
;------------------------------------------------------------
msj_err:
	push	edi
	push	esi
	mov	esi,eax		; Dirección del texto
	mov	eax,KERNEL_VIDEO
	mov	edi,eax		; Dirección de video
	mov	ah,0x7		; Atributo fondo negro letra blanca
msj_0	lodsb			; al <- ds:edi
	or	al,al		; al != 0
	jz	msjferr		; 0 = fin de llamado
	stosw			; ax -> ds:esi
	jmp	msj_0		; busca el siguiente caracter
msjferr:
	cli			; Inhabilita las interrupciones
se:	jmp	se			; Duerme

;----------------------------------------------------------
; Pila del kernel
;----------------------------------------------------------

SECTION .bss
ALIGN 32
stack:
	resb KERNEL_STACK


SECTION .fin
kernel_fin: resb 1;

