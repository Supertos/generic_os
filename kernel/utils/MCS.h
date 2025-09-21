/* Supertos Industries
	Generic OS Kernel Ticket queue
*/
#pragma once

#include <stdbool.h>
#include "Architecture.h"

typedef struct MCSLockTicket MCSLockTicket;
struct MCSLockTicket {
	MCSLockTicket* Prev;
	bool Finished;
} __attribute__((aligned(CACHELINE_LENGTH)));

void MCSLock( MCSLockTicket** top, MCSLockTicket* newTop );

void MCSUnlock( MCSLockTicket* ticket );

