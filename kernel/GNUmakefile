include ../GNUmakefile.cfg

FILE := kernel

OBJ :=	../asm/boot.o ../main/main.o ../stdio/stdio.o ../fd/fd.o
OBJ += ../int/int.o ../mm/mm.o ../exec/exec.o ../file/file.o
#../keyb/keyb.o
$(FILE) : $(FILE).bin

$(FILE).bin : $(OBJ) $(OBJ2)
	$(LD) -T link.ld -o $(FILE).bin $(OBJ)

	
