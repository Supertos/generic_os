/* Supertos Industries (2012 - 2026) 
 * Internal ACPI methods.
 */
#pragma once

#include "Base.h"
#include "ACPI/Tables.h"

#define ACPI_ANY_ENTRY 0xFF

static inline bool Checksum( ACPIHeader* self ) {
	u8 sum = 0, *arr = (u8*)self;
	for( u32 i = 0; i < self->Length; ++i )
		sum += arr[i];

    return sum == 0;
}

static inline void* FindACPITable( XSDT* xsdt, char* sign ) {
    u32 sign32 = sign[0] | sign[1] << 8 | sign[2] << 16 | sign[3] << 24;
    
    u32 count = (xsdt->Header.Length - sizeof(XSDT)) / sizeof(ACPIHeader*);
    for( usize i = 0; i < count; i++ ) {
        ACPIHeader* tbl = xsdt->Tables[i];
        if( tbl && tbl->Signature == sign32 && Checksum(tbl) ) return tbl;
    }

    return NULL;
}

static inline bool ValidType( u8 a, ACPIEntry* b ) {
    return a == ACPI_ANY_ENTRY || b->Type == a;
}

static inline void* NextACPIEntry( ACPIHeader* head, usize headSize, void* base, u8 type, Predicat pred, void* arg ) {
    ACPIEntry* cur = (base == NULL) ? (ACPIEntry*)head : base;
    do {
        usize size = ((usize)cur == (usize)head) ? headSize : cur->Size;
        cur = (ACPIEntry*)( (usize)cur + size );

        if( ValidType(type, cur) && (!pred || pred(cur, arg)) ) return cur;
    }while( (usize)cur < (usize)head + head->Length );
    
    return NULL;
}

static inline usize ACPIEntryCount( ACPIHeader* head, usize headSize, u8 type, Predicat pred, void* arg ) {
    usize out = 0;
    ACPIEntry* temp = NULL;
    while( (temp = NextACPIEntry(head, headSize, temp, type, pred, arg)) != NULL ) 
        out++;
        
    return out;
}