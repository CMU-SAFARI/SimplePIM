#ifndef MAP_TO_VAL_FUNC_H
#define MAP_TO_VAL_FUNC_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <mram.h>
#include <alloc.h>
#include <defs.h>
#include <barrier.h>

#include "../Param.h"
#include "../../../lib/processing/gen_red/GenRedArgs.h"


__dma_aligned T* weights_data;
BARRIER_INIT(barrier_maptoval, NR_TASKLETS);

void start_func(gen_red_arguments_t* args){
    uint32_t total_len = args->table_len * args->output_type_size;
    uint32_t aligned_weights_size = total_len + 8-(total_len%8);
    if(me()==0){
        // initialise weights
        fsb_allocator_t weights_allocator = fsb_alloc(aligned_weights_size, 1);
        weights_data = (void*)fsb_get(weights_allocator);
        mram_read(DPU_MRAM_HEAP_POINTER+args->info, weights_data, aligned_weights_size);
    }
    barrier_wait(&barrier_maptoval);
}

static inline T sigmoid_dpu(T x){
    if(x >= 15)
        return 1.0; 
    else if (x <= -15) 
        return  0.0; 
    else if (x == 0.0)
        return 0.5; 

    float sum = 1.0;
    float temp = 1.0; 
    // iter 100 times 
    for(uint32_t i = 1; i < 101; ++i){
        temp = temp * (-x) / i;
        sum = sum + temp; 
    }
    return (T)(1.0 / (1.0 + sum)); 
}


void map_to_val_func(void* input, void* grads, uint32_t* dummy){
    // the data is preserved and later added to corresponding weights 
    float* grads_ptr = (float*)grads;
    float* input_ptr = (float*)input;
    float* weights_data_ptr = (float*)weights_data;

    // calculate gradients w.r.t. linear weights
    float dot = 0;
    for(int i=0; i<dim; i++){
        dot += input_ptr[i] * weights_data_ptr[i];
    }

    float e = sigmoid_dpu(dot)-input_ptr[dim];
    //printf("error : %f\n", e*e);
    for(int i=0; i<dim; i++){
        grads_ptr[i] = e * input_ptr[i];
        //printf("%f ", grads_ptr[i]);
    }
    //printf("\n");  
    // put weight gradients to the 0th entry
    *dummy = 0;

}

#endif