#ifndef GENREDPROCESSING_H
#define GENREDPROCESSING_H
#include __mapredfunc_pathname__
#include __combinefunc_pathname__

#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <defs.h>
#include <mram.h>
#include <barrier.h>

#include "../ProcessingHelper.h"
#include "../../StructsPIM.h"
#include "../../Table.h"

uint32_t get_shift_bits_for_type(uint32_t value_size){
    switch (value_size) {
        case 2: 
           return 1;
        case 4:
            return 2;
        case 8: 
            return 3;
        case 16:
            return 4;
        case 32:
            return 5;
        case 64:
            return 6;
        case 128:
            return 7;
        case 256:
            return 8;
        case 512:
            return 9;
        case 1024:
            return 10;
        case 2048:
            return 11;
        case 4096:
            return 12;
        default: 
            return 0;
    }
}

void gen_red_dpu(__mram_ptr void* inputs, __mram_ptr void* outputs, uint32_t input_type, uint32_t output_type, uint32_t len, uint32_t table_len){
    uint32_t elem_type_size = input_type;
    uint32_t inter_type_size = output_type;
    uint32_t table_size = table_len;
    uint32_t num_tasklets = NR_TASKLETS;
    uint32_t pid = num_tasklets == 1 ? 0 : me();
    
    uint64_t tuple = copy_block_size_fun(elem_type_size, inter_type_size, len);
    uint32_t* copy_block_size_ = (uint32_t*)&tuple;
    uint32_t copy_block_size = copy_block_size_[0];
    uint32_t copy_block_size_shiftbits = copy_block_size_[1];

    
    __mram_ptr void* elements = inputs;
    // try malloc/free for performance
    fsb_allocator_t elems_block_allocator = fsb_alloc(copy_block_size*elem_type_size, 1);
    __dma_aligned void* elems_block = fsb_get(elems_block_allocator);

    fsb_allocator_t table_allocator = fsb_alloc(sizeof(table_t), 1);
    __dma_aligned table_t* local_table = fsb_get(table_allocator);
    init_table(local_table, table_size, inter_type_size, init_func);

    
    fsb_allocator_t tmp_intermediate_allocator = fsb_alloc(inter_type_size, 1);
    __dma_aligned void* tmp_intermediate = fsb_get(tmp_intermediate_allocator);

    uint32_t last_block = (len/copy_block_size)*copy_block_size;
    uint32_t key = 0;

    void* local_table_entries = local_table->table;
    uint32_t curr_entry;
    uint32_t shift_bits = get_shift_bits_for_type(inter_type_size);


    // TODO : possible optimisation : shifts instead of mult, check bounds for last iterations (later)
    uint32_t total_len_in_bytes = len * elem_type_size;
    uint32_t copy_block_size_in_bytes = copy_block_size*elem_type_size;
    uint32_t stride = copy_block_size_in_bytes*num_tasklets;

    uint32_t divisible_len_in_bytes = ((len>>copy_block_size_shiftbits)<<copy_block_size_shiftbits)* elem_type_size;
    uint32_t rest_len_in_bytes = total_len_in_bytes - divisible_len_in_bytes;

    uint32_t copy_block_end = copy_block_size_in_bytes + (uint32_t)elems_block;

    if(shift_bits == 0){
        for(int i=pid*copy_block_size_in_bytes; i<divisible_len_in_bytes; i+=stride){

            mram_read((__mram_ptr void*)(elements+i), elems_block, copy_block_size_in_bytes);

            for(uint32_t j=(uint32_t)elems_block; j<copy_block_end; j+=elem_type_size){
                // intermediate results 
                map_to_val_func((void*)j, tmp_intermediate, &key);
                // get entry for intermediate results
                curr_entry = key*inter_type_size;
                // combine entry to table
                combine_func(local_table_entries+curr_entry, tmp_intermediate);
    
            }
        }

        }
    else{
        for(int i=pid*copy_block_size_in_bytes; i<divisible_len_in_bytes; i+=stride){

            mram_read((__mram_ptr void*)(elements+i), elems_block, copy_block_size_in_bytes);

            for(uint32_t j=(uint32_t)elems_block; j<copy_block_end; j+=elem_type_size){
                // intermediate results 
                map_to_val_func((void*)j, tmp_intermediate, &key);
                // get entry for intermediate results
                curr_entry = key<<shift_bits;
                // combine entry to table
                combine_func(local_table_entries+curr_entry, tmp_intermediate);
    
            }
        }
    }

    // handle last block 
    if(pid==NR_TASKLETS-1 && rest_len_in_bytes != 0){

        mram_read((__mram_ptr void*)(elements+divisible_len_in_bytes), elems_block, copy_block_size_in_bytes);

        for(uint32_t j=(uint32_t)elems_block; j<(uint32_t)elems_block+rest_len_in_bytes; j+=elem_type_size){
            // intermediate results 
            map_to_val_func((void*)j, tmp_intermediate, &key);
            // get entry for intermediate results
            curr_entry = key*inter_type_size;
            // combine entry to table
            combine_func(local_table_entries+curr_entry, tmp_intermediate);
        }

    }

    combine_tables_lockfree(outputs, local_table, init_func, combine_func);

    fsb_free(elems_block_allocator, elems_block);
    fsb_free(tmp_intermediate_allocator, tmp_intermediate);

    free_table(local_table);
    fsb_free(table_allocator, local_table);
    
}
#endif 