/* ===============================================================================
	*kernel/entry.c
	Entry Point of Generic OS
=============================================================================== */
#include <stddef.h>
#include <stdint.h>

#include "../drv/vga.h"
#include "../drv/acpi.h"
#include "../drv/pci.h"

#include "../utils/string/string.h"
#include "../utils/stdlib/stdlib.h"
#include "../utils/stdio/stdio.h"

uint16_t systemFlags;

// This Procedure Is Called right after machine enters protected mode //
void kernelMainProtected(void) {
}

// Entry Point of Generic OS //
void kernelMain(void) {
	// Read ACPI to test hardware abilities
	acpi_init();
	
	systemFlags = ((FADT_t*)acpi_getTable( "FACP" ))->BootArchitectureFlags;
	
	if( !(systemFlags & 0x4) ) {
		vga_enable(); // Precaution
		
		vga_disablecursor();
		vga_clear();
		vga_print("VGA initialized!\n");
		
	}
	
	// List PCI devices
	pci_init();
	pciDevice* pciList = pci_getDeviceList();
	printf("BUS ID FUNC:|DEVICE|VENDOR|STATUS|COMMAND|CLASS|SUBCLASS|HEADER TYPE|\n");
	for( uint16_t devID = 0; devID < pci_getDeviceCount(); devID++ ) {
		
		printf("%i %i %i: \n", pciList[devID].bus, pciList[devID].id, pciList[devID].function);
		printf("    0x%x|", pci_getDevice(pciList[devID].bus, pciList[devID].id, pciList[devID].function) );
		printf("0x%x|", pci_getVendor(pciList[devID].bus, pciList[devID].id, pciList[devID].function) );
		printf("0x%x|", pci_getStatus(pciList[devID].bus, pciList[devID].id, pciList[devID].function) );
		printf("0x%x|", pci_getCommand(pciList[devID].bus, pciList[devID].id, pciList[devID].function) );
		printf("0x%x|", pciList[devID].pciClass );
		printf("0x%x|", pciList[devID].pciSubClass );
		printf("0x%x|", pci_getHeaderType(pciList[devID].bus, pciList[devID].id, pciList[devID].function) );
		printf("\n");
	}
}






