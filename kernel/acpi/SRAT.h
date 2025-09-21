/* Supertos Industries
   Generic OS System Resource Affinity Table support
*/
#pragma once
#include "SDT.h"
#include <stdint.h>

/* Iterator function used in for's. */
void* NextSRATEntry( ACPI_SRAT* SRAT, union ACPI_Affinity** entry );


/* Returns Proximity Domain from LAPIC Affinity entry in SRAT */
uint32_t LAPICProximityDomain( ACPI_LAPICAffinity* LAPIC );