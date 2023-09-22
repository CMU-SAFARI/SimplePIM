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

__dma_aligned void* centroids_data;


BARRIER_INIT(barrier_maptoval, NR_TASKLETS);
void start_func(gen_red_arguments_t* args){
    uint32_t total_len = args->table_len * args->output_type_size;
    uint32_t aligned_weights_size = total_len + 8-(total_len%8);
    if(me()==0){
        // initialise weights
        fsb_allocator_t weights_allocator = fsb_alloc(aligned_weights_size, 1);
        centroids_data = (void*)fsb_get(weights_allocator);
        mram_read(DPU_MRAM_HEAP_POINTER+args->info, centroids_data, aligned_weights_size);
    }
    barrier_wait(&barrier_maptoval);
}

void map_to_val_func(void* input_point, void* intermediate_input, uint32_t* centroid){
    // the data is preserved and later added to corresponding centroid 
    int32_t* times = (int32_t*)intermediate_input;
    *times = 1;
    intermediate_input+=sizeof(uint32_t);

    T* intermediate_ptr = (T*)intermediate_input;
    T* input_point_ptr = (T*)input_point;
    T* centroids_data_ptr = (T*)centroids_data;
    
    for(int i=0; i<dim; i++){
        intermediate_ptr[i] = input_point_ptr[i];
    }

    // find the right centroid
    uint32_t curr_centroid_pos;
    uint32_t curr_best_centroid;
    T tmp;
    uint64_t curr_dist;
    uint64_t shortest_dist =  UINT64_MAX;

    for(int i=0; i<k; i++){
        curr_centroid_pos = i*dim;
        curr_dist = 0;
        for(int j=0; j<dim; j++){
            tmp = input_point_ptr[j]-centroids_data_ptr[curr_centroid_pos+j];
            curr_dist += tmp*tmp;
        }
        if(curr_dist<shortest_dist){
            curr_best_centroid = i; 
            shortest_dist = curr_dist;
        }
    }

    *centroid = curr_best_centroid;
}


#endif