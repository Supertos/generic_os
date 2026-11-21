/* Supertos Industries (2012 - 2026) */
#include "MachineInfo.h"
#include "Methods.h"
#include "Tables.h"

#define VALID 1
#define DUMMY_DOMAIN(cpus) (DomainInfo){ .CoreCount = cpus, .Flags = VALID, .Range = (Memory){ 0, ~0ULL }, .AvgLatency = SLIT_SELF }


bool DomainValid( DomainInfo self ) {
	return self.Flags & VALID;
}


bool CoreValid( CoreInfo self ) {
	return self.Flags & VALID;
}


bool CoreWithID( void* obj, void* id ) {
	if( !(((CPUAffinity*)obj)->Flags & SRAT_ACTIVE) ) return false;
	return id == NULL || ((CPUAffinity*)obj)->ID == *(CoreID*)id;
}


bool DomainWithID( void* obj, void* id ) {
	if( !(((MemoryAffinity*)obj)->Flags & SRAT_ACTIVE) ) return false;
	return id == NULL || ((MemoryAffinity*)obj)->ID == *(DomainID*)id;
}


bool CoreOwnedBy( void* self, void* id ) {
	return CoreWithID(self, NULL) && ((CPUAffinity*)self)->Domain == *(DomainID*)id;
}


DomainID MedianDomain( MachineInfo* self ) {
	if( self->SLIT == NULL ) return 0;
	u64 	 minLatency = ~0ULL;
	DomainID bestDomain = (DomainID)~0ULL;

	SLIT* slit = self->SLIT;
	for( DomainID i = 0; i < slit->LocalityCount; i++ ) {
		u64 avgLatency = 0;

		for( DomainID j = 0; i < slit->LocalityCount; j++ )
			avgLatency += slit->Latency[i * slit->LocalityCount + j];

		avgLatency /= slit->LocalityCount;

		if( avgLatency < minLatency ) {
			minLatency = avgLatency;
			bestDomain = i;
		}
	}

	return bestDomain;
}


DomainID DomainCount( MachineInfo* self ) {
	SRAT* srat = self->SRAT;
	SLIT* slit = self->SLIT;

	if( srat == NULL ) return 1;
	if( slit != NULL ) return slit->LocalityCount;

	if( self->Domains == 0 ) {
		MemoryAffinity* cur = NULL;
		do {
			cur = NextACPIEntry( self->SRAT, sizeof(SRAT), cur, SRAT_MEM, DomainWithID, NULL );
			if( cur != NULL ) self->Domains = max(self->Domains, cur->ID + 1);
		} while( cur != NULL );
	}

	return self->Domains;
}


CoreID CoreCount( MachineInfo* self ) {
	SRAT* srat = self->SRAT;
	if( srat == NULL ) return 1;

	if( self->Cores == 0 )
		self->Cores = ACPIEntryCount( self->SRAT, sizeof(SRAT), SRAT_CPU, CoreWithID, NULL );
	
	return self->Cores;
}


static inline u8* Latency( MachineInfo* self, usize id ) {
	SLIT* slit = self->SLIT;
	return &slit->Latency[slit->LocalityCount * id];
}


void LatencyOfDomain( MachineInfo* self, DomainInfo info, u8 out[] ) {
	SLIT* slit = self->SLIT;
	
	for( usize i = 0; i < DomainCount(self); i++ )
		if( slit != NULL )
			out[i] = Latency(self, info.ID)[i];
		else
			out[i] = (info.ID == i) ? SLIT_SELF : SLIT_SELF * 2;
}


static inline DomainInfo CollectDomainInfo( MachineInfo* self, u32 id ) {
	if( id >= DomainCount(self) ) return (DomainInfo){0};

	SRAT* srat = self->SRAT;
	if( srat == NULL ) return DUMMY_DOMAIN( CoreCount(self) );

	Memory block = {~0ULL, 0};
	MemoryAffinity* cur = NULL;
	do {
		cur = NextACPIEntry( (ACPIHeader*)srat, sizeof(SRAT), cur, SRAT_MEM, DomainWithID, &id );
		if( cur == NULL ) break;
		block.Base = min(block.Base, cur->Base);
		block.End = max(block.End, cur->Base + cur->Size);
	} while( true );

	u16 latency = 0;
	for( usize i = 0; i < DomainCount(self); ++i )
		latency += Latency(self, id)[i];
	
	return (DomainInfo){
		.AvgLatency = latency / DomainCount(self),
		.CoreCount = ACPIEntryCount( (ACPIHeader*)srat, sizeof(SRAT), SRAT_CPU, CoreOwnedBy, &id ),
		.Flags = VALID,
		.ID = id,
		.Range = block
	};
}


DomainInfo NextDomainInfo( MachineInfo* self ) {
	return CollectDomainInfo(self, self->CurDomain++);
}


CoreInfo NextCoreInfo( MachineInfo* self ) {
	if( self->CurCore >= CoreCount(self) ) return (CoreInfo){0};
	SRAT* srat = self->SRAT;

	if( srat == NULL ) {
		self->CurCore++;
		return (CoreInfo){ .ID = 0, .Domain = 0, .Flags = VALID };
	}

	CPUAffinity* cur = NextACPIEntry( self->SRAT, sizeof(SRAT), NULL, SRAT_CPU, CoreWithID, &self->CurCore );
	self->CurCore++;
	return (CoreInfo){
		.Domain = cur->Domain,
		.Flags = VALID,
		.ID = cur->ID
	};
}

MachineInfo MachineInfoNew( RootTable* root ) {
	return (MachineInfo){
		.SLIT = FindACPITable( root->XSDT, SLIT_SIGN ),
		.SRAT = FindACPITable( root->XSDT, SRAT_SIGN )
	};
}


bool ReachableLatency( u8 latency ) {
	return latency != SLIT_UNREACHABLE;
}