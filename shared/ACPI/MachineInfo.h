/* Supertos Industries (2012 - 2026) 
 * MachineInfo is an abstraction over machine system tables.
 */
#pragma once

#include "Base.h"
#include "BootInfo.h"

typedef u8 DomainLatency;

typedef struct {
    DomainID CurDomain;
    CoreID   CurCore;
    DomainID Domains;
    CoreID   Cores;

    void*    SRAT;
    void*    SLIT;
} MachineInfo;

typedef struct {
    DomainID ID;
    CoreID   CoreCount;
    u16      Flags;
    u16      AvgLatency;

    Memory   Range;
} DomainInfo;

typedef struct {
    CoreID   ID;
    DomainID Domain;
    u16      Flags;
} CoreInfo;

DomainID DomainCount( MachineInfo* self );

CoreID CoreCount( MachineInfo* self );

void LatencyOfDomain( MachineInfo* self, DomainInfo info, u8 out[] );

DomainInfo NextDomainInfo( MachineInfo* self );

CoreInfo NextCoreInfo( MachineInfo* self );

MachineInfo MachineInfoNew( RootTable* root );

bool ReachableLatency( u8 latency );

bool DomainValid( DomainInfo self );

bool CoreValid( CoreInfo self );

DomainID MedianDomain( MachineInfo* self );

bool CoreOwnedBy( void* self, void* id );

bool CoreWithID( void* obj, void* id );

bool DomainWithID( void* obj, void* id );