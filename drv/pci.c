/* ===============================================================================
	*drv/pci.c
	Contains basic functionality for work with PCI devices
=============================================================================== */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include "pci.h"

#include "../utils/stdlib/stdlib.h"
#include "../utils/string/string.h"

#define CONFIG_ADDRESS 0xCF8 // Here you drop device you want to read
#define CONFIG_DATA 0xCFC // Here you read data from the device

#include "io.h"
// Reads word from PCI configuration space
// Odd offset will be floored down with power of math
// i.e. ( 1 & 2 ) * 8 = 0 - Offset inside 32-bit register, 1 & 0xFC = 0 - CONFIG_ADDRESS offset
uint16_t pci_readWord( uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset ) {
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

uint16_t pci_getVendor(uint8_t bus, uint8_t slot, uint8_t function) {
	return pci_readWord(bus, slot, function, 0);
}
uint16_t pci_getDevice(uint8_t bus, uint8_t slot, uint8_t function) {
	return pci_readWord(bus, slot, function, 2);
}
uint16_t pci_getStatus(uint8_t bus, uint8_t slot, uint8_t function) {
	return pci_readWord(bus, slot, function, 6);
}
uint16_t pci_getCommand(uint8_t bus, uint8_t slot, uint8_t function) {
	return pci_readWord(bus, slot, function, 4);
}
uint8_t pci_getClass(uint8_t bus, uint8_t slot, uint8_t function) {
	return pci_readWord(bus, slot, function, 10) >> 8;
}
uint8_t pci_getSubClass(uint8_t bus, uint8_t slot, uint8_t function) {
	return pci_readWord(bus, slot, function, 10) & 0b0000000011111111;
}
uint8_t pci_getHeaderType(uint8_t bus, uint8_t slot, uint8_t function) {
	return pci_readWord(bus, slot, function, 14) & 0b0000000011111111;
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