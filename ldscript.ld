ENTRY(kern_init)		/*改了这个不知道会有什么效果,,,嗯。改了是对的。因为入口变成了kern_init.*/

/*https://wenku.baidu.com/view/1a8e11afaef8941ea76e05da.html*/

SECTIONS
{
	. = 0x100000;
	.init.text :
	{
		*(.init.text)
	}
	.init.data :
	{
		*(.init.data)
	}
	
	. += 0xC0000000;							/*这个.其实是虚拟地址VMA。LMA如果没有指定的话，默认等值于VMA。*/
	PROVIDE( kern_start = .);
	
	.text : AT(ADDR(.text) - 0xC0000000)		/*指定一个减去0xC0000000就可以。因为后边所有的段全是顺序排下来的。 而且，AT指的是LMA.也就是指定链接地址。*/
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