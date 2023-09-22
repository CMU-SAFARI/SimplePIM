#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mram.h>
#include <alloc.h>
#include <defs.h>
#include <barrier.h>

#include "MapProcessing.h"
#include "MapArgs.h"
#include "../ProcessingHelper.h"

__host map_arguments_t MAP_INPUT_ARGUMENTS;
__dma_aligned void* aux;

BARRIER_INIT(my_barrier, NR_TASKLETS);
int main() {
    int pid = me();
    if (pid == 0){ // Initialize once the cycle counter
        mem_reset(); // Reset the heap
    }
    barrier_wait(&my_barrier);

    //printf("\n");

    uint32_t  input_start_offset = MAP_INPUT_ARGUMENTS.input_start_offset;
    uint32_t  input_type_size = MAP_INPUT_ARGUMENTS.input_type_size;
    uint32_t  output_start_offset = MAP_INPUT_ARGUMENTS.output_start_offset;
    uint32_t  output_type_size = MAP_INPUT_ARGUMENTS.output_type_size;
    uint32_t  len = MAP_INPUT_ARGUMENTS.len;
    uint32_t  is_zipped = MAP_INPUT_ARGUMENTS.is_virtually_zipped;

    // for virtually zipped table
    uint32_t start1 = MAP_INPUT_ARGUMENTS.start1;
    uint32_t start2 = MAP_INPUT_ARGUMENTS.start2;
    uint32_t type1 = MAP_INPUT_ARGUMENTS.type1;
    uint32_t type2 = MAP_INPUT_ARGUMENTS.type2;
    start_func(&MAP_INPUT_ARGUMENTS);
    if(is_zipped){
        zip_map_dpu(DPU_MRAM_HEAP_POINTER+start1, DPU_MRAM_HEAP_POINTER+start2, DPU_MRAM_HEAP_POINTER+output_start_offset, type1, type2, output_type_size, len);
    }
    else{
        map_dpu(DPU_MRAM_HEAP_POINTER+input_start_offset, DPU_MRAM_HEAP_POINTER+output_start_offset, input_type_size, output_type_size, len);
    }
    return 0;
}