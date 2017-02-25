ENTRY(init)

SECTIONS
{
	. = 0x100000;
	
	PROVIDE( kern_start = .);
	
	.text :
	{
		*(.text)
	}
	.data :
	{
		*(.data)
	}
	.rodata :
	{
		*(.rodata)
	}
	.bss :
	{
		*(.bss)
	}
	.stab :
	{
		*(.stab)
	}
	.stabstr :
	{
		*(.stabstr)
	}
	
	PROVIDE( kern_end = .);
	
}