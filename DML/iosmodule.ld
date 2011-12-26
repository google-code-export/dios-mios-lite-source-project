OUTPUT_FORMAT("elf32-bigarm")
OUTPUT_ARCH(arm)
ENTRY(_start)

__stack_size = 0x0FF0;

MEMORY
{
	sram		: ORIGIN = 0xFFFF0000,	LENGTH = 0xEfff
	stack		: ORIGIN = 0xFFFFF000,	LENGTH = 0x0fff
}

PHDRS
{
	sram PT_LOAD AT ( 0xFFFF0000 ) ;
	stack PT_LOAD AT ( 0xFFFFF000 ) ;
}

SECTIONS
{
	.init :
	{
		*(.init)
		. = ALIGN(4);
	} >sram :sram

	.text :
	{
		*(.text*)
		*(.text.*)
		*(.gnu.warning)
		*(.gnu.linkonce.t*)
		*(.glue_7)
		*(.glue_7t)
		. = ALIGN(4);
	} >sram :sram

	.rodata :
	{
		*(.rodata)
		*all.rodata*(*)
		*(.roda)
		*(.rodata.*)
		*(.gnu.linkonce.r*)
		. = ALIGN(4);
	} >sram :sram

	.data :
	{
		*(.data)
		*(.data.*)
		*(.gnu.linkonce.d*)
		. = ALIGN(4);
	} >sram :sram

	.bss :
	{
		__bss_start = . ;
		*(.dynbss)
		*(.gnu.linkonce.b*)
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = . ;
	} >sram :sram
	

	.stack :
	{
		__stack_end = .;
		. = . +__stack_size;
		. = ALIGN(4);
		__stack_addr = .;
	} >stack :stack
	
	/DISCARD/ :
	{
		*(.ARM.exidx*)
		*(.ARM.extab*)
	}
}
