// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// *Supertos Industries
// Author: Supertos, 2024
// Generic OS Kernel, PCI driver

#include "pci.h"

uint16_t PCIInWord( uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset ) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
 
    /* =========================================================== \\
	// 31 		| 30-24 	| 23-16  | 15-11  | 10-8    | 7-0	 |
	// Enable   | Reserved	| Bus ID | Dev ID | Func ID | Offset | */
    address = (uint32_t)((lbus << 16) | (lslot << 11) |
              (lfunc << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
 
	PortOutDword(CONFIG_ADDRESS, address);
	
    return (uint16_t)((PortInDword(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}

uint16_t PCIVendor(uint8_t bus, uint8_t slot, uint8_t function) {
	return PCIInWord(bus, slot, function, 0);
}

uint16_t PCIDevice(uint8_t bus, uint8_t slot, uint8_t function) {
	return PCIInWord(bus, slot, function, 2);
}

uint16_t PCIStatus(uint8_t bus, uint8_t slot, uint8_t function) {
	return PCIInWord(bus, slot, function, 6);
}

uint16_t PCICommand(uint8_t bus, uint8_t slot, uint8_t function) {
	return PCIInWord(bus, slot, function, 4);
}

uint8_t PCIClass(uint8_t bus, uint8_t slot, uint8_t function) {
	return PCIInWord(bus, slot, function, 10) >> 8;
}

uint8_t PCISubclass(uint8_t bus, uint8_t slot, uint8_t function) {
	return PCIInWord(bus, slot, function, 10) & 0xFF;
}

uint8_t PCIHeaderType(uint8_t bus, uint8_t slot, uint8_t function) {
	return PCIInWord(bus, slot, function, 14) & 0xFF;
}


pciDevice pciList[512]; // Who would ever connect more than 512 devices?

uint8_t pciDevicesCount;

uint8_t pci_getDeviceCount(void) {
	return pciDevicesCount;
}

pciDevice* pci_getDeviceList(void) {
	return pciList;
}

// Stub function //
// void pci_processFunction(uint8_t bus, uint8_t dev, uint8_t function) {}

// Checks if device exists and has multiple functions //
void pci_processDevice( uint8_t bus, uint8_t dev ) {
	if( pci_getHeaderType( bus, dev, 0 ) & 0x80 ) { // Multifunction device
		for( uint8_t function = 0; function < 8; function++ ) {
			if( pci_getVendor( bus, dev, function ) != 0xFFFF ) {
				pciList[pciDevicesCount++] = (pciDevice){ .bus = bus, .id = dev, .function = function, .pciClass = pci_getClass(bus, dev, function), .pciSubClass = pci_getSubClass(bus, dev, function) };
				// pci_processFunction(bus, dev, function);
			}
		}
	}else{
		pciList[pciDevicesCount++] = (pciDevice){ .bus = bus, .id = dev, .function = 0, .pciClass = pci_getClass(bus, dev, 0), .pciSubClass = pci_getSubClass(bus, dev, 0) };
		// pci_processFunction(bus, dev, 0);
	}
}

// Checks every device on the bus //
void pci_processBus( uint8_t bus ) {
	for( uint8_t dev = 0; dev < 255; dev++ ) {
		if( pci_getVendor( bus, dev, 0 ) != 0xFFFF ) {
			pci_processDevice( bus, dev );
		}
	}
}

// Generates list of all PCI devices //
void pci_init() {
	pciDevicesCount = 0;
	for( uint8_t bus = 0; bus < 255; bus++ )
		pci_processBus(bus);
}