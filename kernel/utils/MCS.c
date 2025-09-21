/* Supertos Industries
	Generic OS Kernel Ticket queue
*/
#include "MCS.h"
#include "Atomic.h"
#include "Architecture.h"

#include <stdbool.h>


void MCSLock( MCSLockTicket** top, MCSLockTicket* newTop ) {
	MCSLockTicket* prevTop = AtomicExchange64( top, newTop );
	*newTop = (MCSLockTicket){.Finished = false, .Prev = prevTop};
	
	while( newTop->Prev && !AtomicLoad(newTop->Prev->Finished) ) Pause();
}

void MCSUnlock( MCSLockTicket* ticket ) {
	ticket->Finished = true;
	ticket->Prev = NULL;
}

