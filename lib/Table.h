#ifndef TABLE_H
#define TABLE_H
#include <alloc.h>
#include <mram.h>
#include <defs.h>
#include <barrier.h>
#include <stdio.h>
#include "TableHost.h"
#include "TableShared.h"
#include "StructsPIM.h"

void init_table(table_t* t, uint32_t table_size, uint32_t value_size, void (*init_func)(uint32_t, void*));
void free_table(table_t* t);
void insert_table(table_t* t, uint32_t key, void* value, void (*combineFunc)(void*, void*));
void combine_tables_lockfree(__mram_ptr void* table_entries_mram, table_t* local_table, void (*init_func)(uint32_t, void*), void (*combineFunc)(void*, void*));
void print_table(table_t* t, void (*printFunc)(void*));
#endif 