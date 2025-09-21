/* Supertos Industries (2012 - 2025)
 * Primitive spinlock implementation.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "Atomic.h"
#include "Architecture.h"

typedef volatile uint8_t spinlock_t;

/* Locks spinlock. If spinlock is alread locked code enters while loop until it unlocks. */
void SpinlockLock( spinlock_t* lock );


/* Releases spinlock for another core to continue to the code. */
void SpinlockRelease( spinlock_t* lock );