/* Supertos Industries (2012 - 2026) | Ivan Sigaev (@ivan-r-sigaev)
 * Read-Write Lock with writer priority.
 * Readers read lock-free unless writer's pending. Writer waits for all current readers to finish.
 * Supports up to 65K writers, not intended to be used in thread context. Per-core locking only.
 */

#pragma once

#include "Base.h"

typedef volatile u32 RWLock;

RWLock* AcquireReader( RWLock* lock );

void ReleaseReader( RWLock* lock );

RWLock* AcquireWriter( RWLock* lock );

void ReleaseWriter( RWLock* lock );

static inline void __RWLockWriterReleaseMacro( RWLock** lock ) {
    ReleaseWriter(*lock);
}

static inline void __RWLockReaderReleaseMacro( RWLock** lock ) {
    ReleaseReader(*lock);
}

#define ___CONCAT_IMPL(a, b) a ## b
#define ___CONCAT(a, b) ___CONCAT_IMPL(a, b)

#define ACQUIRE_AUTO(lock) RWLock* ___CONCAT(__internalLock, __LINE__) __attribute__((cleanup(__RWLockWriterReleaseMacro))) = AcquireWriter(lock);
#define ACQUIRE_READER_AUTO(lock) RWLock* ___CONCAT(__internalLock, __LINE__) __attribute__((cleanup(__RWLockReaderReleaseMacro))) = AcquireReader(lock);