#include "Parallel.h"
#include "../benchmarks/UserPath.h"
#include _user_h_

BARRIER_INIT(barrier_p, NR_TASKLETS);
MUTEX_INIT(mutex);
uint32_t curr_block;

uint64_t _copy_block_size(uint32_t type_size1, uint32_t type_size2,  uint32_t num_elem){
    //1024
    uint64_t res=0;
    uint32_t res_arr[2];
    uint32_t max_type_size = type_size1>type_size2?type_size1:type_size2;
    if(type_size1%8 == 0 && type_size2%8 == 0 && (num_elem <= NR_TASKLETS || max_type_size > 512)){
        res_arr[0] = 1;
        res_arr[1] = 0;
    }
    else if(type_size1%4 == 0 && type_size2%4 == 0 &&(num_elem <= 2*NR_TASKLETS || max_type_size > 256)){
        res_arr[0] = 2;
        res_arr[1] = 1;
    }
    else if(type_size1%2 == 0 && type_size2%2 == 0 && max_type_size > 128){
        res_arr[0] = 4;
        res_arr[1] = 2;
    }
    else if(max_type_size < 16){
        res_arr[0] = 256;
        res_arr[1] = 8;
    }
    else if(max_type_size < 32){
        res_arr[0] = 128;
        res_arr[1] = 7;
    }
    else{
        res_arr[0] = 16;
        res_arr[1] = 4;
    }

    res = *(uint64_t*)res_arr;
    return res;

}

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


void map_dpu(__mram_ptr void* inputs, __mram_ptr void* outputs, uint32_t input_type, uint32_t output_type, uint32_t len){
    uint32_t elem_type_size = input_type;
    uint32_t inter_type_size = output_type;
    uint32_t num_tasklets = NR_TASKLETS; 
    uint32_t pid = num_tasklets == 1 ? 0 : me();
    uint64_t tuple = _copy_block_size(elem_type_size, inter_type_size, len);
    uint32_t* copy_block_size_ = (uint32_t*)&tuple;
    uint32_t copy_block_size = copy_block_size_[0];
    uint32_t copy_block_size_shiftbits = copy_block_size_[1];
    // try malloc/free for performance
    fsb_allocator_t elems_block_allocator = fsb_alloc(elem_type_size<<copy_block_size_shiftbits, 1);
    __dma_aligned void* elems_block = fsb_get(elems_block_allocator);

    fsb_allocator_t inter_block_allocator = fsb_alloc(inter_type_size<<copy_block_size_shiftbits, 1);
    __dma_aligned void* inter_block = fsb_get(inter_block_allocator);

    uint32_t block_size_inputs = elem_type_size<<copy_block_size_shiftbits;
    uint32_t block_size_outputs = inter_type_size<<copy_block_size_shiftbits;
    uint32_t block_times_tasklets = num_tasklets<<copy_block_size_shiftbits;

    uint32_t divisible_len = (len>>copy_block_size_shiftbits)<<copy_block_size_shiftbits;
    uint32_t rest_len = len - divisible_len;

    void* inter, *elem;


    uint32_t unroll_block_size = (copy_block_size>>2)<<2;

    uint32_t i_init = pid<<copy_block_size_shiftbits;
    uint32_t i_elem_init = (uint32_t)inputs+i_init*elem_type_size;
    uint32_t i_inter = (uint32_t)outputs+i_init*inter_type_size;

    uint32_t i_elem_stride = block_times_tasklets * elem_type_size;
    uint32_t i_inter_stride = block_times_tasklets * inter_type_size;
    uint32_t i_elem_divisible_len = divisible_len * elem_type_size;

    uint32_t j_elem_block_size = elem_type_size<<2;
    uint32_t j_inter_block_size = inter_type_size <<2;

    uint32_t j_elem_unroll_block_end = (uint32_t)(unroll_block_size*elem_type_size + elems_block);
    uint32_t j_inter_unroll_block_end = (uint32_t)(unroll_block_size*inter_type_size + inter_block);

    uint32_t j_elem_block_end = (uint32_t)(block_size_inputs + elems_block);

    for(uint32_t i_elem=i_elem_init; i_elem<i_elem_divisible_len; i_elem+=i_elem_stride){
        

        mram_read((__mram_ptr void*)(i_elem), elems_block, block_size_inputs);

        inter = inter_block;
        for(uint32_t j_elem=(uint32_t)elems_block; j_elem<j_elem_unroll_block_end; j_elem += j_elem_block_size){

            elem = (void*)j_elem;
            map_func(elem, inter);

            elem += elem_type_size;
            inter += inter_type_size;
            map_func(elem, inter);

            elem += elem_type_size;
            inter += inter_type_size;
            map_func(elem, inter);

            elem += elem_type_size;
            inter += inter_type_size;
            map_func(elem, inter);

            inter += inter_type_size;
        }

        inter = (void*)j_inter_unroll_block_end;
        for(uint32_t j_elem=j_elem_unroll_block_end; j_elem<j_elem_block_end; j_elem += elem_type_size){
            map_func((void*)j_elem, inter);
            inter += inter_type_size;
        }


        mram_write(inter_block, (__mram_ptr void*)(i_inter),  block_size_outputs);

        i_inter += i_inter_stride;
    }

    // handle last block 
    if(pid==0 && rest_len != 0){
        uint32_t last_block_elem_addr = divisible_len * elem_type_size;
        uint32_t last_block_inter_addr = divisible_len * inter_type_size;

        mram_read((__mram_ptr void*)(outputs+last_block_inter_addr), inter_block, block_size_outputs);
        mram_read((__mram_ptr void*)(inputs+last_block_elem_addr), elems_block, block_size_inputs);

        elem = elems_block;
        inter = inter_block;
        for(uint32_t j=0; j<rest_len; j++){
            map_func(elem, inter);
            elem += elem_type_size;
            inter +=inter_type_size;
        }

        mram_write(inter_block, (__mram_ptr void*)(outputs+last_block_inter_addr),  block_size_outputs);

    }
    

    fsb_free(elems_block_allocator, elems_block);
    fsb_free(inter_block_allocator, inter_block);

    barrier_wait(&barrier_p);
}

void zip_dpu(__mram_ptr void* table_entries_1, __mram_ptr void* table_entries_2, __mram_ptr void* table_entries_res, uint32_t input_type_1, uint32_t input_type_2, uint32_t len){
    uint32_t num_tasklets = NR_TASKLETS; 
    uint32_t pid = num_tasklets == 1 ? 0 : me();
    uint64_t tuple = _copy_block_size(input_type_1, input_type_2, len);
    uint32_t* copy_block_size_ = (uint32_t*)&tuple;
    uint32_t copy_block_size = copy_block_size_[0];
    uint32_t copy_block_size_shiftbits = copy_block_size_[1];

    uint32_t input_block_size_1 = copy_block_size*input_type_1;
    uint32_t input_block_size_2 = copy_block_size*input_type_2;
    uint32_t output_type = input_type_1+input_type_2;
    uint32_t output_block_size = copy_block_size*output_type;

    fsb_allocator_t input1_block_allocator = fsb_alloc(input_block_size_1, 1);
    __dma_aligned void* input1_block = fsb_get(input1_block_allocator);

    fsb_allocator_t input2_block_allocator = fsb_alloc(input_block_size_2, 1);
    __dma_aligned void* input2_block = fsb_get(input2_block_allocator);

    fsb_allocator_t output_block_allocator = fsb_alloc(output_block_size, 1);
    __dma_aligned void* output_block = fsb_get(output_block_allocator);

    uint32_t divisible_len = len - len%copy_block_size;
    uint32_t rest_len = len - divisible_len;

    uint32_t block_times_tasklets = copy_block_size*num_tasklets;

    uint32_t i_input1;
    uint32_t i_input2;
    uint32_t i_output;
    uint32_t j_input1;
    uint32_t j_input2;
    uint32_t j_output;

    void* input1;
    void* input2;
    void* output;
    uint32_t unroll_block_size = (copy_block_size>>2)<<2;

    for(int i=pid*copy_block_size; i<divisible_len; i+=block_times_tasklets){
        i_input1 = i*input_type_1;
        i_input2 = i*input_type_2;
        i_output = i*output_type;
        
        mram_read(table_entries_1+i_input1, input1_block, input_block_size_1);
        mram_read(table_entries_2+i_input2, input2_block, input_block_size_2);

        for(int j=0; j<unroll_block_size; j+=4){
            //j_elem = j<<inputs_shift_bits;
            //j_inter = j<<outputs_shift_bits;
            j_input1 = j*input_type_1;
            j_input2 = j*input_type_2;
            j_output = j*output_type;

            input1 = input1_block+j_input1;
            input2 = input2_block+j_input2;
            output = output_block+j_output;
            memcpy(output, input1, input_type_1);
            memcpy(output+input_type_1, input2, input_type_2);

            input1 += input_type_1;
            input2 += input_type_2;
            output += output_type;
            memcpy(output, input1, input_type_1);
            memcpy(output+input_type_1, input2, input_type_2);

            input1 += input_type_1;
            input2 += input_type_2;
            output += output_type;
            memcpy(output, input1, input_type_1);
            memcpy(output+input_type_1, input2, input_type_2);

            input1 += input_type_1;
            input2 += input_type_2;
            output += output_type;
            memcpy(output, input1, input_type_1);
            memcpy(output+input_type_1, input2, input_type_2);
        }

        mram_write(output_block, table_entries_res+i_output, output_block_size);


    }

    // handle last block 
    if(pid==0 && rest_len != 0){
        uint32_t last_block_input1_addr = divisible_len * input_type_1;
        uint32_t last_block_input2_addr = divisible_len * input_type_2;
        uint32_t last_block_output_addr = divisible_len * output_type;

        mram_read((__mram_ptr void*)table_entries_1+last_block_input1_addr, input1_block, input_block_size_1);
        mram_read((__mram_ptr void*)table_entries_2+last_block_input2_addr, input2_block, input_block_size_2);
        mram_read((__mram_ptr void*)table_entries_res+last_block_output_addr, output_block, output_block_size);

        for(int j=0; j<rest_len; j++){
            j_input1 = j*input_type_1;
            j_input2 = j*input_type_2;
            j_output = j*output_type;

            input1 = input1_block+j_input1;
            input2 = input2_block+j_input2;
            output = output_block+j_output;
            memcpy(output, input1, input_type_1);
            memcpy(output+input_type_1, input2, input_type_2);

        }

        mram_write(output_block, table_entries_res+last_block_output_addr, output_block_size);

    }

    fsb_free(output_block_allocator, output_block);
    fsb_free(input1_block_allocator, input1_block);
    fsb_free(input2_block_allocator, input2_block);
    barrier_wait(&barrier_p);
}


void map_and_combine_oncache_imbalanced(__mram_ptr void* table_entries_mram, void (*init_func)(uint32_t, void*), void (*mapHashFunc)(void*, void*, uint32_t*), void (*combineFunc)(void*, void*), dpu_arguments_t* DPU_INPUT_ARGUMENTS){
    uint32_t len = DPU_INPUT_ARGUMENTS->input_len;
    uint32_t elem_type_size = DPU_INPUT_ARGUMENTS->input_type_size;
    uint32_t inter_type_size = DPU_INPUT_ARGUMENTS->table_type_size;
    uint32_t table_size = DPU_INPUT_ARGUMENTS->table_len;
    uint32_t num_tasklets = NR_TASKLETS;
    uint32_t pid = num_tasklets == 1 ? 0 : me();

    uint64_t tuple = _copy_block_size(elem_type_size, inter_type_size, len);
    uint32_t* copy_block_size_ = (uint32_t*)&tuple;
    uint32_t copy_block_size = copy_block_size_[0];
    uint32_t copy_block_size_shiftbits = copy_block_size_[1];

    __mram_ptr void* elements = DPU_MRAM_HEAP_POINTER+DPU_INPUT_ARGUMENTS->input_start_offset;
    // try malloc/free for performance
    fsb_allocator_t elems_block_allocator = fsb_alloc(copy_block_size*elem_type_size, 1);
    __dma_aligned void* elems_block = fsb_get(elems_block_allocator);

    fsb_allocator_t table_allocator = fsb_alloc(sizeof(table_t), 1);
    __dma_aligned table_t* local_table = fsb_get(table_allocator);
    init_table(local_table, table_size, inter_type_size, init_func);

    fsb_allocator_t tmp_intermediate_allocator = fsb_alloc(inter_type_size, 1);
    __dma_aligned void* tmp_intermediate = fsb_get(tmp_intermediate_allocator);

    uint32_t key = 0;
    // TODO : possible optimisation : shifts instead of mult, check bounds for last iterations (later)
    
    curr_block = 0;
    uint32_t i;
    uint32_t curr_block_local;
    uint32_t num_blocks = (len%copy_block_size==0)?len/copy_block_size:len/copy_block_size+1;
    uint32_t copy_block_size_in_bytes = copy_block_size*elem_type_size;
    uint32_t total_len_in_bytes = len * elem_type_size;
    
    barrier_wait(&barrier_p);
    while (curr_block<num_blocks)
    {
        mutex_lock(mutex);
        curr_block_local = curr_block;
        curr_block++; // current lock being processed
        mutex_unlock(mutex);

        i = curr_block_local*copy_block_size_in_bytes;
        mram_read((__mram_ptr void*)(& ((char*)elements)[i]), elems_block, copy_block_size_in_bytes);
        for(int j=0; j<copy_block_size_in_bytes; j+=elem_type_size){
            if(i+j<total_len_in_bytes){
                void* elem = (void*)(& ((char*)elems_block)[j]);
                // intermediate results are inserted in local_table
                (*mapHashFunc)(elem, tmp_intermediate, &key);
                insert_table(local_table, key, tmp_intermediate, combineFunc);
            }
            else{
                break;
            }
        }
    }

    combine_tables_lockfree(table_entries_mram, local_table, init_func, combineFunc);

    fsb_free(elems_block_allocator, elems_block);
    fsb_free(tmp_intermediate_allocator, tmp_intermediate);

    free_table(local_table);
    fsb_free(table_allocator, local_table);
    

}


void combine_oncache_dpu(__mram_ptr void* table_entries_mram, dpu_arguments_t* DPU_INPUT_ARGUMENTS){
    uint32_t len = DPU_INPUT_ARGUMENTS->input_len;
    uint32_t elem_type_size = DPU_INPUT_ARGUMENTS->input_type_size;
    uint32_t inter_type_size = DPU_INPUT_ARGUMENTS->table_type_size;
    uint32_t table_size = DPU_INPUT_ARGUMENTS->table_len;
    uint32_t num_tasklets = NR_TASKLETS;
    uint32_t pid = num_tasklets == 1 ? 0 : me();
    
    uint64_t tuple = _copy_block_size(elem_type_size, inter_type_size, len);
    uint32_t* copy_block_size_ = (uint32_t*)&tuple;
    uint32_t copy_block_size = copy_block_size_[0];
    uint32_t copy_block_size_shiftbits = copy_block_size_[1];

    
    __mram_ptr void* elements = DPU_MRAM_HEAP_POINTER+DPU_INPUT_ARGUMENTS->input_start_offset;
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
    uint32_t stride = copy_block_size*num_tasklets*elem_type_size;
    for(int i=pid*copy_block_size_in_bytes; i<total_len_in_bytes; i+=stride){

        mram_read((__mram_ptr void*)(& ((char*)elements)[i]), elems_block, copy_block_size_in_bytes);

        for(int j=0; j<copy_block_size_in_bytes; j+=elem_type_size){
            if(i+j<total_len_in_bytes){
                void* elem = (void*)(& ((char*)elems_block)[j]);
                // intermediate results 
                map_to_val_func(elem, tmp_intermediate, &key);

                // get entry for intermediate results
                
                if(key==0){
                    curr_entry = 0;
                }
                else if(shift_bits){
                    curr_entry = key<<shift_bits;
                }
                else{
                    curr_entry = key*inter_type_size;
                }
                
                //curr_entry = key*inter_type_size;
                combine_func(local_table_entries+curr_entry, tmp_intermediate);
            }
            else{
                break;
            }
        }
    }
    combine_tables_lockfree(table_entries_mram, local_table, init_func, combine_func);

    fsb_free(elems_block_allocator, elems_block);
    fsb_free(tmp_intermediate_allocator, tmp_intermediate);

    free_table(local_table);
    fsb_free(table_allocator, local_table);
    
}


__dma_aligned table_t* t_global;
void combine_shared_dpu(__mram_ptr void* table_entries_mram, void (*init_func)(uint32_t, void*), void (*mapHashFunc)(void*, void*, uint32_t*), void (*combineFunc)(void*, void*), dpu_arguments_t* DPU_INPUT_ARGUMENTS){
    uint32_t len = DPU_INPUT_ARGUMENTS->input_len;
    uint32_t elem_type_size = DPU_INPUT_ARGUMENTS->input_type_size;
    uint32_t inter_type_size = DPU_INPUT_ARGUMENTS->table_type_size;
    uint32_t table_size = DPU_INPUT_ARGUMENTS->table_len;
    uint32_t num_tasklets = NR_TASKLETS;
    uint32_t pid = num_tasklets == 1 ? 0 : me();
    
    uint64_t tuple = _copy_block_size(elem_type_size, inter_type_size, len);
    uint32_t* copy_block_size_ = (uint32_t*)&tuple;
    uint32_t copy_block_size = copy_block_size_[0];
    uint32_t copy_block_size_shiftbits = copy_block_size_[1];
    
    __mram_ptr void* elements = DPU_MRAM_HEAP_POINTER+DPU_INPUT_ARGUMENTS->input_start_offset;
    // try malloc/free for performance
    fsb_allocator_t elems_block_allocator = fsb_alloc(copy_block_size*elem_type_size, 1);
    __dma_aligned void* elems_block = fsb_get(elems_block_allocator);

    fsb_allocator_t table_allocator = fsb_alloc(sizeof(table_t), 1);
    if(pid == 0){
        t_global = fsb_get(table_allocator);
    }
    init_shared_table(t_global, table_size, inter_type_size, init_func);

    fsb_allocator_t tmp_intermediate_allocator = fsb_alloc(inter_type_size, 1);
    __dma_aligned void* tmp_intermediate = fsb_get(tmp_intermediate_allocator);

    int last_block = (len/copy_block_size)*copy_block_size;
    uint32_t key = 0;

    // TODO : possible optimisation : shifts instead of mult, check bounds for last iterations (later)
    uint32_t total_len_in_bytes = len * elem_type_size;
    uint32_t copy_block_size_in_bytes = copy_block_size*elem_type_size;
    uint32_t stride = copy_block_size*num_tasklets*elem_type_size;
    for(int i=pid*copy_block_size_in_bytes; i<total_len_in_bytes; i+=stride){

        mram_read((__mram_ptr void*)(& ((char*)elements)[i]), elems_block, copy_block_size_in_bytes);

        for(int j=0; j<copy_block_size_in_bytes; j+=elem_type_size){
            if(i+j<total_len_in_bytes){
                void* elem = (void*)(& ((char*)elems_block)[j]);
                // intermediate results are inserted in local_table
                (*mapHashFunc)(elem, tmp_intermediate, &key);
                insert_shared_table(t_global, key, tmp_intermediate, combineFunc);
            }
            else{
               break;
            }
        }
    }
    
    store_shared_table_on_heap(t_global, table_entries_mram);

    fsb_free(elems_block_allocator, elems_block);
    fsb_free(tmp_intermediate_allocator, tmp_intermediate);

    free_table(t_global);
    if(pid == 0){
        fsb_free(table_allocator, t_global);
    }
}