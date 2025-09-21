/* Supertos Industries (2012 - 2025)
 * Memory Manager interface header file.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "NUMA.h"

void* AllocatePages( NUMAArena* arena, size_t count, bool whole );

void FreePages( NUMAArena* arena, void* begin, size_t count );
