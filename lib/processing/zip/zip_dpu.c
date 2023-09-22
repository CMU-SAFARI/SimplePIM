#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <mram.h>
#include <alloc.h>
#include <defs.h>
#include <barrier.h>

#include "ZipProcessing.h"
#include "ZipArgs.h"
#include "../ProcessingHelper.h"

__host zip_arguments_t ZIP_INPUT_ARGUMENTS;
__dma_aligned void* aux;

BARRIER_INIT(my_barrier, NR_TASKLETS);
int main() {
    int pid = me();
    if (pid == 0){ // Initialize once the cycle counter
        mem_reset(); // Reset the heap
    }
    barrier_wait(&my_barrier);

    //printf("\n");

   uint32_t  input_start_offset1 = ZIP_INPUT_ARGUMENTS.input_start_offset1;
   uint32_t  input_start_offset2 = ZIP_INPUT_ARGUMENTS.input_start_offset2;
   uint32_t  input_type_size1 = ZIP_INPUT_ARGUMENTS.input_type_size1;
   uint32_t  input_type_size2 = ZIP_INPUT_ARGUMENTS.input_type_size2;
   uint32_t  len = ZIP_INPUT_ARGUMENTS.len;
   uint32_t  outputs = ZIP_INPUT_ARGUMENTS.outputs;

    zip_dpu(DPU_MRAM_HEAP_POINTER+input_start_offset1, DPU_MRAM_HEAP_POINTER+input_start_offset2, DPU_MRAM_HEAP_POINTER+outputs, input_type_size1, input_type_size2, len);
    return 0;
}