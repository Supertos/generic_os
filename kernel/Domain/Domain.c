/* Supertos Industries (2012 - 2026) */

#include "Domain/Domain.h"
#include "Core.h"
#include "PhysicalMemory/Frames.h"
#include "VirtualMemory/VAllocator.h"
#include "PhysicalMemory/Cache.h"

#include "Debug.h"

Memory FromAnyDomain( Domain* self, usize size, usize align ) {
	Memory out;
	for( usize i = 0; i < self->LinkCount; ++i ) {
		out = FromFrames( &self->Links[i]->Memory, size, align );
		if( MemoryValid(out) ) break;
	}

	return out;
}


void ToDomains( Domain* self, Memory in ) {
	for( usize i = 0; i < self->LinkCount; ++i ) {
		if( !MemoryValid(MemoryIntersect(in, self->Links[i]->Info.Range)) ) continue;
		ToFrames( &self->Links[i]->Memory, in );
		break;
	}
}


Memory FromDomain( void* self, usize size, usize align ) {
	return FromFrames( &((Domain*)self)->Memory, size, align );
}


Domain* DomainNew( RootTable* root, MachineInfo* info, InterDomainData** shared, usize maxPhys ) {
	DomainInfo domain = NextDomainInfo(info);
	if( !DomainValid(domain) ) return (Domain*)NULL;

	usize cores    = domain.CoreCount, domains = DomainCount(info);
	usize totalMem = AlignUp(sizeof(Domain), 			alignof(Domain*))  +
					 AlignUp(sizeof(Domain*) * domains, CACHELINE) 		   +
					 AlignUp(sizeof(Core),      	    CACHELINE) * cores +
					 ((shared != NULL) ? sizeof(InterDomainData) : 0)	   ;

	Frames frames;
	FramesNew(&frames);

	MemoryEntry* map = root->MemoryMap;
	for( usize i = 0; i < root->MemMapCount; i++ ) {
		if( !(map[i].Type & MEMMAP_AVALIABLE) ) continue;
	
		Memory frame = MemoryIntersect(map[i].Range, domain.Range);
		ToFrames(&frames, frame);
		
	}
	
	Memory mem = FromFrames(&frames, AlignUp(totalMem, PAGE), 0);
	if( !MemoryValid(mem) ) KernelCrashUndBurn("Can't create domain!");

	Bump    bump = BumpNew(mem.Base, MemorySize(mem), "DOMAIN INIT");
	Domain* out  = (Domain*)FromBump( &bump, sizeof(Domain), alignof(Domain*) ).Base;

	*out = (Domain) {
		.Info  	   = domain,
		.LinkCount = domains,
		.CoreCount = 0,
		.Cores	   = (Core*)FromBump( &bump, sizeof(Core) * cores, CACHELINE ).Base,
		.Links	   = (Domain**)FromBump( &bump, sizeof(Domain*) * domains, CACHELINE ).Base
	};

	FramesNew(&out->Memory);
	FramesMove(&frames, &out->Memory);

	CachesNew(&out->Caches, FramesAlloc(&out->Memory));

	if( MedianDomain(info) == domain.ID ) {
		InterDomainData* sharedData = (InterDomainData*)FromBump( &bump, sizeof(InterDomainData), CACHELINE ).Base;
		*sharedData = (InterDomainData){0};

		GlobalVMANew( &sharedData->VirtualRanges, &out->Caches, maxPhys );
		*shared = sharedData;
	}

	return out;
}

static const usize gaps[] = {/*1750, 701, 301, 132, 57,*/ 23, 10, 4, 1, 0}; // (https://oeis.org/A102549).

void LinkDomain( Domain* self, Domain** list, u8 latency[], usize domains ) {
	usize* links = (usize*)self->Links;
	for( usize i = 0; i < domains; i++ ) links[i] = i;

	for( const usize* gap = gaps; *gap; gap++ ) {
		for( usize base = *gap; base < domains; base++ ) {
			usize curVal = links[base], *trgSlot = &links[base];

			while( links + *gap <= trgSlot ) {
				usize xchgVal = *(trgSlot - *gap);
				if( latency[xchgVal] < latency[curVal] ) break;
				*trgSlot = xchgVal;
				trgSlot -= *gap;
			}

			*trgSlot = curVal;
		}
	}
	
	for( usize i = 0; i < domains; i++ )
		self->Links[i] = ReachableLatency(latency[links[i]]) ? 
				   list[links[i]] :
				   NULL;

}


void SetInterDomainData( Domain* self, InterDomainData* in ) {
	self->SharedData = in;
	LocalVMANew( &self->VirtualRanges, &self->Caches, &in->VirtualRanges );
}


Core* ResideCore( RootTable* root, MachineInfo* info, Domain** domains, usize physMemory ) {
	CoreInfo core = NextCoreInfo(info);
	if( !CoreValid(core) ) return NULL;
	
	Domain* domain = domains[core.Domain];
	return CoreNew( &domain->Cores[domain->CoreCount++], physMemory, domain, root, core.ID );
}


LocalVMA* DomainVMA( Domain* self ) {
	return &self->VirtualRanges;
}


DomainInfo* DomainInfoPtr( Domain* self ) {
	return &self->Info;
}


usize DomainFreeMemory( Domain* self ) {
	return self->Memory.Bytes;
}


usize DomainTotalMemory( Domain* self ) {
	return self->Memory.TotalBytes;
}