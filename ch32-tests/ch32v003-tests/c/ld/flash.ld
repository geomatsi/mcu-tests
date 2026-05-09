MEMORY
{
	FLASH (rx) : ORIGIN = 0x00000000, LENGTH =16K
	RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 2K
}

ENTRY(_start)

SECTIONS
{
	.text : {
		*(.init);
		*(.text);
		*(.text.*);
	} >FLASH AT>FLASH

	.rom.text : {
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
		_data_lma = .;
	} >FLASH AT>FLASH

	.ram.data : {
		_data_vma = .;
		*(.data);
		*(.data.*);
		*(.sdata);
		*(.sdata.*);
		_edata = .;
	} >RAM AT>FLASH

	.bss (NOLOAD) : {
		_sbss = .;
		*(.bss*);
		*(.sbss*);
		*(COMMON);
		_ebss = .;
	} >RAM AT>RAM

	.stack (NOLOAD) : {
		_sstack = .;
	} >RAM AT>RAM

	PROVIDE(_estack = ORIGIN(RAM) + LENGTH(RAM));
}
