#include <vbe.h>

#ifndef __BMP__
#define __BMP__

typedef struct {
	unsigned char blue;
	unsigned char green;
	unsigned char red;
} s_bmp_pixel;

#define BMP_CHAR0	0
#define BMP_CHAR1	1
#define BMP_SIZE_FILE	2
#define BMP_OFFSET	10
#define BMP_SIZE_X	18
#define BMP_SIZE_Y	22
#define BMP_BIT_PER_PIXEL 28

typedef struct {
	unsigned char char0;
	unsigned char char1;
	unsigned long offset;
	unsigned long size_file;
	unsigned long x;
	unsigned long y;
	unsigned long xy;
	unsigned long init_img;
	unsigned short bit_per_pixel;
} s_bmp_header;

unsigned long open_mod_bmp(multiboot *m_boot, s_bmp_header *bmp_h);
void bmp2vbe(s_bmp_header *b, s_vbe_header *v);

#endif
