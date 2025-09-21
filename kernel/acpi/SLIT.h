/* Supertos Industries
	Generic OS System Latency Information Table support
*/
#include "SDT.h"
#include <stdint.h>

#define LATENCY_DEFAULT_VALUE 30

typedef struct NUMALatency NUMALatency;
struct NUMALatency {
	uint32_t ProximityDomain;
	uint8_t Latency;
	void* Arena;
};


/* Fills NUMA's Latency table according to SLIT or with default values. Returns set element count. */
size_t SLITToLatencyTable( ACPI_XSDP* XSDP, NUMALatency out[], void** arenas, size_t arenaID );