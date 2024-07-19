
INPUT(kernel.o)
INPUT(vga.o)
INPUT(asm.o)
INPUT(idt.o)
INPUT(int.o)
INPUT(string.o)
INPUT(kcub.o)
OUTPUT(../kernel.bin)
ENTRY(KernelEntry)
OUTPUT_FORMAT(binary)


SECTIONS
{
	.text 0x8000 :
	{
		*(.text.main);
		*(.text)
	}

	.rodata :
	{
		*(.rodata)
	}

	.data :
	{
		*(.data)
	}

	.bss :
	{
		*(COMMON)
		*(.bss)
	}
}