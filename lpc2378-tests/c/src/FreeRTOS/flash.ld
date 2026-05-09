/* Memory Definitions */

MEMORY
{
	CODE (rx) : ORIGIN = 0x0000000, LENGTH = 500K
	DATA (rw) : ORIGIN = 0x40000000, LENGTH = 32K
}


/* Section Definitions */

SECTIONS
{

	/* first section is .text which is used for code */

	.text :
	{
		*startup.o (.text)         /* Startup code */
		*(.text)                   /* remaining code */
	} >CODE =0

	. = ALIGN(4);

	/* .rodata section which is used for read-only data (constants) */

	.rodata :
	{
		*(.rodata)
	} >CODE

	. = ALIGN(4);

	_etext = . ;
	PROVIDE (etext = .);

	/* .data section which is used for initialized data */

	.data : AT (_etext)
	{
		_data = . ;
		*(.data)
	} >DATA

	. = ALIGN(4);

	_edata = . ;
	PROVIDE (edata = .);

	/* .bss section which is used for uninitialized data */

	.bss :
	{
		__bss_start = . ;
		__bss_start__ = . ;
		*(.bss)
	} >DATA
	
	. = ALIGN(4);
	
	__bss_end__ = . ;
	__bss_end__ = . ;

	_end = .;
	PROVIDE (end = .);

}
