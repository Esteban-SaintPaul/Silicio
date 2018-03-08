Silicio :
	make -C fd
#	make -C keyb
	make -C stdio
	make -C file
	make -C asm
	make -C int
	make -C mm
	make -C exec
	make -C main
	make -C kernel

clean :
	rm */*.o
	rm kernel/kernel.bin

cdrom:
	make -C iso-Silicio cdrom

grub-iso:
	make -C iso-Silicio grub-iso

grub-usb:
	make -C iso-Silicio grub-usb

cero :
	rm GNUmakefile
	rm GNUmakefile.cfg
	rm */*.o
	rm kernel/kernel.bin
	rm iso-Silicio/Silicio.iso
qemu :
	make -C iso-Silicio qemu
bochs :
	make -C iso-Silicio bochs
vbox :
	make -C iso-Silicio vbox
