#ifndef UPMEMCUSTOM_H
#define UPMEMCUSTOM_H
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dpu.h>
#include <omp.h>

#include "Structs.h"
#include "TableHost.h"
#include "Common.h"

void* malloc_split_aligned(uint32_t len, uint32_t type_size, uint32_t num_dpus);
void* malloc_broadcast_aligned(uint32_t len, uint32_t type_size);
uint32_t host_split_to_dpu(struct dpu_set_t set, void* elements, uint32_t len, uint32_t type_size, uint32_t num_dpus, uint32_t curr_offset);
uint32_t host_broadcast_to_dpu(struct dpu_set_t set, void* elements, uint32_t len, uint32_t type_size, uint32_t curr_offset);
void prepare_input_len_and_parse_args(struct dpu_set_t set, dpu_arguments_t* input_args, uint32_t input_len, uint32_t input_type_size, uint32_t num_dpus);
void* malloc_gather_aligned(uint32_t len, uint32_t type_size, uint32_t num_dpus);
void gather_tables_to_host(struct dpu_set_t set, void* my_table, uint32_t len, uint32_t type_size, uint32_t curr_offset_on_mram, uint32_t num_dpus, void (*init_func)(uint32_t, void*) ,void (*combineFunc)(void*, void*));
void* gather_to_host(struct dpu_set_t set, uint32_t* lens, uint32_t type_size, uint32_t curr_offset_on_mram, uint32_t num_dpus);
#endif 