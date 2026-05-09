MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH =256K
	RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

ENTRY(_start)

SECTIONS
{
	.init : {
		*(.init);
	} >FLASH AT>FLASH

	.vector : {
		. = ALIGN(64);
		*(.vector);
	} >FLASH AT>FLASH

	.text : {
		*(.text);
		*(.text.*);
	} >FLASH AT>FLASH

	.rom.text : {
		. = ALIGN(4);
		_text_lma = .;
	} >FLASH AT>FLASH

	.ram.text : {
		_text_vma = .;
		*(.sram.text);
		_etext = .;
	} >RAM AT>FLASH

	.rodata : {
		*(.rodata);
		*(.rodata.*);
	} >FLASH AT>FLASH

	.rom.data : {
		. = ALIGN(4);
		_data_lma = .;
	} >FLASH AT>FLASH

	.ram.data : {
		. = ALIGN(4);
		_data_vma = .;
		*(.data);
		*(.data.*);
		*(.sdata);
		*(.sdata.*);
		_edata = .;
	} >RAM AT>FLASH

	.bss (NOLOAD) : {
		. = ALIGN(4);
		_sbss = .;
		*(.bss*);
		*(.sbss*);
		*(COMMON);
		_ebss = .;
	} >RAM AT>RAM

	.stack (NOLOAD) : {
		. = ALIGN(4);
		_sstack = .;
	} >RAM AT>RAM

	PROVIDE(_estack = ORIGIN(RAM) + LENGTH(RAM));
}
