ENTRY (boot)

SECTIONS{
	. = 0x00100000;
	.text :{
		*(.boot)
		*(.text)
	}

	.data ALIGN (0x1000) : {
;		*(.paginacion)
		*(.data)
		*(.rodata)
	}

	.bss : {
		_sbss = .;
		*(COMMON)
		*(.bss)
	}
	.fin : {
		*(.fin)
	}
}

