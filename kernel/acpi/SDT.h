/* Supertos Industries
	Generic OS Memory Manager Header File
*/
#include <stdint.h>

#pragma once

#define ACPI_SIGNATURE_SIZE 4
#define ACPI_SIGNATURE_SIZE_NULL_TERMINATED 5
#define ACPI_SRAT_MEMORY_AFFINITY 1
#define ACPI_SRAT_LAPIC_AFFINITY 0

typedef struct ACPI_SDTHeader ACPI_SDTHeader;
struct ACPI_SDTHeader {
	char Signature[ACPI_SIGNATURE_SIZE];
	uint32_t Length;
	uint8_t Revision;
	uint8_t Checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t CreatorID;
	uint32_t CreatorRevision;
};


typedef struct ACPI_LAPICAffinity ACPI_LAPICAffinity;
struct ACPI_LAPICAffinity {
	uint8_t Type;
	uint8_t Length;
	uint8_t ProximityDomainLow;
	uint8_t LAPICID;
	uint32_t Flags;
	uint8_t LSAPICEID;
	uint8_t ProximityDomainHigh[3];
	uint32_t ClockDomain;
} __attribute__((__packed__));


typedef struct ACPI_MemoryAffinity ACPI_MemoryAffinity;
struct ACPI_MemoryAffinity {
	uint8_t Type;
    uint8_t Length;
    uint32_t ProximityDomain;
    uint64_t Begin;
    uint64_t DomainLength;
    uint32_t Reserved;
    uint32_t Flags;
    uint64_t Reserved2;
} __attribute__((__packed__));


typedef struct ACPI_GenericAffinity ACPI_GenericAffinity;
struct ACPI_GenericAffinity {
	uint8_t Type;
    uint8_t Length;
} __attribute__((__packed__));

union ACPI_Affinity {
	ACPI_GenericAffinity Generic;
	ACPI_LAPICAffinity LAPIC;
	ACPI_MemoryAffinity Memory;
};

typedef struct ACPI_SRAT ACPI_SRAT;
struct ACPI_SRAT {
	ACPI_SDTHeader Header;
	uint8_t reserved[12];
	uint8_t Entries[];
} __attribute__((__packed__));

typedef struct ACPI_SLIT ACPI_SLIT;
struct ACPI_SLIT {
	ACPI_SDTHeader Header;
	uint64_t Localities;
	uint8_t Entries[];
} __attribute__((__packed__));


typedef struct ACPI_XSDP ACPI_XSDP;
struct ACPI_XSDP {
	char Signature[8];
	uint8_t Checksum;
	char OEMID[6];
	uint8_t Revision;
	uint32_t RsdtAddress;

	uint32_t Length;
	ACPI_XSDT* XsdtAddress;
	uint8_t ExtendedChecksum;
	uint8_t reserved[3];
} __attribute__((__packed__));


typedef struct ACPI_XSDT ACPI_XSDT;
struct ACPI_XSDT {
	ACPI_SDTHeader Header;
	ACPI_SDTHeader* Entries[];
} __attribute__((__packed__));


/* Searches and validated specified ACPI table */
void* FindACPITable( ACPI_XSDT* XSDP, char signature[ACPI_SIGNATURE_SIZE_NULL_TERMINATED] );