#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mram.h>
#include <alloc.h>
#include <defs.h>
#include <barrier.h>

#include "GenRedProcessing.h"
#include "GenRedArgs.h"
#include "../ProcessingHelper.h"


__host gen_red_arguments_t GEN_RED_INPUT_ARGUMENTS;
__dma_aligned void* aux;

BARRIER_INIT(my_barrier, NR_TASKLETS);
int main() {
    int pid = me();
    if (pid == 0){ // Initialize once the cycle counter
        mem_reset(); // Reset the heap
    }
    barrier_wait(&my_barrier);

    //printf("\n");

    uint32_t  input_start_offset = GEN_RED_INPUT_ARGUMENTS.input_start_offset;
    uint32_t  input_type_size = GEN_RED_INPUT_ARGUMENTS.input_type_size;
    uint32_t  output_start_offset = GEN_RED_INPUT_ARGUMENTS.output_start_offset;
    uint32_t  output_type_size = GEN_RED_INPUT_ARGUMENTS.output_type_size;
    uint32_t  len = GEN_RED_INPUT_ARGUMENTS.len;
    uint32_t  table_len = GEN_RED_INPUT_ARGUMENTS.table_len;

    start_func(&GEN_RED_INPUT_ARGUMENTS);
    gen_red_dpu(DPU_MRAM_HEAP_POINTER+input_start_offset, DPU_MRAM_HEAP_POINTER+output_start_offset, input_type_size, output_type_size, len, table_len);
    return 0;
}