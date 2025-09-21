/* Supertos Industries
	Generic OS System Resource Affinity Table support
*/
#include "SDT.h"
#include "SRAT.h"
#include <stdint.h>
#include <stddef.h>

/* Iterator function to be used in loops. */
void* NextSRATEntry( ACPI_SRAT* SRAT, union ACPI_Affinity** entry ) {
	if( !(*entry) ) {
		*entry = &SRAT->Entries;
		return *entry;
	}
	uintptr_t end = (uintptr_t)SRAT + SRAT->Header.Length;
	union ACPI_Affinity* e = (union ACPI_Affinity*)(*entry);
	
	*entry = (uintptr_t)(*entry) + e->Generic.Length;
	if( *entry >= end ) *entry = NULL;
	
	return *entry;
}

/* Returns Proximity Domain from LAPIC Affinity entry in SRAT */
uint32_t LAPICProximityDomain( ACPI_LAPICAffinity* LAPIC ) {
	return LAPIC->ProximityDomainLow |
		   (LAPIC->ProximityDomainHigh[0] << 8 )|
		   (LAPIC->ProximityDomainHigh[1] << 16)|
		   (LAPIC->ProximityDomainHigh[2] << 24);
}