/* Supertos Industries
	Generic OS Heapsort implementation
*/

#pragma once
#include <stdbool.h>

void Heapify( void* trg, size_t id, size_t elementSize, size_t count, bool (*cmp)(void*, void*, void*), void (*swap)(void*, void*), void* additionalParameter );