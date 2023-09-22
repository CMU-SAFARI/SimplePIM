#ifndef TABLESHARED_H
#define TABLESHARED_H

#include <stdio.h>

#include <alloc.h>
#include <mram.h>
#include <defs.h>
#include <barrier.h>
#include "mutex.h"
#include "TableHost.h"
#include "StructsPIM.h"

void init_shared_table(table_t* t, uint32_t table_size, uint32_t value_size, void (*init_func)(uint32_t, void*));
void free_shared_table(table_t* t);
void insert_shared_table(table_t* t, uint32_t key, void* value, void (*combineFunc)(void*, void*));
void print_shared_table(table_t* t, void (*printFunc)(void*));

uint32_t store_shared_table_on_heap(table_t* table, __mram_ptr void* heap_ptr);
void load_shared_table_from_heap(table_t* table,  __mram_ptr void* heap_ptr);
void load_arr_aligned(void* arr, __mram_ptr void* heap_ptr, uint32_t len);
void store_arr_aligned(void* arr, __mram_ptr void* heap_ptr, uint32_t len);
#endif 