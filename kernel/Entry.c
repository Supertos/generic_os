/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    Supertos Industries (2012 - 2026) 
    GenericOS - NUMA-centric, 
                Modern x86-64 fit with
                Architecture abstraction layer
                Backwards-compatability abandoned
                Generic Operating system.
    
    | 2024 - 2026 | Author: Supertos | GNU-23 Standard |
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= */

#undef GENERIC_LOADER

#include "Base.h"
#include "MemoryManager.h"
#include "Domains.h"

#include "Domain/Domain.h"

#include "Debug.h"

#include "Logo/Loading.h"
#include "Logo/Logo.h"


static inline usize MaxPhysicalAddress( RootTable* root ) {
    usize out = 0;
    for( usize i = 0; i < root->MemMapCount; i++ ) {
		if( !MemoryValid(root->MemoryMap[i].Range) ) continue;
        out = max(out, root->MemoryMap[i].Range.End);
	}
    return out;
}


static void InitializeBootCore( RootTable* root ) {
	uart_init();

    IRQOff();

    ArchitectureCheck();

	MachineInfo info  = MachineInfoNew(root);
	usize domainCount = DomainCount(&info);
    usize maxPhysAddr = MaxPhysicalAddress(root);

    InterDomainData* sharedData  = NULL;
    Domain**         domainList  = NULL;
    DomainLatency*   latencyList = NULL;

    Domain*  cur  = NULL;
    Domain*  src  = NULL;

	Memory   mLatency;
	Memory	 mDomains;

    while( (cur = DomainNew(root, &info, &sharedData, maxPhysAddr )) != NULL ) {
        if( domainList == NULL ) {
            usize latencySize = sizeof(DomainLatency) * domainCount;
            usize domainSize  = sizeof(Domain*) * domainCount;

			mLatency = FromDomain( cur, domainSize, 0 );
			mDomains  = FromDomain( cur, latencySize, 0 );

            domainList  = (Domain**)mLatency.Base;
            latencyList = (DomainLatency*)mDomains.Base;
            if( domainList == NULL || latencyList == NULL ) KernelCrashUndBurn("Bad topology.");

			src = cur;
        }

        domainList[DomainInfoPtr(cur)->ID] = cur;
    }
	
	kprintf("[CORE] Domain Initialization completed. Initialized total of %d domains.\n", domainCount);
	for( usize i = 0; i < domainCount; i++ ) {

		LatencyOfDomain( &info, *DomainInfoPtr(domainList[i]), latencyList );
		LinkDomain( domainList[i], domainList, latencyList, domainCount );
	}

	Core* core = NULL;
	while( (core = ResideCore(root, &info, domainList, maxPhysAddr)) != NULL )
		if( ThisID() == CoreSMPID(core) )
			ClaimCore(core);

	kprintf("[CORE] Cores successfuly resided. Begining test.\n");
	u64 a = 0, b = 0;
	u32 aux;

	__asm__ volatile (
		"rdtscp\n\t"
		"shl $32, %%rdx\n\t"
		"or %%rdx, %%rax\n\t"
		: "=a"(a), "=c"(aux)
		:
		: "%rdx"
	);

	usize allocs = 0;

	while(1) {
		Memory obj = FromAnyDomain(domainList[0], PAGE, 1);
		if( !MemoryValid( obj ) ) 
			break;

		allocs++;
	}


	__asm__ volatile (
		"rdtscp\n\t"
		"shl $32, %%rdx\n\t"
		"or %%rdx, %%rax\n\t"
		: "=a"(b), "=c"(aux)
		:
		: "%rdx"
	);

	kprintf("Finished tests.\n");
	kprintf("FrameAllocator benchmark: %d ticks/allocation (rdtscp) average over %d allocations\n", (b - a) / allocs, allocs);

	ToDomains(src, mDomains);
	ToDomains(src, mLatency);
    
	IRQOn();

	while(1) {
		usize logoBaseX = (root->Video.Width - LOGO_WIDTH) / 2;
		usize logoBaseY = (root->Video.Height - LOGO_HEIGHT) / 2;

		usize loadBaseX = (root->Video.Width - LOAD_WIDTH) / 2;
		usize loadBaseY = logoBaseY + LOGO_HEIGHT + LOAD_HEIGHT;

		for( usize y = 0; y < LOGO_HEIGHT; y++ ) {
			for( usize x = 0; x < LOGO_WIDTH; x++ ) {
				((u32*)(root->Video.Buffer))[(logoBaseY + y) * root->Video.ScanlineWidth + logoBaseX + x] = logo[y * LOGO_WIDTH + x];
			}
		}

		usize i = 0;
		while(1) {
			for( usize y = 0; y < LOAD_HEIGHT; y++ ) {
				for( usize x = 0; x < LOAD_WIDTH; x++ ) {
					((u32*)(root->Video.Buffer))[(loadBaseY + y) * root->Video.ScanlineWidth + loadBaseX + x] = loadIcon[i][y * LOAD_WIDTH + x];
				}
			}
			i++;
			if( i >= countof(loadIcon) ) i = 0;
			__asm__ volatile (
				"rdtscp\n\t"
				"shl $32, %%rdx\n\t"
				"or %%rdx, %%rax\n\t"
				: "=a"(a), "=c"(aux)
				:
				: "%rdx"
			);
			while(1) {
				__asm__ volatile (
					"rdtscp\n\t"
					"shl $32, %%rdx\n\t"
					"or %%rdx, %%rax\n\t"
					: "=a"(b), "=c"(aux)
					:
					: "%rdx"
				);
				Pause();
				if( (b - a) > 200000000 ) break;
			}
			
		}

	}
}


void KernelEntry( RootTable* root );
void KernelEntry( RootTable* root ) {
	asm volatile("andq $-16, %%rsp" : : : );

	for( usize i = 0; i < root->Video.Height * root->Video.ScanlineWidth; i++ ) 
		((u32*)root->Video.Buffer)[i] = 0x000000;

	InitializeBootCore(root);
}