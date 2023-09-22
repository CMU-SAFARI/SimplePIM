#ifndef TABLEHOST_H
#define TABLEHOST_H
#include <stdint.h>
#include <stdio.h>
#include "Structs.h"
void combine_table_entries(void* table1, void* table2, uint32_t table_size, uint32_t value_size, void (*combineFunc)(void*, void*));
void print_table_entries(void* table, uint32_t table_size, uint32_t value_size, void (*printFunc)(void*));

#endif 
