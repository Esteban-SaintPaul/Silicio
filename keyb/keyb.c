#include <stdio.h>
	/* Para usar
		printk()
	*/
#include <io.h>
	/* Para usar
		inb()
		outb()
		iret()
	*/
void keyb();

void keyb()
{
	unsigned char c;
	c = inb (0x60);
	printk("tecla = 0x%x \n",c);
	iret();

	return;
}
