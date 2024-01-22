/* ========================================================= 
	*dev_control.c
	Device interaction module
 ========================================================= */
#define CONFIG_ADDRESS 0xCF8 // Here you drop device you want to read
#define CONFIG_DATA 0xCFC // Here you read data from the device

// PENTIUM INL instruction
uint32_t inl( uint16_t id ) {
	uint32_t temp = 0;
	__asm__ __volatile__("movw %1, %%dx;in %%dx, %%eax;movl %%eax, %0;"
		:"=rm"(temp)
		:"rm"(id)
		: "eax", "dx"
    );
	return temp;
}

// PENTIUM OUTL instruction
void outl( uint16_t id, uint32_t value ) {
	__asm__ __volatile__("movw %0, %%dx;movl %1, %%eax;outl %%eax, %%dx;"
		:
		:"rm"(id), "rm"(value)
		:"eax", "dx"
	);
}

// Reads word from PCI configuration space
// Odd offset will be floored down with power of math
// i.e. ( 1 & 2 ) * 8 = 0 - Offset inside 32-bit register, 1 & 0xFC = 0 - CONFIG_ADDRESS offset
uint16_t dev_pciReadWord( uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset ) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint32_t tmp = 0;
 
    /* =========================================================== \\
	// 31 		| 30-24 	| 23-16  | 15-11  | 10-8    | 7-0	 |
	// Enable   | Reserved	| Bus ID | Dev ID | Func ID | Offset | */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
    // Write out the address
	outl(CONFIG_ADDRESS, address);
	 
    // Read in the data, max offset - 2, odd offsets ignored, offset > 4 case above
	tmp = (uint16_t)((inl(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
    return tmp;
}
uint16_t dev_pciGetVendor(uint8_t bus, uint8_t slot) {
	return dev_pciReadWord(bus, slot, 0, 0);
}
uint16_t dev_pciGetDevice(uint8_t bus, uint8_t slot) {
	return dev_pciReadWord(bus, slot, 0x0, 2);
}
uint16_t dev_pciGetStatus(uint8_t bus, uint8_t slot) {
	return dev_pciReadWord(bus, slot, 0x0, 4);
}
uint16_t dev_pciGetCommand(uint8_t bus, uint8_t slot) {
	return dev_pciReadWord(bus, slot, 0x0, 6);
}
uint8_t dev_pciGetClass(uint8_t bus, uint8_t slot) {
	return dev_pciReadWord(bus, slot, 0x0, 10) >> 8;
}
uint8_t dev_pciGetSubClass(uint8_t bus, uint8_t slot) {
	return dev_pciReadWord(bus, slot, 0x0, 10) & 0b0000000011111111;
}

