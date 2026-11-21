/* Supertos Industries (2012 - 2026)
 * ACPI table definitons.
 */
#pragma once

#include "Base.h"
#include "BootInfo.h"

#define SLIT_SIGN "SLIT"
#define SRAT_SIGN "SRAT"

#define SLIT_SELF 10
#define SLIT_UNREACHABLE 255

#define SRAT_CPU 0x2
#define SRAT_MEM 0x1

#define SRAT_ACTIVE 0x1

typedef struct {
    u8 Type;
    u8 Size;
} may_alias ACPIEntry;

typedef struct {
    u32  Signature;
    u32  Length;
    u8   Revision;
    u8   Checksum;
    char OEMID[6];
    char OEMTableID[8];
    u32  OEMRevision;
    u32  CreatorID;
    u32  CreatorRevision;
} may_alias ACPIHeader;

typedef struct {
    ACPIHeader Header;
    ACPIHeader* Tables[];
} packed XSDT;

typedef struct {
    ACPIHeader Header;
    u64 LocalityCount;
    u8 Latency[];
} packed SLIT;

typedef struct {
    u8  EntryType;
    u8  EntrySize;
    u32 ID;
    u8  __Reserved1[2];
    u64 Base;
    u64 Size;
    u8  __Reserved2[4];
    u32 Flags;
    u8  __Reserved3[8];
} packed may_alias MemoryAffinity;

typedef struct {
    u8 Type;
    u8 Length;
    u8 __Reserved1[2];
    u32 Domain;
    u32 ID;
    u32 Flags;
    u32 _CDM;
    u8 __Reserved2[4];
} packed may_alias CPUAffinity;

typedef struct SRAT {
    ACPIHeader Header;
    u8 __Reserved[12];
} packed SRAT;

typedef struct {
    u64  Signature;
    u8   Checksum;
    char OEMID[6];
    u8   Revision;
    u32  RSDTAddress;

    u32  Length;
    void*  XSDT;
    u8   ExtendedChecksum;
    u8   reserved[3];
} __attribute__ ((__packed__)) XSDP;

#define XSDP_CHECKSUM_AREA_SIZE 20