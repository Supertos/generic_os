/* ===============================================================================
	*drv/pci.h
	Contains basic functionality for work with PCI devices
=============================================================================== */
#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct pciDevice pciDevice;
struct pciDevice {
	uint8_t bus;
	uint8_t id;
	uint8_t function;
	
	uint8_t pciClass; // To limit I/O port usage
	uint8_t pciSubClass;
};
uint16_t pci_readWord( uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset );

uint16_t pci_getVendor(uint8_t bus, uint8_t slot, uint8_t function);
uint16_t pci_getDevice(uint8_t bus, uint8_t slot, uint8_t function);
uint16_t pci_getStatus(uint8_t bus, uint8_t slot, uint8_t function);
uint16_t pci_getCommand(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pci_getClass(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pci_getSubClass(uint8_t bus, uint8_t slot, uint8_t function);
uint8_t pci_getHeaderType(uint8_t bus, uint8_t slot, uint8_t function);

void pci_processDevice( uint8_t bus, uint8_t dev );

void pci_processBus( uint8_t bus );

void pci_init();

uint8_t pci_getDeviceCount();
pciDevice* pci_getDeviceList();