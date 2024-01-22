#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "drv/dev_pci.c"
#include "drv/dev_isa.c"
#include "drv/dev_vga.c"
 
/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif


typedef struct {
	uint32_t base; // Start pos
	uint32_t size : 20; // Size
	uint8_t present : 1; // 1 always
	uint8_t privilege : 2;
	uint8_t type : 1; // 1 code or data of 0
	uint8_t typeFlags : 4; // Is Code | Conforming | Readable | Accesed
	uint8_t otherFlags : 4; // Granularity | 32 bit mem | RESERVED | RESERVED
} segmentDescriptor;
struct GDT {
	// Null descriptor
	uint64_t nullDescriptor;
	segmentDescriptor segments[16];
	
};
	
struct GDT GDTprt;

void kernelMainProtected() {
 
	/* Newline support is left as an exercise. */
	terminal_writestring("The system now in a Protected Mode! Congratulations!\n");
}

/*  enterProtectedMode(void) */
void enterProtectedMode() {
	terminal_writestring("Generating table....");
	struct GDT GDTprt = { 0 };
	terminal_writestring("GDT Created...");
	
	// Code Segment
	GDTprt.segments[0].base = 0x0;
	GDTprt.segments[0].size = 0xFFFFF;
	GDTprt.segments[0].present = 1;
	GDTprt.segments[0].privilege = 0;
	GDTprt.segments[0].type = 1;
	GDTprt.segments[0].typeFlags = 0b1010;
	GDTprt.segments[0].otherFlags = 0b1100;
	
	GDTprt.segments[1].base = 0x0;
	GDTprt.segments[1].size = 0xFFFFF;
	GDTprt.segments[1].present = 1;
	GDTprt.segments[1].privilege = 0;
	GDTprt.segments[1].type = 1;
	GDTprt.segments[1].typeFlags = 0b0010;
	GDTprt.segments[1].otherFlags = 0b1100;
	
	terminal_writestring("OK\n");
	terminal_writestring("Here goes ASM....\n");
	__asm__("cli;");
	
	__asm__ __volatile__("lgdt (%0); mov %%cr0, %%eax; or 1, %%eax; mov %%eax, %%cr0;"
		:
		:"rm"(&GDTprt)
		: "eax", "dx"
    );
	__asm__("sti;");
	terminal_writestring("Finished doing stuff....\n");
	__asm__ ("call kernelMainProtected;");
	terminal_writestring("Call is not working for some reason....\n");
}


void kernel_main() {
	/* Initialize terminal interface */
	terminal_initialize();
	terminal_writestring("Trying to enter Protected Mode....\n");
	enterProtectedMode();
}



