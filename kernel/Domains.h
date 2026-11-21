/* Supertos Industries (2012 - 2026) */
#pragma once

#include "Base.h"
#include "MemoryManager.h"

typedef struct Core            Core;
typedef struct Domain          Domain;
typedef struct InterDomainData InterDomainData;
typedef struct VTree           VTree;
typedef struct VSpace          VSpace;
typedef struct VInclude        VInclude;
typedef struct GlobalVMA       GlobalVMA;
typedef struct LocalVMA        LocalVMA;


Memory FromAnyDomain( Domain* self, usize size, usize align );

void ToDomains( Domain* self, Memory in );

Memory FromDomain( void* self, usize size, usize align );

Core* ThisCore();

Core* CoreNew( void* base, usize physMemory, Domain* domain, RootTable* root, CoreID id );

DomainID CoreSMPID( Core* self );

VTree* CoreVTree( Core* self );

void ClaimCore( Core* self );

Domain* DomainNew( RootTable* root, MachineInfo* info, InterDomainData** shared, usize maxPhys );

void LinkDomain( Domain* self, Domain** list, u8 latency[], usize domains );

void SetInterDomainData( Domain* self, InterDomainData* in );

Core* ResideCore( RootTable* root, MachineInfo* info, Domain** domains, usize physMemory );

LocalVMA* DomainVMA( Domain* self );

DomainInfo* DomainInfoPtr( Domain* self );

usize DomainFreeMemory( Domain* self );

usize DomainTotalMemory( Domain* self );

void VRegionTransfer( VRegion* self, Caches* caches );