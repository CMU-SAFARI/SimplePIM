#ifndef PARALLEL_H
#define PARALLEL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloc.h>
#include <defs.h>
#include <mram.h>
#include "mutex.h"

#include "Structs.h"
#include "StructsPIM.h"
#include "Table.h"
#include "TableShared.h"
#include "Common.h"

void map_dpu(__mram_ptr void* inputs, __mram_ptr void* outputs, uint32_t input_type, uint32_t output_type, uint32_t len);
void combine_oncache_dpu(__mram_ptr void* table_entries_mram, dpu_arguments_t* DPU_INPUT_ARGUMENTS);
void combine_shared_dpu(__mram_ptr void* table_entries_mram, void (*init_func)(uint32_t, void*), void (*key_func)(void*, void*, uint32_t*), void (*combine_func)(void*, void*), dpu_arguments_t* DPU_INPUT_ARGUMENTS);
void zip_dpu(__mram_ptr void* table_entries_1, __mram_ptr void* table_entries_2, __mram_ptr void* table_entries_res, uint32_t input_type_1, uint32_t input_type_2, uint32_t len);
uint32_t get_shift_bits_for_type(uint32_t value_size);

#endif 