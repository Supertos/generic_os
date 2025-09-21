/* Supertos Industries
	Generic OS Simple Math
*/
#include "SDT.h"
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool CheckTableChecksum( ACPI_SDTHeader* table ) {
	uint8_t checksum = 0;
	for( size_t i = 0; i < table->Length; ++i )
		checksum += ((uint8_t*)table)[i];
	
	return !checksum;
}

size_t memcmp( void* s1, void* s2, size_t n ) { // There is an error in libc where const is discarded
	size_t result = 0;
	
	uint8_t* a = (uint8_t*)s1;
	uint8_t* b = (uint8_t*)s2;
	
	for( size_t i = 0; i < n; ++i ) {
		result += a[i];
		result -= b[i];
	}
	
	return result;
}

void* FindACPITable( ACPI_XSDT* XSDP, char signature[ACPI_SIGNATURE_SIZE_NULL_TERMINATED] ) {
	size_t tableCount = (XSDP->Header.Length - sizeof(ACPI_XSDT)) / sizeof(ACPI_SDTHeader*);
	for( size_t i = 0; i < tableCount; ++i ) {
		if( !memcmp( signature, XSDP->Entries[i]->Signature, ACPI_SIGNATURE_SIZE ) &&
			CheckTableChecksum( XSDP->Entries[i] ) ) {
			return XSDP->Entries[i];
		}
	}
	
	return NULL;
}