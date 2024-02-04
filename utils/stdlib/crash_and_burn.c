/* ========================================================= 
	Kernel "Crash And Burn" feature
 ========================================================= */
#include "stdlib.h"
#include "../../drv/vga.h"

void crash_and_burn(const char *msg) {
	vga_print("Kernel Crash Und Burn: ");
	vga_print(msg);
	
	__asm__("1: cli; hlt; jmp 1;");
}