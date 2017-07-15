#ifndef __IO__
#define __IO__

extern unsigned char inb(unsigned short port);
extern void outb(unsigned short port, unsigned char val);

#define cli() __asm__ __volatile__ (" cli; \n\t" ::)

#define sti() __asm__ __volatile__ (" sti; \n\t" ::)


#define iretd() __asm__ __volatile__ (" iretl; \n\t" ::)
#define leave() __asm__ __volatile__ (" leave; \n\t" ::)

void set_8259(void);

/* reset_8259() genera un reset del controlador de interrupciones */
#define reset_8259() __asm__ __volatile__ (" movb $0x20,%%al; \n\t \
					     outb %%al,$0x20; \n\t" \
						:::"eax")

#define iret() reset_8259(); leave(); iretd();


#define movla(x) __asm__ __volatile__ (" movl %0,%%eax; \n\t" \
					::"g" (x))

#define moval(x) __asm__ __volatile__ (" movl %%eax,%0; \n\t" \
					::"g" (x))

#define movlb(x) __asm__ __volatile__ (" movl %0,%%ebx; \n\t" \
					::"g" (x))

#define movbl(x) __asm__ __volatile__ (" movl %%ebx,%0; \n\t" \
					::"g" (x))

#define movlc(x) __asm__ __volatile__ (" movl %0,%%ecx; \n\t" \
					::"g" (x))

#define movcl(x) __asm__ __volatile__ (" movl %%ecx,%0; \n\t" \
					::"g" (x))

#define movld(x) __asm__ __volatile__ (" movl %0,%%edx; \n\t" \
					::"g" (x))

#define movdl(x) __asm__ __volatile__ (" movl %%edx,%0; \n\t" \
					::"g" (x))

#endif
