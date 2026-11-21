/* Supertos Industries (2012 - 2026) 
 * Domain represents a collection of Cores with their shared RAM.
 * ProximityDomain in NUMA terminology.
 */
#pragma once

#include "Base.h"
#include "PhysicalMemory/Frames.h"
#include "VirtualMemory/VAllocator.h"
#include "Domains.h"


struct InterDomainData{
	GlobalVMA VirtualRanges;
	/* GlobalScheduler Scheduler; */
};


struct Domain {
    DomainInfo 		   Info;
	InterDomainData*   SharedData;
	
	Frames 			   Memory;
	LocalVMA 		   VirtualRanges;
	
	Caches Caches;
	/* DomainScheduler Scheduler; */
	
	u32 			   LinkCount;
	u32 			   CoreCount;
	
	Core* 			   Cores;
	may_alias Domain** Links;
};

