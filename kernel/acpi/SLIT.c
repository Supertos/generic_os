/* Supertos Industries
	Generic OS System Latency Information Table support
*/
#include "SDT.h"
#include "SLIT.h"
#include "utils/Architecture.h"
#include <stdint.h>
#include <stddef.h>

#define ACPI_SLIT_UNREACHABLE 255

/* Fills NUMA's Latency table according to SLIT or with default values. Returns set element count. */
size_t SLITToLatencyTable( ACPI_XSDP* XSDP, NUMALatency* out, void** arenas, size_t arenaID ) {
	ACPI_SLIT* SLIT = FindACPITable( XSDP->XsdtAddress, "SLIT" ); // Returns valid SDT or NULL
	size_t pos = 0;
	for( size_t i = 0; i < MAX_NUMAS; ++i ) {
		if( !arenas[i] || SLIT && SLIT->Entries[ i * SLIT->Localities + arenaID ] == ACPI_SLIT_UNREACHABLE ) continue;
		
		uint8_t latency = arenaID == i ? 10 : LATENCY_DEFAULT_VALUE;
		if( SLIT ) latency = SLIT->Entries[ i * SLIT->Localities + arenaID ];
		
		out[pos++] = (NUMALatency){ 
			.ProximityDomain = i, 
			.Latency = latency, 
			.Arena = arenas[i] 
		};
	}
	
	return pos;
}