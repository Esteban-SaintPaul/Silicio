#ifndef __STDIO__

#define __STDIO__

void cls ( void );
void putc ( int c );
void printk ( char *format , ... );
void itoa ( char *buf, int format, int dat );
void memcpy( char *orig, char *des, unsigned long cant);
void memset( char *des, unsigned long cant, char dat);

#endif
