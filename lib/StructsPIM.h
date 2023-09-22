#ifndef STRUCTSPIM_H
#define STRUCTSPIM_H

#include <alloc.h>
#include <mram.h>
#include <stdint.h>

typedef struct {
   uint8_t* locks;
   uint32_t table_size;
   uint32_t value_size;
   __mram_ptr void* heap_ptr;
   void* table; // table is an array of table_size, each element of size (uint32_t, uint32_t, value_size)
   fsb_allocator_t table_allocator;
   fsb_allocator_t locks_allocator;
} table_t;

#endif 