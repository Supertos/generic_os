/* Supertos Industries (2012 - 2025)
 * Primitive spinlock implementation.
 */

#include "Spinlock.h"

void SpinlockLock( spinlock_t* lock ) {
    while( AtomicCompareAndExchange8(lock, (uint8_t)true) ); // Let's burn all our energy without Pause();
}

void SpinlockRelease( spinlock_t* lock ) {
    __asm__ __volatile__ ( "sfence;" : : : ); // Just to be sure we finish all writing.
    AtomicCompareAndExchange8( lock, (uint8_t)false );
}