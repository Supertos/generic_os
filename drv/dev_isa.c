/* ========================================================= 
	*dev_control.c
	Device interaction module
 ========================================================= */
#include <stdbool.h>
#include "dev_vga.c"
 
#define ADDRESS 0x279 // Here you drop device you want to read
#define WRITE_DATA 0xA79 // Here you read data from the device
#define READ_DATA 0x203 // First ID

// PENTIUM OUT instruction
void out( uint16_t id, uint8_t value ) {
	__asm__ __volatile__("movw %0, %%dx;mov %1, %%eax;out %%eax, %%dx;"
		:
		:"rm"(id), "rm"((uint32_t)value)
		:"eax", "dx"
	);
}
// PENTIUM IN instruction
uint8_t in( uint16_t id ) {
	uint16_t temp = 0;
	__asm__ __volatile__("movw %1, %%dx;in %%dx, %%ax;movw %%ax, %0;"
		:"=rm"(temp)
		:"rm"(id)
		: "ah", "dx", "memory"
    );
	return temp;
}

void dev_isaIsolate() {
	terminal_write("Begin ISA isolation!\n");
	__asm__("cli;"); // To prevent interference with sending init key
	
	out(ADDRESS, 0); // Reset port (It is required to do twice)
	out(ADDRESS, 0);
	
	out(ADDRESS, 0x6A); // Initialization key
	out(ADDRESS, 0xB5);
	out(ADDRESS, 0xDA);
	out(ADDRESS, 0xED);
	out(ADDRESS, 0xF6);
	out(ADDRESS, 0xFB);
	out(ADDRESS, 0x7D);
	out(ADDRESS, 0xBE);
	out(ADDRESS, 0xDF);
	out(ADDRESS, 0x6F);
	out(ADDRESS, 0x37);
	out(ADDRESS, 0x1B);
	out(ADDRESS, 0x0D);
	out(ADDRESS, 0x86);
	out(ADDRESS, 0xC3);
	out(ADDRESS, 0x61);
	out(ADDRESS, 0xB0);
	out(ADDRESS, 0x58);
	out(ADDRESS, 0x2C);
	out(ADDRESS, 0x16);
	out(ADDRESS, 0x8B);
	out(ADDRESS, 0x45);
	out(ADDRESS, 0xA2);
	out(ADDRESS, 0xD1);
	out(ADDRESS, 0xE8);
	out(ADDRESS, 0x74);
	out(ADDRESS, 0x3A);
	out(ADDRESS, 0x9D);
	out(ADDRESS, 0xCE);
	out(ADDRESS, 0xE7);
	out(ADDRESS, 0x73);
	out(ADDRESS, 0x39); // End of initialization key
	terminal_write("ISA initialized!\n");
	
	char a[32];
	// Begin ISA isolation protocol
	for( uint8_t isa = 0; isa < 32; isa++ ) {
		terminal_write("Trying ISA...");
		out(ADDRESS, isa); // Card Select Number (CSN) select
		out(ADDRESS, 0x0); // "Set READ_PORT" "command" select
		
		bool weirdFlag = false; // True if there's no ports avaiable (or we've encountered last ISA bus)
		
		for( uint16_t port_id = isa; port_id < 4; port_id++ ) {
			itoa( port_id, a );
			terminal_write("Port ");
			terminal_write(a);
			terminal_write("...");
			
			out(WRITE_DATA, port_id); // Set "Set READ_PORT" to port_id
			uint8_t checksum = 0x6A;
			uint8_t read_checksum = 0;
			uint8_t next_shift_value = 0;
			uint8_t readA = 0;
			uint8_t readB = 0;
			
			uint32_t data = 0;
			
			for( uint8_t bit_id = 0; bit_id < 4; bit_id++ ) {
				readA = in( (port_id & 0x3F8) >> 3 );
				readB = in( (port_id & 0x3F8) >> 3 );
				next_shift_value = ( (checksum & 0x2) >> 1 ) ^ ( checksum & 0x1 ) ;
				if( (readA == 0x55 && readB == 0xAA) || (readA == 0xAA && readB == 0x55) ) { // 1
					next_shift_value = next_shift_value ^ 1;
					data = data | (1 << bit_id );
				} else { // 0
					next_shift_value = next_shift_value ^ 0;
					data = data | (0 << bit_id );
				}
				
				checksum = checksum >> 1;
				checksum = checksum | (next_shift_value << 7);
			}
			
			for( uint8_t bit_id = 64; bit_id < 72; bit_id++ ) {
				readA = in( (port_id & 0x3F8) >> 3 );
				readB = in( (port_id & 0x3F8) >> 3 );
				if( (readA == 0x55 && readB == 0xAA) || (readA == 0xAA && readB == 0x55) ) { // 1
					read_checksum = read_checksum | (1 << bit_id );
				} else { // 0
					read_checksum = read_checksum | (0 << bit_id );
				}
			}
			
			if( data != 0 && checksum == read_checksum ) {
				terminal_write("ISA device initialized!\n");
				weirdFlag = false;
				break;
			}else{
				if( data != 0 ){ terminal_write("Weird checksum!\n"); }
				weirdFlag = true;
			}
		}
		
		if( weirdFlag ) { 
			terminal_write("Finished!\n");
			break;
		}
	}		
	
	__asm__("sti;"); // Allow interrupts
}


