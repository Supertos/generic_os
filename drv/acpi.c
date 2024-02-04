/* ===============================================================================
	*drv/acpi.c
	Contains basic functionality for ACPI tables
=============================================================================== */
#pragma once
#include "acpi.h"


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../utils/stdlib/stdlib.h"
#include "../utils/string/string.h"

#define EBDA_ADR 0x40E

XSDP_t* XSDP_ptr;
XSDT_t* XSDT_ptr;

// Returns sum of all bytes in structure. Used in checksum validating //
size_t acpi_bytesum( const uint8_t* start, size_t amount ) {
	size_t val = 0;
	const uint8_t* ptra = start;
	for( size_t a = 0; a < amount; a++ ) {
		val += *(ptra++);
	}
	return val;
}

// Searches for base ACPI tables and crashes system if none present //
void acpi_init(void) {
	// Search for XSDP
	size_t* volatile ebda_ptr = (size_t* volatile)EBDA_ADR; // Extended BIOS Data Area
	size_t ebda = *(ebda_ptr);
	
	XSDP_t* XSDP_ptr = (XSDP_t*)memstr("RSD PTR ", ebda, ebda + 1024);
	
	if( XSDP_ptr == NULL ) {
		XSDP_ptr = (XSDP_t*)memstr("RSD PTR ", 0x000E0000, 0x000FFFFF);
		if( XSDP_ptr == NULL ) {
			crash_and_burn("This hardware does not support ACPI and, as such, Generic OS is unable to operate.\n");
		}
	}
	
	// Validate XSDP
	switch( XSDP_ptr->Revision ) {
		case 0:
			crash_and_burn("Unsupported Revision 1.0. Check hardware for introduction date.");
		break;
		case 2:
		break;
		default:
			crash_and_burn("Invalid ACPI Revision. Check hardware for malfunction.");
		break;
	}
	
	if( ( acpi_bytesum( (uint8_t*)XSDP_ptr, sizeof(XSDP_t) ) & 0x100 ) != 0 ) {
		crash_and_burn("Invalid XSDP checksum. Check hardware for malfunction.");
	}
	
	XSDT_ptr = XSDP_ptr->XSDTAddress;
	if( ( acpi_bytesum( (uint8_t*)XSDT_ptr, XSDT_ptr->Header.Length ) % 0x100 ) != 0 ) {
		crash_and_burn("Invalid XSDT checksum. Check hardware for malfunction.");
	}
	
}

// Returns true if table's checksum is valid //
bool acpi_isvalidtable( SDTHeader_t* SDTHead ) {
	return ( acpi_bytesum( (uint8_t*)SDTHead, SDTHead->Length ) % 0x100 ) == 0;
}

// Searches in memory for ACPI table with provided signature //
void* acpi_getTable( const char* signature ) {
	for( size_t i = 0; i < (XSDT_ptr->Header.Length - sizeof(SDTHeader_t)) / sizeof(uint64_t); i++ ) {
		SDTHeader_t* SDTHead = (SDTHeader_t*)((uint32_t)XSDT_ptr->PointerToOtherSDT[i]);
		if( !strncmp( signature, SDTHead->Signature, 4 ) && acpi_isvalidtable( SDTHead ) ) {
			return (void*)(SDTHead);
		}
	}
	return NULL;
}
