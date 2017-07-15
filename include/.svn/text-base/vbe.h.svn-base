#include <multiboot.h>

#ifndef __VBE__
#define	__VBE__

	typedef struct {
		unsigned char blue;
		unsigned char green;
		unsigned char red;
		unsigned char opt;
	}rgb32;

	typedef struct {
		unsigned char blue;
		unsigned char green;
		unsigned char red;
	}rgb24;

	typedef struct {
		unsigned long x;
		unsigned long y;
		unsigned long xy;
		unsigned long color;
		unsigned long modo;
		char *addr;
	}s_vbe_header;

unsigned long open_mod_vbe(multiboot *multi, s_vbe_header *vbe_head);

#endif
