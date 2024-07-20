// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
// *Supertos Industries
// Author: Supertos, 2024
// sort.h library used in Generic OS
#pragma once

// Programmer shall implement type##Compare(type* a, type* b) and type##Swap(type* a, type* b) themself.
// For type##Compare: 1 if a > b otherwise 0
#define DEFINE_SORT(type, name)																			\
	void type##Heapify( type arr[], size_t i, size_t size ) {											\
		size_t left = (i+1)*2 - 1;																		\
		size_t right = (i+1)*2;																			\
																										\
		size_t largest = i;																				\
		if( left < size && type##Compare( &arr[left], &arr[largest] ) ) largest = left;					\
		if( right < size && type##Compare( &arr[right], &arr[largest] ) ) largest = right;				\
																										\
		if( largest != i ) {                                                                            \
		    type##Swap( &arr[largest], &arr[i] );										                \
		    type##Heapify( arr, largest, size );														\
		}                                                                                               \
	}																									\
																										\
	void type##BuildHeap( type arr[], size_t size ) {													\
		size_t heapsize = size;																			\
		for( size_t i = size / 2 + 1; i > 0 ; i-- ) {													\
			type##Heapify( arr, i - 1, heapsize );														\
		}																								\
	}																									\
																										\
	void name( type arr[], size_t size ) {																\
		type##BuildHeap( arr, size );																	\
		size_t heapsize = size;																			\
		for( size_t i = size; i > 0; i-- ) {															\
			type##Swap( &arr[0], &arr[i - 1] );															\
			type##Heapify( arr, 0, --heapsize );														\
		}																								\
	}																									\
	
	