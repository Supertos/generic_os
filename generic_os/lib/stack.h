// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// *Supertos Industries
// Author: Supertos, 2024
// stack.h library used in Generic OS
#pragma once

#define DEFINE_STACK(type, name) 							\
	typedef struct name name;						        \
	struct name {											\
		size_t totalsize;									\
		size_t size;										\
															\
		type contents[];									\
	};														\
															\
	type name##Pop(name* stack) {							\
		type out = stack->contents[stack->size--];			\
		return out;											\
	}														\
															\
	type* name##Push(name* stack) {							\
		return &(stack->contents[stack->size++]);			\
	}														\
															\
	type* name##Peek(name* stack) {							\
		return &(stack->contents[stack->size - 1]);			\
	}														\
															\
	void name##SetSize(name* stack, size_t size) {			\
		stack->totalsize = size;							\
	}														\
															\
	type* name##Direct(name* stack, size_t id) {			\
		return &stack->contents[id];						\
	}	
	