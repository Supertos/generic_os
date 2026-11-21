/* Supertos Industries (2012 - 2026) 
 * Core represents single core on SMP Machine, complete with its scheduler, cpu-local caches, etc.
 * Pointer to core is saved in Self member, its address is written to MSR for %%gs:0 to return core object.
 */
#pragma once

#include "Base.h"

#include "Domains.h"
#include "Domain/Domain.h"
#include "MemoryManager.h"
#include "Primitives/RBTree.h"
#include "VirtualMemory/VSpace.h"

struct Core {
	void* 	 Self;
	Domain*  Owner;
	CoreID   SMPID;
	
	Caches 	 Caches;
	
	VSpace*  KernelSpace;
	IMap 	 KernelDummyInterface;
	VTree 	 Spaces;
	/* CoreScheduler Scheduler; */
};